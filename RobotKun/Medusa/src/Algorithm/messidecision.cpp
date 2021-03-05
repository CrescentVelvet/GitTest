#include "messidecision.h"
#include "receivePos.h"
#include "SkillUtils.h"
#include <QDebug>
#include "GDebugEngine.h"
#include "BallStatus.h"
#include "RobotSensor.h"
#include "parammanager.h"
#include "staticparams.h"
#include "ShootRangeList.h"
#include <iostream>
#include "WorldModel.h"
#include "Semaphore.h"
#include "Global.h"
#include "ballmodel.h"
#include "ShootModule.h"
#include "RefereeBoxIf.h"
#include "passposevaluate.h"
#include "defencesquence.h"
#include "BallPredictor.h"
#include "CMmotion.h"
#ifdef USE_CUDA_MODULE
#include "CUDAModule/CUDAModule.h"
Semaphore messi_to_cuda(0);
#endif

namespace {
double waittime_coef;
bool USE_CHIPSOLVER;
bool SELF_PASS;
bool MESSI_DEBUG;
bool INTER_DEBUG;
bool ONLY_PASSPOS = false;
bool ONLY_SHOOTPOS = false;
bool ONLY_FLATPOS = false;
bool ONLY_CHIPPOS = false;
bool SIGNAL_TO_CUDA = false;
bool IS_RIGHT = false;
bool IS_SIMULATION = false;
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
const CGeoPoint STATIC_POS[2] = {CGeoPoint(PARAM::Field::PITCH_LENGTH / 4, PARAM::Field::PITCH_WIDTH / 4),
                                 CGeoPoint(PARAM::Field::PITCH_LENGTH / 4, -PARAM::Field::PITCH_WIDTH / 4)};
namespace PASS {
bool passMode = true;
bool selfPass = false;
double maxPassScore = -9999;
double maxShootScore = -9999;
double flatPassQ = -9999;
double chipPassQ = -9999;
int CHOOSE_FLAT_DIFF = 25;
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
//是否忽略近处的车
double IGNORE_CLOSE_ENEMY_DIST = 1000;
bool IGNORE_THEIR_GUARD = true;
//无效传球点保持的最大帧数
const int MAX_INVALID_CNT = 30;
}
bool CHIP_GETBALL_DEBUG;
}

CMessiDecision::CMessiDecision()
    : _leader(1)
    , _receiver(2)
    , _cycle(0)
    , otherPosChangeCnt(0)
    , _lastRecomputeCycle(0)
    , _lastChangeReceiverCycle(0)
    , _lastChangeLeaderCycle(-9999)
    , _lastUpdateRunPosCycle(-9999)
    //, _timeToGetResult(-9999) //可以拿傳球點結果的時間
    , _passVel(0)
    , inValidPassCnt(0)
    , inValidShootCnt(0)
    , _messiRun(true)
    , _isFlat(true)
    , _canKick(false)
    , _canKickReceiver(false)
    , _state("GetBall")
    , _laststate("GetBall")
    , _leaderState("PASS")
    , _passPos(CGeoPoint(PARAM::Field::PITCH_LENGTH/2, 0))
    , _receiverPos(STATIC_POS[0])
    , _flatPassPos(CGeoPoint(PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - 200, 0))
    , _flatShootPos(CGeoPoint(PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - 200, 0))
    , _chipPassPos(CGeoPoint(PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - 200, 0))
    , _chipShootPos(CGeoPoint(PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - 200, 0))
    , _pVision(nullptr) {
    ZSS::ZParamManager::instance()->loadParam(waittime_coef,"chipsolver/waittime_coef",0.2);
    ZSS::ZParamManager::instance()->loadParam(USE_CHIPSOLVER, "chipsolver/USE_CHIPSOLVER_IN_MEDUSA", false);
    ZSS::ZParamManager::instance()->loadParam(SELF_PASS, "Messi/SelfPass", false);
    ZSS::ZParamManager::instance()->loadParam(MESSI_DEBUG, "Messi/Debug", false);
    ZSS::ZParamManager::instance()->loadParam(INTER_DEBUG, "Messi/INTER_DEBUG", false);
    ZSS::ZParamManager::instance()->loadParam(PASS::CHOOSE_FLAT_DIFF, "Messi/CHOOSE_FLAT_DIFF", 15);
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
    ZSS::ZParamManager::instance()->loadParam(IS_RIGHT, "ZAlert/IsRight", false);
    ZSS::ZParamManager::instance()->loadParam(PASS::bufferTime, "Messi/bufferTime", 0.4);
    ZSS::ZParamManager::instance()->loadParam(PASS::MAX_NO_KICK, "Messi/MAX_NO_KICK", 10);
    ZSS::ZParamManager::instance()->loadParam(RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, "Messi/IGNORE_CLOSE_ENEMY_DIST", 700);
    ZSS::ZParamManager::instance()->loadParam(CHIP_GETBALL_DEBUG, "chipsolver/CHIP_GETBALL_DEBUG", false);
    _otherPos[0] = CGeoPoint(PARAM::Field::PITCH_LENGTH / 8, PARAM::Field::PITCH_WIDTH / 4);
    _otherPos[1] = CGeoPoint(PARAM::Field::PITCH_LENGTH / 8, -PARAM::Field::PITCH_WIDTH / 4);
    _otherPos[2] = CGeoPoint(PARAM::Field::PITCH_LENGTH * 3 / 8, PARAM::Field::PITCH_WIDTH / 4);
    _otherPos[3] = CGeoPoint(PARAM::Field::PITCH_LENGTH * 3 / 8, -PARAM::Field::PITCH_WIDTH / 4);
}

