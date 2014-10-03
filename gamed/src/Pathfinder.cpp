#include "Pathfinder.h"
#include "Map.h"
#include "AIMesh.h"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cstdlib>

Map * Pathfinder::chart = 0;
AIMesh *Pathfinder::mesh = 0;

Path Pathfinder::getPath(Vector2 from, Vector2 to, float boxSize = PATH_DEFAULT_BOX_SIZE)
{
   Path path;
   //path.cleanLists();
   path.gridNodeSize = boxSize;
   Vector2 distance = (to - from) / boxSize;

   if ((256 - abs(distance.X)) > 0 && (256 - abs(distance.Y)) > 0) // difference in path is size on both width and height is less than the distance/boxsize
   {
      path.mapTranslation.X = ((to.X < from.X) ? (to.X) : (from.X)) - (256 - abs(distance.X))*0.5f;
      path.mapTranslation.Y = ((to.Y < from.Y) ? (to.Y) : (from.Y)) - (256 - abs(distance.Y))*0.5f;

      if (path.mapTranslation.X < 0) path.mapTranslation.X = 0;
      if (path.mapTranslation.Y < 0) path.mapTranslation.Y = 0;
   }
   else return getPath(from, to, boxSize*2.0f); // Otherwise increase the boxsize until we're at something we can work with.

   path.start = path.fromPositionToGrid(from); // Save start in grid info
   path.destination = path.fromPositionToGrid(to); // Save destination in grid info

   path.insertObstructions(chart, mesh); // Ready the map.

   path.addToOpenList(path.start, 0); // Let's start at the start.

   for (int tries = 0; path.openList.size() != 0; tries++) // Go through the open list while it's not empty
   {
      if (tries == MAX_PATHFIND_TRIES)
      {
         path.error = PATH_ERROR_OUT_OF_TRIES;
         path.waypoints = reconstructUnfinishedPath(path);
      }
      else if (traverseOpenList(path))
      {
         path.error = PATH_ERROR_NONE;
         path.waypoints = reconstructPath(path);
      }
   }

   path.error = PATH_ERROR_OPENLIST_EMPTY;
   path.waypoints.push_back(from);
   return path;
}

bool Pathfinder::traverseOpenList(Path path)
{
   if (path.openList.size() == 0) return true;
   
   // This sorts every iteration, which means that everything but the last couple of elements are sorted.
   // TODO: That means, this can probably be optimised. Sort only the last elements and add them into the vector where they belong.
   // But honestly, it's running pretty fast so why bother
   std::sort(path.openList.begin(), path.openList.end(), SortByF);

   PathNode * currentNode = path.openList.back();
   path.openList.pop_back();

   bool atDestination = (currentNode->x == (int)path.destination.X && currentNode->y == (int)path.destination.Y);

   if (!atDestination) // While we're not there
   {
      for (int dx = -1; dx <= 1; dx++) if (currentNode->x + dx >= 0 && currentNode->x + dx < GRID_WIDTH) // Search in 8 directions, but we're supposed to stay in map
      {
         for (int dy = -1; dy <= 1; dy++) if (!(dx == 0 && dy == 0)) // in y 8 directions, ignore the x==y==0 where we dont move
         {
            if (!path.isGridNodeOccupied(currentNode->x + dx, currentNode->y + dy)) // Is something here?
            {
               PathNode* conflictingNode = path.isNodeOpen(currentNode->x + dx, currentNode->y + dy); // Nothing is here, did we already add this to the open list?
               if (!conflictingNode) // We did not, add it
                  path.addToOpenList(Vector2((float)(currentNode->x + dx), (float)(currentNode->y + dy)), currentNode);
               else if (conflictingNode->g > CALC_G(currentNode->g)) // I found a shorter route to this node.
               {
                  conflictingNode->setParent(currentNode); // Give it a new parent
                  conflictingNode->setScore((int)CALC_H(conflictingNode->x, conflictingNode->y, path.destination.X, path.destination.Y), CALC_G(currentNode->g)); // set the new score.
               }
            }
         }
      }
   }

   path.closedList.push_back(currentNode);
   return atDestination;
}

std::vector<Vector2> Pathfinder::reconstructPath(Path path) // Make a std::vector of the waypoints
{
   std::vector<Vector2> ret;
   auto i = (path.closedList.back());

   while (i)
   {
      ret.push_back(path.fromGridToPosition(Vector2((float)i->x, (float)i->y)));
      i = i->parent;
   }

   return ret;
}

std::vector<Vector2> Pathfinder::reconstructUnfinishedPath(Path path) // Let's go over the closed list and go back to the start, create a path from the best choice.
{
   std::vector<Vector2> ret;
   
   auto a = path.closedList.back();
   int lowestG = 9e7;
   for (auto i = path.closedList.begin(); i != path.closedList.end(); i++)
      if ((*i)->g < lowestG)
      {
         lowestG = (*i)->g;
         a = (*i);
      }

   while (a)
   {
      ret.push_back(path.fromGridToPosition(Vector2((float)a->x, (float)a->y)));
      a = a->parent;
   }

   return ret;
}

