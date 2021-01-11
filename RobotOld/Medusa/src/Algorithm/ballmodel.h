#ifndef BallModel_H
#define BallModel_H

#include <geometry.h>
#include <parammanager.h>
#include "singleton.hpp"
class CBallModel
{

public:
    CBallModel();
    double flatSlideDist(double startVel);
    double flatRollDist(double startVel);
    double flatSlideTime(double startVel);
    double flatRollTime(double startVel);
    double flatStopTime(double startVel);
    CGeoPoint flatPos(CGeoPoint currentPos, CVector startVel, double t);
    double chipToTargetTime(double chipPower, double distToTarget); //chipPower == first jump dist
    double chipSecondJumpDist(double chipPower);
    double chipJumpTime(double chipPower);
};
typedef Singleton <CBallModel> BallModel;

#endif // BallModel_H