void CMessiDecision::generateAttackDecision(const CVisionModule* pVision) {
    //更新图像信息
    _pVision = pVision;
    _cycle = pVision->getCycle();
    //计算state
    judgeState();
    //确定leader
    confirmLeader();
    //计算leader的截球点
    generateLeaderPos();
    //计算Receiver跑位点
    generateReceiverAndPos();
    //计算其他进攻车的跑位点
    generateOtherRunPos();
    //判断leader状态
    judgeLeaderState();
    // DEBUG INFO
    if(MESSI_DEBUG) {
        GDebugEngine::Instance()->gui_debug_msg(_passPos, QString("PPPPPP").toLatin1(), _isFlat ? COLOR_GREEN : COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2+200, PARAM::Field::PITCH_WIDTH/2-200) * (IS_RIGHT ? -1 : 1), _state.c_str() + QString("    Self Pass: %1").arg(PASS::selfPass).toLatin1() + QString("    leader: %1  receiver: %2  Cycle: %3").arg(_leader).arg(_receiver).arg(_lastChangeLeaderCycle).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2+200, PARAM::Field::PITCH_WIDTH/2-400) * (IS_RIGHT ? -1 : 1), QString("canshoot: %1  BallVel: %2 Calculated Vel: %3").arg(ShootModule::Instance()->canShoot(_pVision, _leaderPos)).arg(_pVision->ball().Vel().mod()).arg(_passVel).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2+200, PARAM::Field::PITCH_WIDTH/2-600) * (IS_RIGHT ? -1 : 1), QString("passMode: %1 PassScore: %2 ShootScore: %3").arg(PASS::passMode ? "Pass" : "Shoot").arg(PASS::maxPassScore).arg(PASS::maxShootScore).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2+200, PARAM::Field::PITCH_WIDTH/2-800) * (IS_RIGHT ? -1 : 1), QString("isFlat: %1  canKick: %2  Flat: %3  Chip: %4").arg(_isFlat).arg(_canKick).arg(PASS::flatPassQ).arg(PASS::chipPassQ).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2+200, PARAM::Field::PITCH_WIDTH/2-1000) * (IS_RIGHT ? -1 : 1), (QString("recompute cycle: %1 %2  ").arg(_lastRecomputeCycle).arg(inValidPassCnt) + QString("condition: ").append(RECALCULATE::recomputeCondition)).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2+200, PARAM::Field::PITCH_WIDTH/2-1200) * (IS_RIGHT ? -1 : 1), QString("leaderState: ").toLatin1() + _leaderState.c_str() + QString("  noKick: %1").arg(PASS::NO_KICK_CNT).toLatin1(), COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(_leaderPos, QString("LeaderPos").toLatin1(), COLOR_CYAN);
        GDebugEngine::Instance()->gui_debug_msg(_leaderWaitPos, QString("WaitPos").toLatin1(), COLOR_CYAN);
        GDebugEngine::Instance()->gui_debug_msg(_receiverPos, QString("ReceiverPos").toLatin1(), COLOR_CYAN);
        GDebugEngine::Instance()->gui_debug_line(_pVision->ourPlayer(_leader).Pos(), _passPos, _isFlat ? COLOR_GREEN : COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_x(_passPos, _isFlat ? COLOR_GREEN : COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(_otherPos[1], QString("OtherPos:").toLatin1(), COLOR_CYAN);
    }
    //获得laststate方便计算点的判断
    _laststate = _state;
}

