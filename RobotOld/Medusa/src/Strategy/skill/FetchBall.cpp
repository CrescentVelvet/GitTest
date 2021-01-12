#include "FetchBall.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "RobotSensor.h"
#include "skill/Factory.h"
#include <utils.h>
#include <DribbleStatus.h>
#include "TaskMediator.h"
#include "SkillUtils.h"
#include "parammanager.h"

namespace {
    const int verbose = true;
    double SHOOT_ACCURACY = 2 * Param::Math::PI / 180;
    int fraredOn = 0;
    int fraredOff = 0;
    constexpr double DEBUG_TEXT_HIGH = 50;
    int BPFraredOn = 30;
}

CFetchBall::CFetchBall(){
    ZSS::ZParamManager::instance()->loadParam(BPFraredOn, "GetBall/FraredBuffer", 30);
}

void CFetchBall::plan(const CVisionModule* pVision) {
    if (pVision->getCycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1) {
        setState(BEGINNING);
        fraredOn = 0;
        fraredOff = 0;
        goBackBall = true;
        notDribble = false;
        MIN_DIST = 3;
        cnt = 0;
    }
    notDribble = false;
    const MobileVisionT& ball = pVision->ball();
    const CGeoPoint target = task().player.pos;
    int vecNumber = task().executor;
    const PlayerVisionT& me = pVision->ourPlayer(vecNumber);
    double power = task().player.kickpower;
    int kick_flag = task().player.kick_flag;
    bool SAFEMODE = kick_flag & PlayerStatus::SAFE;

    CVector me2ball = ball.RawPos() - me.RawPos();
    CVector me2target = target - me.RawPos();
    bool frared = RobotSensor::Instance()->IsInfraredOn(vecNumber);
    if (frared) {
        fraredOn = fraredOn >= 1024 ? 1024 : fraredOn + 1;
        fraredOff = 0;
    } else {
        fraredOn = 0;
        fraredOff = fraredOff >= 1024 ? 1024 : fraredOff + 1;
    }
    /*********************** set subTask ********************/
    if(ball.Valid() && Utils::OutOfField(ball.RawPos(), 2*Param::Vehicle::V2::PLAYER_SIZE)){//ball out of field
        if(fraredOn < 2*BPFraredOn){//not have ball
            if(goBackBall){
//                CGeoPoint gotoPoint = Utils::MakeInField(ball.RawPos(), 20*Param::Vehicle::V2::PLAYER_SIZE);
                CGeoPoint gotoPoint = ball.RawPos();
                if((me.RawPos() - gotoPoint).mod() < 2){
                    goBackBall = false;
                }
                if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "GoBackBall", COLOR_WHITE);
                GDebugEngine::Instance()->gui_debug_msg(me.Pos(),"1111111111");
                setSubTask(PlayerRole::makeItGoto(vecNumber, gotoPoint, me2ball.dir(), CVector(0, 0), 0, 900, 5, 1000, 5, PlayerStatus::ALLOW_DSS|PlayerStatus::NOT_AVOID_PENALTY));
            }
            else{
                if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "GotoBall", COLOR_WHITE);
                GDebugEngine::Instance()->gui_debug_msg(me.Pos(),"2222222222");
                setSubTask(PlayerRole::makeItGoto(vecNumber, ball.RawPos(), me2ball.dir(), CVector(0, 0), 0, 900, 5, 1000, 5, PlayerStatus::ALLOW_DSS|PlayerStatus::NOT_AVOID_PENALTY));
            }
        }
        else{//have ball
            if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "MakeInField", COLOR_WHITE);
            GDebugEngine::Instance()->gui_debug_msg(me.Pos(),"3333333333");
            setSubTask(PlayerRole::makeItGoto(vecNumber, Utils::MakeInField(me.RawPos(), 500), me.RawDir(), CVector(0, 0), 0, 900, 5, 1000, 5, PlayerStatus::ALLOW_DSS|PlayerStatus::NOT_AVOID_PENALTY));