PathNode* Path::isNodeOpen(int x, int y)
{
   // TODO: Optimise? This is where the application is residing in 96% of the time during pathfinding.

   // It checks if we've already added this x and y to the openlist. If we did, return it. 
   for (auto i = openList.begin(); i != openList.end(); i++)
   {
      if ((*i)->x == x && (*i)->y == y)
         return (*i);
   }

   return 0;
}

bool SortByF(const PathNode* first, const PathNode* second) // Sort function for finding best score
{
   return (first->g + first->h > second->g + second->h); // Greater than because we want the lower scores in the back (first)
}

Vector2 Path::fromGridToPosition(Vector2 position)
{
   return (position+mapTranslation)*gridNodeSize;
}

Vector2 Path::fromPositionToGrid(Vector2 position)
{
   auto pos = (position / gridNodeSize) - mapTranslation;
   pos.X = floorf(pos.X);
   pos.Y = floorf(pos.Y);
   return pos;
}

bool Path::isGridNodeOccupied(int x, int y)
{
   if ((x >= 0 && x < GRID_SIZE) &&
       (y >= 0 && y < GRID_SIZE))
   {
      return map[x][y].isOccupied();
   }
   else return true;
}


void Path::insertObstructions(Map * chart, AIMesh *mesh) // insert all objects into the map
{
   std::memset(map, false, sizeof(Grid)*GRID_WIDTH*GRID_HEIGHT); // Empty map

   if (chart == NULL) return;

   const std::map<uint32, Object*>& objects = chart->getObjects();

   for (auto i = objects.begin(); i != objects.end(); i++) // For every object
   {
      Vector2 gridPos = fromPositionToGrid(i->second->getPosition()); // get the position in grid size
      int radius = ((int)(i->second->getCollisionRadius() / gridNodeSize)) / 2; // How many boxes does the radius of this object cover?
      
      for (int dx = -radius; dx < radius; dx++) // For the whole radius in the width
         if (gridPos.X + dx >= 0 && gridPos.X + dx <GRID_WIDTH) // As long as we're in the map (x)
            for (int dy = -radius; dy < radius; dy++) // for the whole radius in the y
               if (gridPos.Y + dy >= 0 && gridPos.Y + dy < GRID_HEIGHT) // As long as we're in the map (y)
                  map[(int)gridPos.X + dx][(int)gridPos.Y + dy].occupied = true; // Occupy this piece of the map.
   }

   if (mesh == NULL) return;

   // Now to draw the mesh onto the thing.
   if (mesh->isLoaded()) // if we have loaded the mesh
      for (int x = 0; x < GRID_WIDTH; x++) // for every grid piece
         for (int y = 0; y < GRID_WIDTH; y++)
         {
            Vector2 translated = fromGridToPosition(Vector2((float)x, (float)y));
            if (mesh->getY(translated.X, translated.Y) < 0.1f) // If there's nothing at this position
               map[x][y].occupied = true; // This is obstructed
         }

}

#ifdef USE_OPTIMISATION
char PathNode::tableInitialised = -1;
std::vector<PathNode*> PathNode::nodeTable;

// Create a table of items to make sure we don't new/delete during runtime
void PathNode::InitTable()
{
   if (tableInitialised!=-1) return; // We have already initialised it or we're busy doing it.
   tableInitialised = 0;
   nodeTable.reserve(TABLE_SIZE);
   for (int i = 0; i < TABLE_SIZE; i++)
   {
      nodeTable.push_back(new PathNode());
   }

   tableInitialised = 1;
}

void* PathNode::operator new(size_t size) // catch news
{
   PathNode *ret;
   switch(tableInitialised)
   {
   case -1: // Should start
      InitTable();

   case 1: // Already cached
      //if (nodeTable.size() == 0) 0;
         //__debugbreak(); // Tried to get a new node while we're all out of nodes.
      ret = nodeTable.back();
      ret->Init(0, 0, 0, 0, 0);
      nodeTable.pop_back();
      break;

   case 0: // creating cache
      ret = (PathNode*)std::malloc(size);
      //if (ret == 0) 0;//__debugbreak(); // No memory?
   }
   return ret;
}

void PathNode::operator delete(void * object)
{
   if (tableInitialised == 2) std::free(object);
   else nodeTable.push_back((PathNode*)object);
}
#endif


void Path::cleanLists()
{
   for (auto i = openList.begin(); i != openList.end(); i++) 
      delete (*i);
   openList.clear();
   for (auto i = closedList.begin(); i != closedList.end(); i++) 
      delete (*i);
   closedList.clear();
}

void Pathfinder::setMap(Map * map)
{ 
   chart = map; 
   mesh = chart->getAIMesh(); 
}