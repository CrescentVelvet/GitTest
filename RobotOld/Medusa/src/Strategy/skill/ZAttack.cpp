#include "ZAttack.h"
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
#include "WorldModel.h"

namespace {
    const int verbose = true;
    double deltaTime = 0.2;
    enum class TaskMode{
        ZGET,
        ZPASS,
        ZBREAK,
        ZCIRCLE_PASS,
        ZBLOCKING,
        ZMARKING,
        NO_TASK
    };
    auto taskMode = TaskMode::NO_TASK;
    auto lastTaskMode = TaskMode::NO_TASK;
    const double DEBUG_TEXT_MOD = -50;
    const double DEBUG_TEXT_DIR = -Param::Math::PI/2.5;
    double FRICTION = 135;
    const int maxFrared = 100;
}

CZAttack::CZAttack() : fraredOn(0) {
    bool IS_SIMULATION;
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION,"Alert/IsSimulation",false);
    if (IS_SIMULATION)
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Sim",80);
    else
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Real",152);

}

void CZAttack::plan(const CVisionModule* pVision) {
    /******** get infomation for lua and vision *******/
    const auto& ball = pVision->Ball();
    const CGeoPoint passTarget = task().player.pos;
    const CGeoPoint waitPos(task().player.kickpower, task().player.chipkickpower);
    int robotNum = task().executor;
    const auto& me = pVision->OurPlayer(robotNum);
    int bestEnemyNum = ZSkillUtils::instance()->getTheirBestPlayer();
    const auto& enemy = pVision->TheirPlayer(bestEnemyNum);
    double power = task().player.rotdir;
    const int kick_flag = task().player.kick_flag;
    const int flag = PlayerStatus::ALLOW_DSS;
    // 传球精度控制,-1表示使用默认精度
    double precision = task().player.kickprecision > 0 ? task().player.kickprecision : -1;
    /******** get important variable *******/
    CGeoPoint ourGoal = CGeoPoint(-Param::Field::PITCH_LENGTH/2, 0);
    CVector me2ball = ball.Pos() - me.Pos();
    CVector me2enemy = enemy.Pos() - me.Pos();
//    CVector ball2target = passTarget - ball.Pos();
//    CVector ball2ourGoal = ourGoal - ball.Pos();
//    CVector me2target = passTarget - me.Pos();
    CVector enemy2ball = ball.Pos() - enemy.Pos();
    CVector enemy2ourGoal = ourGoal - enemy.Pos();

    bool frared = RobotSensor::Instance()->IsInfraredOn(robotNum);
    if(frared) fraredOn = fraredOn > maxFrared ? maxFrared : fraredOn + 1;
    else fraredOn = 0;
//    auto interPoint = ZSkillUtils::instance()->getOurInterPoint(robotNum);

    /*********** task *************/
//    TaskT getballTask(task());

    /********  judge task mode ************/
    auto ourTime = ZSkillUtils::instance()->getOurInterTime(robotNum);
    auto theirTime = ZSkillUtils::instance()->getTheirInterTime(bestEnemyNum);
    bool shootGoal = Utils::InTheirPenaltyArea(passTarget, 0);
//    bool enemyDribble = enemy2ball.mod() < Param::Vehicle::V2::PLAYER_SIZE + 5.0;
    bool enemyDribble = enemy2ball.mod() < Param::Vehicle::V2::PLAYER_SIZE * 2;
    double theta = KickDirection::Instance()->GenerateShootDir(robotNum, me.Pos());
    bool canShootGoal = (fabs(theta) > 3 * Param::Math::PI/180);
//    bool hasShoot = BallStatus::Instance()->IsBallKickedOut(robotNum);

    // our ball
    bool use_zpass = (me.X() > 350 && fabs(me.Y()) < 220) && !canShootGoal;
    if (ourTime + deltaTime < theirTime || fraredOn > 10){
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(2*DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "OurBall", COLOR_ORANGE);
        // shoot goal
        if (shootGoal){
            // can shoot goal
            if (canShootGoal){
                //zget
                taskMode = TaskMode::ZGET;
            }
            // cannot shoot goal
            else{
                //zpass new
                if (use_zpass) {
                    taskMode = TaskMode::ZPASS;
                }
                else {
                    taskMode = TaskMode::ZBREAK;
                }
            }
        }
        // pass ball
        else{
            // zget
            taskMode = theirTime < 0.50 ? TaskMode::ZBREAK : TaskMode::ZGET;
        }
        if(taskMode == TaskMode::ZGET && !Utils::InTheirPenaltyArea(me.Pos(), 15*Param::Vehicle::V2::PLAYER_SIZE)){
            static const double minBreakDist = 100;
            std::vector<int> enemyNumVec;
            if (WorldModel::Instance()->getEnemyAmountInArea(me.Pos(), minBreakDist, enemyNumVec, 0)) {
                for (auto enemyNum : enemyNumVec) {
                    if(Utils::InTheirPenaltyArea(pVision->TheirPlayer(enemyNum).Pos(), 0)) continue;
                    bool enemyInFront = fabs(Utils::Normalize((pVision->TheirPlayer(enemyNum).Pos() - me.Pos()).dir() - me.Dir())) < Param::Math::PI/2;
                    bool enemyTooClose = pVision->TheirPlayer(enemyNum).Pos().dist(me.Pos()) <= 3*Param::Vehicle::V2::PLAYER_SIZE;
                    if (enemyInFront || enemyTooClose){
                        taskMode = TaskMode::ZBREAK;
                        break;
                    }
                }
            }
        }
    }
    // their ball
    else{
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(2*DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "TheirBall", COLOR_ORANGE);
        bool enemyHoldBall = enemy2ball.mod() < 20 && fabs(Utils::Normalize(enemy.Dir() - enemy2ball.dir())) < Param::Math::PI/4;
        bool enemyFaceGoal = fabs(Utils::Normalize(enemy2ourGoal.dir() - enemy.Dir())) < Param::Math::PI/2;
//        if(ourTime - theirTime > -0.2){
//            taskMode = TaskMode::ZBREAK;
//        }
//        else {
//            taskMode = TaskMode::ZMARKING;
//        }
        if(enemyHoldBall && enemyFaceGoal && me2ball.mod() - enemy2ball.mod() > Param::Vehicle::V2::PLAYER_SIZE){
            taskMode = TaskMode::ZBLOCKING;
        }
        else{
            taskMode = TaskMode::ZBREAK;
        }
    }

    if(frared){
        if (lastTaskMode == TaskMode::ZPASS && shootGoal) {
            taskMode = TaskMode::ZPASS;
        }
        if (lastTaskMode == TaskMode::ZBREAK && !shootGoal) {
            taskMode = TaskMode::ZBREAK;
        }
    }
    lastTaskMode = taskMode;
//    taskMode = TaskMode::ZBLOCKING;
    /************ set sub-task *************/
    if (taskMode == TaskMode::ZGET){
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "ZGET", COLOR_RED);
        if(shootGoal) power = 630;
        setSubTask(PlayerRole::makeItGetBallV4(robotNum, kick_flag, passTarget, waitPos, power, precision));
    }
    if (taskMode == TaskMode::ZPASS){
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "ZPASS", COLOR_RED);
        setSubTask(PlayerRole::makeItZPass(robotNum, passTarget, power, kick_flag, precision));
    }
    if (taskMode == TaskMode::ZBREAK) {
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "ZBREAK", COLOR_RED);
        setSubTask(PlayerRole::makeItZBreak(robotNum, passTarget, power, kick_flag, precision));
    }
    if (taskMode == TaskMode::ZBLOCKING){
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "ZBLOCKING", COLOR_RED);
        double blockingDist = 20;//Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER + Param::Vehicle::V2::PLAYER_SIZE;
        double rotDir = enemy.RotVel() > 0 ? 1.0 : -1.0;
        auto blockingPoint = enemy.Pos() + Utils::Polar2Vector(blockingDist, enemy2ourGoal.dir());
        auto blockingVel = enemy.Vel() + Utils::Polar2Vector(enemy.RotVel()*blockingDist , enemy.Dir() + rotDir*Param::Math::PI/2);
        if (me2enemy.mod() < 90)
            setSubTask(PlayerRole::makeItGoto(robotNum, blockingPoint, me2enemy.dir(), blockingVel, 0, flag));
        else
            setSubTask(PlayerRole::makeItGoto(robotNum, blockingPoint, me2enemy.dir(), blockingVel, 0, flag));
    }
    if (taskMode == TaskMode::ZCIRCLE_PASS){
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "ZCIRCLE_PASS", COLOR_RED);
        setSubTask(PlayerRole::makeItZCirclePass(robotNum, passTarget, power, kick_flag));
    }
    if (taskMode == TaskMode::ZMARKING){
        if(verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_MOD, DEBUG_TEXT_DIR), "ZMARKING", COLOR_RED);

        CGeoSegment shootLine = CGeoSegment(enemy.Pos(), enemy.Pos()+Utils::Polar2Vector(1500, enemy.Dir()));
        CGeoPoint foot = shootLine.projection(me.Pos());
        bool intersectant = shootLine.IsPointOnLineOnSegment(foot);

        CGeoPoint gotoPoint = ball.Pos();
        if(intersectant){
            CGeoPoint interPoint = CGeoPoint(-9999, -999);
            double interceptTime = 9999;
            ZSkillUtils::instance()->predictedInterTimeV2(Utils::Polar2Vector(500, enemy.Dir()), enemy.Pos(), me, interPoint, interceptTime, 0);
            auto enemy2interPoint = interPoint - enemy.Pos();
            gotoPoint = interPoint;
            if((enemy.Pos()-foot).mod() < enemy2interPoint.mod()) gotoPoint = foot;
            if((foot-me.Pos()).mod() < 30) gotoPoint = ball.Pos();
            if(Utils::OutOfField(gotoPoint, 9)) gotoPoint = ball.Pos();
            if(Utils::InTheirPenaltyArea(gotoPoint, 32)) gotoPoint = Utils::MakeOutOfTheirPenaltyArea(gotoPoint, 32);
//            GDebugEngine::Instance()->gui_debug_arc(interPoint, 10, 0.0f, 360.0f, COLOR_ORANGE);
//            GDebugEngine::Instance()->gui_debug_line(me.Pos(), foot, COLOR_YELLOW);
//            GDebugEngine::Instance()->gui_debug_line(enemy.Pos(), enemy.Pos()+Utils::Polar2Vector(1500, enemy.Dir()), COLOR_RED);
        }
        setSubTask(PlayerRole::makeItGoto(robotNum, gotoPoint, me2ball.dir(), flag));
    }
    // force dribble -- mark
    DribbleStatus::Instance()->setDribbleCommand(robotNum, 3);
    /************** end ****************/
    _lastCycle = pVision->Cycle();
    return CStatedTask::plan(pVision);
}

CPlayerCommand* CZAttack::execute(const CVisionModule* pVision) {
    if (subTask()) {
        return subTask()->execute(pVision);
    }
    return nullptr;
}
