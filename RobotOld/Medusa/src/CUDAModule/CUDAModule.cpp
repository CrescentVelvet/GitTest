#include "CUDAModule.h"
#include <cuda.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "cuda_runtime.h"
#include <vector>
#include <QDebug>
#include <GDebugEngine.h>
#include <geometry.h>
#include "parammanager.h"
#include "Semaphore.h"
#include "PlayInterface.h"
#include "TaskMediator.h"
#include "ShootRangeList.h"
#include "parammanager.h"
#include "drawscore.h"
#include "runpos.h"
#include <algorithm>
#include "Global.h"
#include "RefereeBoxIf.h"

#define ROBOT_NUM (16)
#define VEL_NUM (16)
#define DIR_NUM (128)

extern Semaphore messi_to_cuda;
extern Semaphore vision_to_cuda;

extern "C" {
    void BestPass(Player*, Point*, rType* result, float* rollingFraction, float* slidingFraction);
//    void PosScore(Player* players, Point* ballPos, Point* bestPositions);
}

namespace {
bool SELF_PASS;
bool show_heatmap = false;
bool show_shoot_pos = true;
bool show_other_pos_score = false;
bool show_free_kick_pos = false;
bool show_pass_points = false;
bool show_pass_lines = false;
bool show_run_pos = false;
bool show_result_info = false;
int showWeightNum = 0;
bool IS_SIMULATION = false;
bool FREE_KICK_DEBUG = false;
bool IGNORE_THEIR_GUARD = true;
// 摩擦系数
double ROLLING_FRICTION = 40;
double SLIDING_FRICTION = ROLLING_FRICTION * 15;

int FREEKICK_RANGE = 1000;
// 传球点的参数
namespace PASS_PARAM {
const float wDist = 0.15f;
const float wPassLineDist = 0;
const float wTurnAngle = 0; //0.1;
const float wClosestEnemyDist = 0.03f;
const float wUnderPass = 0.0;
}
// 射门点的参数
namespace SHOOT_PARAM {
const float wShootAngle = 0.15f;
const float wDist = 0.2f;
const float wRefracAngle = 0.05f;
const float wAngle2Goal = 0.1f;
const float wPassLineDist = 0;
const float wGuardTime = 0.073f;
}
// 任意球的参数
namespace FREE_KICK_PARAM {
const float wShootAngle = 0.3f;
const float wClosestEnemyDist = 0.1f;
const double wDist = 0.2;
const float wRefracAngle = 0.0f;
double wAngle2Goal = 0.0;
const float wGuardTime = 0.1f;
const float wSector = 0.0f;
const float wPassLineDist = 0.8f;
double min2PenaltyDist = 20;
}
//传球角度误差, 角度制
double PASS_ANGLE_ERROR = 4;

std::thread* passPosThread = nullptr;
std::thread* scoreThread = nullptr;
std::thread* freeKickThread = nullptr;
}
CUDAModule::CUDAModule()
    : bestFlatPassPos(CGeoPoint(9999, 9999))
    , bestChipPassPos(CGeoPoint(9999, 9999))
    , bestFlatShootPos(CGeoPoint(9999, 9999))
    , bestChipShootPos(CGeoPoint(9999, 9999))
    , bestFlatPassQ(-9999)
    , bestChipPassQ(-9999)
    , bestFlatShootQ(-9999)
    , bestChipShootQ(-9999)
    , bestFlatPassVel(0)
    , bestChipPassVel(0)
    , bestFlatShootVel(0)
    , bestChipShootVel(0)
    , leader(1)
    , receiver(2)
    , pVision(nullptr) {
    ZSS::ZParamManager::instance()->loadParam(SELF_PASS, "Messi/SelfPass", true);
    ZSS::ZParamManager::instance()->loadParam(show_pass_points, "Alert/ShowPassPoints", false);
    ZSS::ZParamManager::instance()->loadParam(show_pass_lines, "Alert/ShowPassLines", false);
    ZSS::ZParamManager::instance()->loadParam(show_run_pos, "Alert/ShowRunPos", false);
    ZSS::ZParamManager::instance()->loadParam(show_heatmap, "HeatMap/Show", false);
    ZSS::ZParamManager::instance()->loadParam(show_other_pos_score, "HeatMap/ShowOtherPos", false);
    ZSS::ZParamManager::instance()->loadParam(show_shoot_pos, "HeatMap/ShowShootPos", true);
    ZSS::ZParamManager::instance()->loadParam(show_free_kick_pos, "HeatMap/ShowFreeKickPos", true);
    ZSS::ZParamManager::instance()->loadParam(showWeightNum, "HeatMap/ShowWeightNum", 0);
    ZSS::ZParamManager::instance()->loadParam(show_result_info, "CUDA/ShowResultInfo", false);
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION,"Alert/IsSimulation",false);
    ZSS::ZParamManager::instance()->loadParam(FREE_KICK_DEBUG, "FreeKick/DEBUG", false);
    ZSS::ZParamManager::instance()->loadParam(IGNORE_THEIR_GUARD, "Messi/IGNORE_THEIR_GUARD", true);
    ZSS::ZParamManager::instance()->loadParam(PASS_ANGLE_ERROR, "Messi/PASS_ANGLE_ERROR", 4);
    ZSS::ZParamManager::instance()->loadParam(FREEKICK_RANGE, "FreeKick/range", 1000);
    ZSS::ZParamManager::instance()->loadParam(FREE_KICK_PARAM::wAngle2Goal, "FreeKick/Angle2Goal", 0.0);
    ZSS::ZParamManager::instance()->loadParam(FREE_KICK_PARAM::min2PenaltyDist, "FreeKick/ToPenaltyDist", 20);
    auto cudastatus = cudaMallocManaged((void**)&players, 2 * PARAM::Field::MAX_PLAYER * sizeof(Player));
    cudaMallocManaged((void**)&ball, sizeof(Point));
    cudaMallocManaged((void**)&rollingFraction, sizeof(float));
    cudaMallocManaged((void**)&slidingFraction, sizeof(float));
    if(cudastatus != cudaSuccess){
        qDebug() <<  "CUDA ERROR: " <<cudaGetErrorString(cudastatus);
    }
    if(IS_SIMULATION)
        ZSS::ZParamManager::instance()->loadParam(ROLLING_FRICTION, "AlertParam/Friction4Sim", 400.0);
    else
        ZSS::ZParamManager::instance()->loadParam(ROLLING_FRICTION, "AlertParam/Friction4Real", 400.0);
    SLIDING_FRICTION = ROLLING_FRICTION * 15;
    *rollingFraction = static_cast<float>(ROLLING_FRICTION);
    *slidingFraction = static_cast<float>(SLIDING_FRICTION);

    passPosThread = new std::thread([ = ] {run();});
    passPosThread->detach();
    scoreThread = new std::thread([ = ] { drawScore();});
    scoreThread->detach();
    freeKickThread = new std::thread([ = ] { calculateFreeKickPos();});
    freeKickThread->detach();
}