// 确定当前的状态
void CMessiDecision::judgeState() {
    // 判断球的状态
    string ballStatus = ZSkillUtils::instance()->getBallStatus();
    bool _ourBall = (ballStatus == "Our" || ballStatus == "OurHolding");
    bool _theirBall = (ballStatus == "Their" || ballStatus == "TheirHolding");
    bool _both = (ballStatus == "Both" || ballStatus == "BothHolding");//这里和pp不同

    static int fixBuf = 0;
    // 解决传球失败时错误的匹配
    if ((_state == "Pass"/* || _state == "fix"*/) && ZSkillUtils::instance()->getOurInterTime(_leader) - ZSkillUtils::instance()->getOurBestInterTime() > WRONG_LEADER_INTERTIME
             && !Utils::InOurPenaltyArea(_pVision->ourPlayer(ZSkillUtils::instance()->getOurBestPlayer()).Pos(), 500) ) {
        _state = "GetBall";
        return;
    }
    if(_state != "fix") fixBuf = 0;
    // 保持fix状态
    if (fixBuf > 0) {
        _state = "fix";
        if(USE_CHIPSOLVER){
            if(_pVision->ball().ChipBallState() && Utils::IsInField(_pVision->ball().SecondChipPos()))
                fixBuf = 0; //todo consider second pass
//                fixBuf = 10;
            else
                fixBuf--;
        }
        else
            fixBuf--;
    }
    // 进入fix状态

//    else if(/*_state == "Pass" && */BallStatus::Instance()->IsChipKickedOut(_leader) && !PASS::selfPass){
    else if(/*_state == "Pass" && */RobotSensor::Instance()->IsKickerOn(_leader) == IS_CHIP_KICK_ON && !PASS::selfPass){
        _state = "fix";
        double fixTime = BallModel::instance()->chipJumpTime(_passVel); //-0.0000015498 * pow(_passVel, 2) + 0.0025344180 * _passVel + 0.2463515283;
        fixBuf = static_cast<int>(fixTime * PARAM::Vision::FRAME_RATE * FIX_BUF_RATIO);
    }
    // Pass或者Getball状态
    else {
        static int count = 0;
        static string lastTemp = "GetBall";
        string temp = "GetBall";
        //所有状态机的判断
        const PlayerVisionT& enemy = _pVision->theirPlayer(ZSkillUtils::instance()->getTheirBestPlayer());
        bool enemyHoldBall = (enemy.Pos() + Utils::Polar2Vector(PARAM::Vehicle::V2::PLAYER_SIZE, enemy.Dir())).dist(_pVision->ball().Pos()) < HOLDBALL_DIST;
        const PlayerVisionT& leader = _pVision->ourPlayer(_leader);
        bool leaderHoldBall = (leader.Pos() + Utils::Polar2Vector(PARAM::Vehicle::V2::PLAYER_SIZE, leader.Dir())).dist(_pVision->ball().Pos()) < HOLDBALL_DIST;
        bool leaderInfrared = RobotSensor::Instance()->IsInfraredOn(_leader);
        bool isChipState = _pVision->ball().ChipBallState() && Utils::IsInField(_pVision->ball().SecondChipPos());
        if(leaderInfrared || ((_ourBall || (leaderHoldBall && !enemyHoldBall) /*|| leaderInfrared*/) && !isChipState)) {
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
        //测试时候出现状态跳转过慢情况，此处减小PASS状态跳转持续时间
        else if (temp == "Pass" && count >= 15/*20*/) {
            _state = "Pass";
            count = 0;
        } else if (temp == "GetBall" && count >= 5) {
            _state = "GetBall";
            count = 0;
        }
        lastTemp = temp;
    }

}

// leader的截球点
void CMessiDecision::generateLeaderPos() {
    // fix状态使用的截球点
    static int ballStopCnt = 0;
    ballStopCnt = (_pVision->ball().Vel().mod() < 15 && _pVision->ourPlayer(_leader).Pos().dist(_pVision->ball().Pos()) > 1500) ? ballStopCnt + 1 : 0;
    /*if (USE_CHIPSOLVER && !IS_SIMULATION && _state == "fix" && VisionModule::Instance()->ball().ChipBallState()){
        CGeoPoint tmp_pos = _pVision->ball().SecondChipPos();
        if(Utils::IsInField(tmp_pos))
            _leaderWaitPos = tmp_pos;
    }
    else */if(ballStopCnt >= 5)
        _leaderWaitPos = _pVision->ball().Pos();
    else if(_cycle == _lastChangeLeaderCycle && _laststate != "fix")
        _leaderWaitPos = _receiverPos;
    // CUDA计算使用的leader的位置
    if(PASS::selfPass)
    {
        _leaderPos = _pVision->ball().Pos();
    }
    else if(_pVision->ourPlayer(_leader).Pos().dist(_pVision->ball().Pos()) < 500)
        _leaderPos = _pVision->ball().Pos();
    else if(_cycle == _lastChangeLeaderCycle && _laststate != "fix")
        _leaderPos = _receiverPos;
    else
        _leaderPos = ZSkillUtils::instance()->getOurInterPoint(_leader);
}

// 确定leader
void CMessiDecision::confirmLeader() {
    bool changeLeader = false;
    //防止leader变化过快
    if (_cycle - _lastChangeLeaderCycle > MIN_CHANGE_LEADER_INTERVEL) {
        //选择最快拦截球的车作为leader,而不是单纯使用距离
        int bestInterPlayer = _leader;
//        double bestInterTime = ZSkillUtils::instance()->getOurInterTime(bestInterPlayer) - 0.3; //避免leader狂跳
        double bestInterTime = ZSkillUtils::instance()->getOurInterTime(bestInterPlayer) - 0.1;
//        double bestInterDist = _pVision->ourPlayer(bestInterPlayer).Pos().dist(_pVision->ball().Pos()) - 300;
        double bestInterDist = _pVision->ourPlayer(bestInterPlayer).Pos().dist(_pVision->ball().Pos()) - 150;
        // kickout时change leader
//        if (BallStatus::Instance()->IsBallKickedOut(_leader) && !Utils::InTheirPenaltyArea(_passPos, 0)) {
        //这里的修改是为了防止硬件层面有车空踢导致踢球判断错误
        if (RobotSensor::Instance()->IsKickerOn(_leader) > 0 && !Utils::InTheirPenaltyArea(_passPos, 0)) {

            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2+200, PARAM::Field::PITCH_WIDTH/2-1400) * (IS_RIGHT ? -1 : 1), QString("Is Ball Kicked OUT: %1 ").arg(BallStatus::Instance()->IsBallKickedOut(_leader)).toLatin1(), COLOR_ORANGE);

            _leader = _receiver;
            changeLeader = true;
        }
        // 状态跳转、Getball状态、Pass时球接近静止时，选择最快截球车
        else if ((_state != _laststate || /*_state == "GetBall" ||*/ (_state == "Pass" && _pVision->ball().Vel().mod() < 300)) && _state != "fix"){
            for (int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
                // 不选择我们的后卫
                if(_pVision->ourPlayer(i).Valid() && !Utils::InOurPenaltyArea(_pVision->ourPlayer(i).Pos(), 6*PARAM::Vehicle::V2::PLAYER_SIZE)){
                    double interTime = ZSkillUtils::instance()->getOurInterTime(i);
                    double newInterDist = _pVision->ourPlayer(i).Pos().dist(_pVision->ball().Pos());
                    if(interTime < bestInterTime && newInterDist < bestInterDist) {
                        bestInterPlayer = i;
                        bestInterTime = interTime;
                        bestInterDist = newInterDist;
                    }
                }
            }
            if(_leader != bestInterPlayer){
                _leader = bestInterPlayer;
                changeLeader = true;
            }
        }
        else if (_state == "GetBall")
        {
            int ourBestPlayer = ZSkillUtils::instance()->getOurBestPlayer();
            if (ourBestPlayer != _leader)
            {
                changeLeader = true;
                _leader = ourBestPlayer;
            }
        }

        if(USE_CHIPSOLVER){
            if(ZSkillUtils::instance()->isInFlyTime()){
                int bestChipPlayer = -1;
                double bestChipDist = 99999;
                double bestChipTime = 99999;
                for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++){
                    if(_pVision->ourPlayer(i).Valid() && !Utils::InOurPenaltyArea(_pVision->ourPlayer(i).Pos(), 6*PARAM::Vehicle::V2::PLAYER_SIZE)){
                        CGeoPoint chipInterPoint = ZSkillUtils::instance()->getChipInterPoint(i);
                        double dist = _pVision->ourPlayer(i).Pos().dist(chipInterPoint);
//                        CVector inter2player = _pVision->ourPlayer(i).Pos() - chipInterPoint;

                        // when in second chip, the data is invalid, use static variable to remain the dir
//                        static CVector idealballdir;
//                        if(ZSkillUtils::instance()->canGetChipResult())
//                            idealballdir = _pVision->ball().SecondChipPos() - _pVision->ball().BestChipPredictPos();

//                        double theta = fabs(Utils::Normalize(inter2player.dir()-idealballdir.dir()));
                        if(CHIP_GETBALL_DEBUG){
                            GDebugEngine::Instance()->gui_debug_arc(chipInterPoint,100,360,COLOR_PURPLE);
                            GDebugEngine::Instance()->gui_debug_arc(chipInterPoint,150,0,360,COLOR_PURPLE);
                        }
//                        double offset = (theta <= PARAM::Math::PI/2 ? dist * sin(theta) : dist * (1 + sin(theta - PARAM::Math::PI/2)));
//                        double time = predictedTime(_pVision->ourPlayer(i), chipInterPoint + idealballdir/idealballdir.mod()*offset);
                        double time = ZSkillUtils::instance()->getChipInterTime(i);
                        if(dist /*+ offset*/ < bestChipDist && time < bestChipTime){
                            bestChipDist = dist /*+ offset*/;
                            bestChipTime = time;
                            bestChipPlayer = i;
                        }
                    }
                }

                if(_leader != bestChipPlayer && bestChipPlayer != -1){
                    _leader = bestChipPlayer;
                    changeLeader = true;
                }
            }
        }
    }
    if (changeLeader) _lastChangeLeaderCycle = _cycle;
}

