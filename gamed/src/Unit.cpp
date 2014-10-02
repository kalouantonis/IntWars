#include "Unit.h"
#include "Map.h"
#include "Game.h"

#include <algorithm>
#include <cstdlib>

#define DETECT_RANGE 400
#define EXP_RANGE 1400

using namespace std;

Unit::~Unit() {
    delete stats;
}

void Unit::update(int64 diff) {

    if(unitScript.isLoaded()){
      try{
        unitScript.lua.get <sol::function> ("onUpdate").call <void> (diff);
      } catch(sol::error e){
        printf("%s", e.what());
      }
   }

   if (isDead()) {
    return;
   }

   if (unitTarget && unitTarget->isDead()) {
      setUnitTarget(0);
      isAttacking = false;
      map->getGame()->notifySetTarget(this, 0);
      initialAttackDone = false;
   }
   
   if(!unitTarget && isAttacking) {
      isAttacking = false;
      initialAttackDone = false;
   }

   if (isAttacking) {
      autoAttackCurrentDelay += diff / 1000000.f;
      if (autoAttackCurrentDelay >= autoAttackDelay/stats->getAttackSpeedMultiplier()) {
         if(!isMelee()) {
            Projectile* p = new Projectile(map, autoAttackProjId, x, y, 5, this, unitTarget, 0, autoAttackProjectileSpeed, 0);
            map->addObject(p);
            map->getGame()->notifyShowProjectile(p);
         } else {
            autoAttackHit(unitTarget);
         }
         autoAttackCurrentCooldown = 1.f / (stats->getTotalAttackSpeed());
         isAttacking = false;
      }
   } else if (unitTarget && distanceWith(unitTarget) <= stats->getRange()) {
      refreshWaypoints();
      nextAutoIsCrit = ((rand() % 100 + 1) <= stats->getCritChance() * 100) ? true : false;
      if (autoAttackCurrentCooldown <= 0) {
         isAttacking = true;
         autoAttackCurrentDelay = 0;
         autoAttackProjId = GetNewNetID();
         autoAttackFlag = true;
         
         if (!initialAttackDone) {
            initialAttackDone = true;
            map->getGame()->notifyBeginAutoAttack(this, unitTarget, autoAttackProjId, nextAutoIsCrit);
         } else {
            nextAttackFlag = !nextAttackFlag; // The first auto attack frame has occurred
            map->getGame()->notifyNextAutoAttack(this, unitTarget, autoAttackProjId, nextAutoIsCrit, nextAttackFlag);
         }
      }
   } else {
      refreshWaypoints();
      if (moveOrder == MOVE_ORDER_ATTACKMOVE && !unitTarget) {
         const Map::ObjectMap& objects = map->getObjects();

         for (auto& it : objects) {
            Unit* u = dynamic_cast<Unit*> (it.second);

            if (!u || u->isDead() || u->getSide() == getSide() || distanceWith(u) > DETECT_RANGE) {
               continue;
            }

            setUnitTarget(u);
            map->getGame()->notifySetTarget(this, u);

            break;
         }
      }

      Object::update(diff);
   }

   if (autoAttackCurrentCooldown > 0) {
      autoAttackCurrentCooldown -= diff / 1000000.f;
   }

   statUpdateTimer += diff;
   if (statUpdateTimer >= 500000) { // update stats (hpregen, manaregen) every 0.5 seconds
      stats->update(statUpdateTimer);
      statUpdateTimer = 0;
   }
}

void Unit::autoAttackHit(Unit* target) {
  float damage = (nextAutoIsCrit) ? stats->getCritDamagePct() * stats->getTotalAd() : stats->getTotalAd();
    dealDamageTo(target, damage, DAMAGE_TYPE_PHYSICAL, DAMAGE_SOURCE_ATTACK);
       if(unitScript.isLoaded()){
      try{
         unitScript.lua.get <sol::function> ("onAutoAttack").call <void> (target);
      }catch(sol::error e){
         printf("Error callback ondealdamage: \n%s", e.what());
      }
   }
}

/**
 * TODO : handle armor, magic resistance [...]
 */