CUDAModule::~CUDAModule() {
    cudaFree(players);
    cudaFree(ball);
}

void CUDAModule::initialize(const CVisionModule *pVision) {
    // 更新图像信息
    this->pVision = pVision;
}

ZSS_THREAD_FUNCTION void CUDAModule::run() {
    while(true) {
        messi_to_cuda.Wait();
        ZCUDAModule::instance()->calculateBestPass();
    }
}

ZSS_THREAD_FUNCTION void CUDAModule::drawScore() {
    while(true) {
        vision_to_cuda.Wait();
        if(show_heatmap){
            for (auto x = -PARAM::Field::PITCH_LENGTH/2; x < PARAM::Field::PITCH_LENGTH/2; x+=100) {
                for (auto y = PARAM::Field::PITCH_WIDTH/2; y > -PARAM::Field::PITCH_WIDTH/2; y-=100) {
                    std::vector<float> scores;
                    if (show_other_pos_score) {
                        scores = RunPosModule::Instance()->evaluateFunc(pVision,CGeoPoint(x,y));
                    } else {
                        CGeoPoint candidate(x, y);
                        if(show_shoot_pos)
                            scores = evaluateFunc(candidate, SHOOT);
                        else if (show_free_kick_pos)
                            scores = evaluateFunc(candidate, FREE_KICK);
                        else
                            scores = evaluateFunc(candidate, PASS);
                    }
                    if(showWeightNum < 0 || showWeightNum > static_cast<int>(scores.size())-1)
                        showWeightNum = 0;
                    float Q = scores.at(showWeightNum);
                    DrawScore::instance()->storePoint(x, y, Q);
                }
            }
            DrawScore::instance()->sendPackages();
        }
    }
}

// 重置传球点
void CUDAModule::reset() {
    bestFlatPassPos = CGeoPoint(9999, 9999);
    bestChipPassPos = CGeoPoint(9999, 9999);
    bestFlatShootPos = CGeoPoint(9999, 9999);
    bestChipShootPos = CGeoPoint(9999, 9999);
    bestFlatPassQ = -9999;
    bestChipPassQ = -9999;
    bestFlatShootQ = -9999;
    bestChipShootQ = -9999;
    passPoints.clear();
    chipPoints.clear();
}

void CUDAModule::calculateBestPass() {
    static rType result[ROBOT_NUM * VEL_NUM * DIR_NUM * 2];
    reset();

    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        float x =  this->pVision->ourPlayer(i ).Pos().x();
        players[i].Pos.x = x;
        players[i].Pos.y = this->pVision->ourPlayer(i ).Pos().y();
        players[i].Vel.x = this->pVision->ourPlayer(i ).VelX();
        players[i].Vel.y = this->pVision->ourPlayer(i ).VelY();
        if(receiver < 0)
            players[i].isValid = this->pVision->ourPlayer(i ).Valid();
        else
            players[i].isValid = (i == receiver) ? this->pVision->ourPlayer(i ).Valid() : false;
        //判断禁区和后卫
        if(players[i].isValid && Utils::InOurPenaltyArea(CGeoPoint(players[i].Pos.x, players[i].Pos.y), 50))
            players[i].isValid = false;
    }
    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        players[i + PARAM::Field::MAX_PLAYER].Pos.x = this->pVision->theirPlayer(i ).Pos().x();
        players[i + PARAM::Field::MAX_PLAYER].Pos.y = this->pVision->theirPlayer(i ).Pos().y();
        players[i + PARAM::Field::MAX_PLAYER].Vel.x = this->pVision->theirPlayer(i ).VelX();
        players[i + PARAM::Field::MAX_PLAYER].Vel.y = this->pVision->theirPlayer(i ).VelY();
        players[i + PARAM::Field::MAX_PLAYER].isValid = this->pVision->theirPlayer(i ).Valid();
        //判断禁区和后卫
