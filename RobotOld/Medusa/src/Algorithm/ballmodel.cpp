#include "ballmodel.h"
#include <param.h>
#include <iostream>
using namespace std;
namespace {
///chip param
const double CHIP_FIRST_ANGLE = 54.29 / 180.0 * Param::Math::PI;
const double CHIP_SECOND_ANGLE = 45.59 / 180.0 * Param::Math::PI;
const double CHIP_LENGTH_RATIO = 1.266;
const double CHIP_VEL_RATIO = 0.6372;
const double G = 9.8;

///flat param
double ROLL_FRICTION;
double SLIDE_FRICTION;
double SLIDE_BALL_ACC;
double ROLL_BALL_ACC;
const double SLIDE_FACTOR = 15.0;
const double FLAT_SLIDE_PART = 2.0 / 7.0;
}

CBallModel::CBallModel() {
    bool isSimulation;
    ZSS::ZParamManager::instance()->loadParam(isSimulation, "Alert/IsSimulation", false);
    if(isSimulation)
        ZSS::ZParamManager::instance()->loadParam(ROLL_FRICTION, "AlertParam/Friction4Sim", 152.0);
    else
        ZSS::ZParamManager::instance()->loadParam(ROLL_FRICTION, "AlertParam/Friction4Real", 80.0);
    SLIDE_FRICTION = SLIDE_FACTOR * ROLL_FRICTION;
    SLIDE_BALL_ACC = SLIDE_FRICTION / 2.0;
    ROLL_BALL_ACC = ROLL_FRICTION / 2.0;
}

double CBallModel::flatRollTime(double startVel) {
    if(startVel < 0.0) {
        cout << "Oh shit!!! Error startVel to calculate roll time!!! ---CBallModel.cpp" << endl;
        return 0.0;
    }
    return (startVel * (1 - FLAT_SLIDE_PART) / ROLL_BALL_ACC);
}

double CBallModel::flatSlideTime(double startVel) {
    if(startVel < 0.0) {
        cout << "Oh shit!!! Error startVel to calculate slide time!!! ---CBallModel.cpp" << endl;
        return 0.0;
    }
    return (startVel * FLAT_SLIDE_PART / SLIDE_BALL_ACC);
}

double CBallModel::flatSlideDist(double startVel) {
    if(startVel < 0.0) {
        cout << "Oh shit!!! Error startVel to calculate slide distance!!! ---CBallModel.cpp" << endl;
        return 0.0;
    }
    return (pow(startVel, 2.0) - pow(startVel * (1.0 - FLAT_SLIDE_PART), 2.0)) / (2.0 * SLIDE_BALL_ACC);
}

double CBallModel::flatRollDist(double startVel) {
    if(startVel < 0.0) {
        cout << "Oh shit!!! Error startVel to calculate roll distance!!! ---CBallModel.cpp" << endl;
        return 0.0;
    }
    return pow(startVel * (1.0 - FLAT_SLIDE_PART), 2.0) / (2.0 * ROLL_BALL_ACC);
}

double CBallModel::flatStopTime(double startVel) {
    return flatRollTime(startVel) + flatSlideTime(startVel);
}

CGeoPoint CBallModel::flatPos(CGeoPoint currentPos, CVector startVel, double t) {
    if(t < 0.0) {
        cout << "Oh shit!!! Error time to calculate ball positon!!! ---CBallModel.cpp" << endl;
        return CGeoPoint(1e8, 1e8);
    }
    double velMod = startVel.mod();
    double slidingTime = flatSlideTime(velMod);
    double totalTime = flatStopTime(velMod);
    double moveDist = 0.0;
    if(t > totalTime) {
        moveDist = flatRollDist(velMod) + flatSlideDist(velMod);
    }
    else if(t < slidingTime) {
        moveDist = velMod * t - 0.5 * SLIDE_BALL_ACC * pow(t, 2.0);
    }
    else {
        double slideDist = (pow(velMod, 2.0) - pow(velMod * (1.0 - FLAT_SLIDE_PART), 2.0)) / (2.0 * SLIDE_BALL_ACC);
        double rollStartVel = velMod * (1.0 - FLAT_SLIDE_PART);
        double rollTime = t - slidingTime;
        double rollDist = rollStartVel * rollTime - 0.5 * ROLL_BALL_ACC * pow(rollTime, 2.0);
        moveDist = slideDist + rollDist;
    }
    startVel = startVel / velMod;
    return (currentPos + startVel * moveDist);
}

double CBallModel::chipToTargetTime(double chipPower, double distToTarget) {
    double chipLength1 = chipPower / 100.0; //cm->m
    double chipTime1 = sqrt(2.0 * chipLength1 * tan(CHIP_FIRST_ANGLE) / G);
    double chipLength2 = chipSecondJumpDist(chipLength1);
    double chipTime2 = sqrt(2.0 * chipLength2 * tan(CHIP_SECOND_ANGLE) / G);
    chipLength1 *= 100.0;
    chipLength2 *= 100.0;
    double rollDist = distToTarget - chipLength1 - chipLength2;
    double rollVel = pow(chipTime1 * 100 * G / (2 * sin(CHIP_FIRST_ANGLE)), 2) * CHIP_VEL_RATIO / 980;;
    double time = chipTime1 + chipTime2;
    if(rollDist > 0) {
        double velLeft = pow(rollVel, 2) - 2.0 * ROLL_BALL_ACC * rollDist;
        if(velLeft < 0)
            time += rollVel / ROLL_BALL_ACC;
        else
            time += (rollVel - sqrt(velLeft)) / ROLL_BALL_ACC;
    }
    return time;
}

double CBallModel::chipSecondJumpDist(double chipPower) {
    return (CHIP_LENGTH_RATIO - 1.0) * chipPower;
}

double CBallModel::chipJumpTime(double chipPower) {
    double chipLength1 = chipPower / 100.0; //cm->m
    double chipTime1 = sqrt(2.0 * chipLength1 * tan(CHIP_FIRST_ANGLE) / G);
    double chipLength2 = chipSecondJumpDist(chipLength1);
    double chipTime2 = sqrt(2.0 * chipLength2 * tan(CHIP_SECOND_ANGLE) / G);
    return chipTime1;
}
