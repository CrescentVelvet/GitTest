#include "messidecision.h"
#include "receivePos.h"
#include "SkillUtils.h"
#include <QDebug>
#include "GDebugEngine.h"
#include "BallStatus.h"
#include "RobotSensor.h"
#include "parammanager.h"
#include "param.h"
#include "ShootRangeList.h"
#include <iostream>
#include "WorldModel.h"
#include "Semaphore.h"
#include "Global.h"
#include "ballmodel.h"
#include "ShootModule.h"
#include "RefereeBoxIf.h"
#ifdef USE_CUDA_MODULE
#include "CUDAModule/CUDAModule.h"
#include <stdlib.h>
Semaphore messi_to_cuda(0);
#endif

namespace {
bool TEST_PASS;
bool SELF_PASS;
bool MESSI_DEBUG;
bool INTER_DEBUG;
bool SHOW_PASSLINE;
bool ONLY_PASSPOS = false;
bool ONLY_SHOOTPOS = false;
bool ONLY_FLATPOS = false;
bool ONLY_CHIPPOS = false;
bool SIGNAL_TO_CUDA = false;
bool IS_RIGHT = false;
bool IS_SIMULATION = false;
//判断进攻车数量的参数
enum Field {
    FrontField = 1,
    MiddleField = 2,
    BackField = 3
};
// 判断leader拿着球的距离
const int HOLDBALL_DIST = 200;
//避免射门后马上改变leader
const int MIN_CHANGE_LEADER_INTERVEL = 10;
//改变接球车的最小间隔
const int MIN_CHANGE_RECEIVER_INTERVAL = 60;
// 传球失败时的截球时间差
const int WRONG_LEADER_INTERTIME = 2;
// fixBuf的倍数
double FIX_BUF_RATIO = 1;
//射门时其他车的静态跑位点
const CGeoPoint STATIC_POS[2] = {CGeoPoint(Param::Field::PITCH_LENGTH / 4, Param::Field::PITCH_WIDTH / 4),
                                 CGeoPoint(Param::Field::PITCH_LENGTH / 4, -Param::Field::PITCH_WIDTH / 4)};
namespace SHOOT {
double shootAngle = 0;
bool leftShoot = false;
bool rightShoot = false;
bool middleShoot = false;
bool FORCE_SHOOT = true;
}
namespace PASS {
bool passMode = true;
bool selfPass = false;
double maxPassScore = -9999;
double maxShootScore = -9999;
double flatPassQ = -9999;
double chipPassQ = -9999;
int CHOOSE_FLAT_DIFF = 5;
int CHOOSE_SHOOT_THRESHOLD = 60;
int NO_KICK_CNT = 0;
int MAX_NO_KICK = 10;
double bufferTime = 0.4;
}
namespace RECALCULATE {
//对方的反应时间
double THEIR_RESPONSE_TIME = 0.5;
double THEIR_CHIP_RESPONSE_TIME = 0.3;
//传球角度误差, 角度制
double PASS_ANGLE_ERROR = 2;
//重算的条件
QString recomputeCondition = "";
//传球点距离太近重算
const int RECOMPUTE_DIST = 1500;
const int SELFPASS_RECOMPUTE_DIST = static_cast<int>(2*Param::Vehicle::V2::PLAYER_SIZE);
//是否忽略近处的车
const double IGNORE_CLOSE_ENEMY_DIST = 1000;
bool IGNORE_THEIR_GUARD = true;
//无效传球点保持的最大帧数
const int MAX_INVALID_CNT = 30;
}
}

