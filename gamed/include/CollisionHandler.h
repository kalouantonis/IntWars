#ifndef _COLLISIONHANDLER_H
#define _COLLISIONHANDLER_H

#include "stdafx.h"
#include "Vector2.h"
#include "Pathfinder.h"

class Object;
class CollisionHandler : public Pathfinder
{
public:
   CollisionHandler() {}
   ~CollisionHandler() {}

	void update(float a_DT);
};

#endif