//            setSubTask(PlayerRole::makeItGoto(vecNumber, target + Utils::Polar2Vector(-Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, me2target.dir()), me2target.dir(), CVector(0, 0), 0, 300, 5, 300, 5, PlayerStatus::ALLOW_DSS|PlayerStatus::NOT_AVOID_PENALTY));
        }
    }
    else{
        if(fraredOn < 2*BPFraredOn && cnt == 0){//not have ball
            if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "GetBall", COLOR_WHITE);
            GDebugEngine::Instance()->gui_debug_msg(me.Pos(),"4444444444");
            setSubTask(PlayerRole::makeItGoto(vecNumber, ball.RawPos(), me2ball.dir(), CVector(0, 0), 0, 900, 50, 1000, 50, PlayerStatus::ALLOW_DSS|PlayerStatus::NOT_AVOID_PENALTY));
        }
        else{//have ball
            if(!SAFEMODE){
                if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "KickBall", COLOR_WHITE);
                GDebugEngine::Instance()->gui_debug_msg(me.Pos(),"5555555555");
                setSubTask(PlayerRole::makeItGoto(vecNumber, me.RawPos()+Utils::Polar2Vector(30, me2target.dir()), me2target.dir(), CVector(0, 0), 0, 900, 50, 3000, 50, PlayerStatus::ALLOW_DSS|PlayerStatus::NOT_AVOID_PENALTY));
                if (fabs(Utils::Normalize(me.RawDir() - me2target.dir())) < SHOOT_ACCURACY){
                    KickStatus::Instance()->setKick(vecNumber, power);
                }
            }
            else{
                if( (ball.Valid() && ((ball.RawPos() - target).mod() < MIN_DIST)) ||
                        (!ball.Valid() && ((me.RawPos()+Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, me.RawDir()) - target).mod()< MIN_DIST))
                        ){
                    notDribble = true;
                    if(cnt > 10) MIN_DIST = 10;
//                    if(cnt > 30){
//                        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Return", COLOR_WHITE);
//                        MIN_DIST = 10;
//                        setSubTask(PlayerRole::makeItGoto(vecNumber, target + Utils::Polar2Vector(-50, me2target.dir()), me2target.dir(), CVector(0, 0), 0, 300, 50, 300, 50, PlayerStatus::ALLOW_DSS|PlayerStatus::NOT_AVOID_PENALTY));
//                    }
//                    else{
                        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Stop", COLOR_WHITE);
//                        setSubTask(PlayerRole::makeItGoto(vecNumber, me.RawPos(), me.RawDir(), CVector(0, 0), 0, 300, 50, 300, 50, PlayerStatus::ALLOW_DSS|PlayerStatus::NOT_AVOID_PENALTY));
                        GDebugEngine::Instance()->gui_debug_msg(me.Pos(),"6666666666");
                        setSubTask(PlayerRole::makeItGoto(vecNumber, me.RawPos(), me2target.dir(), CVector(0, 0), 0, 900, 20, 200, 10, PlayerStatus::ALLOW_DSS|PlayerStatus::NOT_AVOID_PENALTY));
//                    }
                }
                else{
                    if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "PushBall", COLOR_WHITE);
                    GDebugEngine::Instance()->gui_debug_msg(me.Pos(),"7777777777");
                    setSubTask(PlayerRole::makeItGoto(vecNumber, target + Utils::Polar2Vector(-Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, me2target.dir()), me2target.dir(), CVector(0, 0), 0, 900, 20, 3000, 10, PlayerStatus::ALLOW_DSS|PlayerStatus::NOT_AVOID_PENALTY));
                }
            }
        }
    }
    if(notDribble) cnt++;
    else cnt = 0;
    if(cnt > 15) WorldModel::Instance()->setBPFinish(true);
    GDebugEngine::Instance()->gui_debug_msg(me.RawPos()+Utils::Polar2Vector(DEBUG_TEXT_HIGH*2, -Param::Math::PI/1.5),QString("%1").arg(cnt).toLatin1(),COLOR_BLACK);
    /****************** other ***************/
//    if (cnt < 30 && (me2ball.mod() < 50 || frared)) {
    if ((me2ball.mod() < 50 || frared) && cnt < 10) {
        DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
    }
    /******************************/
    _lastCycle = pVision->getCycle();
    return CStatedTask::plan(pVision);
}

CPlayerCommand* CFetchBall::execute(const CVisionModule* pVision) {
    if (subTask()) {
        return subTask()->execute(pVision);
    }
    return nullptr;
}
