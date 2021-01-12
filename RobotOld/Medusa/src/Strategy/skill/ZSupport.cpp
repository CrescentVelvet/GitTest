#include "ZSupport.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "RobotSensor.h"
#include "skill/Factory.h"
#include "ControlModel.h"
#include <utils.h>
#include <DribbleStatus.h>
#include <ControlModel.h>
#include "TaskMediator.h"
#include "SkillUtils.h"

namespace {
    const int verbose = true;
    enum class TaskMode{
        BLOCKING,
        MARKING,
        ZMARKING,
        GETBALL,
        WAIT,
        NO_TASK
    };
    auto taskMode = TaskMode::NO_TASK;
    auto lastTaskMode = TaskMode::NO_TASK;
    const double DEBUG_TEXT_MOD = -50*10;
    const double DEBUG_TEXT_DIR = -PARAM::Math::PI/2.5;
    double FRICTION = 40;
}

CZSupport::CZSupport(){
    bool IS_SIMULATION;
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION,"Alert/IsSimulation",false);
    if (IS_SIMULATION)
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Sim",800);
    else
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Real",1520);

}

int CZSupport::getTheirSupport(const CVisionModule* pVision, const int oppNum, const double dist){
    int supportNum = -999;
    double min_dist = 9999*10;
    auto ball = pVision->ball();
    for(int i=0; i<PARAM::Field::MAX_PLAYER; i++){
        auto enemy = pVision->theirPlayer(i);
        if (!enemy.Valid()) continue;//不存在
        if (i==oppNum) continue;//是对方最佳player
        auto enemy2ball = enemy.Pos() - ball.RawPos();
        if(enemy2ball.mod() < dist && enemy2ball.mod() < min_dist){//在距离范围内，
            supportNum = i;
            min_dist = enemy2ball.mod();
        }
    }
    return supportNum;//小于0则不存在
}

