
#ifndef RUNPOS_H
#define RUNPOS_H
#include "singleton.h"
#include "geometry.h"
#include "VisionModule.h"
#include <vector>
#include <algorithm>

/*************************************************/
/*            calculate run pos module           */
/*************************************************/

namespace RUNPOS_PARAM {
// 各项峰值
const float maxDistToGoal = sqrt(pow(PARAM::Field::PITCH_LENGTH, 2) + pow(PARAM::Field::PITCH_WIDTH, 2));
const float maxShootAngle = PARAM::Math::PI / 2;
const float maxDistToBall = 4500;
const float maxAngle2Goal = PARAM::Math::PI / 4;
const float maxDist2Enemy = 1500;
const float maxGuardTime  = 2.0;
const float maxSectorDir  = PARAM::Math::PI / 180 * 17;
const float maxSectorDist = 15000;
// 评估函数各项的权重
const float weight1 = 40;// 1.距离对方球门的距离
const float weight2 =  0;// 2.射门有效角度
const float weight3 = 10;// 3.跟球的距离
const float weight4 =  5;// 4.angle to goal
const float weight5 =  0;// 5.dist to enemy
const float weight6 = 20;// 6.their guard time
const float weight7 = 25;// 7.sector behind enemy
}

struct runPosProperties {
    CGeoPoint pos;
    int areaNum;
    float score = 0;
    bool isValid = true;
};

class RunPos
{
public:
    RunPos();
    CGeoPoint runPos(int index) { return _runPos[index].pos; }
//    CGeoPoint freeKickWaitPos();
    void generateRunPos(const CVisionModule* pVision, const CGeoPoint& avoidPos, const CGeoPoint& avoidPos2);
    std::vector<float> evaluateFunc(const CVisionModule* pVision, const CGeoPoint& candidate);
private:
    runPosProperties _runPos[2];
    const CVisionModule* _pVision;
    CGeoPoint _avoidPos;
    CGeoPoint _avoidPos2;
    CGeoPoint _ballPos;
    bool _pos2Switch = true;
    void generateBroad();
    void selectAreaBestPoint(int areaNum);
    void judgeRunPosValid();
    void saoAction(const CVisionModule* pvision);
    bool _saoSwitch = false;
    std::vector<CGeoPoint> sampling(int x, int y, int threshold_x, int threshold_y);
    bool isInRangeOfPos(CGeoPoint& _pos1, CGeoPoint& _pos2, float threshold);
    bool isInStripZone(CGeoPoint& testPos, CGeoPoint& _pos1, CGeoPoint& _pos2, float threshold);
};
typedef NormalSingleton<RunPos> RunPosModule;
#endif // RUNPOS_H
