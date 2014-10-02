#ifndef TARGET_H_
#define TARGET_H_

#include <memory>

class Target;
typedef std::shared_ptr<Target> TargetPtr;

class Target {

public:
   
   virtual ~Target() { }
   Target(float x, float y) : x(x), y(y) { }

   float distanceWith(const TargetPtr& target);
   float distanceWith(float xtarget, float ytarget);

   float getX() const { return x; }
   float getY() const { return y; }

   void setPosition(float x, float y) { this->x = x; this->y = y; }

   virtual bool isSimpleTarget() { return true; }

protected:
	
   float x, y;
};


#endif