void Unit::dealDamageTo(Unit* target, float damage, DamageType type, DamageSource source) {
    //printf("0x%08X deals %f damage to 0x%08X !\n", getNetId(), damage, target->getNetId());
    
   if(unitScript.isLoaded()){
      try{
         /*damage = */ unitScript.lua.get <sol::function> ("onDealDamage").call <void> (target, damage, type, source);
      }catch(sol::error e){
         printf("Error callback ondealdamage: \n%s", e.what());
      }
   }
    
    
    float defense = 0;
    float regain = 0;
    switch (type) {
        case DAMAGE_TYPE_PHYSICAL:
            defense = target->getStats().getArmor();
            defense = ((1 - stats->getArmorPenPct()) * defense) - stats->getArmorPenFlat();
            
            break;
        case DAMAGE_TYPE_MAGICAL:
            defense = target->getStats().getMagicArmor();
            defense = ((1 - stats->getMagicPenPct()) * defense) - stats->getMagicPenFlat();
            break;
    }
    
    switch(source) {
        case DAMAGE_SOURCE_SPELL:
            regain = stats->getSpellVamp();
            break;
        case DAMAGE_SOURCE_ATTACK:
            regain = stats->getLifeSteal();
            break;
    }
    
    //Damage dealing. (based on leagueoflegends' wikia)
    damage = defense >= 0 ? (100 / (100 + defense)) * damage : (2 - (100 / (100 - defense))) * damage;

    target->getStats().setCurrentHealth(max(0.f, target->getStats().getCurrentHealth() - damage));
    if (!target->deathFlag && target->getStats().getCurrentHealth() <= 0) {
        target->deathFlag = true;
        target->die(this);
    }
    map->getGame()->notifyDamageDone(this, target, damage, type);
    
    //Get health from lifesteal/spellvamp
    if (regain != 0) {
        stats->setCurrentHealth (max (0.f, stats->getCurrentHealth() + (regain * damage)));
        map->getGame()->notifyUpdatedStats(this);
    }
}

bool Unit::isDead() const {
    return deathFlag;
}

void Unit::setModel(const std::string& newModel) {
    model = newModel;
    modelUpdated = true;
}

const std::string& Unit::getModel() {
    return model;
}

void Unit::die(Unit* killer) {
   setToRemove();
   map->stopTargeting(this);

   map->getGame()->notifyNpcDie(this, killer);

	float exp = map->getExpFor(this);
	auto champs = map->getChampionsInRange(this, EXP_RANGE);
	//Cull allied champions
	champs.erase(std::remove_if(champs.begin(), 
								champs.end(), 
								[this](Champion * l) { return l->getSide() == getSide(); }),
				champs.end());
	if (champs.size() > 0) {
		float expPerChamp = exp / champs.size();
		for (auto c = champs.begin(); c != champs.end(); ++c) {
			(*c)->getStats().setExp((*c)->getStats().getExp() + expPerChamp);
		}
	}

   if (killer)
   {
      Champion* cKiller = dynamic_cast<Champion*>(killer);

      if (!cKiller) {
         return;
      }

      float gold = map->getGoldFor(this);

      if (!gold) {
         return;
      }

      cKiller->getStats().setGold(cKiller->getStats().getGold() + gold);
      map->getGame()->notifyAddGold(cKiller, this, gold);

      if (cKiller->killDeathCounter < 0){
         cKiller->setChampionGoldFromMinions(cKiller->getChampionGoldFromMinions() + gold);
         printf("Adding gold form minions to reduce death spree: %f\n", cKiller->getChampionGoldFromMinions());
      }

      if (cKiller->getChampionGoldFromMinions() >= 50 && cKiller->killDeathCounter < 0){
         cKiller->setChampionGoldFromMinions(0);
         cKiller->killDeathCounter += 1;
      }
   }
}

void Unit::setUnitTarget(Unit* target) {
   unitTarget = target;
   refreshWaypoints();
}

void Unit::refreshWaypoints() {
   if (!unitTarget || (distanceWith(unitTarget) <= stats->getRange() && waypoints.size() == 1)) {
      return;
   }

   if (distanceWith(unitTarget) <= stats->getRange()-2.f) {
      setWaypoints({MovementVector(x, y)});
   } else {
      Target* t = waypoints[waypoints.size()-1].toTarget();
      if(t->distanceWith(unitTarget) >= 25.f) {
         setWaypoints({MovementVector(x, y), MovementVector(unitTarget->getX(), unitTarget->getY())});
      }
      delete t;
   }
}

Buff* Unit::getBuff(std::string name){
   for(auto& buff : buffs){
      if(buff->getName() == name){
         return buff;
       }
   }
   return 0;
}

//Prioritize targets
unsigned int Unit::classifyTarget(Unit* target) {
   Turret* t = dynamic_cast<Turret*>(target);
   
   // Turrets before champions
   if (t) {
      return 6;
   }
   
   Minion* m = dynamic_cast<Minion*>(target);

   if (m) {
      switch (m->getType()) {
         case MINION_TYPE_MELEE:
            return 4;
         case MINION_TYPE_CASTER:
            return 5;
         case MINION_TYPE_CANNON:
         case MINION_TYPE_SUPER:
            return 3;
      }
   }

   Champion* c = dynamic_cast<Champion*>(target);
   if (c) {
      return 7;
   }

   //Trap (Shaco box) return 1
   //Pet (Tibbers) return 2

   return 10;
}