void CZSupport::plan(const CVisionModule* pVision) {
    if (pVision->getCycle() - _lastCycle > PARAM::Vision::FRAME_RATE * 0.1) {
        setState(BEGINNING);
    }
    const MobileVisionT& ball = pVision->ball();
    int robotNum = task().executor;
    oppNum = ZSkillUtils::instance()->getTheirBestPlayer();
    int supportNum = -999;
    int leaderNum = MessiDecision::Instance()->messiRun() ? (MessiDecision::Instance()->leaderNum()) : ZSkillUtils::instance()->getOurBestPlayer();

    const PlayerVisionT& me = pVision->ourPlayer(robotNum);
    const PlayerVisionT& enemy = pVision->theirPlayer(oppNum);
    const PlayerVisionT& leader = pVision->ourPlayer(leaderNum);
//    GDebugEngine::Instance()->gui_debug_arc(leader.RawPos(), 30, 0.0f, 360.0f, COLOR_ORANGE);

    CVector me2ball = ball.RawPos() - me.RawPos();
    CVector me2enemy = enemy.Pos() - me.RawPos();
    CGeoPoint ourGoal = CGeoPoint(-PARAM::Field::PITCH_LENGTH/2, 0);
//    CGeoPoint theirGoal = CGeoPoint(PARAM::Field::PITCH_LENGTH/2, 0);
    CVector enemy2ball = ball.RawPos() - enemy.Pos();
    CVector leader2ball = ball.RawPos() - leader.RawPos();
    CVector enemy2me = me.RawPos() - enemy.Pos();
    CVector enemy2ourGoal = ourGoal - enemy.Pos();
    CVector enemy2leader = leader.RawPos() - enemy.Pos();

    int flag = PlayerStatus::ALLOW_DSS;
    taskMode = TaskMode::BLOCKING;
    /***************** judge *******************/
    if(enemy2ball.mod() > 100*10){
        taskMode = TaskMode::BLOCKING;
    }
    else if(me2ball.mod() > 150*10){
        taskMode = TaskMode::MARKING;
    }
    else{
        //我是最佳截球队员
        if(leaderNum == robotNum) taskMode = TaskMode::WAIT;

        supportNum = getTheirSupport(pVision, oppNum, 100*10);
        if(supportNum < 0){//对方没有support
            taskMode = TaskMode::WAIT;
        }
        else{//对方有support
            taskMode = TaskMode::ZMARKING;
        }
    }
    if(!enemy.Valid()) taskMode = TaskMode::NO_TASK;
    /***************** execute *******************/
    if (taskMode == TaskMode::MARKING){
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "MARKING", COLOR_ORANGE);

        CGeoSegment shootLine = CGeoSegment(enemy.Pos(), enemy.Pos()+Utils::Polar2Vector(1500*10, enemy.Dir()));
        CGeoPoint foot = shootLine.projection(me.Pos());
        bool intersectant = shootLine.IsPointOnLineOnSegment(foot);

        CGeoPoint gotoPoint = ball.Pos();
        if(intersectant){
            CGeoPoint interPoint = CGeoPoint(-9999*10, -999*10);
            double interceptTime = 9999;
            ZSkillUtils::instance()->predictedInterTimeV2(Utils::Polar2Vector(500*10, enemy.Dir()), enemy.Pos(), me, interPoint, interceptTime, 0);
            auto enemy2interPoint = interPoint - enemy.Pos();
            gotoPoint = interPoint;
            if((enemy.Pos()-foot).mod() < enemy2interPoint.mod()) gotoPoint = foot;
            if((foot-me.Pos()).mod() < 30*10) gotoPoint = ball.Pos();
            if(!Utils::IsInField(gotoPoint, 9*10)) gotoPoint = ball.Pos();
        }
        setSubTask(PlayerRole::makeItGoto(robotNum, gotoPoint, me2ball.dir(), flag));
    }

    if (taskMode == TaskMode::WAIT){//与我方leader有关的状态
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "WAIT", COLOR_GRAY);
        if(leaderNum == robotNum){//我变最佳截球车,防自己门
            double blockingDist = 20*10;
            auto blockingPoint = enemy.Pos() + Utils::Polar2Vector(blockingDist, enemy2ourGoal.dir());
            auto blockingVel = CVector(enemy.Vel().x(), enemy.Vel().y());
            setSubTask(PlayerRole::makeItGoto(robotNum, blockingPoint, me2enemy.dir(), blockingVel, 0, flag));
        }
        else{//我不是最佳截球车
            //leader离球较远时，紧盯，判转速，盯朝向
            if(leader2ball.mod() > 50*10){
                double blockingDist = 20*10;
                double rotDir = enemy.RotVel() > 0 ? 1.0 : -1.0;
                auto blockingPoint = enemy.Pos() + Utils::Polar2Vector(blockingDist, enemy.Dir());
                auto blockingVel = enemy.Vel() + Utils::Polar2Vector(enemy.RotVel()*blockingDist , enemy.Dir() + rotDir*PARAM::Math::PI/2);
                setSubTask(PlayerRole::makeItGoto(robotNum, blockingPoint, me2enemy.dir(), blockingVel, 0, flag));
            }
            else{//leader靠近球时
                //enemy朝向我但leader在另一侧，紧盯，判转速，盯朝向
                if (fabs(Utils::Normalize(enemy.Dir() - enemy2me.dir())) < PARAM::Math::PI/4 && fabs(Utils::Normalize(enemy2me.dir() - enemy2leader.dir())) > PARAM::Math::PI/2){
                    double blockingDist = 20*10;
                    double rotDir = enemy.RotVel() > 0 ? 1.0 : -1.0;
                    auto blockingPoint = enemy.Pos() + Utils::Polar2Vector(blockingDist, enemy.Dir());
                    auto blockingVel = enemy.Vel() + Utils::Polar2Vector(enemy.RotVel()*blockingDist , enemy.Dir() + rotDir*PARAM::Math::PI/2);
                    setSubTask(PlayerRole::makeItGoto(robotNum, blockingPoint, me2enemy.dir(), blockingVel, 0, flag));
                }
                else{//enemy不朝向我时不紧盯
                    double blockingDist = 50*10;
                    auto blockingPoint = enemy.Pos() + Utils::Polar2Vector(-blockingDist, enemy2leader.dir());
                    GDebugEngine::Instance()->gui_debug_line(enemy.Pos(), blockingPoint, COLOR_CYAN);
                    auto blockingVel = CVector(enemy.Vel().x(), enemy.Vel().y());
                    setSubTask(PlayerRole::makeItGoto(robotNum, blockingPoint, me2enemy.dir(), blockingVel, 0, flag));
                }
            }
        }
    }
    if (taskMode == TaskMode::BLOCKING){
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "BLOCKING", COLOR_RED);
        auto blockPoint = ZSkillUtils::instance()->getTheirInterPoint(oppNum);
        setSubTask(PlayerRole::makeItAMarkEnemy(robotNum, oppNum, blockPoint));
    }
    if (taskMode == TaskMode::ZMARKING){
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "ZMARKING", COLOR_YELLOW);
        setSubTask(PlayerRole::makeItZMarkEnemy(robotNum, supportNum));
    }
//    if (taskMode == TaskMode::GETBALL){
//        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "BLOCKING", COLOR_CYAN);
//        setSubTask(PlayerRole::makeItGetBallV4(robotNum, PlayerStatus::KICK + PlayerStatus::DRIBBLE, theirGoal, CGeoPoint(999, 999), 0));
//    }
    if(taskMode == TaskMode::NO_TASK){
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "NO_TASK", COLOR_YELLOW);
        setSubTask(PlayerRole::makeItGoto(robotNum, CGeoPoint(0, 0), me2ball.dir(),flag));
    }

    _lastCycle = pVision->getCycle();
    return CStatedTask::plan(pVision);
}

CPlayerCommand* CZSupport::execute(const CVisionModule* pVision) {
    if (subTask()) {
        return subTask()->execute(pVision);
    }
    return nullptr;
}
