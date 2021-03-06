#ifndef _PATHFINDER_H
#define _PATHFINDER_H

#include <vector>
#include <list>
#include "Vector2.h"
#include "Object.h"

#define MAX_PATHFIND_TRIES 1000

#define TABLE_SIZE (2<<15)
#define GRID_SIZE 256
#define GRID_WIDTH GRID_SIZE
#define GRID_HEIGHT GRID_SIZE

// functions to calculate the G and H factor of the score of the node
#define CALC_H(CURX, CURY, ENDX, ENDY) (abs(CURX - ENDX) + abs(CURY - ENDY))
#define CALC_G(PARENT_G) (PARENT_G+1) // Yes. I know.

#define PATH_DEFAULT_BOX_SIZE 50.0f

// Use to increase speed on pathfinding, could cause trouble when used on multiple threads
//#define USE_OPTIMISATION

class PathNode
{  
public:
   PathNode(){ InitTable(); }
   PathNode(int ax, int ay, int ag, int ah, PathNode *p){ Init(ax, ay, ag, ah, p); }
   PathNode(Vector2 pos, int ag, int ah, PathNode *p){ Init((int)pos.X, (int)pos.Y, ag, ah, p); }
   void Init(int ax, int ay, int ag, int ah, PathNode *p) { InitTable(); x = ax; y = ay; h = ah; g = ag; parent = p; }
   void setScore(int ah, int ag) { g = ag, h = ah; }
   void setParent(PathNode* p) { parent = p; }
   int x, y, h, g;
   PathNode* parent;
#ifdef USE_OPTIMISATION
public:
   void* operator new(size_t size);
   void operator delete(void * objects);
private:
   static bool isTableEmpty() { InitTable(); return nodeTable.size() == TABLE_SIZE; }
   static unsigned int missingNodes() { InitTable(); return TABLE_SIZE - nodeTable.size(); }
   static void DestroyTable() { tableInitialised = 2; for (int i = 0; i < TABLE_SIZE; i = 0) delete nodeTable[i]; nodeTable.clear(); tableInitialised = -1; }
   static void InitTable();
   static std::vector<PathNode*> nodeTable;
   static char tableInitialised;
#else
   static void InitTable(){}
   static void DestroyTable(){}
#endif
};

struct Grid
{
   bool isOccupied() { return occupied;/*return (occupant != NULL);*/ }
   bool occupied;
   //Object * occupant;
};

class Map;
class AIMesh;

enum PathError
{
   PATH_ERROR_NONE = 0,
   PATH_ERROR_OUT_OF_TRIES,
   PATH_ERROR_OPENLIST_EMPTY
};

class Path
{
public:
   friend class Pathfinder;
   std::vector<Vector2> waypoints;

   bool isPathed() { return error == 0; }
   PathError getError() { return error; }
   std::vector<Vector2> getWaypoints() { return waypoints; }
protected:
   Vector2 fromGridToPosition(Vector2 position);
   Vector2 fromPositionToGrid(Vector2 position);

   void addRealPosToOpenList(Vector2 position, PathNode* parent)
   {
      addGridPosToOpenList(fromPositionToGrid(position), parent);
   }

   void addGridPosToOpenList(Vector2 position, PathNode* parent)
   {
      openList.push_back(new PathNode(position, (int)CALC_G((parent) ? (parent->g) : (0)), (int)CALC_H(position.X, position.Y, destination.X, destination.Y), parent));
   }

   void addToOpenList(Vector2 position, PathNode* parent)
   {
      addGridPosToOpenList(position, parent);
   }

   bool isGridNodeOccupied(Vector2 pos) { isGridNodeOccupied((int)pos.X, (int)pos.Y); }
   bool isGridNodeOccupied(int x, int y);
   PathNode* isNodeOpen(int x, int y);

   void cleanLists();
   void insertObstructions(Map * chart, AIMesh *mesh);

   std::vector<PathNode*> openList, closedList;
   Grid map[GRID_SIZE][GRID_SIZE];
   Vector2 mapTranslation, start, destination;
   float gridNodeSize;
   PathError error;
};

class Pathfinder
{
public:
   Pathfinder()/*:mesh(0),chart(0)*/ {}
   ~Pathfinder() {}

   static Path getPath(Vector2 from, Vector2 to, float boxSize);
   void setMap(Map * map);// { chart = map; mesh = chart->getAIMesh(); }
protected:
   static Map * chart;
   static AIMesh *mesh;
private:
private:

   static bool traverseOpenList(Path path);
   static std::vector<Vector2> reconstructPath(Path path);
   static std::vector<Vector2> reconstructUnfinishedPath(Path path);
};

bool SortByF(const PathNode* first, const PathNode* second);
#endif