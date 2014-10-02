#include "Target.h"
#include <cmath>

float Target::distanceWith(Target* target) {
  return distanceWith(target->getX(), target->getY());
}

float Target::distanceWith(float xtarget, float ytarget) {
    return std::sqrt((x-xtarget)*(x-xtarget)+(y-ytarget)*(y-ytarget));
}
