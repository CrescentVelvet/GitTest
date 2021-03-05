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
#include "passposevaluate.h"
#define PLAYER_NUM (16)
#define ANGLE_NUM (12)
#define DIST_NUM (8)

extern Semaphore messi_to_cuda;
extern Semaphore vision_to_cuda;

extern "C" {
    extern "C" void BestPass(Player* players, Point* ball, nType* result, int leader, double theirFlatResponseTime, double theirChipResponseTime, bool isSim);
//    void PosScore(Player* players, Point* ballPos, Point* bestPositions);
}

namespace {
bool SELF_PASS;
bool show_heatmap = false;
bool show_shoot_pos = true;
bool show_other_pos_score = false;
bool show_free_kick_pos = false;
bool show_pass_points = true;
bool show_pass_lines = false;
bool show_run_pos = false;
bool show_result_info = false;
int showWeightNum = 0;
bool IS_SIMULATION = true;
bool FREE_KICK_DEBUG = false;
bool IGNORE_THEIR_GUARD = true;
// 摩擦系数
double ROLLING_FRICTION = 40;
double SLIDING_FRICTION = ROLLING_FRICTION * 15;
//传球角度误差, 角度制
double PASS_ANGLE_ERROR = 4;
std::thread* passPosThread = nullptr;
std::thread* scoreThread = nullptr;
std::thread* freeKickThread = nullptr;
double THEIR_CHIP_RESPONSE_TIME = 0.0;
double THEIR_RESPONSE_TIME = 0.0;
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
    ZSS::ZParamManager::instance()->loadParam(THEIR_CHIP_RESPONSE_TIME, "Messi/THEIR_CHIP_RESPONSE_TIME", 0.0);
    ZSS::ZParamManager::instance()->loadParam(THEIR_RESPONSE_TIME, "Messi/THEIR_RESPONSE_TIME", 0.0);
    auto cudastatus = cudaMallocManaged((void**)&players, 2 * PARAM::Field::MAX_PLAYER * sizeof(Player));
    cudaMallocManaged((void**)&ball, sizeof(Point));
    cudaMallocManaged((void**)&rollingFraction, sizeof(float));
    cudaMallocManaged((void**)&slidingFraction, sizeof(float));
    //cudaMallocManaged((void**)&IS_SIMULATION, sizeof(bool));
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

//    passPosThread = new std::thread([ = ] {run();});
//    passPosThread->detach();
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
                            scores = PassPosEvaluate::instance()->evaluateFunc(candidate,leaderPos,SHOOT);
                        else if (show_free_kick_pos)
                            scores = PassPosEvaluate::instance()->evaluateFunc(candidate,leaderPos,FREE_KICK);
                        else
                            scores = PassPosEvaluate::instance()->evaluateFunc(candidate,leaderPos,PASS);
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

    int i,j;
    static nType result[2 * PLAYER_NUM * ANGLE_NUM * DIST_NUM * PLAYER_NUM];
    reset();

    for(i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        players[i].Pos.x = this->pVision->ourPlayer(i).Pos().x();
        players[i].Pos.y = this->pVision->ourPlayer(i).Pos().y();
        players[i].Vel.x = this->pVision->ourPlayer(i).VelX();
        players[i].Vel.y = this->pVision->ourPlayer(i).VelY();
        //printf("(vx,vy):(%f,%f)\n",players[i].Vel.x,players[i].Vel.y);
        players[i].isValid = this->pVision->ourPlayer(i).Valid();
//        if(receiver < 0)
//            players[i].isValid = this->pVision->ourPlayer(i).Valid();
//        else
//            players[i].isValid = (i == receiver) ? this->pVision->ourPlayer(i).Valid() : false;
        //判断禁区和后卫
        if(Utils::InOurPenaltyArea(this->pVision->ourPlayer(i).Pos(), 6*PARAM::Vehicle::V2::PLAYER_SIZE))
            players[i].isValid = false;
    }
    for(i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        players[i + PARAM::Field::MAX_PLAYER].Pos.x = this->pVision->theirPlayer(i).Pos().x();
        players[i + PARAM::Field::MAX_PLAYER].Pos.y = this->pVision->theirPlayer(i).Pos().y();
        players[i + PARAM::Field::MAX_PLAYER].Vel.x = this->pVision->theirPlayer(i).VelX();
        players[i + PARAM::Field::MAX_PLAYER].Vel.y = this->pVision->theirPlayer(i).VelY();
        players[i + PARAM::Field::MAX_PLAYER].isValid = this->pVision->theirPlayer(i).Valid();
        //判断禁区和后卫
        //if(Utils::InTheirPenaltyArea(this->pVision->theirPlayer(i).Pos(), IGNORE_THEIR_GUARD ? 6*PARAM::Vehicle::V2::PLAYER_SIZE : 0))
        //    players[i + PARAM::Field::MAX_PLAYER].isValid = false;
        //printf("%d (%f,%f)\n",i + PARAM::Field::MAX_PLAYER,players[i + PARAM::Field::MAX_PLAYER].Pos.x,players[i + PARAM::Field::MAX_PLAYER].Pos.y);
    }
    if (!SELF_PASS) players[leader].isValid = false;