CMessiDecision::CMessiDecision()
    : _leader(1)
    , _receiver(2)
    , _cycle(0)
    , _lastRecomputeCycle(0)
    , _lastChangeReceiverCycle(0)
    , _lastChangeLeaderCycle(-9999)
    , _lastUpdateRunPosCycle(-9999)
    , _attackerAmount(1)
    , _timeToGetResult(-9999) //可以拿傳球點結果的時間
    , _passVel(0)
    , _messiRun(true)
    , _ourBall(false)
    , _theirBall(false)
    , _both(false)
    , _isFlat(true)
    , _canKick(false)
    , _state("GetBall")
    , _laststate("GetBall")
    , _leaderState("PASS")
    , _passPos(CGeoPoint(Param::Field::PITCH_LENGTH/2, 0))
    , _receiverPos(STATIC_POS[0])
    , _flatPassPos(CGeoPoint(Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH - 200, 0))
    , _flatShootPos(CGeoPoint(Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH - 200, 0))
    , _chipPassPos(CGeoPoint(Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH - 200, 0))
    , _chipShootPos(CGeoPoint(Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH - 200, 0))
    , _pVision(nullptr) {
    ZSS::ZParamManager::instance()->loadParam(TEST_PASS, "Messi/NoShoot", false);
    ZSS::ZParamManager::instance()->loadParam(SELF_PASS, "Messi/SelfPass", false);
    ZSS::ZParamManager::instance()->loadParam(MESSI_DEBUG, "Messi/Debug", false);
    ZSS::ZParamManager::instance()->loadParam(INTER_DEBUG, "Messi/INTER_DEBUG", false);
    ZSS::ZParamManager::instance()->loadParam(SHOW_PASSLINE, "Messi/SHOW_PASSLINE", true);
    ZSS::ZParamManager::instance()->loadParam(PASS::CHOOSE_FLAT_DIFF, "Messi/CHOOSE_FLAT_DIFF", 5);
    ZSS::ZParamManager::instance()->loadParam(ONLY_PASSPOS, "Messi/ONLY_PASSPOS", false);
    ZSS::ZParamManager::instance()->loadParam(ONLY_SHOOTPOS, "Messi/ONLY_SHOOTPOS", false);
    ZSS::ZParamManager::instance()->loadParam(ONLY_FLATPOS, "Messi/ONLY_FLATPOS", false);
    ZSS::ZParamManager::instance()->loadParam(ONLY_CHIPPOS, "Messi/ONLY_CHIPPOS", false);
    ZSS::ZParamManager::instance()->loadParam(SIGNAL_TO_CUDA, "Messi/SIGNAL_TO_CUDA", false);
    ZSS::ZParamManager::instance()->loadParam(PASS::CHOOSE_SHOOT_THRESHOLD, "Messi/CHOOSE_SHOOT_THRESHOLD", 60);
    ZSS::ZParamManager::instance()->loadParam(FIX_BUF_RATIO, "Messi/FIX_BUF_RATIO", 1);
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION, "Alert/IsSimulation", false);
    ZSS::ZParamManager::instance()->loadParam(RECALCULATE::THEIR_RESPONSE_TIME, "Messi/THEIR_RESPONSE_TIME", 0.5);
    ZSS::ZParamManager::instance()->loadParam(RECALCULATE::THEIR_CHIP_RESPONSE_TIME, "Messi/THEIR_CHIP_RESPONSE_TIME", 0.3);
    ZSS::ZParamManager::instance()->loadParam(RECALCULATE::PASS_ANGLE_ERROR, "Messi/PASS_ANGLE_ERROR", 2);
    ZSS::ZParamManager::instance()->loadParam(RECALCULATE::IGNORE_THEIR_GUARD, "Messi/IGNORE_THEIR_GUARD", false);
    ZSS::ZParamManager::instance()->loadParam(SHOOT::FORCE_SHOOT, "Messi/FORCE_SHOOT", true);
    ZSS::ZParamManager::instance()->loadParam(IS_RIGHT, "ZAlert/IsRight", false);
    ZSS::ZParamManager::instance()->loadParam(PASS::bufferTime, "Messi/bufferTime", 0.4);
    ZSS::ZParamManager::instance()->loadParam(PASS::MAX_NO_KICK, "Messi/MAX_NO_KICK", 10);
    _otherPos[0] = CGeoPoint(Param::Field::PITCH_LENGTH / 8, Param::Field::PITCH_WIDTH / 4);
    _otherPos[1] = CGeoPoint(Param::Field::PITCH_LENGTH / 8, -Param::Field::PITCH_WIDTH / 4);
    _otherPos[2] = CGeoPoint(Param::Field::PITCH_LENGTH * 3 / 8, Param::Field::PITCH_WIDTH / 4);
    _otherPos[3] = CGeoPoint(Param::Field::PITCH_LENGTH * 3 / 8, -Param::Field::PITCH_WIDTH / 4);
}

void CMessiDecision::generateAttackDecision(const CVisionModule* pVision) {
    //更新图像信息
    _pVision = pVision;
    _cycle = pVision->getCycle();

    //Ball state
    judgeBallState();

    //计算state
    judgeState();

    //判断进攻车数量
    judgeAttackerAmount();

    //确定leader
    if (confirmLeader())
        _lastChangeLeaderCycle = _cycle;

    //计算leader的截球点
    generateLeaderPos();

    //计算Receiver跑位点
    generateReceiverAndPos();

    //判断leader状态
    judgeLeaderState();

    //计算其他进攻车的跑位点
    if (needRunPos()) {
        generateOtherRunPos();
    }

    std::srand((unsigned)time(nullptr));
    //_otherPos[0] = CGeoPoint(std::rand()%2000+2000,std::rand()%6000-3000);
    _otherPos[0] = CGeoPoint(std::rand()%1000+3000,std::rand()%1000-2000);
    //qDebug()<<_otherPos[0].x();

    // DEBUG INFO
    if(MESSI_DEBUG) {
        GDebugEngine::Instance()->gui_debug_msg(_passPos, QString("PPPPPP").toLatin1(), _isFlat ? COLOR_GREEN : COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-5800, 2600) * (IS_RIGHT ? -1 : 1), _state.c_str() + QString("    Self Pass: %1").arg(PASS::selfPass).toLatin1() + QString("    leader: %1  receiver: %2  Cycle: %3").arg(_leader).arg(_receiver).arg(_lastChangeLeaderCycle).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-5800, 2800) * (IS_RIGHT ? -1 : 1), QString("canshoot: %1  shootRange: %2  valid %3 %4 %5  BallVel: %6").arg(ShootModule::Instance()->canShoot(_pVision, _leaderPos)).arg(SHOOT::shootAngle).arg(SHOOT::leftShoot).arg(SHOOT::middleShoot).arg(SHOOT::rightShoot).arg(_pVision->ball().Vel().mod()).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-5800, 3000) * (IS_RIGHT ? -1 : 1), QString("passMode: %1 PassScore: %2 ShootScore: %3").arg(PASS::passMode ? "Pass" : "Shoot").arg(PASS::maxPassScore).arg(PASS::maxShootScore).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-5800, 3200) * (IS_RIGHT ? -1 : 1), QString("isFlat: %1  canKick: %2  Flat: %3  Chip: %4").arg(_isFlat).arg(_canKick).arg(PASS::flatPassQ).arg(PASS::chipPassQ).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-5800, 3400) * (IS_RIGHT ? -1 : 1), (QString("recompute cycle: %1 %2  ").arg(_lastRecomputeCycle).arg(inValidCnt) + QString("condition: ").append(RECALCULATE::recomputeCondition)).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-5800, 3600) * (IS_RIGHT ? -1 : 1), QString("leaderState: ").toLatin1() + _leaderState.c_str() + QString("  noKick: %1").arg(PASS::NO_KICK_CNT).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(_leaderPos, QString("LeaderPos").toLatin1(), COLOR_CYAN);
        GDebugEngine::Instance()->gui_debug_msg(_leaderWaitPos, QString("WaitPos").toLatin1(), COLOR_CYAN);
    }
    if(SHOW_PASSLINE){
        GDebugEngine::Instance()->gui_debug_line(_pVision->ourPlayer(_leader+1).Pos(), _passPos, _isFlat ? COLOR_GREEN : COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_x(_passPos, _isFlat ? COLOR_GREEN : COLOR_ORANGE);
    }
    //获得laststate方便计算点的判断
    _laststate = _state;
}