//        if(players[i + PARAM::Field::MAX_PLAYER].isValid && Utils::InTheirPenaltyArea(CGeoPoint(players[i + PARAM::Field::MAX_PLAYER].Pos.x, players[i + PARAM::Field::MAX_PLAYER].Pos.y), IGNORE_THEIR_GUARD ? 0 : 30))
//            players[i + PARAM::Field::MAX_PLAYER].isValid = false;
    }
    if (!SELF_PASS) players[leader].isValid = false;

    ball->x = leaderPos.x();
    ball->y = leaderPos.y();

    BestPass(players, ball, result, rollingFraction, slidingFraction);

    // Flat
    for(int i = 0; i < ROBOT_NUM * VEL_NUM * DIR_NUM; i++) {
        // 考虑传球误差
        int left = i - ROBOT_NUM * PASS_ANGLE_ERROR;
        int right = i + ROBOT_NUM * PASS_ANGLE_ERROR;
        left = left < 0 ? left + ROBOT_NUM * VEL_NUM * DIR_NUM : left;
        right = right >= ROBOT_NUM * VEL_NUM * DIR_NUM ? right - ROBOT_NUM * VEL_NUM * DIR_NUM : right;
        if(result[i].interTime < 10 && result[i].interTime > 0 /*&& result[left].interTime < 10 && result[left].interTime > 0 && result[right].interTime < 10 && result[right].interTime > 0*/) {
            CGeoPoint candidate(result[i].interPos.x, result[i].interPos.y), p1(PARAM::Field::PITCH_LENGTH / 2, -PARAM::Field::GOAL_WIDTH / 2), p2(PARAM::Field::PITCH_LENGTH / 2, PARAM::Field::GOAL_WIDTH / 2);
            if(!Utils::isValidFlatPass(pVision, leaderPos, candidate, false, true)) continue;
            // 互传
            if(result[i].playerIndex != leader){
                if(candidate.dist(leaderPos) < 2000) continue;
                // 避免往后场传球
                if(leaderPos.x() < 3000 && candidate.x() < min(leaderPos.x() + 3000, 3000.0)) continue;
                if(leaderPos.x() > 3000 && result[i].interPos.x < 3000) continue;
                // 避免往进攻威胁度低的区域传
                if(leaderPos.x() > PARAM::Field::PITCH_LENGTH/4 && fabs(candidate.y()) > PARAM::Field::PITCH_WIDTH/3) continue;
            }
            // 自传
            else {
                if(candidate.dist(leaderPos) > 2000) continue;
                if(!Utils::IsInField(candidate, 500)) continue;
                if(leaderPos.x() < 2000 && candidate.x() < leaderPos.x()) continue;
            }
            passPoints.push_back(result[i]);
            // 更新最佳平射传球点
            std::vector<float> passScores = evaluateFunc(candidate, PASS);
            float passScore = passScores.front();
            if (passScore > bestFlatPassQ) {
                bestFlatPassQ = passScore;
                bestFlatPassPos = candidate;
                bestFlatPassNum = result[i].playerIndex;
                bestFlatPassVel = result[i].Vel;
            }
            // 更新最佳平射射门点
            std::vector<float> shootScores = evaluateFunc(candidate, SHOOT);
            float shootScore = shootScores.front();
            if(shootScore > bestFlatShootQ){
                bestFlatShootQ = shootScore;
                bestFlatShootPos = candidate;
                bestFlatShootNum = result[i].playerIndex;
                bestFlatShootVel = result[i].Vel;
            }
        }
    }
    // Chip
    for(int i = ROBOT_NUM * VEL_NUM * DIR_NUM; i < ROBOT_NUM * VEL_NUM * DIR_NUM * 2; i++) {
        // 考虑传球误差
        int left = i - ROBOT_NUM * PASS_ANGLE_ERROR;
        int right = i + ROBOT_NUM * PASS_ANGLE_ERROR;
        left = left < ROBOT_NUM * VEL_NUM * DIR_NUM ? left + ROBOT_NUM * VEL_NUM * DIR_NUM : left;
        right = right >= ROBOT_NUM * VEL_NUM * DIR_NUM * 2 ? right - ROBOT_NUM * VEL_NUM * DIR_NUM : right;
        if(result[i].interTime < 10 && result[i].interTime > 0 && result[left].interTime < 10 && result[left].interTime > 0 && result[right].interTime < 10 && result[right].interTime > 0) {
            CGeoPoint candidate(result[i].interPos.x, result[i].interPos.y), p1(PARAM::Field::PITCH_LENGTH / 2, -PARAM::Field::GOAL_WIDTH / 2), p2(PARAM::Field::PITCH_LENGTH / 2, PARAM::Field::GOAL_WIDTH / 2);
            if(!Utils::isValidChipPass(pVision, leaderPos, candidate)) continue;
            if(result[i].playerIndex != leader && candidate.dist(leaderPos) < 3000) continue;
            if(result[i].playerIndex == leader) continue;
            if(!Utils::IsInField(candidate, 200)) continue;
//            if(p0.dist(leaderPos) > 600) continue;
            // 避免往后场传球
            if(leaderPos.x() < 3000 && candidate.x() < min(leaderPos.x() + 3000, 3000.0)) continue;
            if(leaderPos.x() > 3000 && result[i].interPos.x < 3000) continue;
            // 避免往进攻威胁度低的区域传
            if(leaderPos.x() > PARAM::Field::PITCH_LENGTH/4 && fabs(candidate.y()) > PARAM::Field::PITCH_WIDTH/3) continue;
            chipPoints.push_back(result[i]);
            // 更新最佳挑射传球点
            std::vector<float> passScores = evaluateFunc(candidate, PASS);
            float passScore = passScores.front();
            if (passScore > bestChipPassQ) {
                bestChipPassQ = passScore;
                bestChipPassPos = candidate;
                bestChipPassNum = result[i].playerIndex;
                bestChipPassVel = result[i].Vel;
            }
            // 更新最佳挑射射门点
            std::vector<float> shootScores = evaluateFunc(candidate, SHOOT);
            float shootScore = shootScores.front();
            if(shootScore > bestChipShootQ){
                bestChipShootQ = shootScore;
                bestChipShootPos = candidate;
                bestChipShootNum = result[i].playerIndex;
                bestChipShootVel = result[i].Vel;
            }
        }
    }
    // DEBUG INFO
    if(show_result_info){
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, 20), QString("Flat-Pass Q: %1  Player: %2  Vel: %3").arg(bestFlatPassQ).arg(bestFlatPassNum).arg(bestFlatPassVel).toLatin1());
        GDebugEngine::Instance()->gui_debug_line(leaderPos, bestFlatPassPos, COLOR_GREEN);
        GDebugEngine::Instance()->gui_debug_msg(bestFlatPassPos, QString("Pass").toLatin1(), COLOR_BLUE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, 40), QString("Chip-Pass Q: %1  Player: %2  Vel: %3").arg(bestChipPassQ).arg(bestChipPassNum).arg(bestChipPassVel).toLatin1());
        GDebugEngine::Instance()->gui_debug_line(leaderPos, bestChipPassPos, COLOR_YELLOW);
        GDebugEngine::Instance()->gui_debug_msg(bestChipPassPos, QString("Pass").toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, 60), QString("Flat-Shoot Q: %1  Player: %2  Vel: %3").arg(bestFlatShootQ).arg(bestFlatShootNum).arg(bestFlatShootVel).toLatin1());
        GDebugEngine::Instance()->gui_debug_line(leaderPos, bestFlatShootPos, COLOR_GREEN);
        GDebugEngine::Instance()->gui_debug_msg(bestFlatShootPos, QString("Shoot").toLatin1(), COLOR_BLUE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, 80), QString("Chip-Shoot Q: %1  Player: %2  Vel: %3").arg(bestChipShootQ).arg(bestChipShootNum).arg(bestChipShootVel).toLatin1());
        GDebugEngine::Instance()->gui_debug_line(leaderPos, bestChipShootPos, COLOR_YELLOW);
        GDebugEngine::Instance()->gui_debug_msg(bestChipShootPos, QString("Shoot").toLatin1(), COLOR_ORANGE);
    }
    if (show_pass_lines) {
        if(Utils::IsInField(bestFlatPassPos)){
            GDebugEngine::Instance()->gui_debug_x(bestFlatPassPos, COLOR_GREEN);
            GDebugEngine::Instance()->gui_debug_line(leaderPos, bestFlatPassPos, COLOR_GREEN);
            GDebugEngine::Instance()->gui_debug_line(bestFlatPassPos, CGeoPoint(PARAM::Field::PITCH_LENGTH / 2, 0), COLOR_GREEN);
        }
        if(Utils::IsInField(bestChipPassPos)){
            GDebugEngine::Instance()->gui_debug_x(bestChipPassPos, COLOR_YELLOW);
            GDebugEngine::Instance()->gui_debug_line(leaderPos, bestChipPassPos, COLOR_YELLOW);
            GDebugEngine::Instance()->gui_debug_line(bestChipPassPos, CGeoPoint(PARAM::Field::PITCH_LENGTH / 2, 0), COLOR_YELLOW);
        }
    }
    if (show_pass_points) {
        std::vector<CGeoPoint> points;
        for(int i = 0; i < passPoints.size(); i += 10) {
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(passPoints[i].interPos.x, passPoints[i].interPos.y), QString("%1").arg(passPoints[i].playerIndex).toLatin1(), COLOR_BLUE);
            points.push_back(CGeoPoint(passPoints[i].interPos.x, passPoints[i].interPos.y));
//            GDebugEngine::Instance()->gui_debug_line(CGeoPoint(passPoints[i].interPos.x, passPoints[i].interPos.y), ballPos, COLOR_GRAY);
//            GDebugEngine::Instance()->gui_debug_line(CGeoPoint(passPoints[i].interPos.x, passPoints[i].interPos.y), pVision->ourplayer(passPoints[i].playerIndex ).Pos(), COLOR_GRAY);
//            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(passPoints[i].interPos.x, passPoints[i].interPos.y),QString("%1 %2 %3 %4").arg(passPoints[i].Vel).arg(passPoints[i].dir).arg(passPoints[i].interTime).arg(passPoints[i].deltaTime).toLatin1(), COLOR_RED);
        }
        GDebugEngine::Instance()->gui_debug_points(points, COLOR_CYAN);
        points.clear();
        for(int i = 0; i < chipPoints.size(); i += 20) {
            points.push_back(CGeoPoint(chipPoints[i].interPos.x, chipPoints[i].interPos.y));
//            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(chipPoints[i].interPos.x, chipPoints[i].interPos.y), QString("%1").arg(chipPoints[i].playerIndex).toLatin1(), COLOR_BLUE);
//            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(chipPoints[i].interPos.x, chipPoints[i].interPos.y),QString("%1 , %2").arg(chipPoints[i].Vel).arg(chipPoints[i].dir).toLatin1(), COLOR_BLUE);
//            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(chipPoints[i].interPos.x, chipPoints[i].interPos.y), QString("%1").arg(chipPoints[i].interTime).toLatin1(), COLOR_GREEN);
        }
        GDebugEngine::Instance()->gui_debug_points(points, COLOR_ORANGE);
    }
}