// 判断是否需要重算
bool CMessiDecision::needReceivePos() {
    CGeoPoint leaderPosition = _pVision->ourPlayer(_leader ).Pos() + Utils::Polar2Vector(PARAM::Vehicle::V2::PLAYER_SIZE, _pVision->ourPlayer(_leader).Dir());
    CGeoPoint ballPosition = _pVision->ball().Pos();
    //增加条件时写出调用哪个函数时使用，方便分辨条件之间的与或关系
/*  // change Leader To Receiver
    if (_cycle == _lastChangeLeaderCycle) {
//        DEBUG::recomputeCondition = "CHANGE LEADER";
//        return true;
//    }
    // 解決leader和receiver重合問題
    else */if(_leader == _receiver && !SELF_PASS){
        RECALCULATE::recomputeCondition = "LEADER = RECEIVER";
        return true;
    }
    // leader拿到球的时候，判断此时传球点是否有效，排除射门的情况
    else if (inValidPassCnt == RECALCULATE::MAX_INVALID_CNT) {
        RECALCULATE::recomputeCondition = "INVALID PASS";
        return true;
    }
    else if (inValidShootCnt == RECALCULATE::MAX_INVALID_CNT){
        RECALCULATE::recomputeCondition = "INVALID SHOOT";
        return true;
    }
    // 当传球点太近时,重新计算
    else if (!Utils::InTheirPenaltyArea(_passPos, 0) && leaderPosition.dist(ballPosition) < HOLDBALL_DIST && PassPosEvaluate::instance()->passTooClose(_passPos,_leaderPos,PASS::selfPass)) {
        RECALCULATE::recomputeCondition = "TOO CLOSE";
        return true;
    }
    // 用于测试的开关
    else if (SIGNAL_TO_CUDA) {
        RECALCULATE::recomputeCondition = "SIGNAL TO CUDA";
        return true;
    }
    // 极端条件:receiver被拿下去,receiver在禁区
    else if ((!Utils::InTheirPenaltyArea(_passPos, 0) && !_pVision->ourPlayer(_receiver).Valid()) || Utils::InOurPenaltyArea(_pVision->ourPlayer(_receiver).Pos(),0)) {
        RECALCULATE::recomputeCondition = "INVALID RECEIVER";
        return true;
    }
    // 极端条件:leader接球时传球点已经在后面
    else if (leaderPosition.dist(ballPosition) < HOLDBALL_DIST && PassPosEvaluate::instance()->passBack(_receiverPos,_leaderPos)) {
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
    else if (_cycle - _lastUpdateRunPosCycle > 25) {
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
    if(needReceivePos()){
        ZCUDAModule::instance()->setLeader(_leader);
        ZCUDAModule::instance()->setLeaderPos(_leaderPos);
        if(_cycle - _lastChangeReceiverCycle > MIN_CHANGE_RECEIVER_INTERVAL || _leader == _receiver || !_pVision->ourPlayer(_receiver).Valid())
            ZCUDAModule::instance()->setReceiver(-1);
        else
            ZCUDAModule::instance()->setReceiver(_receiver);
        ZCUDAModule::instance()->calculateBestPass();
        getPassPos();
    }
    // 加入時間考慮，避免三幀內多次調用cuda
//    if (_cycle >= _timeToGetResult && needReceivePos()) {
//        _lastRecomputeCycle = _cycle;
//        ZCUDAModule::instance()->setLeader(_leader);
//        ZCUDAModule::instance()->setLeaderPos(_leaderPos);
//        if(_cycle - _lastChangeReceiverCycle > MIN_CHANGE_RECEIVER_INTERVAL || _leader == _receiver || !_pVision->ourPlayer(_receiver).Valid())
//            ZCUDAModule::instance()->setReceiver(-1);
//        else
//            ZCUDAModule::instance()->setReceiver(_receiver);
//        messi_to_cuda.Signal();
//        _timeToGetResult = _cycle + 1;
//        qDebug() << "new receivePos" << _cycle << _timeToGetResult;
//    }
//    if (_cycle == _timeToGetResult)
//        getPassPos();
#else
    if (needReceivePos()) {
        getPassPos();
    }
#endif
    // 意外情况处理:拿不到点时leader还在上一个receiverPos
    static int inValidPosCnt = 0;
    static const double CLOSE_DIST = 1000;
    inValidPosCnt = _receiverPos.dist(_pVision->ourPlayer(_leader).Pos()) < CLOSE_DIST ? inValidPosCnt + 1 : 0;
    if(inValidPosCnt > 3*PARAM::Vision::FRAME_RATE){
        double dist1 = STATIC_POS[0].dist(_pVision->ourPlayer(_leader).Pos());
        double dist2 = STATIC_POS[1].dist(_pVision->ourPlayer(_leader).Pos());
        if(dist1 > CLOSE_DIST && dist2 > CLOSE_DIST)
            _receiverPos = dist1 < dist2 ? STATIC_POS[0] : STATIC_POS[1];
        else
            _receiverPos = dist1 > CLOSE_DIST ? STATIC_POS[0] : STATIC_POS[1];
    }
    // 判断能不能射门
    if (ShootModule::Instance()->canShoot(_pVision, _leaderPos)) {
        _passPos = CGeoPoint(PARAM::Field::PITCH_LENGTH / 2.0, 0);
        _passVel = 6500;
        _isFlat = true;
        //receiver应该跑的点，避免挡住射门路线
//        _receiverPos = _pVision->ourPlayer(_leader).Pos().y() < 0 ? STATIC_POS[0] : STATIC_POS[1];
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
    else if (ONLY_SHOOTPOS || Utils::InTheirPenaltyArea(_pVision->ourPlayer(_leader).Pos(), 1000)){
        PASS::passMode = false;
    }
    else{
        PASS::passMode = PASS::maxShootScore > PASS::CHOOSE_SHOOT_THRESHOLD ? false : true;//DEBUG::maxShootScore - DEBUG::maxPassScore > CHOOSE_SHOOT_DIFF ? false : true;
    }

    // 维护最佳平射点和挑射点
    _flatPassPos = Utils::IsInField(ZCUDAModule::instance()->getBestFlatPass()) ? ZCUDAModule::instance()->getBestFlatPass() : _flatPassPos;
    _flatShootPos = Utils::IsInField(ZCUDAModule::instance()->getBestFlatShoot()) ? ZCUDAModule::instance()->getBestFlatShoot() : _flatShootPos;
    _chipPassPos = Utils::IsInField(ZCUDAModule::instance()->getBestChipPass()) ? ZCUDAModule::instance()->getBestChipPass() : _chipPassPos;
    _chipShootPos = Utils::IsInField(ZCUDAModule::instance()->getBestChipShoot()) ? ZCUDAModule::instance()->getBestChipShoot() : _chipShootPos;

    // 获得平射点和挑球点
    CGeoPoint flatPassPoint = PASS::passMode ? ZCUDAModule::instance()->getBestFlatPass() : ZCUDAModule::instance()->getBestFlatShoot();
    CGeoPoint chipPassPoint = PASS::passMode ? ZCUDAModule::instance()->getBestChipPass() : ZCUDAModule::instance()->getBestChipShoot();
    PASS::flatPassQ = PASS::passMode ? ZCUDAModule::instance()->getBestFlatPassQ() : ZCUDAModule::instance()->getBestFlatShootQ();
    PASS::chipPassQ = PASS::passMode ? ZCUDAModule::instance()->getBestChipPassQ() : ZCUDAModule::instance()->getBestChipShootQ();
    // 选择平射点或者挑射点
    if(Utils::IsInField(flatPassPoint) && Utils::IsInField(chipPassPoint)){
        _isFlat = PASS::chipPassQ - PASS::flatPassQ > PASS::CHOOSE_FLAT_DIFF ? false : true;
    }
    else if(Utils::IsInField(chipPassPoint)){
        _isFlat = false;
    }
    else if(Utils::IsInField(flatPassPoint)){
        _isFlat = true;
    }
    if(ONLY_FLATPOS)
        _isFlat = true;
    if(ONLY_CHIPPOS)
        _isFlat = false;

    //特殊情况，CUDA平射挑射都算不出来，那么一直更新跑位点
    if (ZCUDAModule::instance()->getBestFlatPassQ() == -9999 && ZCUDAModule::instance()->getBestChipPassQ() == -9999)
    {
        _otherPos[1] = RunPosModule::Instance()->runPos(1);
        _lastUpdateRunPosCycle = _cycle;
        if (RECALCULATE::recomputeCondition == "PASS TO BACK")
        {
            _receiverPos = _pVision->ourPlayer(_leader).Pos().y() < 0 ? STATIC_POS[0] : STATIC_POS[1];
        }
    }

    // 计算receiver、passvel和receivePos
    int lastReceiver = _receiver;
    if(_isFlat && Utils::IsInField(flatPassPoint)){
        _passPos = _receiverPos = flatPassPoint;
        _receiver = PASS::passMode ? ZCUDAModule::instance()->getBestFlatPassNum() : ZCUDAModule::instance()->getBestFlatShootNum();
        inValidPassCnt = 0;
        inValidShootCnt = 0;
    }
    else if(!_isFlat && Utils::IsInField(chipPassPoint)){
        _passPos = _receiverPos = chipPassPoint;
        _receiver = PASS::passMode ? ZCUDAModule::instance()->getBestChipPassNum() : ZCUDAModule::instance()->getBestChipShootNum();
        inValidPassCnt = 0;
        inValidShootCnt = 0;
    }
    else{
        _passPos = CGeoPoint(PARAM::Field::PITCH_LENGTH/2,0);
    }
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
    if(needRunPos() && _saoActionSwitch){

        if(_leaderPos.dist(_receiverPos) > 800 && _state == "Pass" && _canKick == true && RECALCULATE::recomputeCondition != "LEADER = RECEIVER"){
            double disToPenaltyArea = PARAM::Vehicle::V2::PLAYER_SIZE/1.5;
            std::vector<int> defender_list;
            CGeoPoint averageDefenderPos(0,0);
            int buffer = 800;

            for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++){
                const PlayerVisionT& enemy = _pVision->theirPlayer(i);
                if(!Utils::InTheirPenaltyArea(enemy.Pos(), 0) && Utils::InTheirPenaltyArea(enemy.Pos(), buffer)){
                defender_list.push_back(i);
                averageDefenderPos.fill(averageDefenderPos.x()+enemy.Pos().x(), averageDefenderPos.y()+enemy.Pos().y());
                }
            }

            averageDefenderPos.fill(averageDefenderPos.x() / defender_list.size(), averageDefenderPos.y() / defender_list.size());

            double penalty_x = PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH;
            double saoX = PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - disToPenaltyArea;
            double saoY = (0 - _passPos.y()) / (PARAM::Field::PITCH_LENGTH / 2 - _passPos.x()) * (penalty_x - _passPos.x()) + _passPos.y();

            if (abs(saoY) > PARAM::Field::PITCH_LENGTH / 2 + 300){
                RunPosModule::Instance()->generateRunPos(_pVision, _receiverPos, _leaderPos);//czk temp change
                _otherPos[0] = RunPosModule::Instance()->runPos(0);
                _otherPos[1] = RunPosModule::Instance()->runPos(1);
                _otherPos[2] = RunPosModule::Instance()->runPos(2);
                _lastUpdateRunPosCycle = _cycle;
                return;
            }

            saoY = (averageDefenderPos.y() - saoY) * 0.25 + saoY;

            CGeoPoint saoPosition(saoX, saoY);

            _otherPos[1] = saoPosition;

            GDebugEngine::Instance()->gui_debug_x(saoPosition,  COLOR_ORANGE);
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,-300), QString("pass position: x: %1, y: %2").arg(_passPos.x()).arg(_passPos.y()).toLatin1(), COLOR_ORANGE);
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, 0) , QString("calculate Sao Position: x: %1, y: %2").arg(saoPosition.x()).arg(saoPosition.y()).toLatin1(), COLOR_ORANGE);
            _lastUpdateRunPosCycle = _cycle;
            return;
        }
    }



     if (needRunPos()) {
//    if(1){
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2+200, PARAM::Field::PITCH_WIDTH/2-1600) , QString("Is New OtherPos Calculated: %1 ").arg(1).toLatin1(), COLOR_ORANGE);
        RunPosModule::Instance()->generateRunPos(_pVision, _receiverPos, _leaderPos);
        _otherPos[0] = RunPosModule::Instance()->runPos(0);
        _otherPos[1] = RunPosModule::Instance()->runPos(1);
        _otherPos[2] = RunPosModule::Instance()->runPos(2);