// 判断球的状态
void CMessiDecision::judgeBallState() {
    string ballStatus = ZSkillUtils::instance()->getBallStatus();
    _ourBall = (ballStatus == "Our" || ballStatus == "OurHolding");
    _theirBall = (ballStatus == "Their" || ballStatus == "TheirHolding");
    _both = (ballStatus == "Both" || ballStatus == "BothHolding");//这里和pp不同
}

// 确定当前的状态
void CMessiDecision::judgeState() {
    static int fixBuf = 0;
    // 解决传球失败时错误的匹配
    if ((_state == "Pass"/* || _state == "fix"*/) && ZSkillUtils::instance()->getOurInterTime(_leader+1) - ZSkillUtils::instance()->getOurBestInterTime() > WRONG_LEADER_INTERTIME
             && !Utils::InOurPenaltyArea(_pVision->ourPlayer(ZSkillUtils::instance()->getOurBestPlayer()).Pos(), 500) ) {
        _state = "GetBall";
        return;
    }
    if(_state != "fix") fixBuf = 0;
    // 保持fix状态
    if (fixBuf > 0) {
        _state = "fix";
        fixBuf--;
    }
    // 进入fix状态
    else if(/*_state == "Pass" && */BallStatus::Instance()->IsChipKickedOut(_leader + 1) && !PASS::selfPass){
        _state = "fix";
        double fixTime = BallModel::instance()->chipJumpTime(_passVel); //-0.0000015498 * pow(_passVel, 2) + 0.0025344180 * _passVel + 0.2463515283;
        fixBuf = static_cast<int>(fixTime * Param::Vision::FRAME_RATE * FIX_BUF_RATIO);
    }
    // Pass或者Getball状态
    else {
        static int count = 0;
        static string lastTemp = "GetBall";
        string temp = "GetBall";
        //所有状态机的判断
        const PlayerVisionT& enemy = _pVision->theirPlayer(ZSkillUtils::instance()->getTheirBestPlayer());
        bool enemyHoldBall = (enemy.Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE, enemy.Dir())).dist(_pVision->ball().Pos()) < HOLDBALL_DIST;
        const PlayerVisionT& leader = _pVision->ourPlayer(_leader+1);
        bool leaderHoldBall = (leader.Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE, leader.Dir())).dist(_pVision->ball().Pos()) < HOLDBALL_DIST;
        bool leaderInfrared = RobotSensor::Instance()->IsInfraredOn(_leader+1);
        if(_ourBall || (leaderHoldBall && !enemyHoldBall) || leaderInfrared) {
            temp = "Pass";
        } else if(_theirBall || /*_state == "initState" || */enemyHoldBall) {
            temp = "GetBall";
        } else {
            temp = _state;
        }
        //状态机持续时间大于5帧，跳转
        if (lastTemp != temp) {
            count = 0;
        } else {
            count++;
        }

        if(_state == "fix"){
            _state = _ourBall ? "Pass" : "GetBall";
            count = 0;
        }
        else if (temp == "Pass" && count >= 20) {
            _state = "Pass";
            count = 0;
        } else if (temp == "GetBall" && count >= 5) {
            _state = "GetBall";
            count = 0;
        }
        lastTemp = temp;
    }
}

