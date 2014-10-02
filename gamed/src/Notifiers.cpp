#include "stdafx.h"
#include "Game.h"
#include "Packets.h"

#include <iostream>

using namespace std;

void Game::notifyMinionSpawned(Minion* m, int side) {
   MinionSpawn ms(m);
   
   broadcastPacketTeam(side == 0 ? TEAM_BLUE : TEAM_PURPLE, ms, CHL_S2C);
   
   notifySetHealth(m);
}

void Game::notifySetHealth(Unit* u) {
   SetHealth sh(u);
   broadcastPacketVision(u, sh, CHL_S2C);
}

void Game::notifyUpdatedStats(Unit* u, bool partial) {
   UpdateStats us(u, partial);
   
   Turret* t = dynamic_cast<Turret*>(u);
   if(t) {
      broadcastPacket(us, CHL_LOW_PRIORITY, 2);
      return;
   }
   
   if(!partial) {
      broadcastPacketTeam((1-u->getSide()) == 0 ? TEAM_BLUE : TEAM_PURPLE, us, CHL_LOW_PRIORITY, 2);
   } else {
      broadcastPacketVision(u, us, CHL_LOW_PRIORITY, 2);
   }
}

void Game::notifyAddBuff(Unit* u, Unit* source, std::string buffName) {
   AddBuff add(u, source, 1, buffName);
   broadcastPacket(add, CHL_S2C);
}

void Game::notifyRemoveBuff(Unit* u, std::string buffName) {
   RemoveBuff remove(u, buffName);
   broadcastPacket(remove, CHL_S2C);
}

void Game::notifyTeleport(Unit* u, float _x, float _y){
    
   // TeleportRequest first(u->getNetId(), u->teleportToX, u->teleportToY, true);
    
    
    u->setPosition(_x, _y);
    
   // sendPacket(currentPeer, first, CHL_S2C);
    _x = MovementVector::targetXToNormalFormat(_x);
    _y = MovementVector::targetYToNormalFormat(_y);
    
    
    TeleportRequest second(u->getNetId(), _x, _y, false);
    broadcastPacket(second, CHL_S2C);
    
       
    //todo check for vision? and send to each player 
    
}

void Game::notifyMovement(Object* o) {
   const std::vector<MovementVector>& waypoints = o->getWaypoints();
   MovementAns *answer = MovementAns::create(waypoints.size()*2);
   
   answer->nbUpdates = 1;
   answer->netId = o->getNetId();
   for(size_t i = 0; i < waypoints.size(); i++) {
      answer->getVector(i)->x = waypoints[i].x;
      answer->getVector(i)->y = waypoints[i].y;
   }
   
   broadcastPacketVision(o, reinterpret_cast<uint8 *>(answer), answer->size(), 4);
   MovementAns::destroy(answer);
}

void Game::notifyDamageDone(Unit* source, Unit* target, float amount, DamageType type) {
   DamageDone dd(source, target, amount, type);
   broadcastPacket(dd, CHL_S2C);
}

void Game::notifyBeginAutoAttack(Unit* attacker, Unit* victim, uint32 futureProjNetId, bool isCritical) {
   BeginAutoAttack aa(attacker, victim, futureProjNetId, isCritical);
   broadcastPacket(aa, CHL_S2C);
}

void Game::notifyNextAutoAttack(Unit* attacker, Unit* target, uint32 futureProjNetId, bool isCritical, bool nextAttackFlag) {
   NextAutoAttack aa(attacker, target, futureProjNetId, isCritical, nextAttackFlag);
   broadcastPacket(aa, CHL_S2C);
}

void Game::notifyProjectileSpawn(Projectile* p) {
   SpawnProjectile sp(p);
   broadcastPacket(sp, CHL_S2C);
}

void Game::notifyProjectileDestroy(Projectile* p) {
   DestroyProjectile dp(p);
   broadcastPacket(dp, CHL_S2C);
}

void Game::notifyParticleSpawn(Champion* source, Target* target, const std::string& particleName) {
   SpawnParticle sp(source, target, particleName, GetNewNetID());
   broadcastPacket(sp, CHL_S2C);
}