//        GDebugEngine::Instance()->gui_debug_msg(_otherPos[1], QString("OtherPos").toLatin1(), COLOR_CYAN);
        _lastUpdateRunPosCycle = _cycle;
//        otherPosChangeCnt++;
//        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2+200, PARAM::Field::PITCH_WIDTH/2-2200) , QString("Average otherPos change period: %1 ").arg(_cycle / otherPosChangeCnt).toLatin1(), COLOR_ORANGE);

    }
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
    if(_pVision->ourPlayer(_leader).Pos().x() > PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH){
        for (int i=1; i<PARAM::Field::MAX_PLAYER; i++) {
            const PlayerVisionT& enemy = _pVision->theirPlayer(i);
            if(!enemy.Valid()) continue;
            if(!Utils::InTheirPenaltyArea(enemy.Pos(), 0) && Utils::InTheirPenaltyArea(enemy.Pos(), PARAM::Vehicle::V2::PLAYER_SIZE*2)){
//                qDebug() << "their guard:" << i;
                valid = true;
                CVector enemyToMe = _pVision->ourPlayer(_leader).Pos() - enemy.Pos();
                CVector enemyToGoal = CGeoPoint(PARAM::Field::PITCH_LENGTH/2, 0) - enemy.Pos();
                double compensateAngle = Utils::Normalize(enemyToGoal.dir()  - enemyToMe.dir()) / (1 + COMPENSATE_RATIO);
                double finalAngle = enemyToMe.dir() + compensateAngle;
                _passPos = enemy.Pos() + Utils::Polar2Vector(PARAM::Vehicle::V2::PLAYER_SIZE, finalAngle);
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
    CGeoPoint leaderPos = _pVision->ourPlayer(_leader ).Pos();
    // leader在前场时传leader
    if(leaderPos.x() > 0)
        return leaderPos + CVector(500, 0);
    // 否则传到对方禁区前沿制造混乱
    else
        return CGeoPoint(PARAM::Field::PITCH_LENGTH / 2, 0);
}

// 判断leader状态
void CMessiDecision::judgeLeaderState() {
    //计算传球力度
    if(PASS::selfPass)
        _leaderState = "SELFPASS";
    else
        _leaderState = "PASS";
    if(Utils::InTheirPenaltyArea(_passPos, 0))
        _passVel = 6000;
    else if(_isFlat){
//        if(_leaderState == "SELFPASS")
//            _passVel = 1000.0;
//        else
            _passVel = BallModel::instance()->flatPassVel(_pVision, _passPos, _receiver, PASS::bufferTime, RECALCULATE::PASS_ANGLE_ERROR);
    }
    else
        _passVel = BallModel::instance()->chipPassVel(_pVision, _passPos);
    //判断leader应该调什么skill
    /*if(_leader == _receiver)
        _leaderState = "COMPUTE";
    else */

    //判断leader能否传球
    CGeoPoint leaderPosition = _pVision->ourPlayer(_leader ).Pos() + Utils::Polar2Vector(PARAM::Vehicle::V2::PLAYER_SIZE, _pVision->ourPlayer(_leader).Dir());
    CGeoPoint ballPosition = _pVision->ball().Pos();
    CVector passLine = _passPos - ballPosition;
//    CGeoPoint abnormalPos1 = ballPosition + Utils::Polar2Vector(ballPosition.dist(_passPos), passLine.dir() + RECALCULATE::PASS_ANGLE_ERROR*PARAM::Math::PI/180);
//    CGeoPoint abnormalPos2 = ballPosition + Utils::Polar2Vector(ballPosition.dist(_passPos), passLine.dir() - RECALCULATE::PASS_ANGLE_ERROR*PARAM::Math::PI/180);
    double interTime;
    _canKick = false;
    _canKickReceiver = false;



    const static double riskPosX = PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - 4*PARAM::Vehicle::V2::PLAYER_SIZE;
    const static double riskPassX = PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - 10*PARAM::Vehicle::V2::PLAYER_SIZE;

    //特殊情况放宽条件:禁区附近
    if(!_isFlat && Utils::InTheirPenaltyArea(_receiverPos, 1000) && _receiverPos.x() > riskPosX && leaderPosition.x() > riskPassX){
//        _canKick = true;
        _canKickReceiver = true;
    }
    //非自传的平射点
    else if(_isFlat && !PASS::selfPass && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, _receiverPos, interTime, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG)/* && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos1, interTime, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG) && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos2, interTime, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG)*/){
//        _canKick = true;
        _canKickReceiver = true;
    }
    //自传的平射点
    else if(_isFlat && PASS::selfPass && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, _receiverPos, interTime, 0.6, -9999, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG)/* && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos1, interTime, 0.6, -9999, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG) && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos2, interTime, 0.6, -9999, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG)*/){
//        _canKick = true;
        _canKickReceiver = true;
    }
    //挑射点
    else if (!_isFlat && ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, _receiverPos, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG) /*&& ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, abnormalPos1, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG) && ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, abnormalPos2, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG)*/) {
//        _canKick = true;
        _canKickReceiver = true;
    }

    //射门
    if(Utils::InTheirPenaltyArea(_passPos, 0)){
        if(ShootModule::Instance()->canShoot(_pVision, _leaderPos))
            _canKick = true;
    }
    else _canKick = _canKickReceiver;
