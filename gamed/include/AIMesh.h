#ifndef _AIMESH_H
#define _AIMESH_H
#include <vector>
#include <string>
#include "Vector2.h"

#define MAX_TRIANGLE_COUNT (2<<15) 
#define AIMESH_TEXTURE_SIZE 1024

struct ScanLine
{
   float x, y, z, u, v;
};

struct Vertex
{
   float x, y, z;
};

#pragma pack(push, 1)
struct Triangle
{
	struct
	{
		float v1[3]; // Seems to be a vertex [x,y,z]
		float v2[3]; // Seems to be a vertex [x,y,z]
		float v3[3]; // Seems to be a vertex [x,y,z]
	} Face; // If above is true

	short unk1;
	short unk2;
	short triangle_reference;
};

struct __AIMESHFILE
{
	char magic[8];
	int version;
	int triangle_count;
	int zero[2];

   Triangle triangles[MAX_TRIANGLE_COUNT];
   // TODO: I have to find a better way to do this. :/
   // Basically, it's a set of triangles that go on for triangle_count. 
   // Dynamic allocation is stupid, and this way is potentially well, unsafe.
   // pointers will make the app think that the first 4 bytes are a pointer.. Sigh.
}; 
#pragma pack(pop)

class AIMesh
{
public:
	AIMesh();
	~AIMesh();

   bool load(std::string inputFile);

   float getY(float argX, float argY);
   bool isWalkable(float argX, float argY) { return getY(argX, argY) > -254.0f; }
   float castRay(Vector2 origin, Vector2 direction) { return sqrt(castRaySqr(origin, direction)); }
   float castRaySqr(Vector2 origin, Vector2 direction);

   float getWidth() { return mapWidth; }
   float getHeight() { return mapHeight; }

   Vector2 TranslateToTextureCoordinate(Vector2 vector);
	
   bool isLoaded() { return loaded; }
private:
   void drawLine(float x1, float y1, float x2, float y2, char *heightInfo, unsigned width, unsigned height);
   void drawTriangle(Triangle triangle, float *heightInfo, unsigned width, unsigned height);
   void fillScanLine(Vertex vertex1, Vertex vertex2);
	bool outputMesh(unsigned width, unsigned height);
	bool writeFile(float *pixelInfo, unsigned width, unsigned height);


	std::vector<unsigned char> buffer;
   __AIMESHFILE *fileStream;

   double lowX, lowY, highX, highY, lowestZ;
   ScanLine scanlineLowest[AIMESH_TEXTURE_SIZE], scanlineHighest[AIMESH_TEXTURE_SIZE];
   float *heightMap; float *xMap; float *yMap;
   float mapWidth, mapHeight;
   bool loaded;
};

#endif