// 根据球的位置判断attacker amount
void CMessiDecision::judgeAttackerAmount() {
    static const CGeoPoint OUR_GOAL(-Param::Field::PITCH_LENGTH/2, 0);
    std::vector<int> enemyNumVec;
    int enemyNumLeft = WorldModel::Instance()->getEnemyAmountInArea(-Param::Field::PITCH_LENGTH / 2, 0, -Param::Field::PITCH_WIDTH / 2, Param::Field::PITCH_WIDTH / 2, enemyNumVec);
//    int enemyNumRight = WorldModel::Instance()->getEnemyAmountInArea(-Param::Field::PITCH_LENGTH / 2, 50, -Param::Field::PITCH_WIDTH / 2, Param::Field::PITCH_WIDTH / 2, enemyNumVec);
//    if (_attackerAmount < enemyNumLeft) {
        _attackerAmount = enemyNumLeft;
//    } else if(_attackerAmount > enemyNumRight) {
//        _attackerAmount = enemyNumRight;
//    }
    if (_state == "Pass") {
        if (_attackerAmount > 3) {
            _attackerAmount = 3;
        }
    } else if (_state == "GetBall") {
//        int theirBestPlayer = ZSkillUtils::instance()->getTheirBestPlayer();
//        for (auto enemy : enemyNumVec) {
//            if(enemy == theirBestPlayer){
//                _attackerAmount--;
//                enemyNumVec.erase(std::remove(enemyNumVec.begin(), enemyNumVec.end(), enemy), enemyNumVec.end());
//                break;
//            }
//        }
        if (_attackerAmount > 4) {
            _attackerAmount = 4;
        }
        if (_attackerAmount < 0) {
            _attackerAmount = 0;
        }
    }
    _attacker.clear();
    while (enemyNumVec.size()) {
        double bestDist = 99999;
        int bestEnemy = -9999;
        for (auto enemyNum : enemyNumVec) {
            double newDist = _pVision->theirPlayer(enemyNum).Pos().dist(OUR_GOAL);
            if(newDist < bestDist){
                bestDist = newDist;
                bestEnemy = enemyNum;
            }
        }
        _attacker.push_back(bestEnemy);
        enemyNumVec.erase(std::remove(enemyNumVec.begin(), enemyNumVec.end(), bestEnemy), enemyNumVec.end());
    }
//    qDebug() << "state in messi: " << _state.c_str() << _attackerAmount;
}

// leader的截球点
void CMessiDecision::generateLeaderPos() {
    // fix状态使用的截球点
    static int ballStopCnt = 0;
    ballStopCnt = (_pVision->ball().Vel().mod() < 15 && _pVision->ourPlayer(_leader+1).Pos().dist(_pVision->ball().Pos()) > 1500) ? ballStopCnt + 1 : 0;
    /*if (!IS_SIMULATION && _state == "fix")
        _leaderWaitPos = _pVision->Ball().ChipPredictPos();
    else */if(ballStopCnt >= 5)
        _leaderWaitPos = _pVision->ball().Pos();
    else if(_cycle == _lastChangeLeaderCycle && _laststate != "fix")
        _leaderWaitPos = _receiverPos;
    // CUDA计算使用的leader的位置
    if(_pVision->ourPlayer(_leader+1).Pos().dist(_pVision->ball().Pos()) < 500)
        _leaderPos = _pVision->ball().Pos();
    else if(_cycle == _lastChangeLeaderCycle && _laststate != "fix")
        _leaderPos = _receiverPos;
    else
        _leaderPos = ZSkillUtils::instance()->getOurInterPoint(_leader + 1);
}

// 确定leader
bool CMessiDecision::confirmLeader() {
    //防止leader变化过快
    if (_cycle - _lastChangeLeaderCycle > MIN_CHANGE_LEADER_INTERVEL) {
        //选择最快拦截球的车作为leader,而不是单纯使用距离
        int bestInterPlayer = _leader + 1;
        double bestInterTime = ZSkillUtils::instance()->getOurInterTime(bestInterPlayer) - 0.3; //避免leader狂跳
        double bestInterDist = _pVision->ourPlayer(bestInterPlayer).Pos().dist(_pVision->ball().Pos()) - 300;
        // kickout时change leader
        if (BallStatus::Instance()->IsBallKickedOut(_leader + 1) && !Utils::InTheirPenaltyArea(_passPos, 0)) {
            _leader = _receiver;
            return true;
        }
        // 状态跳转、Getball状态、Pass时球接近静止时，选择最快截球车
        else if ((_state != _laststate || _state == "GetBall" || (_state == "Pass" && _pVision->ball().Vel().mod() < 300)) && _state != "fix"){
            for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
                // 不选择我们的后卫
                if(_pVision->ourPlayer(i).Valid() && !Utils::InOurPenaltyArea(_pVision->ourPlayer(i).Pos(), 6*Param::Vehicle::V2::PLAYER_SIZE)){
                    double interTime = ZSkillUtils::instance()->getOurInterTime(i);
                    double newInterDist = _pVision->ourPlayer(i).Pos().dist(_pVision->ball().Pos());
                    if(interTime < bestInterTime && newInterDist < bestInterDist) {
                        bestInterPlayer = i;
                        bestInterTime = interTime;
                        bestInterDist = newInterDist;
                    }
                }
            }
            if(_leader == bestInterPlayer - 1) return false;
            _leader = bestInterPlayer - 1;
            return true;
        }
    }
    return false;
}

// 判断是否需要重算
bool CMessiDecision::needReceivePos() {
    CGeoPoint leaderPosition = _pVision->ourPlayer(_leader + 1).Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE, _pVision->ourPlayer(_leader+1).Dir());
    CGeoPoint ballPosition = _pVision->ball().Pos();

    //增加条件时写出调用哪个函数时使用，方便分辨条件之间的与或关系