// 计算最佳定位球的点
void CUDAModule::calculateFreeKickPos(){
    // 跟球门的距离
    static const int MIN_DIST_TO_GOAL = 800;
    static const int MAX_DIST_TO_GOAL = MIN_DIST_TO_GOAL + FREEKICK_RANGE;
    // 跟球门的夹角
    static const int MAX_ANGLE_TO_GOAL = 60;
    // 距离、角度的最小单位
    static const int DIST_UNIT = 100;
    static const int ANGLE_UNIT = 3;
    // 挑球距离
    static const int MAX_CHIP_DIST = 6000;

    while (true) {
        // 根据不同开球位置设置不同的起始点
        CGeoPoint GOAL;
        double freekickPointBuf = 0;
        CGeoPoint kickStartPos;
        if (VisionModule::Instance()->getCurrentRefereeMsg() == "OurBallPlacement") {
            kickStartPos.fill(RefereeBoxInterface::Instance()->getBallPlacementPosX(), RefereeBoxInterface::Instance()->getBallPlacementPosY());
        } else {
            kickStartPos = vision->ball().Pos();
        }
        GOAL.fill((PARAM::Field::PITCH_LENGTH / 2 - kickStartPos.x()) < MAX_CHIP_DIST ? PARAM::Field::PITCH_LENGTH / 2 : kickStartPos.x() + MAX_CHIP_DIST, 0);
        freekickPointBuf = (PARAM::Field::PITCH_WIDTH / 2 - std::fabs(kickStartPos.y())) / 3;
        // 生成候选点
        std::vector<CGeoPoint> candidates;
        for (int dist=MIN_DIST_TO_GOAL; dist<=MAX_DIST_TO_GOAL; dist+=DIST_UNIT) {
            for (int angle=-MAX_ANGLE_TO_GOAL; angle<=MAX_ANGLE_TO_GOAL; angle+=ANGLE_UNIT) {
                CGeoPoint candidate = GOAL + Utils::Polar2Vector(dist, (angle + 180)*PARAM::Math::PI/180);
                candidate = candidate + Utils::Polar2Vector(freekickPointBuf, PARAM::Math::PI / 2 * ((candidate.y() > 0) ? 1 : -1));
                if (Utils::InTheirPenaltyArea(candidate, FREE_KICK_PARAM::min2PenaltyDist))
                    continue;
                if (candidate.dist(vision->ball().Pos()) < PARAM::Field::PITCH_LENGTH / 4) //防止离球过近
                    continue;
                if (std::fabs(kickStartPos.y()) > PARAM::Field::PENALTY_AREA_WIDTH / 2 && candidate.y() * kickStartPos.y() > 0)
                    continue;
                if (std::fabs(kickStartPos.y()) < PARAM::Field::PENALTY_AREA_WIDTH / 2 && std::fabs(candidate.y()) < PARAM::Field::PENALTY_AREA_WIDTH / 2)
                    continue;
                candidates.push_back(candidate);
            }
        }
        vision_to_cuda.Wait();
        float bestScore = -9999;
        for (auto candidate : candidates) {
            if(FREE_KICK_DEBUG) GDebugEngine::Instance()->gui_debug_x(candidate);
            std::vector<float> shootScores = evaluateFunc(candidate, FREE_KICK);
            float score = shootScores.front();
            if(score > bestScore){
                bestScore = score;
                bestFreeKickPos = candidate;
            }
        }
        if(FREE_KICK_DEBUG) GDebugEngine::Instance()->gui_debug_msg(bestFreeKickPos, QString("F").toLatin1(), COLOR_ORANGE);
        if(FREE_KICK_DEBUG) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,0), QString("BUFDIST: %1").arg(freekickPointBuf).toLatin1(), COLOR_ORANGE);
    }
}

