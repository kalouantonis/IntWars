#include "Turret.h"
#include "Map.h"
#include "TurretStats.h"
#include "Game.h"

#define TURRET_RANGE 775

Turret::Turret(Map* map, uint32 id, const std::string& name, float x, float y, float hp, float ad, int side) : Unit(map, id, "", new TurretStats(), 50, x, y, 1200), name(name)  {
   stats->setCurrentHealth(hp);
   stats->setMaxHealth(hp);
   stats->setBaseAd(ad);
   stats->setRange(TURRET_RANGE);
   
   autoAttackDelay = 4.95f/30.f;
   autoAttackProjectileSpeed = 1200.f;
   
   setSide(side);
}

void Turret::update(int64 diff)
{
   // No target : try to find a new one
   if(!isAttacking) {
      const Map::ObjectMap& objects = map->getObjects();
      Unit* nextTarget = 0;
      unsigned int nextTargetPriority = 10;
      for(auto& it : objects) {
         Unit* u = dynamic_cast<Unit*>(it.second);

         if(!u || u->isDead() || u->getSide() == getSide() || distanceWith(u) > TURRET_RANGE) {
            continue;
         }
         
         if (!unitTarget) {
            auto priority = classifyTarget(u);
            if (priority < nextTargetPriority) {
               nextTarget = u;
               nextTargetPriority = priority;
            }
         } else {
            Champion* currentTarget = dynamic_cast<Champion*>(unitTarget);
            
            // Is the current target a champion? If it is, don't do anything
            if (!currentTarget) {
               // Find the next champion in range targeting an enemy champion who is also in range
               Champion* c = dynamic_cast<Champion*>(u);
               if (c && c->getUnitTarget() != 0) {
                  Champion* target = dynamic_cast<Champion*>(c->getUnitTarget());
                  if (target && c->distanceWith(target) <= c->getStats().getRange() && distanceWith(target) <= TURRET_RANGE) {
                     nextTarget = c; // No priority required
                     break;
                  }
               }
            }
         }
      }
      if (nextTarget) {
         unitTarget = nextTarget;
         map->getGame()->notifySetTarget(this, nextTarget);
      }
   }
   
   // Lose focus of the unit target if the target is out of range
   if(unitTarget && distanceWith(unitTarget) > TURRET_RANGE) {
      setUnitTarget(0);
      map->getGame()->notifySetTarget(this, 0);
   }

   Unit::update(diff);
}