/*    if (_state == "Pass" && _laststate == "GetBall") {// Change state to Pass
        RECALCULATE::recomputeCondition = "CHANGE STATE";
        return true;
    }
    // change Leader To Receiver
//    else if (_cycle == _lastChangeLeaderCycle) {
//        DEBUG::recomputeCondition = "CHANGE LEADER";
//        return true;
//    }
    // 解決leader和receiver重合問題
    else */if(_leader == _receiver && !SELF_PASS){
        RECALCULATE::recomputeCondition = "LEADER = RECEIVER";
        return true;
    }
    // leader拿到球的时候，判断此时传球点是否有效，排除射门的情况
    else if (inValidCnt == RECALCULATE::MAX_INVALID_CNT) {
        RECALCULATE::recomputeCondition = "INVALID PASS";
        return true;
    }
    // 当传球点太近时,重新计算
    else if (!Utils::InTheirPenaltyArea(_passPos, 0) && leaderPosition.dist(ballPosition) < HOLDBALL_DIST && ((leaderPosition.dist(_passPos) < RECALCULATE::RECOMPUTE_DIST && !PASS::selfPass) || (leaderPosition.dist(_passPos) < RECALCULATE::SELFPASS_RECOMPUTE_DIST))) {
        RECALCULATE::recomputeCondition = "TOO CLOSE";
        return true;
    }
    // 用于测试的开关
    else if (SIGNAL_TO_CUDA) {
        RECALCULATE::recomputeCondition = "SIGNAL TO CUDA";
        return true;
    }
    // 极端条件:receiver被拿下去,receiver在禁区
    else if ((!Utils::InTheirPenaltyArea(_passPos, 0) && !_pVision->ourPlayer(_receiver+1).Valid()) || Utils::InOurPenaltyArea(_pVision->ourPlayer(_receiver+1).Pos(),0)) {
        RECALCULATE::recomputeCondition = "INVALID RECEIVER";
        return true;
    }
    // 极端条件:leader接球时传球点已经在后面
    else if (leaderPosition.dist(ballPosition) < HOLDBALL_DIST && leaderPosition.x() < Param::Field::PITCH_LENGTH/6 && _passPos.x() < leaderPosition.x()) {
        RECALCULATE::recomputeCondition = "PASS TO BACK";
        return true;
    }
    return false;
}

bool CMessiDecision::needRunPos() {
    bool needRunPos = false;
    //GetBall
    if (_laststate != _state) {
        needRunPos = true;
    }
    //changeLeaderToReceiver
//    else if (_cycle == _lastChangeLeaderCycle) {
//        needRunPos = true;
//    }
    //每隔一段时间更新runPos
    else if (_cycle - _lastUpdateRunPosCycle > 10) {
        needRunPos = true;
    }
    //射门时不需要算
    if(ShootModule::Instance()->canShoot(_pVision, _leaderPos))
        needRunPos = false;
    return needRunPos;
}

// 计算Receiver跑位点
void CMessiDecision::generateReceiverAndPos() {
#ifdef USE_CUDA_MODULE
    // 加入時間考慮，避免三幀內多次調用cuda
    if (_cycle > _timeToGetResult && needReceivePos()) {
        _lastRecomputeCycle = _cycle;
        ZCUDAModule::instance()->setLeader(_leader);
        ZCUDAModule::instance()->setLeaderPos(_leaderPos);
        if(_cycle - _lastChangeReceiverCycle > MIN_CHANGE_RECEIVER_INTERVAL || _leader == _receiver || !_pVision->ourPlayer(_receiver+1).Valid())
            ZCUDAModule::instance()->setReceiver(-1);
        else
            ZCUDAModule::instance()->setReceiver(_receiver);
        messi_to_cuda.Signal();
        _timeToGetResult = _cycle + 3;
//        qDebug() << "new receivePos" << _cycle << _timeToGetResult;
    }
    if (_cycle == _timeToGetResult)
        getPassPos();
#else
    if (needReceivePos()) {
        getPassPos();
    }
#endif
    // 意外情况处理:拿不到点时leader还在上一个receiverPos
    static int inValidPosCnt = 0;
    static const double CLOSE_DIST = 1000;
    inValidPosCnt = _receiverPos.dist(_pVision->ourPlayer(_leader+1).Pos()) < CLOSE_DIST ? inValidPosCnt + 1 : 0;
    if(inValidPosCnt > 3*Param::Vision::FRAME_RATE){
        double dist1 = STATIC_POS[0].dist(_pVision->ourPlayer(_leader+1).Pos());
        double dist2 = STATIC_POS[1].dist(_pVision->ourPlayer(_leader+1).Pos());
        if(dist1 > CLOSE_DIST && dist2 > CLOSE_DIST)
            _receiverPos = dist1 < dist2 ? STATIC_POS[0] : STATIC_POS[1];
        else
            _receiverPos = dist1 > CLOSE_DIST ? STATIC_POS[0] : STATIC_POS[1];
    }
    // 判断能不能射门
    if (ShootModule::Instance()->canShoot(_pVision, _leaderPos)) {
        _passPos = CGeoPoint(Param::Field::PITCH_LENGTH / 2.0, 0);
        _passVel = 6500;
        _isFlat = true;
        //receiver应该跑的点，避免挡住射门路线
        _receiverPos = _pVision->ourPlayer(_leader+1).Pos().y() < 0 ? STATIC_POS[0] : STATIC_POS[1];
    }
}