//    //传球
//    else {
//        const static double riskPosX = PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - 4*PARAM::Vehicle::V2::PLAYER_SIZE;
//        const static double riskPassX = PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - 10*PARAM::Vehicle::V2::PLAYER_SIZE;
//        //特殊情况放宽条件:禁区附近
//        if(!_isFlat && Utils::InTheirPenaltyArea(_passPos, 1000) && _passPos.x() > riskPosX && leaderPosition.x() > riskPassX){
//            _canKick = true;
//        }
//        //非自传的平射点
//        else if(_isFlat && !PASS::selfPass && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, _passPos, interTime, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG)/* && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos1, interTime, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG) && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos2, interTime, RECALCULATE::THEIR_RESPONSE_TIME, RECALCULATE::IGNORE_CLOSE_ENEMY_DIST, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG)*/){
//            _canKick = true;
//        }
//        //自传的平射点
//        else if(_isFlat && PASS::selfPass && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, _passPos, interTime, 0.6, -9999, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG)/* && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos1, interTime, 0.6, -9999, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG) && ZSkillUtils::instance()->validShootPos(_pVision, leaderPosition, _passVel, abnormalPos2, interTime, 0.6, -9999, true, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG)*/){
//            _canKick = true;
//        }
//        //挑射点
//        else if (!_isFlat && ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, _passPos, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG) /*&& ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, abnormalPos1, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG) && ZSkillUtils::instance()->validChipPos(_pVision, leaderPosition, _passVel, abnormalPos2, RECALCULATE::THEIR_CHIP_RESPONSE_TIME, RECALCULATE::IGNORE_THEIR_GUARD, INTER_DEBUG)*/) {
//            _canKick = true;
//        }
//    }
    //判断传球点是否非法
    if((leaderPosition.dist(ballPosition) < HOLDBALL_DIST || RobotSensor::Instance()->IsInfraredOn(_leader)) && !_canKickReceiver){
        inValidPassCnt = inValidPassCnt >= RECALCULATE::MAX_INVALID_CNT ? RECALCULATE::MAX_INVALID_CNT : inValidPassCnt + 1;
    }
    else {
        inValidPassCnt = 0;
    }

    if((leaderPosition.dist(ballPosition) < HOLDBALL_DIST || RobotSensor::Instance()->IsInfraredOn(_leader)) && !_canKick){
        inValidShootCnt = inValidShootCnt >= RECALCULATE::MAX_INVALID_CNT ? RECALCULATE::MAX_INVALID_CNT : inValidShootCnt + 1;
    }
    else {
        inValidShootCnt = 0;
    }


    // 当receiver离挑球点比较近的时候踢出
    if((!_isFlat && !PASS::selfPass && _pVision->ourPlayer(_receiver).Pos().dist(_passPos) > 1500) || (_isFlat && !PASS::selfPass && !Utils::InTheirPenaltyArea(_passPos, 0) && predictedTime(_pVision->ourPlayer(_receiver), _passPos) > 1))
        _canKick = false;
    // 后场没点时强行挑射
