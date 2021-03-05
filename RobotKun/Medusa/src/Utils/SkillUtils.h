/**************************************
* Skill 通用函数模块
* Author: Wang
* Created Date: 2018/5/2
**************************************/
#ifndef _SKILL_UTILS_
#define _SKILL_UTILS_

#include "VisionModule.h"
#include <CMmotion.h>
#include <iostream>
#include "utils.h"
#include "singleton.hpp"
namespace ZBallState {
enum {
    Our = 0,
    Their = 1,
    Both = 2,
    OurHolding = 3,
    TheirHolding = 4,
    BothHolding = 5
};
const std::string toStr[6] = {"Our", "Their", "Both", "OurHolding", "TheirHolding", "BothHolding"};
}
typedef int BallStateVar;
class SkillUtils {
  public:
    SkillUtils();
    ~SkillUtils();

    //摩擦
    double FRICTION;

    //判断球是否能走那么远距离
    bool IsBallReachable(double ballVel, double length, double friction);

    //获得我方车的截球时间
    double getOurInterTime(int);

    //获得敌方车的截球时间
    double getTheirInterTime(int);

    //获得我方车的截球点
    CGeoPoint getOurInterPoint(int);

    //获得敌方车的截球点
    CGeoPoint getTheirInterPoint(int);

    // whether ball is in fly time(chipsolver must calculate the result first!)
    bool isInFlyTime();

    // whether can use chipsolver's result
    bool canGetChipResult();

    //compute chip inter time and inter point
    bool predictedChipInterTime(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, CGeoPoint secondPoint, CVector3 chipKickVel,double firstChipTime,double RestChiptime, double responseTime);

    // reset compute chip inter time and inter point into invalid value
    void resetChipInterMessage();

    // get chip inter time
    double getChipInterTime(int);

    // get chip inter point
    CGeoPoint getChipInterPoint(int);

    //假设自己以ballVel速度射出球到target点，计算是否有对方车能截到球
    bool isSafeShoot(const CVisionModule* pVision, double ballVel, CGeoPoint target);

    //从射门点以ballVel速度射出球到target点，计算是否有对方车能截到球
    bool validShootPos(const CVisionModule* pVision, CGeoPoint shootPos, double ballVel, CGeoPoint target, double &interTime, const double responseTime=0, double ignoreCloseEnemyDist=-9999, bool ignoreTheirGoalie=false, bool ignoreTheirGuard=false, bool DEBUG_MODE=false);

    //从射门点以ballVel速度射出球到target点，计算是否有对方车能截到球
    bool validChipPos(const CVisionModule* pVision, CGeoPoint shootPos, double ballVel, CGeoPoint target, const double responseTime=0, /*double ignoreCloseEnemyDist=-9999, */bool ignoreTheirGuard=false, bool DEBUG_MODE=false);

    //获得敌方车预测截球时间
    double getPredictTime(int);

    //获得敌方车预测截球点
    CGeoPoint getPredictPoint(int);

    //计算截球信息和预测信息
    void run(const CVisionModule* pVision);

    //计算某辆车的截球时间和截球点,返回true代表能截球，返回false代表无法截球,responseTime为车截球缓冲时间，默认为0
    bool predictedInterTime(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, double responseTime = 0);

    //计算敌方某辆车的截球时间和截球点
    bool predictedTheirInterTime(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, double responseTime = 0);

    //计算对方某辆车的截球时间和截球点
    CGeoPoint predictedTheirInterPoint(CGeoPoint enemy, CGeoPoint ball);
    //
    double getOurBestInterTime();

    //
    double getTheirBestInterTime();

    //
    int getOurBestPlayer();

    //
    int getTheirBestPlayer();

    //
    int getTheirGoalie();
    //
    std::string getBallStatus() {
        return ZBallState::toStr[ballState];
    }

    //计算immortalRush时间
    double getImmortalRushTime(const CVisionModule* pVision, int robotNum, CGeoPoint targetPos, double targetDir = 0);

    //计算marking 阻挡跑位点
    CGeoPoint getMarkingPoint(CGeoPoint markingPos, CVector markingVel, double aMax, double dMax, double aRotateMax, double vMax, CGeoPoint protectPos);

    //计算多车包夹时转身角度
    double holdBallDir(const CVisionModule* pVision, int robotNum);

    bool predictedInterTimeV2(const CVector ballVel, const CGeoPoint ballPos, const PlayerVisionT me, CGeoPoint& interceptPoint, double& interTime, double responseTime=0);

    //返回zmarking的点
    CGeoPoint getZMarkingPos(const CVisionModule* pVision, const int robotNum, const int enemyNum, const int pri, const int flag=0);

  private:
    void calculateBallBelongs(const CVisionModule* pVision);

    //每帧获取inter有关的信息
    void generateInterInformation(const CVisionModule* pVision);

    //生成预测截球的有关信息
    void generatePredictInformation(const CVisionModule* pVision);

    // check their goalie
    void updateTheirGoalie(const CVisionModule* pVision);
    //帧数
    int _lastCycle = 0;

    //所有我方车的截球点
    CGeoPoint ourInterPoint[PARAM::Field::MAX_PLAYER];

    //所有我方截球时间
    double ourInterTime[PARAM::Field::MAX_PLAYER];

    //所有敌方车的截球点
    CGeoPoint theirInterPoint[PARAM::Field::MAX_PLAYER];

    //所有敌方截球时间
    double theirInterTime[PARAM::Field::MAX_PLAYER];

    //预测静止球以最大速度射出、敌方车最大速度加速的截球时间
    double predictTheirInterTime[PARAM::Field::MAX_PLAYER];

    //预测静止球以最大速度射出、敌方车最大速度加速的截球点
    CGeoPoint predictTheirInterPoint[PARAM::Field::MAX_PLAYER];

    // chip inter point
    CGeoPoint chipInterPoint[PARAM::Field::MAX_PLAYER];

    // chip inter time
    double chipInterTime[PARAM::Field::MAX_PLAYER];

    // whether ball is in chip fly, include first chip fly and second chip fly
    bool _isInFlyTime;

    // judge whether in first chip fly, if true, can get chipsolver's predict resultsss
    bool firstChipState;
    // judge whether in second chip fly
    bool secondChipState;

    void judgeChipResult(const CVisionModule* pVision);

    int ourBestInterRobot, theirBestInterRobot, theirGoalie;

    BallStateVar ballState;
};
typedef Singleton<SkillUtils> ZSkillUtils;
#endif