//bool CUDAModule::canRunDirect(const CGeoPoint start, const CGeoPoint end, float avoidDist){
//    CGeoSegment seg(start, end);
//    bool canRun = true;
//    for (int i=0; i<PARAM::Field::MAX_PLAYER; i++) {
//        if(pVision->theirPlayer(i).Valid()){
//            CGeoPoint enemyPos = pVision->theirPlayer(i).Pos();
//            if(seg.dist2Point(enemyPos) < avoidDist){
//                canRun = false;
//                break;
//            }
//        }
//    }
//    return canRun;
//}

//void CUDAModule::calculateBestPosition() {
//    static Point bestPosition[16];
//    //由于这个函数只需要我方接球车的信息，所以直接把前12个车位给对方车
//    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
//        players[i].Pos.x = this->pVision->theirplayer(i ).Pos().x();
//        players[i].Pos.y = this->pVision->theirplayer(i ).Pos().y();
//        players[i].Vel.x = this->pVision->theirplayer(i ).VelX();
//        players[i].Vel.y = this->pVision->theirplayer(i ).VelY();
//        players[i].isValid = this->pVision->theirplayer(i ).Valid();
//    }
//    //把下标为12的player设置为
//    players[PARAM::Field::MAX_PLAYER].Pos.x = this->pVision->ourPlayer(this->receiver).Pos().x();
//    players[PARAM::Field::MAX_PLAYER].Pos.y = this->pVision->ourPlayer(this->receiver).Pos().y();
//    players[PARAM::Field::MAX_PLAYER].Vel.x = this->pVision->ourPlayer(this->receiver).VelX();
//    players[PARAM::Field::MAX_PLAYER].Vel.y = this->pVision->ourPlayer(this->receiver).VelY();
//    players[PARAM::Field::MAX_PLAYER].isValid = this->pVision->ourPlayer(this->receiver).Valid();
//    ball->x = this->pVision->Ball().Pos().x();
//    ball->y = this->pVision->Ball().Pos().y();

