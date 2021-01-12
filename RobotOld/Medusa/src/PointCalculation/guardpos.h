#ifndef GUARDPOS_H
#define GUARDPOS_H

#include "VisionModule.h"
#include "singleton.h"

class CGuardPos
{
public:
    CGuardPos();
    CGeoPoint backPos(int guardNum, int index);
    void setBackNum(int realNum, int index);
    int checkValidNum(int guardNum);
    bool validBackPos(CGeoPoint backPos, int realNum);
    int missingBackIndex(int i);
private:
    void generatePos(int guardNum);
    bool leftNextPos(CGeoPoint basePos, CGeoPoint& nextPos, double dist=-9999);
    bool rightNextPos(CGeoPoint basePos, CGeoPoint& nextPos, double dist=-9999);
    CGeoPoint _backPos[PARAM::Field::MAX_PLAYER];
    int _backNum[PARAM::Field::MAX_PLAYER];
    int _missingBack[PARAM::Field::MAX_PLAYER];
};

typedef NormalSingleton<CGuardPos> GuardPos;
#endif // GUARDPOS_H
