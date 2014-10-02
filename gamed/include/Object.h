/**
 * An object is an abstract entity.
 * This is the base class for units and projectiles.
 */

#ifndef OBJECT_H_
#define OBJECT_H_
#pragma warning(disable:4458)

#include <vector>

#include "Target.h"
#include "stdafx.h"

class Map;

#define MAP_WIDTH (13982 / 2)
#define MAP_HEIGHT (14446 / 2)

struct MovementVector {
    int16 x;
    int16 y;
    
    MovementVector() : x(0), y(0){ }
    MovementVector(int16 x, int16 y) : x(x), y(y) { }
    MovementVector(float x, float y) : x(targetXToNormalFormat(x)), y(targetYToNormalFormat(y)) { }
    Target* toTarget() { return new Target(2.0f*x + MAP_WIDTH, 2.0f*y + MAP_HEIGHT); }
    
    static int16 targetXToNormalFormat(float _x){
        return (int16)((_x) - MAP_WIDTH)/2;
    }
    static int16 targetYToNormalFormat(float _y){
        return (int16)((_y) - MAP_HEIGHT)/2;
    }
    void setCoordinatesToNormalFormat(){
        x = targetXToNormalFormat(x);
        y = targetYToNormalFormat(y);
    }
};

class Unit;

class Object : public Target {
protected:
  	uint32 id;

	float xvector, yvector;
   
   /**
    * Current target the object running to (can be coordinates or an object)
    */
	Target* target;

   std::vector<MovementVector> waypoints;
   uint32 curWaypoint;
   Map* map;

   unsigned int side;
   bool movementUpdated;
   bool toRemove;
   uint32 attackerCount;
   
   uint32 collisionRadius;
   uint32 visionRadius;
   
   bool visibleByTeam[2];
      
public:
	
   virtual ~Object();
   Object(Map* map, uint32 id, float x, float y, uint32 collisionRadius, uint32 visionRadius = 0);

   /**
   * Moves the object depending on its target, updating its coordinate.
   * @param diff the amount of milliseconds the object is supposed to move
   */
   void Move(int64 diff);

   void calculateVector(float xtarget, float ytarget);

   /**
   * Sets the side (= team) of the object
   * @param side the new side
   */
   void setSide(unsigned int side) { this->side = side; }
   unsigned int getSide() { return side; }

   virtual void update(int64 diff);
   virtual float getMoveSpeed() const = 0;

   virtual bool isSimpleTarget() { return false; }

   Target* getTarget() const { return target; }
   void setTarget(Target* target);
   void setWaypoints(const std::vector<MovementVector>& waypoints);

   const std::vector<MovementVector>& getWaypoints() const { return waypoints; }
   uint32 getCurWaypoint() const { return curWaypoint; }
   bool isMovementUpdated() { return movementUpdated; }
   void clearMovementUpdated() { movementUpdated = false; }
   bool isToRemove() { return toRemove; }
   virtual void setToRemove() { toRemove = true; }

   uint32 getNetId() const { return id; }
   Map* getMap() const { return map; }

   void setPosition(float x, float y);

   uint32 getCollisionRadius() const { return collisionRadius; }
   uint32 getVisionRadius() const { return visionRadius; }
   bool collide(Object* o);
   
   uint32 getAttackerCount() const { return attackerCount; }
   void incrementAttackerCount() { ++attackerCount; }
   void decrementAttackerCount() { --attackerCount; }

   bool isVisibleByTeam(uint32 side);
   void setVisibleByTeam(uint32 side, bool visible);
};

#endif /* OBJECT_H_ */