//    PosScore(players, ball, bestPosition);
//    for(int i = 0; i < 16; i++) {
//        this->bestScorePosition[i] = CGeoPoint(bestPosition[i].x, bestPosition[i].y);
//        if(show_run_pos)
//            GDebugEngine::Instance()->gui_debug_x(this->bestScorePosition[i], COLOR_GREEN);
//    }
//    for(int i = 1; i <= 16; i++) {
//        if(show_run_pos)
//            GDebugEngine::Instance()->gui_debug_msg(getBestPosition(i), QString::number(i).toLatin1(), COLOR_RED);
//    }
//}

//CGeoPoint CUDAModule::getBestPosition(int i) {
//    i = i < 1 ? 1 : i;
//    i = i > 16 ? 16 : i;
//    i -= 1;
//    int block_Y = i % 4;
//    int block_X = 3 - i / 4;
//    float Unit_Y = PARAM::Field::PITCH_WIDTH / 4;
//    float Unit_X = PARAM::Field::PITCH_LENGTH / 4;
//    float Left_x = block_X * Unit_X - PARAM::Field::PITCH_LENGTH / 2;
//    float Right_x = (block_X + 1) * Unit_X - PARAM::Field::PITCH_LENGTH / 2;
//    float Top_y = (block_Y + 1) * Unit_Y - PARAM::Field::PITCH_WIDTH / 2;
//    float Down_y = block_Y * Unit_Y - PARAM::Field::PITCH_WIDTH / 2;

////    if(show_run_pos){
////        CGeoPoint p1(Left_x, Top_y);
////        CGeoPoint p2(Left_x, Down_y);
////        CGeoPoint p3(Right_x, Top_y);
////        CGeoPoint p4(Right_x, Down_y);
////        GDebugEngine::Instance()->gui_debug_line(p1, p2);
////        GDebugEngine::Instance()->gui_debug_line(p1, p3);
////        GDebugEngine::Instance()->gui_debug_line(p4, p2);
////        GDebugEngine::Instance()->gui_debug_line(p3, p4);
////    }

////    std::cout << Left_x << " " << Right_x << " " << Down_y << " " << Top_y << std::endl;
//    for(int j = 0; j < 16; j++) {
//        if(this->bestScorePosition[j].x() >= Left_x && this->bestScorePosition[j].x() <= Right_x && this->bestScorePosition[j].y() <= Top_y && this->bestScorePosition[j].y() >= Down_y) {
//            return this->bestScorePosition[j];
//        }
//    }
//    return CGeoPoint(0,0);
//}

//CGeoPoint CUDAModule::getBestRunPos(int areaNum) {
//    areaNum = areaNum < 1 ? 1 : areaNum;
//    areaNum = areaNum > 16 ? 16 : areaNum;
//    areaNum -= 1;
//    int block_Y = areaNum % 4;
//    int block_X = 3 - areaNum / 4;
//    float Unit_Y = PARAM::Field::PITCH_WIDTH / 4;
//    float Unit_X = PARAM::Field::PITCH_LENGTH / 4;
//    float Left_x = block_X * Unit_X - PARAM::Field::PITCH_LENGTH / 2;
//    float Right_x = (block_X + 1) * Unit_X - PARAM::Field::PITCH_LENGTH / 2;
//    float Top_y = (block_Y + 1) * Unit_Y - PARAM::Field::PITCH_WIDTH / 2;
//    float Down_y = block_Y * Unit_Y - PARAM::Field::PITCH_WIDTH / 2;
//    float buffer = 20;

//    int enemyNum = 0;
//    if (enemyNum < WorldModel::Instance()->getEnemyAmountInArea(Left_x, Right_x, Top_y, Down_y, -buffer)) {
//        enemyNum = WorldModel::Instance()->getEnemyAmountInArea(Left_x, Right_x, Top_y, Down_y, -buffer);
//    } else if (enemyNum > WorldModel::Instance()->getEnemyAmountInArea(Left_x, Right_x, Top_y, Down_y, buffer)) {
//        enemyNum = WorldModel::Instance()->getEnemyAmountInArea(Left_x, Right_x, Top_y, Down_y, buffer);
//    }
//    if (enemyNum > 1) enemyNum = 2;
//    switch (enemyNum) {
//    case 1:

//        break;
//    case 2:

//        break;
//    default:

//        break;
//    }


//}

