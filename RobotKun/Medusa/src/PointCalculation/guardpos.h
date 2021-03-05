#ifndef GUARDPOS_H
#define GUARDPOS_H

#include "VisionModule.h"
#include "singleton.h"
#include <vector>

class CGuardPos
{
public:
    CGuardPos();
    CGeoPoint backPos(int guardNum, int index, int realNum ,int defendNum);
    CGeoPoint backPos(int guardNum, int index);
    void setBackNum(int realNum, int index);
    int checkValidNum(int guardNum);
    bool validBackPos(CGeoPoint backPos, int realNum);
    int missingBackIndex(int i);
private:
    void generatePos(int guardNum);
    CGeoPoint generateDefendPos(int index, int defendNum);
    bool leftNextPos(CGeoPoint basePos, CGeoPoint& nextPos, double dist=-9999);
    bool rightNextPos(CGeoPoint basePos, CGeoPoint& nextPos, double dist=-9999);
    bool validdefendNum(int index, int defendNum);
    bool checkGuardPos(CGeoPoint &guardPos, int realNum, int index, int defendNum);
    CGeoPoint _backPos[PARAM::Field::MAX_PLAYER];
    int _backNum[PARAM::Field::MAX_PLAYER];
    int _defendNum[PARAM::Field::MAX_PLAYER];
    int _missingBack[PARAM::Field::MAX_PLAYER];
    int _priority[PARAM::Field::MAX_PLAYER];
    int _changeDefend[PARAM::Field::MAX_PLAYER];
    int _guardNum;
};

typedef NormalSingleton<CGuardPos> GuardPos;
#endif // GUARDPOS_H
