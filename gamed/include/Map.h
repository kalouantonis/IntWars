#ifndef _MAP_H
#define _MAP_H

#include <unordered_map>
#include <vector>

#include "stdafx.h"
#include "Object.h"
#include "Champion.h"

class Game;

class Map {
public:
   typedef std::unordered_map<uint32, Object*> ObjectMap;

protected:
   ObjectMap objects;
   std::unordered_map<uint32, Unit*> visionUnits[3];

   std::vector<uint32> expToLevelUp;
   int waveNumber;
   const uint64 firstSpawnTime;
   const uint64 spawnInterval;
   uint64 gameTime;
   uint64 nextSpawnTime;
   uint64 firstGoldTime; // Time that gold should begin to generate
   Game* game;
   bool firstBlood;
   bool killReduction;
   
public:
   Map(Game* game, uint64 firstSpawnTime, uint64 spawnInterval, uint64 firstGoldTime) : game(game), waveNumber(0), firstSpawnTime(firstSpawnTime), firstGoldTime(firstGoldTime), spawnInterval(spawnInterval), gameTime(0), nextSpawnTime(firstSpawnTime), firstBlood(true), killReduction(true){ }
   
   virtual ~Map() { }
   virtual void update(long long diff);
   virtual float getGoldPerSecond() const = 0;
   virtual bool spawn() = 0;
   
   Object* getObjectById(uint32 id);
   void addObject(Object* o);
   void removeObject(Object* o);

   const std::vector<uint32>& getExpToLevelUp() { return expToLevelUp; }
   uint64 getGameTime() const { return gameTime; }
   uint64 getFirstGoldTime() const { return firstGoldTime; }

   virtual const Target getRespawnLoc(int side) const = 0;
   virtual float getGoldFor(Unit* u) const = 0;
   virtual float getExpFor(Unit* u) const = 0 ;
   
   Game* getGame() const { return game; }
   
   const ObjectMap& getObjects() const { return objects; }
   void stopTargeting(Unit* target);

   std::vector<Champion*> getChampionsInRange(Target* t, float range);
   
   bool getFirstBlood() const { return firstBlood; }
   void setFirstBlood(bool state) { firstBlood = state; }
   
   bool getKillReduction() const { return killReduction; }
   void setKillReduction(bool state) { killReduction = state; }
   
   bool teamHasVisionOn(int side, Object* o);
};

#endif