// 評估函數：根據評估模式計算score，提高代碼復用
std::vector<float> CUDAModule::evaluateFunc(CGeoPoint candidate, EvaluateMode mode) {
    std::vector<float> scores;
    CGeoPoint p0(candidate), p1(PARAM::Field::PITCH_LENGTH / 2, -PARAM::Field::GOAL_WIDTH / 2), p2(PARAM::Field::PITCH_LENGTH / 2, PARAM::Field::GOAL_WIDTH / 2);

    // 計算有效射門角度
    float shootAngle = 0;
    CShootRangeList shootRangeList(pVision, 1, p0);
    const CValueRangeList& shootRange = shootRangeList.getShootRange();
    if (shootRange.size() > 0) {
        auto bestRange = shootRange.getMaxRangeWidth();
        if(bestRange) {
            shootAngle = bestRange->getWidth();
        }
    }
    // 計算折射角,折射角越小越适合touch，touch角度要求小于60度
    CVector v1 = leaderPos - p0;
    CVector v2 = p1.midPoint(p2) - p0;
    float refracAngle = fabs(v1.dir() - v2.dir());
    refracAngle = refracAngle > PARAM::Math::PI ? 2*PARAM::Math::PI - refracAngle : refracAngle;
    // 計算離球門的距離
    float defDist = fabs(p0.dist(CGeoPoint(PARAM::Field::PITCH_LENGTH / 2, 0)));
    float Angle2Goal = fabs(fabs(Utils::Normalize(v2.dir())) - PARAM::Math::PI / 4);
    float Angle2Goal4FreeKick = (fabs(Utils::Normalize(v2.dir())) < PARAM::Math::PI / 4.0f && fabs(Utils::Normalize(v2.dir())) > PARAM::Math::PI / 9.0f) ? 1 : 0;
    // 計算敵方到傳球線的距離
    float passLineDist = 9999;
    CGeoSegment BallLine(vision->ball().Pos(), p0);
    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        if(!pVision->theirPlayer(i ).Valid())
            continue;
        CGeoPoint targetPos = pVision->theirPlayer(i ).Pos();
        float dist = std::min(BallLine.dist2Point(targetPos), 1000.0);
        if(dist < passLineDist)
            passLineDist = dist;
    }
    // 计算接球转身传球需要的角度
    CGeoPoint ballPos = pVision->ball().Pos();
    CVector leaderToBall = ballPos - pVision->ourPlayer(leader).Pos();
    CVector leaderToPassPos = p0 - pVision->ourPlayer(leader).Pos();
    float turnAngle = fabs(leaderToBall.dir() - leaderToPassPos.dir());
    turnAngle = turnAngle > PARAM::Math::PI ? 2*PARAM::Math::PI - turnAngle : turnAngle;
    if(ballPos.x() < pVision->ourPlayer(leader).X()) turnAngle = PARAM::Math::PI;
    // 安全性 计算最近的敌方车的距离
    float closestEnemyDist = 9999;
    for (int i=0; i < PARAM::Field::MAX_PLAYER; i++) {
        if(!pVision->theirPlayer(i).Valid())
            continue;
        if(Utils::InTheirPenaltyArea(pVision->theirPlayer(i).Pos(), 0))
            continue;
        if(pVision->theirPlayer(i).Pos().dist(p0) < closestEnemyDist)
            closestEnemyDist = pVision->theirPlayer(i).Pos().dist(p0);
    }
    // 下底传中
    float underPass = (p0.x() < PARAM::Field::PITCH_LENGTH/2 && p0.x() > PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH) ? 1 : 0;
    //预测后卫
    float guardMinTime = 9999;
//    auto& ballp = pVision->Ball().Pos();
    CGeoSegment shootLine1(p0, p1), shootLine2(p0, p2);
    CGeoPoint p = WorldModel::Instance()->penaltyIntersection(shootLine1),
              q = WorldModel::Instance()->penaltyIntersection(shootLine2);
    WorldModel::Instance()->normalizeCoordinate(p);
    WorldModel::Instance()->normalizeCoordinate(q);
//    GDebugEngine::Instance()->gui_debug_msg(p, "IP",COLOR_RED);
//    GDebugEngine::Instance()->gui_debug_msg(q,"IQ", COLOR_RED);
    for(int i = 1; i < PARAM::Field::MAX_PLAYER; i++) {
        const PlayerVisionT& enemy = pVision->theirPlayer(i);
        if (!Utils::InTheirPenaltyArea(enemy.Pos(), 0) && Utils::InTheirPenaltyArea(enemy.Pos(), 50)) {
            float pTime = WorldModel::Instance()->preditTheirGuard(enemy, p);
            float qTime = WorldModel::Instance()->preditTheirGuard(enemy, q);
            if (pTime < guardMinTime) {
                guardMinTime = pTime;
            }
            if (qTime < guardMinTime) {
                guardMinTime = qTime;
            }
        }
    }

    float sectorScore;
    float candiDir = RUNPOS_PARAM::maxSectorDir;
    float candiDist = RUNPOS_PARAM::maxSectorDist;
