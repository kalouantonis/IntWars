#ifndef TARGET_H_
#define TARGET_H_

#include <memory>

class Target {

public:
   
   virtual ~Target() { }
   Target(float x, float y) : x(x), y(y) { }

   float distanceWith(Target* target);
   float distanceWith(float xtarget, float ytarget);

   float getX() const { return x; }
   float getY() const { return y; }

   void setPosition(float x, float y) { this->x = x; this->y = y; }

   virtual bool isSimpleTarget() { return true; }

protected:
	
   float x, y;
};


#endif
