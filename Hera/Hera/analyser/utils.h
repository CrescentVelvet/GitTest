#ifndef UTILS_H
#define UTILS_H

#include "visionmodule.h"
#include "singleton.hpp"
#include "geometry.h"


class CUtils
{
public:
    CUtils();
    bool inArea(CGeoPoint _p, double x1, double x2, double y1, double y2);
    bool inArea(CGeoPoint _p, const CGeoPoint &center , double radius);
};
typedef Singleton< CUtils > Utils;
#endif // UTILS_H
