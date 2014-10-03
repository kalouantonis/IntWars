#include "CollisionHandler.h"
#include "Map.h"
#include "AIMesh.h"
#include "Unit.h"

void CollisionHandler::update(float a_DT)
{
	const std::map<uint32, Object*>& objects = chart->getObjects();

	for (auto i = objects.begin(); i != objects.end(); i++)
   //auto i = objects.begin();
	{
		Object* o1 = i->second;

      Unit * unit = dynamic_cast<Unit*>(o1);
      if (unit && unit->isDead()) continue; // Don't collide with anything if we're dead.

      for (auto j = objects.begin(); j != objects.end(); j++) if (j != i) // Object-Object collision
      {
         Object* o2 = j->second;

         unit = dynamic_cast<Unit*>(o2);
         if (unit && unit->isDead()) continue; // Can't collide with dead things.

         if ((o1->getPosition() - o2->getPosition()).SqrLength() < (o1->getCollisionRadius() + o2->getCollisionRadius())*(o1->getCollisionRadius() + o2->getCollisionRadius())) //distance check
         {
            o1->onCollision(o2); // Call the collision callback
            //o2->onCollision(01); 
            // Don't do this, it's being done in a later iteration, given that o1 doesn't move away in onCollision.
            // Uh, if it does, uncomment it I guess.
         }
		}

      Target *target = o1->getTarget();
      if (target)
      {
         Vector2 test = o1->getPosition() + (o1->getDirection()*((float)o1->getCollisionRadius()*2.0f)); // Get the outer bound of your character in the direction I am walking.
                                                                                                         // Have to multiply this radius by 2 for some reason.. Idk why.
                                                                                                         // TODO: Figure out why.

         if (!mesh->isWalkable(test.X, test.Y)) // If we cant walk here according to the mesh.
            o1->onCollision(0);
      }
	}
}