    ball->x = leaderPos.x();
    ball->y = leaderPos.y();

    //printf("bushibaasir\n");

    BestPass(players, ball, result, leader, THEIR_RESPONSE_TIME, THEIR_CHIP_RESPONSE_TIME, IS_SIMULATION);

    //printf("jiushiaasir\n");

    static bool passValid[PLAYER_NUM * ANGLE_NUM * DIST_NUM][2];

    for(i = 0; i < PLAYER_NUM * ANGLE_NUM * DIST_NUM; ++i){
        passValid[i][0] = passValid[i][1] = true;
        for(j = 0; j < PLAYER_NUM; ++j){
//            if(result[i * PLAYER_NUM + j].playerIndex == 6)
//                printf("%d  ",result[i * PLAYER_NUM + j].isValid);
            if(result[i * PLAYER_NUM + j].isValid == 0)
                passValid[i][0] = 0;
            if(result[PLAYER_NUM * ANGLE_NUM * DIST_NUM * PLAYER_NUM + i * PLAYER_NUM + j].isValid == 0)
                passValid[i][1] = 0;
        }
//        if(result[i * PLAYER_NUM].playerIndex == 6)
//            printf("\nresult:%d\n",passValid[i][0]);
    }

    // Flat
    for(i = 0; i < PLAYER_NUM * ANGLE_NUM * DIST_NUM; i++) {
        //printf("intertime:%f\n", result[i].interTime);
        if(result[i * PLAYER_NUM].interTime < 10 && result[i * PLAYER_NUM].interTime > 0 /*&& result[left].interTime < 10 && result[left].interTime > 0 && result[right].interTime < 10 && result[right].interTime > 0*/) {
            CGeoPoint candidate(result[i * PLAYER_NUM].interPos.x, result[i * PLAYER_NUM].interPos.y), p1(PARAM::Field::PITCH_LENGTH / 2, -PARAM::Field::GOAL_WIDTH / 2), p2(PARAM::Field::PITCH_LENGTH / 2, PARAM::Field::GOAL_WIDTH / 2);
//            if(!Utils::isValidFlatPass(pVision, leaderPos, candidate, false, true)) continue;
//            // 互传
//            if(result[i * PLAYER_NUM].playerIndex != leader){
//                if(!PassPosEvaluate::instance()->validPass(candidate,leaderPos,true)) continue;
//            }
//            // 自传
//            else {
//                if(!(PassPosEvaluate::instance()->validSelfPass(candidate,leaderPos)))continue;
//            }

            if(passValid[i][0] && ((SELF_PASS && result[i * PLAYER_NUM].playerIndex == leader) || PassPosEvaluate::instance()->validPass(candidate, leaderPos, true)))   {
            passPoints.push_back(result[i * PLAYER_NUM]);
//            if(result[i * PLAYER_NUM].playerIndex == 6) printf("catcher:%d\n",result[i * PLAYER_NUM].playerIndex);
//            }
            // 更新最佳平射传球点
            std::vector<float> passScores = PassPosEvaluate::instance()->evaluateFunc(candidate,leaderPos,PASS);
            float passScore = passScores.front();
            if (passScore > bestFlatPassQ) {
                bestFlatPassQ = passScore;
                bestFlatPassPos = candidate;
                bestFlatPassNum = result[i * PLAYER_NUM].playerIndex;
                bestFlatPassVel = result[i * PLAYER_NUM].Vel;
            }
            // 更新最佳平射射门点
            std::vector<float> shootScores = PassPosEvaluate::instance()->evaluateFunc(candidate,leaderPos,SHOOT);
            float shootScore = shootScores.front();
            if(shootScore > bestFlatShootQ){
                bestFlatShootQ = shootScore;
                bestFlatShootPos = candidate;
                bestFlatShootNum = result[i * PLAYER_NUM].playerIndex;
                bestFlatShootVel = result[i * PLAYER_NUM].Vel;
            }
            }
        }
    }
    // Chip
    for(int i = PLAYER_NUM * ANGLE_NUM * DIST_NUM; i < PLAYER_NUM * ANGLE_NUM * DIST_NUM * 2; i++) {
        if(result[i * PLAYER_NUM].interTime < 10 && result[i * PLAYER_NUM].interTime > 0) {
            CGeoPoint candidate(result[i * PLAYER_NUM].interPos.x, result[i * PLAYER_NUM].interPos.y), p1(PARAM::Field::PITCH_LENGTH / 2, -PARAM::Field::GOAL_WIDTH / 2), p2(PARAM::Field::PITCH_LENGTH / 2, PARAM::Field::GOAL_WIDTH / 2);
//            if(!Utils::isValidChipPass(pVision, leaderPos, candidate)) continue;
//            if(result[i * PLAYER_NUM].playerIndex != leader && !PassPosEvaluate::instance()->validPass(candidate,leaderPos,false)) continue;
//            if(result[i * PLAYER_NUM].playerIndex == leader) continue;
//            if(p0.dist(leaderPos) > 600) continue;
            if(passValid[i - PLAYER_NUM * ANGLE_NUM * DIST_NUM][1] && ((SELF_PASS && result[i * PLAYER_NUM].playerIndex == leader) || PassPosEvaluate::instance()->validPass(candidate, leaderPos, false)))   {
            chipPoints.push_back(result[i * PLAYER_NUM]);
            // 更新最佳挑射传球点
            std::vector<float> passScores =  PassPosEvaluate::instance()->evaluateFunc(candidate,leaderPos,PASS);
            float passScore = passScores.front();
            if (passScore > bestChipPassQ) {
                bestChipPassQ = passScore;
                bestChipPassPos = candidate;
                bestChipPassNum = result[i * PLAYER_NUM].playerIndex;
                bestChipPassVel = result[i * PLAYER_NUM].Vel;
            }
            // 更新最佳挑射射门点
            std::vector<float> shootScores =PassPosEvaluate::instance()->evaluateFunc(candidate,leaderPos,SHOOT);
            float shootScore = shootScores.front();
            if(shootScore > bestChipShootQ){
                bestChipShootQ = shootScore;
                bestChipShootPos = candidate;
                bestChipShootNum = result[i * PLAYER_NUM].playerIndex;
                bestChipShootVel = result[i * PLAYER_NUM].Vel;
            }
            }
        }
    }
    // DEBUG INFO
    if(show_result_info){
        //GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, 20), QString("Flat-Pass Q: %1  Player: %2  Vel: %3").arg(bestFlatPassQ).arg(bestFlatPassNum).arg(bestFlatPassVel).toLatin1());
        //GDebugEngine::Instance()->gui_debug_line(leaderPos, bestFlatPassPos, COLOR_GREEN);
        //GDebugEngine::Instance()->gui_debug_msg(bestFlatPassPos, QString("Pass").toLatin1(), COLOR_BLUE);
        //GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, 40), QString("Chip-Pass Q: %1  Player: %2  Vel: %3").arg(bestChipPassQ).arg(bestChipPassNum).arg(bestChipPassVel).toLatin1());
        //GDebugEngine::Instance()->gui_debug_line(leaderPos, bestChipPassPos, COLOR_YELLOW);
        //GDebugEngine::Instance()->gui_debug_msg(bestChipPassPos, QString("Pass").toLatin1(), COLOR_ORANGE);
        //GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, 60), QString("Flat-Shoot Q: %1  Player: %2  Vel: %3").arg(bestFlatShootQ).arg(bestFlatShootNum).arg(bestFlatShootVel).toLatin1());
        GDebugEngine::Instance()->gui_debug_line(leaderPos, bestFlatShootPos, COLOR_GREEN);
        //GDebugEngine::Instance()->gui_debug_msg(bestFlatShootPos, QString("Shoot").toLatin1(), COLOR_BLUE);
        //GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, 80), QString("Chip-Shoot Q: %1  Player: %2  Vel: %3").arg(bestChipShootQ).arg(bestChipShootNum).arg(bestChipShootVel).toLatin1());
        GDebugEngine::Instance()->gui_debug_line(leaderPos, bestChipShootPos, COLOR_YELLOW);
        //GDebugEngine::Instance()->gui_debug_msg(bestChipShootPos, QString("Shoot").toLatin1(), COLOR_ORANGE);
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
//    printf("pass cnt: %d\n", passPoints.size());
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, -3300),QString::number(passPoints.size()).toLatin1(),COLOR_YELLOW);
    if (show_pass_points) {
        std::vector<CGeoPoint> points;
        for(i = 0; i < passPoints.size(); i += 1) {
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

//计算最佳定位球的点
void CUDAModule::calculateFreeKickPos(){
    while (true) {
        std::vector<CGeoPoint> candidates;
        candidates = PassPosEvaluate::instance()->generateFreeKickPoints();
//        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2+200, PARAM::Field::PITCH_WIDTH/2-3000) , QString("dhz debug:candidates size: %1").arg(candidates.size()).toLatin1(), COLOR_ORANGE);

        vision_to_cuda.Wait();
        float bestScore = -9999;
        for (auto candidate : candidates) {
            if(FREE_KICK_DEBUG) GDebugEngine::Instance()->gui_debug_x(candidate);
            std::vector<float> shootScores = PassPosEvaluate::instance()->evaluateFunc(candidate,leaderPos,FREE_KICK);
            float score = shootScores.front();
            if(score > bestScore){
//                GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2+200, PARAM::Field::PITCH_WIDTH/2-3200) , QString("dhz debug:best score: %1").arg(score).toLatin1(), COLOR_ORANGE);
                bestScore = score;
                bestFreeKickPos = candidate;
            }
        }
        if(FREE_KICK_DEBUG) GDebugEngine::Instance()->gui_debug_msg(bestFreeKickPos, QString("F").toLatin1(), COLOR_ORANGE);
    }
}

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