//    if(!_isFlat && _pVision->ourPlayer(_leader).X() < /*-PARAM::Field::PITCH_LENGTH/8*/0 && Utils::InTheirPenaltyArea(_passPos, 50))
//        _canKick = true;

    // NO KICK计数
    if ((leaderPosition.dist(ballPosition) < HOLDBALL_DIST || RobotSensor::Instance()->IsInfraredOn(_leader)) && !_canKick) PASS::NO_KICK_CNT = PASS::NO_KICK_CNT < static_cast<int>(PARAM::Vision::FRAME_RATE * PASS::MAX_NO_KICK) ? PASS::NO_KICK_CNT + 1 : static_cast<int>(PARAM::Vision::FRAME_RATE * PASS::MAX_NO_KICK);
    else PASS::NO_KICK_CNT = 0;
    if (PASS::NO_KICK_CNT >= static_cast<int>(PARAM::Vision::FRAME_RATE * PASS::MAX_NO_KICK) || leaderPosition.x() <= -PARAM::Field::PITCH_LENGTH/3){
        _passPos = _receiverPos = CGeoPoint(PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - 200, 0);
        _passVel = BallModel::instance()->chipPassVel(_pVision, _passPos);
        _isFlat = false;
        _canKick = true;
    }
}
CGeoPoint CMessiDecision::freeKickPos() {
#ifdef USE_CUDA_MODULE
//    ZCUDAModule::instance()->calculateFreeKickPos();
    return ZCUDAModule::instance()->getBestFreeKickPos();
#else
    return CGeoPoint(PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - 20, 0);
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
    waitPos.fill(ballPos.x() > PARAM::Field::PITCH_LENGTH / 3 ? (ballPos.x() - PARAM::Field::PITCH_LENGTH / 12) : ballPos.x() / 2 + PARAM::Field::PITCH_LENGTH / 4, //(ballPos.x() * 3.0 / 4.0 + PARAM::Field::PITCH_LENGTH / 8)
                 std::fabs(ballPos.y()) < PARAM::Field::PENALTY_AREA_WIDTH / 2 ? 0 :
                 ballPos.y() > 0 ? PARAM::Field::PITCH_WIDTH / 4 : -PARAM::Field::PITCH_WIDTH / 4);
    RunPosModule::Instance()->generateRunPos(vision, waitPos, _leaderPos);
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
    return (WorldModel::Instance()->getEnemyAmountInArea(PARAM::Field::PITCH_LENGTH / 2 - PARAM::Field::PENALTY_AREA_DEPTH, PARAM::Field::PITCH_LENGTH / 2, - PARAM::Field::PENALTY_AREA_WIDTH / 2, PARAM::Field::PENALTY_AREA_WIDTH / 2, enemyNumVec) > 1)
        && std::fabs(ballPos.y()) < PARAM::Field::PITCH_WIDTH / 4
        && std::fabs(ballPos.x()) > PARAM::Field::PITCH_LENGTH / 4;
}

// add chip predict api in messi, gh
bool CMessiDecision::chipBallState(){
    bool flag = VisionModule::Instance()->ball().ChipBallState();
    return flag;
}

// add chip predict api in messi, gh
CGeoPoint CMessiDecision::chipPredictPos(){
    CGeoPoint p = VisionModule::Instance()->ball().BestChipPredictPos();
    return p;
}