// 获取传球点模块的结果
void CMessiDecision::getPassPos() {
#ifdef USE_CUDA_MODULE
    // 选择射门点或者传球点
    PASS::maxPassScore = max(ZCUDAModule::instance()->getBestFlatPassQ(), ZCUDAModule::instance()->getBestChipPassQ());
    PASS::maxShootScore = max(ZCUDAModule::instance()->getBestFlatShootQ(), ZCUDAModule::instance()->getBestChipShootQ());
    if(ONLY_PASSPOS){
        PASS::passMode = true;
    }
    else if (ONLY_SHOOTPOS || Utils::InTheirPenaltyArea(_pVision->ourPlayer(_leader+1).Pos(), 1000)){
        PASS::passMode = false;
    }
    else{
        PASS::passMode = PASS::maxShootScore > PASS::CHOOSE_SHOOT_THRESHOLD ? false : true;//DEBUG::maxShootScore - DEBUG::maxPassScore > CHOOSE_SHOOT_DIFF ? false : true;
    }

    // 维护最佳平射点和挑射点
    _flatPassPos = ZSkillUtils::instance()->IsInField(ZCUDAModule::instance()->getBestFlatPass()) ? ZCUDAModule::instance()->getBestFlatPass() : _flatPassPos;
    _flatShootPos = ZSkillUtils::instance()->IsInField(ZCUDAModule::instance()->getBestFlatShoot()) ? ZCUDAModule::instance()->getBestFlatShoot() : _flatShootPos;
    _chipPassPos = ZSkillUtils::instance()->IsInField(ZCUDAModule::instance()->getBestChipPass()) ? ZCUDAModule::instance()->getBestChipPass() : _chipPassPos;
    _chipShootPos = ZSkillUtils::instance()->IsInField(ZCUDAModule::instance()->getBestChipShoot()) ? ZCUDAModule::instance()->getBestChipShoot() : _chipShootPos;

    // 获得平射点和挑球点
    CGeoPoint flatPassPoint = PASS::passMode ? ZCUDAModule::instance()->getBestFlatPass() : ZCUDAModule::instance()->getBestFlatShoot();
    CGeoPoint chipPassPoint = PASS::passMode ? ZCUDAModule::instance()->getBestChipPass() : ZCUDAModule::instance()->getBestChipShoot();
    PASS::flatPassQ = PASS::passMode ? ZCUDAModule::instance()->getBestFlatPassQ() : ZCUDAModule::instance()->getBestFlatShootQ();
    PASS::chipPassQ = PASS::passMode ? ZCUDAModule::instance()->getBestChipPassQ() : ZCUDAModule::instance()->getBestChipShootQ();
    // 选择平射点或者挑射点
    if(ZSkillUtils::instance()->IsInField(flatPassPoint) && ZSkillUtils::instance()->IsInField(chipPassPoint)){
        _isFlat = PASS::chipPassQ - PASS::flatPassQ > PASS::CHOOSE_FLAT_DIFF ? false : true;
    }
    else if(ZSkillUtils::instance()->IsInField(chipPassPoint)){
        _isFlat = false;
    }
    else if(ZSkillUtils::instance()->IsInField(flatPassPoint)){
        _isFlat = true;
    }
    if(ONLY_FLATPOS)
        _isFlat = true;
    if(ONLY_CHIPPOS)
        _isFlat = false;

    // 计算receiver、passvel和receivePos
    int lastReceiver = _receiver;
//    static int noReceivePos = 0;
//    static const int MAX_NO_RECEIVEPOS_CNT = 100;
    if(_isFlat && !Utils::OutOfField(flatPassPoint)){
        _passPos = _receiverPos = flatPassPoint;
        _receiver = PASS::passMode ? ZCUDAModule::instance()->getBestFlatPassNum() : ZCUDAModule::instance()->getBestFlatShootNum();
        inValidCnt = 0;
//        noReceivePos = 0;
    }
    else if(!_isFlat && !Utils::OutOfField(chipPassPoint)){
        _passPos = _receiverPos = chipPassPoint;
        _receiver = PASS::passMode ? ZCUDAModule::instance()->getBestChipPassNum() : ZCUDAModule::instance()->getBestChipShootNum();
        inValidCnt = 0;
//        noReceivePos = 0;
    }
    // 拿不到点
//    else {
//        noReceivePos++;
//        // 直接往前场挑一脚
//        if(noReceivePos >= MAX_NO_RECEIVEPOS_CNT && _pVision->ourPlayer(_leader+1).X() < 0/*-Param::Field::PITCH_LENGTH/8*/){
//            _passPos = _receiverPos = CGeoPoint(Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH - 20, 0);
//            _isFlat = false;
//            inValidCnt = 0;
//            noReceivePos = 0;
//        }
//    }
    // 判断当前是否改变receiver
    if(_receiver != lastReceiver) _lastChangeReceiverCycle = _cycle;

    // 判断是否自传
    if(_leader == _receiver && !Utils::InTheirPenaltyArea(_passPos, 0))
        PASS::selfPass = true;
    else
        PASS::selfPass = false;
#else
    ReceivePosModule::Instance()->generatePassPos(_pVision, _leader);
    _receiver = ReceivePosModule::Instance()->bestReceiver();
    _passPos = _receiverPos = ReceivePosModule::Instance()->bestPassPoint();
    _isFlat = true;
#endif
}