//    float candiDir = (candidate - ball.Pos()).dir();
//    float candiDist = candidate.dist(ball.Pos());
    for (int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        if(pVision->theirPlayer(i).Valid()) {
            const PlayerVisionT& enemy = pVision->theirPlayer(i);
            float baseDir = (enemy.Pos() - vision->ball().Pos()).dir();
            float baseDist = enemy.Pos().dist(vision->ball().Pos());
//            if (fabs(Utils::Normalize(candiDir - baseDir)) < RUNPOS_PARAM::maxSectorDir && candiDist > baseDist) {
//                candiDir = fabs(Utils::Normalize(candiDir - baseDir));
//                candiDist = (candiDist - baseDist) > RUNPOS_PARAM::maxSectorDist ? RUNPOS_PARAM::maxSectorDist : candiDist - baseDist;
//            }
            if (fabs(Utils::Normalize((candidate - vision->ball().Pos()).dir() - baseDir)) < RUNPOS_PARAM::maxSectorDir && candidate.dist(vision->ball().Pos()) > baseDist) {
                candiDir = std::min(fabs(Utils::Normalize((candidate - vision->ball().Pos()).dir() - baseDir)), double(candiDir));
                candiDist = std::min(candidate.dist(vision->ball().Pos()), double(candiDist));
            }
        }
    }

    // 归一化
    shootAngle = shootAngle > 25 ? 1 : shootAngle/25;
    refracAngle = refracAngle < PARAM::Math::PI/6 ? PARAM::Math::PI/6 : refracAngle;
    refracAngle = defDist < PARAM::Field::PITCH_LENGTH/2 ? 1 - refracAngle/PARAM::Math::PI : 0;  // 折射角在一定范围内给固定奖励
    defDist = 1 - defDist/PARAM::Field::PITCH_LENGTH;
    Angle2Goal = 1 - Angle2Goal/PARAM::Math::PI;
    passLineDist = passLineDist > 300 ? 1 : passLineDist / 300;
    turnAngle = turnAngle < PARAM::Math::PI/12 ? PARAM::Math::PI/12 : turnAngle;
    turnAngle = 1 - turnAngle/PARAM::Math::PI;
    closestEnemyDist = closestEnemyDist > 150 ? 1 : (closestEnemyDist < 50 ? 0 : (closestEnemyDist - 50)/100);
    guardMinTime = guardMinTime > 999 ? 0 : guardMinTime > 2.0 ? 2.0 : guardMinTime / 2.0;
    sectorScore = candiDir / RUNPOS_PARAM::maxSectorDir * 0.3f + candiDist / RUNPOS_PARAM::maxSectorDist * 0.7f;
    float totalWeight = 0;
    switch (mode) {
    case PASS:
        totalWeight = PASS_PARAM::wDist + PASS_PARAM::wPassLineDist
                + PASS_PARAM::wTurnAngle + PASS_PARAM::wClosestEnemyDist + PASS_PARAM::wUnderPass * underPass;
        scores.push_back(1/totalWeight * 100 * (PASS_PARAM::wDist * defDist
                         + PASS_PARAM::wPassLineDist * passLineDist
                         + PASS_PARAM::wTurnAngle * turnAngle
                         + PASS_PARAM::wClosestEnemyDist * closestEnemyDist
                         + PASS_PARAM::wUnderPass * underPass));
        scores.push_back(1/totalWeight * 100 * PASS_PARAM::wDist * defDist);
        scores.push_back(1/totalWeight * 100 * PASS_PARAM::wPassLineDist * passLineDist);
        scores.push_back(1/totalWeight * 100 * PASS_PARAM::wTurnAngle * turnAngle);
        scores.push_back(1/totalWeight * 100 * PASS_PARAM::wClosestEnemyDist * closestEnemyDist);
        scores.push_back(1/totalWeight * 100 * PASS_PARAM::wUnderPass * underPass);
        break;
    case SHOOT:
        totalWeight = SHOOT_PARAM::wShootAngle + SHOOT_PARAM::wRefracAngle
                + SHOOT_PARAM::wAngle2Goal + SHOOT_PARAM::wDist + SHOOT_PARAM::wPassLineDist + SHOOT_PARAM::wGuardTime;
        scores.push_back(1/totalWeight * 100 * (SHOOT_PARAM::wShootAngle * shootAngle
                         + SHOOT_PARAM::wRefracAngle * refracAngle
                         + SHOOT_PARAM::wDist * defDist
                         + SHOOT_PARAM::wAngle2Goal * Angle2Goal
                         + SHOOT_PARAM::wPassLineDist * passLineDist
                         + SHOOT_PARAM::wGuardTime * guardMinTime));
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wShootAngle * shootAngle);
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wRefracAngle * refracAngle);
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wDist * defDist);
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wAngle2Goal * Angle2Goal);
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wPassLineDist * passLineDist);
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wGuardTime * guardMinTime);
        break;
    case FREE_KICK:
        totalWeight = FREE_KICK_PARAM::wShootAngle + FREE_KICK_PARAM::wClosestEnemyDist + FREE_KICK_PARAM::wRefracAngle
                + FREE_KICK_PARAM::wAngle2Goal + FREE_KICK_PARAM::wDist + FREE_KICK_PARAM::wGuardTime + FREE_KICK_PARAM::wSector + FREE_KICK_PARAM::wPassLineDist;
        scores.push_back(1/totalWeight * 100 * (FREE_KICK_PARAM::wShootAngle * shootAngle
                         + FREE_KICK_PARAM::wClosestEnemyDist * closestEnemyDist
                         + FREE_KICK_PARAM::wRefracAngle * refracAngle
                         + FREE_KICK_PARAM::wDist * defDist
                         + FREE_KICK_PARAM::wAngle2Goal * Angle2Goal4FreeKick
                         + FREE_KICK_PARAM::wGuardTime * guardMinTime
                         + FREE_KICK_PARAM::wSector * sectorScore
                         + FREE_KICK_PARAM::wPassLineDist * passLineDist));
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wShootAngle * shootAngle);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wClosestEnemyDist * closestEnemyDist);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wRefracAngle * refracAngle);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wDist * defDist);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wAngle2Goal * Angle2Goal4FreeKick);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wGuardTime * guardMinTime);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wSector * sectorScore);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wPassLineDist * passLineDist);
        break;
    }
    return scores;
}

//float CUDAModule::rotateTime(float initDir, float finalDir){
//    static const float ROTATE_ACC = 10;
//    static const float MAX_ROTATE_VEL = 15;
//    float rotateDir = fabs(Utils::Normalize(finalDir - initDir));
//    float time = 9999;
//    if(pow(MAX_ROTATE_VEL, 2)/ROTATE_ACC > rotateDir){
//        float v_max = sqrt(rotateDir * ROTATE_ACC);
//        time = 2*v_max/ROTATE_ACC;
//    }
//    else {
//        float flatDist = rotateDir - pow(MAX_ROTATE_VEL, 2)/ROTATE_ACC;
//        float flatTime = flatDist/MAX_ROTATE_VEL;
//        time = flatTime + 2*MAX_ROTATE_VEL/ROTATE_ACC;
//    }
//    return time;
//}
