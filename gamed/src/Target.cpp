#include "Target.h"
#include <cmath>

float Target::distanceWith(const TargetPtr& target) {
  return distanceWith(target->getX(), target->getY());
}

float Target::distanceWith(float xtarget, float ytarget) {
    return std::sqrt((x-xtarget)*(x-xtarget)+(y-ytarget)*(y-ytarget));
}