void CMessiDecision::generateOtherRunPos() {
    RunPosModule::Instance()->generateRunPos(_pVision, _receiverPos);
    _otherPos[0] = RunPosModule::Instance()->runPos(0);
    _otherPos[1] = RunPosModule::Instance()->runPos(1);
    _otherPos[2] = RunPosModule::Instance()->runPos(2);
    _lastUpdateRunPosCycle = _cycle;
}

void CMessiDecision::reset() {
    //在特殊状态后需要重置角色和点
    _state = "GetBall";
    _laststate = "GetBall";
}

// 射对方后卫
bool CMessiDecision::canShootGuard() {
    bool valid = false;
    static const double COMPENSATE_RATIO = 1.1;
    if(_pVision->ourPlayer(_leader+1).Pos().x() > Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH){
        for (int i=1; i<Param::Field::MAX_PLAYER_NUM; i++) {
            const PlayerVisionT& enemy = _pVision->theirPlayer(i);
            if(!enemy.Valid()) continue;
            if(!Utils::InTheirPenaltyArea(enemy.Pos(), 0) && Utils::InTheirPenaltyArea(enemy.Pos(), Param::Vehicle::V2::PLAYER_SIZE*2)){
//                qDebug() << "their guard:" << i;
                valid = true;
                CVector enemyToMe = _pVision->ourPlayer(_leader+1).Pos() - enemy.Pos();
                CVector enemyToGoal = CGeoPoint(Param::Field::PITCH_LENGTH/2, 0) - enemy.Pos();
                double compensateAngle = Utils::Normalize(enemyToGoal.dir()  - enemyToMe.dir()) / (1 + COMPENSATE_RATIO);
                double finalAngle = enemyToMe.dir() + compensateAngle;
                _passPos = enemy.Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE, finalAngle);
                _passVel = 5000;
                _isFlat = true;
                _canKick = true;
                break;
            }
        }
    }
    return valid;
}

// 门将开大脚时的传球点
CGeoPoint CMessiDecision::goaliePassPos() {
    CGeoPoint leaderPos = _pVision->ourPlayer(_leader + 1).Pos();
    // leader在前场时传leader
    if(leaderPos.x() > 0)
        return leaderPos + CVector(500, 0);
    // 否则传到对方禁区前沿制造混乱
    else
        return CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0);
}

// 判断leader状态
void CMessiDecision::judgeLeaderState() {
    //计算传球力度
    if(Utils::InTheirPenaltyArea(_passPos, 0))
        _passVel = 6000;
    else if(_isFlat)
        _passVel = BallModel::instance()->flatPassVel(_pVision, _passPos, _receiver, PASS::bufferTime, RECALCULATE::PASS_ANGLE_ERROR);
    else
        _passVel = BallModel::instance()->chipPassVel(_pVision, _passPos);
    //判断leader应该调什么skill
    if(_cycle <= _timeToGetResult)
        _leaderState = "COMPUTE";
    else if(PASS::selfPass)
        _leaderState = "SELFPASS";
    else
        _leaderState = "PASS";

    //判断leader能否传球
    CGeoPoint leaderPosition = _pVision->ourPlayer(_leader + 1).Pos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_SIZE, _pVision->ourPlayer(_leader+1).Dir());
    CGeoPoint ballPosition = _pVision->ball().Pos();
    CVector passLine = _passPos - ballPosition;
    CGeoPoint abnormalPos1 = ballPosition + Utils::Polar2Vector(ballPosition.dist(_passPos), passLine.dir() + RECALCULATE::PASS_ANGLE_ERROR*Param::Math::PI/180);
    CGeoPoint abnormalPos2 = ballPosition + Utils::Polar2Vector(ballPosition.dist(_passPos), passLine.dir() - RECALCULATE::PASS_ANGLE_ERROR*Param::Math::PI/180);
    _canKick = false;
    //射门
    if(Utils::InTheirPenaltyArea(_passPos, 0)){
        if(ShootModule::Instance()->canShoot(_pVision, _leaderPos) || (Utils::InTheirPenaltyArea(leaderPosition, 10*Param::Vehicle::V2::PLAYER_SIZE) && leaderPosition.x() <= Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH)) _canKick = true;
    }
    //传球
    else if(_cycle > _timeToGetResult){
        const double riskPosX = Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH - 4*Param::Vehicle::V2::PLAYER_SIZE;
        const double riskPassX = Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH - 10*Param::Vehicle::V2::PLAYER_SIZE;
        //特殊情况放宽条件:禁区附近
        if(!_isFlat && Utils::InTheirPenaltyArea(_passPos, 1000) && _passPos.x() > riskPosX && leaderPosition.x() > riskPassX){
            _canKick = true;
        }
        //非自传的平射点
        else if(_isFlat && !PASS::selfPass && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, _passPos, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD) && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos1, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD) && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos2, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD)){
            _canKick = true;
        }
        //自传的平射点
        else if(_isFlat && PASS::selfPass && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, _passPos, 0.6, -9999) && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos1, 0.6, -9999) && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos2, 0.6, -9999)){
            _canKick = true;
        }
        //挑射点
        else if (!_isFlat && ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, _passPos, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD) && ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, abnormalPos1, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD) && ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, abnormalPos2, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD)) {
            _canKick = true;
        }
    }
    if(INTER_DEBUG){
        if(_isFlat){
            if(!PASS::selfPass){
                ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos1, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD, true);
                ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos2, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD, true);
                ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, _passPos, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD, true);
            }
            else {
                ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, _passPos, 0.6, -9999, false, false, true);
                ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos1, 0.6, -9999, false, false, true);
                ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos2, 0.6, -9999, false, false, true);
            }
        }
        else {
            ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, _passPos, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD, true);
            ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, abnormalPos1, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD, true);
            ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, abnormalPos2, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD, true);
        }
    }
    //判断传球点是否非法
    if((leaderPosition.dist(ballPosition) < HOLDBALL_DIST || RobotSensor::Instance()->IsInfraredOn(_leader+1)) && !_canKick){
        inValidCnt = inValidCnt >= RECALCULATE::MAX_INVALID_CNT ? RECALCULATE::MAX_INVALID_CNT : inValidCnt + 1;
    }
    else {
        inValidCnt = 0;
    }

    // 当receiver离挑球点比较近的时候踢出
    if((!_isFlat && !PASS::selfPass && _pVision->ourPlayer(_receiver+1).Pos().dist(_passPos) > 1500) || (_isFlat && !PASS::selfPass && !Utils::InTheirPenaltyArea(_passPos, 0) && predictedTime(_pVision->ourPlayer(_receiver+1), _passPos) > 1))
        _canKick = false;
    // 后场没点时强行挑射