void Game::notifyModelUpdate(Unit* object) {
    UpdateModel mp(object->getNetId(), object->getModel().c_str());
    broadcastPacket(reinterpret_cast<uint8 *>(&mp), sizeof(UpdateModel), CHL_S2C);
}

void Game::notifyItemBought(Champion* c, const ItemInstance* i) {
   BuyItemAns response;
   response.header.netId = c->getNetId();
   response.itemId = i->getTemplate()->getId();
   response.slotId = i->getSlot();
   response.stack = i->getStacks();
   
   broadcastPacket(reinterpret_cast<uint8 *>(&response), sizeof(response), CHL_S2C);
}

void Game::notifyItemsSwapped(Champion* c, uint8 fromSlot, uint8 toSlot) {
   SwapItemsAns sia(c, fromSlot, toSlot);
   broadcastPacket(sia, CHL_S2C);
}

void Game::notifyLevelUp(Champion* c) {
   LevelUp lu(c);
   broadcastPacket(lu, CHL_S2C);
}

void Game::notifyRemoveItem(Champion* c, uint8 slot) {
   RemoveItem ri(c, slot);
   broadcastPacket(ri, CHL_S2C);
}

void Game::notifySetTarget(Unit* attacker, Unit* target) {
   SetTarget st(attacker, target);
   broadcastPacket(st, CHL_S2C);
}

void Game::notifyChampionDie(Champion* die, Unit* killer) {
   ChampionDie cd(die, killer);
   broadcastPacket(cd, CHL_S2C);
}

void Game::notifyChampionRespawn(Champion* c) {
   ChampionRespawn cr(c);
   broadcastPacket(cr, CHL_S2C);
}

void Game::notifyShowProjectile(Projectile* p) {
   ShowProjectile sp(p);
   broadcastPacket(sp, CHL_S2C);
}

void Game::notifyNpcDie(Unit* die, Unit* killer) {
   NpcDie nd(die, killer);
   broadcastPacket(nd, CHL_S2C);
}

void Game::notifyAddGold(Champion* c, Unit* died, float gold) {
   AddGold ag(c, died, gold);
   broadcastPacket(ag, CHL_S2C);
}

void Game::notifyStopAutoAttack(Unit* attacker) {
   StopAutoAttack saa(attacker);
   broadcastPacket(saa, CHL_S2C);
}

void Game::notifyDebugMessage(std::string htmlDebugMessage) {
   DebugMessage dm(htmlDebugMessage);
   broadcastPacket(dm, CHL_S2C);
}

void Game::notifySpawn(Unit* u) {

   Minion* m = dynamic_cast<Minion*>(u);
   
   if(m) {
      notifyMinionSpawned(m, 1-m->getSide());
   }
   
   Champion* c = dynamic_cast<Champion*>(u);
   
   if(c) {
      notifyChampionSpawned(c, 1-c->getSide());
   }
   
   notifySetHealth(u);
}

void Game::notifyLeaveVision(Object* o, uint32 side) {
   LeaveVision lv(o);
   broadcastPacketTeam(side == 0 ? TEAM_BLUE : TEAM_PURPLE, lv, CHL_S2C);
}

void Game::notifyEnterVision(Object* o, uint32 side) {
   Minion* m = dynamic_cast<Minion*>(o);
   
   if(m) {
      EnterVisionAgain eva(m);
      broadcastPacketTeam(side == 0 ? TEAM_BLUE : TEAM_PURPLE, eva, CHL_S2C);
      notifySetHealth(m);
      return;
   }
   
   Champion* c = dynamic_cast<Champion*>(o);
   
   if(c) {
      notifyChampionSpawned(c, side);
      notifySetHealth(c);
      return;
   }
}

void Game::notifyChampionSpawned(Champion* c, uint32 side) {
   HeroSpawn2 hs(c);
   broadcastPacketTeam(side == 0 ? TEAM_BLUE : TEAM_PURPLE, hs, CHL_S2C);
}

void Game::notifySetCooldown(Champion* c, uint8 slotId, float currentCd, float totalCd) {
   SetCooldown cd(c->getNetId(), slotId, currentCd, totalCd);
   broadcastPacket(cd, CHL_S2C);
}