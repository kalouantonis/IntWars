#ifndef __SUMMONERSRIFT_H
#define __SUMMONERSRIFT_H

#include "Map.h"

class SummonersRift : public Map {


public:
   SummonersRift(Game* game);
   
   virtual ~SummonersRift() { }
   virtual void update(long long diff) override;
   float getGoldPerSecond() const override { return 1.9f; }
   
   const Target getRespawnLoc(int side) const override;
   float getGoldFor(Unit* u) const override;
   float getExpFor(Unit* u) const override;
   
   bool spawn() override;

};

#endif