//    if(!_isFlat && _pVision->ourPlayer(_leader+1).X() < /*-Param::Field::PITCH_LENGTH/8*/0 && Utils::InTheirPenaltyArea(_passPos, 50))
//        _canKick = true;

    // NO KICK计数
    if ((leaderPosition.dist(ballPosition) < HOLDBALL_DIST || RobotSensor::Instance()->IsInfraredOn(_leader+1)) && !_canKick) PASS::NO_KICK_CNT = PASS::NO_KICK_CNT < static_cast<int>(Param::Vision::FRAME_RATE * PASS::MAX_NO_KICK) ? PASS::NO_KICK_CNT + 1 : static_cast<int>(Param::Vision::FRAME_RATE * PASS::MAX_NO_KICK);
    else PASS::NO_KICK_CNT = 0;
    if (PASS::NO_KICK_CNT >= static_cast<int>(Param::Vision::FRAME_RATE * PASS::MAX_NO_KICK) || leaderPosition.x() <= -Param::Field::PITCH_LENGTH/3){
        _passPos = _receiverPos = CGeoPoint(Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH - 200, 0);
        _passVel = BallModel::instance()->chipPassVel(_pVision, _passPos);
        _isFlat = false;
        _canKick = true;
    }
}
CGeoPoint CMessiDecision::freeKickPos() {
#ifdef USE_CUDA_MODULE
    return ZCUDAModule::instance()->getBestFreeKickPos();
#else
    return CGeoPoint(Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH - 20, 0);
#endif
}
CGeoPoint CMessiDecision::freeKickWaitPos() {
    CGeoPoint waitPos;
    CGeoPoint ballPos;
    if(vision->getCurrentRefereeMsg() == "OurBallPlacement") {
        ballPos.fill(RefereeBoxInterface::Instance()->getBallPlacementPosX(), RefereeBoxInterface::Instance()->getBallPlacementPosY());
    } else {
        ballPos = vision->ball().Pos();
    }
    waitPos.fill(ballPos.x() > Param::Field::PITCH_LENGTH / 3 ? (ballPos.x() - Param::Field::PITCH_LENGTH / 12) : ballPos.x() / 2 + Param::Field::PITCH_LENGTH / 4, //(ballPos.x() * 3.0 / 4.0 + Param::Field::PITCH_LENGTH / 8)
                 std::fabs(ballPos.y()) < Param::Field::PENALTY_AREA_WIDTH / 2 ? 0 :
                 ballPos.y() > 0 ? Param::Field::PITCH_WIDTH / 4 : -Param::Field::PITCH_WIDTH / 4);
    RunPosModule::Instance()->generateRunPos(vision, waitPos);
    return waitPos;
}

bool CMessiDecision::canDirectKick() {
    CGeoPoint ballPos;
    if(vision->getCurrentRefereeMsg() == "OurBallPlacement") {
        ballPos = vision->getBallPlacementPosition();
    } else {
        ballPos = vision->ball().Pos();
    }
    std::vector<int> enemyNumVec;
    return (WorldModel::Instance()->getEnemyAmountInArea(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH, Param::Field::PITCH_LENGTH / 2, - Param::Field::PENALTY_AREA_WIDTH / 2, Param::Field::PENALTY_AREA_WIDTH / 2, enemyNumVec) > 1)
        && std::fabs(ballPos.y()) < Param::Field::PITCH_WIDTH / 4
        && std::fabs(ballPos.x()) > Param::Field::PITCH_LENGTH / 4;
}
