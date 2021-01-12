/**************************************
* CUDA C acceleration module
* Author: Wang Yun Kai
* Created Date: 2019/3/17
**************************************/
#ifndef __CUDA_MODULE_H__
#define __CUDA_MODULE_H__

#include "singleton.hpp"
#include "VisionModule.h"
#include <MultiThread.h>
#include "geometry.h"

enum EvaluateMode{
    PASS = 1,
    SHOOT = 2,
    FREE_KICK = 3,
};

typedef struct {
    float x, y;
} Vector;

typedef struct {
    float x, y;
} Point;

typedef struct {
    Point Pos;
    Vector Vel;
    bool isValid;
} Player;

typedef struct {
    Point interPos;
    float interTime;
    float Vel;
    float dir;
    int playerIndex;
    float deltaTime;
    float Q;
} rType;

class CUDAModule {
  public:
    CUDAModule();
    ~CUDAModule();
    void initialize(const CVisionModule *);
    ZSS_THREAD_FUNCTION void run();
    ZSS_THREAD_FUNCTION void calculateFreeKickPos();
    ZSS_THREAD_FUNCTION void drawScore();
//    CGeoPoint getBestPosition(int i);
    CGeoPoint getBestFreeKickPos(){ return bestFreeKickPos; }
    CGeoPoint getBestFlatPass(){ return bestFlatPassPos; }
    CGeoPoint getBestChipPass(){ return bestChipPassPos; }
    CGeoPoint getBestFlatShoot(){ return  bestFlatShootPos; }
    CGeoPoint getBestChipShoot(){ return bestChipShootPos;}
    float getBestFlatPassQ(){ return bestFlatPassQ; }
    float getBestChipPassQ(){ return bestChipPassQ; }
    float getBestFlatShootQ(){ return bestFlatShootQ; }
    float getBestChipShootQ(){ return bestChipShootQ; }
//    CGeoPoint getBestRunPos(int areaNum);
    int getBestFlatPassNum(){ return bestFlatPassNum; }
    int getBestChipPassNum(){ return  bestChipPassNum; }
    int getBestFlatShootNum(){ return bestFlatShootNum; }
    int getBestChipShootNum(){ return bestChipShootNum; }
    float getBestFlatPassVel() { return bestFlatPassVel; }
    float getBestFlatShootVel(){ return bestFlatShootVel;}
    float getBestChipPassVel() {
        float time = 2 * bestChipPassVel * sin(54.29 / 180 * Param::Math::PI)/1000/9.8;
        float length = 1.0 / 2 * 9.8 * time *time / tan(54.29 / 180 * Param::Math::PI);
        return length*1000;}
    float getBestChipShootVel(){
        float time = 2 * bestChipShootVel * sin(54.29 / 180 * Param::Math::PI)/1000/9.8;
        float length = 1.0 / 2 * 9.8 * time *time / tan(54.29 / 180 * Param::Math::PI);
        return length*1000;}
    void calculateBestPass();
//    void calculateBestPosition();
    void setLeader(int _leader) { leader = _leader; }
    void setLeaderPos(CGeoPoint leaderPos){ this->leaderPos = leaderPos; }
    void setReceiver(int _receiver) { receiver = _receiver; }
    void reset();
private:
    std::vector<float> evaluateFunc(CGeoPoint candidate, EvaluateMode mode);
    CGeoPoint bestFlatPassPos;
    CGeoPoint bestChipPassPos;
    CGeoPoint bestFlatShootPos;
    CGeoPoint bestChipShootPos;
    CGeoPoint bestFreeKickPos;
    float bestFlatPassQ;
    float bestChipPassQ;
    float bestFlatShootQ;
    float bestChipShootQ;
    CGeoPoint bestScorePosition[16];
    int bestFlatPassNum;
    int bestChipPassNum;
    int bestFlatShootNum;
    int bestChipShootNum;
    float bestFlatPassVel;
    float bestChipPassVel;
    float bestFlatShootVel;
    float bestChipShootVel;
    int leader;
    CGeoPoint leaderPos;
    int receiver;
    Player *players;
    Player *enemy;
    Point *ball;
    float* rollingFraction;
    float* slidingFraction;
    const CVisionModule* pVision;
    std::vector<rType> passPoints;
    std::vector<rType> chipPoints;
//    bool canRunDirect(const CGeoPoint start, const CGeoPoint end, float avoidDist);
//    float rotateTime(float initDir, float finalDir);
};
typedef Singleton<CUDAModule> ZCUDAModule;
#endif //__CUDA_MODULE_H__
