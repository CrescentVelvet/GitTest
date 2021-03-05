#include "GetBallV4.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <DribbleStatus.h>
#include "BallSpeedModel.h"
#include <RobotSensor.h>
#include <KickStatus.h>
#include "KickDirection.h"
#include "PlayInterface.h"
#include "TaskMediator.h"
#include "CMmotion.h"
#include "SkillUtils.h"
#include "WaitKickPos.h"
#include "Compensate.h"
#include "ShootModule.h"
#include "kickregulation.h"
#include <algorithm>
#include "staticparams.h"
#include "parammanager.h"


namespace {
    auto zpm = ZSS::ZParamManager::instance();
    double SHOOT_ACCURACY = 5.0;       //射门精度，角度制
    double PASS_ACCURACY = 3.0;       //射门精度，角度制
    double TOUCH_ACCURACY = 6.0;       //Touch射门精度，角度制
    double NORMAL_CHASE_DIR = 45.0;   //根据球踢出后偏移的角度判断是否要chase
    double NORMAL_TOUCH_DIR = 60.0;   //根据球踢出后偏移的角度判断是否要touch
    const double MIN_BALL_MOVING_VEL = 70*10;  // 球速小于这个值就算不在滚
    double RUSH_BALL_DIR = 24;       //车到球向量与球速线夹角小于这个值就rush
    double AVOID_PENALTY = 320;
    double AVOID_FIELD_SIDE = 100;

    double RUSH_ACC = 500.0*10;
    double RUSH_SPEED = 1200;
    double RUSH_ROTATE_ACC = 160;
    double STATIC_ROTATE_ACC = 200;
    bool FORCE_AVOID = true;

    const double directGetBallDist = 35*10;               // 直接冲上去拿球的距离
    const double directGetBallDirLimit = PARAM::Math::PI / 6;
    const double touchDist = 150*10;					   //在touchDist内能截球则Touch
    int FraredBuffer = 30;

    double FRICTION;

    const int WAIT_TOUCH = 0;
    const int STATIC = 1;
    const int INTER_TOUCH = 2;
    const int INTER = 3;
    const int CHASE = 4;
    const int RUSH = 5;
    const int CHIP = 6;

    bool IF_DEBUG = true;
    const double DEBUG_TEXT_HIGH = 50*10;
    double responseTime = 0;

    bool IS_SIMULATION;
    bool FlatParamTest;
    bool ChipParamTest;

    bool USE_CHIPSOLVER;
    double waittime_coef;
    double secpos_coef;
    const double DELTA_T = 8.0 / PARAM::Vision::FRAME_RATE;
    bool CHIP_GETBALL_DEBUG;
    const double ourResponseTime = 0.2;
}

CGetBallV4::CGetBallV4() {
    ZSS::ZParamManager::instance()->loadParam(IF_DEBUG, "Debug/ZGet", false);
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION, "Alert/IsSimulation", false);
    if (IS_SIMULATION)
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Sim",800);
    else
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Real",1520);

    ZSS::ZParamManager::instance()->loadParam(RUSH_SPEED, "GetBall/RushSpeed", 1200);
    ZSS::ZParamManager::instance()->loadParam(SHOOT_ACCURACY, "GetBall/ShootAccuracy", 5.0);
    ZSS::ZParamManager::instance()->loadParam(PASS_ACCURACY, "GetBall/PassAccuracy", 3.0);
    ZSS::ZParamManager::instance()->loadParam(TOUCH_ACCURACY, "GetBall/TouchAccuracy", 6.0);
    ZSS::ZParamManager::instance()->loadParam(NORMAL_TOUCH_DIR, "GetBall/TouchAngle", 70.0);
    ZSS::ZParamManager::instance()->loadParam(NORMAL_CHASE_DIR, "GetBall/ChaseAngle", 45.0);
    ZSS::ZParamManager::instance()->loadParam(RUSH_BALL_DIR, "GetBall/RushBallAngle", 30.0);
    ZSS::ZParamManager::instance()->loadParam(FORCE_AVOID, "GetBall/ForceAvoid", true);
    ZSS::ZParamManager::instance()->loadParam(responseTime, "GetBall/ResponseTime", 0.15);
    ZSS::ZParamManager::instance()->loadParam(AVOID_PENALTY, "GetBall/AvoidPenalty", 20.0*10);
    ZSS::ZParamManager::instance()->loadParam(FraredBuffer, "GetBall/FraredBuffer", 30);
    ZSS::ZParamManager::instance()->loadParam(FlatParamTest,"ZAutoFit/flatparamtest",false);
    ZSS::ZParamManager::instance()->loadParam(ChipParamTest,"ZAutoFit/chipparamtest",false);
    ZSS::ZParamManager::instance()->loadParam(USE_CHIPSOLVER, "chipsolver/USE_CHIPSOLVER_IN_MEDUSA", false);
    ZSS::ZParamManager::instance()->loadParam(waittime_coef,"chipsolver/waittime_coef",0.2);
    ZSS::ZParamManager::instance()->loadParam(secpos_coef,"chipsolver/secpos_coef",0.5);
    ZSS::ZParamManager::instance()->loadParam(CHIP_GETBALL_DEBUG, "chipsolver/CHIP_GETBALL_DEBUG", false);
    _lastCycle = 0;
    canForwardShoot = false;
    getBallMode = STATIC;
//    inters.clear();
}

void CGetBallV4::plan(const CVisionModule* pVision) {
    const int robotNum = task().executor;
    // new call
    if (pVision->getCycle() - _lastCycle > PARAM::Vision::FRAME_RATE * 0.1) {
        setState(BEGINNING);
        needAvoidBall = false;
        canGetBall = false;

        lastInterState = false;
//        hysteresisPredict(pVision, robotNum, interPoint, interTime, 0.1);
        interPoint = ZSkillUtils::instance()->getOurInterPoint(robotNum);
        interTime = ZSkillUtils::instance()->getOurInterTime(robotNum);
        chipInterPoint = CGeoPoint(99999,99999);

//        for(int i=0; i<FILTER_NUM; i++){
//            interTimes[i] = interTime;
//            interPointXs[i] = interPoint.x();
//            interPointYs[i] = interPoint.y();
//        }
//        inters.clear();
    }

    /******************视觉初步处理****************************/
    const MobileVisionT& ball = pVision->ball();
    const PlayerVisionT& me = pVision->ourPlayer(robotNum);

//    int power = task().player.rotdir;
    double power = task().player.rotdir;
    const CVector me2Ball = ball.RawPos() - me.RawPos();
    const CVector ball2Me = me.RawPos() - ball.RawPos();
    CGeoLine ballLine(ball.Pos(), ball.Vel().dir());
    ballLineProjection = ballLine.projection(me.RawPos() + Utils::Polar2Vector(PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, me.RawDir()));
    CVector ball2Projection = ballLineProjection - ball.Pos();
    targetPoint = task().player.pos;//目标点
//    CVector me2target = targetPoint - me.Pos();
    waitPoint = CGeoPoint(task().player.kickpower, task().player.chipkickpower);//等待截球点
    int goalieNumber = TaskMediator::Instance()->goalie();

    const int kick_flag = task().player.kick_flag;
    needkick = kick_flag & PlayerStatus::KICK;//是否需要射出球
    chip = kick_flag & PlayerStatus::CHIP;//是否挑射
    needdribble = kick_flag & PlayerStatus::DRIBBLE;
    safeMode = kick_flag & PlayerStatus::SAFE;
    rushMode = kick_flag & PlayerStatus::RUSH;
    ballplacement = kick_flag & PlayerStatus::NOT_AVOID_PENALTY;
   // qDebug()<<"czkdebug::getballv4 flag: "<<kick_flag;
    /*****************拿球Task初始化***************/
    TaskT getballTask(task());
    if(ballplacement){
        getballTask.player.flag |= PlayerStatus::NOT_AVOID_PENALTY;
    }

    bool shootGoal = Utils::InTheirPenaltyArea(targetPoint, 0);
    if(shootGoal && !ballplacement){//shoot goal
        power = 630*10;
        ShootModule::Instance()->generateBestTarget(pVision, targetPoint, me.RawPos());
    }
    getballTask.player.angle = (targetPoint - me.RawPos()).dir();
    if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_line(me.RawPos(), targetPoint, COLOR_RED);

    double finalDir = getballTask.player.angle;

    if(robotNum == goalieNumber || !FORCE_AVOID || (me2Ball.mod() < 100*10 && me.Vel().mod() < 200*10)){
        //nothing
    } else {
        getballTask.player.flag |= PlayerStatus::ALLOW_DSS;
        //GDebugEngine::Instance()->gui_debug_arc(me.Pos(), 20*10, 0.0f, 360.0f, COLOR_YELLOW);
    }
    if (Utils::IsInField(waitPoint)) {//若下发了wait的点，则尽量用touch
        isTouch = true;
    }

    // 传球精度控制
    double precision = task().player.kickprecision > 0 ? task().player.kickprecision
                                                       : shootGoal ? (SHOOT_ACCURACY) : PASS_ACCURACY;


    bool frared = RobotSensor::Instance()->IsInfraredOn(robotNum);
    fraredOn = RobotSensor::Instance()->fraredOn(robotNum);
    fraredOff = RobotSensor::Instance()->fraredOff(robotNum);
    /********* 判断方式 **********/
    judgeMode(pVision);
    /*****************决策执行********************/
    /*************** Touch **************/
    if (getBallMode == WAIT_TOUCH) {
//        getballTask.player.angle = (targetPoint - me.RawPos()).dir() * 0.75 + (ball.RawPos() - me.RawPos()).dir() * 0.25;
//        GDebugEngine::Instance()->gui_debug_line(me.RawPos(), me.RawPos()+Utils::Polar2Vector(99999, getballTask.player.angle), COLOR_RED);
//        double beta = Utils::Normalize(ball.Vel().dir() + PARAM::Math::PI - (targetPoint - me.RawPos()).dir());
//        double alpha = atan2(power * sin(beta) / 0.7 + 1e-5, ball.Vel().mod() + power * cos(beta) / 0.7 + 1e-5);
//        getballTask.player.angle = (targetPoint - me.RawPos()).dir() + beta - alpha;
        double inVel;
        if(ball.Vel().mod2() > FRICTION * ball2Me.mod())
            inVel = sqrt(ball.Vel().mod2() - FRICTION * ball2Me.mod());
        else
            inVel = ball.Vel().mod();
        double beta = Utils::Normalize(ball.Vel().dir() + PARAM::Math::PI - (targetPoint - me.RawPos()).dir());
        double alpha = atan2(power * sin(beta) / 0.55 + 1e-5, inVel + power * cos(beta) / 0.55 + 1e-5);
//        if(abs(beta) > PARAM::Math::PI / 6) alpha = atan2(power * sin(beta) / 0.3 + 1e-5, inVel + power * cos(beta) / 0.3 + 1e-5);
        getballTask.player.angle = (targetPoint - me.RawPos()).dir() + beta - alpha;

        CGeoLine ballVelLine(ball.Pos(), ball.Vel().dir());//球线
        double perpendicularDir = Utils::Normalize(ball.Vel().dir() + PARAM::Math::PI / 2);//垂直球线的方向
        CGeoLine perpLineAcrossMyPos(waitPoint, perpendicularDir);//过截球点的垂线
        CGeoPoint projectionPos = CGeoLineLineIntersection(ballVelLine, perpLineAcrossMyPos).IntersectPoint();//垂线与球线的交点
        double ballDist = (ball.RawPos() - projectionPos).mod();
        bool isBallMovingToWaitPoint = ball.Vel().mod() > 30*10;//球是30否在动
        bool canInterceptBall = false;
        //在touch半径内，且离场地边界9cm内，且球向waitpoint移动
        if (Utils::IsInFieldV2(projectionPos, PARAM::Vehicle::V2::PLAYER_SIZE) && projectionPos.dist(waitPoint) < touchDist && isBallMovingToWaitPoint) {
            double meArriveTime = predictedTime(me, projectionPos + Utils::Polar2Vector(-PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, getballTask.player.angle));
            if((me.RawPos() - (projectionPos + Utils::Polar2Vector(-PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, getballTask.player.angle))).mod() < 5*10) meArriveTime = 0;
            double ballArriveTime = 0;
            if (ball.Vel().mod2() / FRICTION > ballDist) {//球能到
                ballArriveTime = (ball.Vel().mod() - sqrt(ball.Vel().mod2() - FRICTION * ballDist)) / (FRICTION/2);
                if (meArriveTime < ballArriveTime) canInterceptBall = true;
            }
        }
        CVector interPoint2target =  targetPoint - projectionPos;
        double ballBias = fabs(Utils::Normalize(ball.Vel().dir() - interPoint2target.dir()));
        double ballAngleDiff = fabs(Utils::Normalize(ball.Vel().dir() + PARAM::Math::PI - interPoint2target.dir()));

        //加特判，不要用屁股截球
        if (canInterceptBall && ballAngleDiff > 90 * PARAM::Math::PI / 180){
            waitPoint = CGeoPoint(99999,99999);
            isTouch = false;
            judgeMode(pVision);
        }
        else if (canInterceptBall) {
            if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -PARAM::Math::PI/1.5), "Wait Touch", COLOR_YELLOW);
            getballTask.player.pos = projectionPos + Utils::Polar2Vector(-PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, getballTask.player.angle);
            if (Utils::InTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY))
                getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY,ball.Vel().dir());
            setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle, getballTask.player.flag));
        } else {
            isTouch = false;
            judgeMode(pVision);//随你咋办
        }
    }
    /***************** rush *****************/
    if(getBallMode == RUSH) {
        if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -PARAM::Math::PI/1.5), "Rush", COLOR_ORANGE);
        double angle = (ball.Pos() - me.Pos()).dir();
        if(!ball.Valid()) angle = (interPoint-me.RawPos()).dir();
        CVector ball2target = targetPoint - ball.RawPos();
        if (!Utils::InTheirPenaltyArea(targetPoint, PARAM::Vehicle::V2::PLAYER_SIZE)) {
            getballTask.player.angle = ball2target.dir();
        }
        getballTask.player.flag |= PlayerStatus::DRIBBLING;
        getballTask.player.flag |= PlayerStatus::RUSH;
        needdribble = true;
        if(fraredOn < 4){
            CVector vel = CVector(0,0);
            if(ball.Valid()) {
                double temp_angle  = ball2Me.dir();
                getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(-PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, temp_angle/*angle*/);
            }
            else getballTask.player.pos = interPoint + Utils::Polar2Vector(2*10, (interPoint-me.RawPos()).dir());
//            if(ball.Vel().mod() < 1000) vel = Utils::Polar2Vector(2000, ball.Vel().dir());
//            else vel = CVector(ball.VelX()*2, ball.VelY()*2);
            double NormalizeDistance = min(ball.Pos().dist(me.Pos()), 1500.0);
            double NormalizeVelocity = 2500*abs(me.Vel()*ball.Vel()/(ball.Vel().mod()*me.Vel().mod()+0.01));
            if(ball.Vel().mod()>300) vel = Utils::Polar2Vector(NormalizeDistance / 1500 * NormalizeVelocity +ball.Vel().mod(), ball.Vel().dir());
            else vel = Utils::Polar2Vector(RUSH_SPEED, angle);
            if(vel.mod() < RUSH_SPEED) vel = vel / vel.mod() * RUSH_SPEED;
            if(Utils::InTheirPenaltyArea(getballTask.player.pos, 80*10) || Utils::InOurPenaltyArea(getballTask.player.pos, 80*10)){
                vel = Utils::Polar2Vector(100*10, ball.Vel().dir());
            }
            if(!Utils::IsInField(getballTask.player.pos,PARAM::Vehicle::V2::PLAYER_SIZE) || !Utils::IsInField(ball.RawPos(),PARAM::Vehicle::V2::PLAYER_SIZE)){
                Utils::MakeInField(getballTask.player.pos,PARAM::Vehicle::V2::PLAYER_SIZE);
                vel = CVector(0,0);
            }
            if (Utils::InTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY)){
                getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY,ball.Vel().dir());
                vel = CVector(0,0);
            }
            if (robotNum != goalieNumber && Utils::InOurPenaltyArea(getballTask.player.pos,PARAM::Vehicle::V2::PLAYER_SIZE)){
                getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(ball.RawPos(),PARAM::Vehicle::V2::PLAYER_SIZE);
                vel = CVector(0,0);
            }
            if(IF_DEBUG) {
                GDebugEngine::Instance()->gui_debug_line(getballTask.player.pos,getballTask.player.pos+Utils::Polar2Vector(400,angle),10,1001001);
                GDebugEngine::Instance()->gui_debug_msg(me.Pos() + CVector(0, 100), QString("velocity: %1").arg(vel.mod()).toLatin1());
                GDebugEngine::Instance()->gui_debug_msg(me.Pos() + CVector(0, 200), QString("distance: %1").arg(ball.Pos().dist(me.Pos())).toLatin1());
            }
            setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, angle, vel,0, RUSH_ACC, RUSH_ROTATE_ACC, 400*10, 15,getballTask.player.flag));
        }else{
            angle = getballTask.player.angle;
            auto&& angle_diff = fabs(Utils::Normalize(me.Dir() - angle));
            double limit_angle_diff = Utils::limitRange(angle_diff,0.0,PARAM::Math::PI/4);
            auto&& bias_dir = Utils::Normalize(me.Dir()+limit_angle_diff+PARAM::Math::PI/3);
            auto&& p = PARAM::Vehicle::V2::PLAYER_FRONT_TO_CENTER;
            auto&& limit_dist = (180+p)*angle_diff-p;
            getballTask.player.pos = ball.Pos()+Utils::Polar2Vector(limit_dist, bias_dir);
            if (Utils::InTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY)){
                getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY);
            }
            int goalieNumber = TaskMediator::Instance()->goalie();
            if (robotNum != goalieNumber && Utils::InOurPenaltyArea(getballTask.player.pos,AVOID_PENALTY)){
                getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(getballTask.player.pos,AVOID_PENALTY);
            }
            //红外触发达到一定程度时，原地持球
            if(angle_diff<PARAM::Math::PI/10)
                getballTask.player.pos = me.Pos();
            if(IF_DEBUG) {
                GDebugEngine::Instance()->gui_debug_line(getballTask.player.pos,getballTask.player.pos+Utils::Polar2Vector(400,angle),10,255255255);
                GDebugEngine::Instance()->gui_debug_msg(getballTask.player.pos+Utils::Polar2Vector(400,angle),QString("d:%1 %2 %3").arg(angle_diff).arg(limit_angle_diff).arg(limit_dist).toLatin1(),10,255255255);
            }
            setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, angle,
                                              CVector(0,0),0,
                                              500*10, 50,
                                              500*10, 50,
                                              getballTask.player.flag));
        }
    }

    /***************** inter touch *****************/
    if(getBallMode == INTER_TOUCH) {
//        getballTask.player.angle = (targetPoint - me.RawPos()).dir() * 0.75 + (ball.RawPos() - me.RawPos()).dir() * 0.25;
//        GDebugEngine::Instance()->gui_debug_line(me.RawPos(), me.RawPos()+Utils::Polar2Vector(99999, getballTask.player.angle), COLOR_RED);
//        double beta = Utils::Normalize(ball.Vel().dir() + PARAM::Math::PI - (targetPoint - me.RawPos()).dir());
//        double alpha = atan2(power * sin(beta) / 0.7 + 1e-5, ball.Vel().mod() + power * cos(beta) / 0.7 + 1e-5);
//        getballTask.player.angle = (targetPoint - me.RawPos()).dir() + beta - alpha;
        double inVel;
        if(ball.Vel().mod2() > FRICTION * ball2Me.mod())
            inVel = sqrt(ball.Vel().mod2() - FRICTION * ball2Me.mod());
        else
            inVel = ball.Vel().mod();
        double beta = Utils::Normalize(ball.Vel().dir() + PARAM::Math::PI - (targetPoint - me.RawPos()).dir());
        double alpha = atan2(power * sin(beta) / 0.55 + 1e-5, inVel + power * cos(beta) / 0.55 + 1e-5);
//        if(abs(beta) > PARAM::Math::PI / 6) alpha = atan2(power * sin(beta) / 0.3 + 1e-5, inVel + power * cos(beta) / 0.3 + 1e-5);
        getballTask.player.angle = (targetPoint - me.RawPos()).dir() + beta - alpha;

        if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -PARAM::Math::PI/1.5), "Inter Touch", COLOR_CYAN);
        CVector projection2Me = me.RawPos() - ballLineProjection;
        getballTask.player.pos = interPoint + Utils::Polar2Vector(-PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, getballTask.player.angle);
        if (fabs(Utils::Normalize(ball2Me.dir() -  ball.Vel().dir())) < 30 * PARAM::Math::PI / 180 && (ball.RawPos() - me.RawPos()).mod() < 30*10 + ball.Vel().mod() * 0.25 && fabs(Utils::Normalize(ball2Projection.dir() - ball.Vel().dir())) < 0.1) {
            getballTask.player.pos = ballLineProjection + Utils::Polar2Vector(-PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, getballTask.player.angle);
        }
        if ((fabs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) < PARAM::Math::PI / 4) ||
                (fabs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) < PARAM::Math::PI / 2 && me2Ball.mod() <= 40*10)){//追在球屁股后面，且可能撞上球
            getballTask.player.pos = getballTask.player.pos + (projection2Me / projection2Me.mod() * 40*10);//跑到球的侧面
        }
        if (Utils::InTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY)){
            getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY,ball.Vel().dir());
        }
        if(me2Ball.mod() < 100*10){
            setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle,
                                              CVector(0,0),0,
                                              450*10, 30,
                                              300*10, 15,
                                              getballTask.player.flag));
        }
        else{
            setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle,getballTask.player.flag));
        }
    }
    /*************** chase kick **************/
    if (getBallMode == CHASE) {
        if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -PARAM::Math::PI/1.5), "Chase Kick", COLOR_RED);
        canForwardShoot = judgeShootMode(pVision);
        setSubTask(PlayerRole::makeItZChaseKick(robotNum, getballTask.player.angle));
    }

    /************inter get ball************/
    if (getBallMode == INTER) {
        CVector projection2Me = me.RawPos() - ballLineProjection;
        getballTask.player.angle = me2Ball.dir();//面向球截球
        getballTask.player.flag |= PlayerStatus::MIN_DSS; ///这种情况把dss开到最小，否则容易因为避障请不到球 by jsh 2019/6/27
        if (me.RawPos().dist(ballLineProjection) < 50*10 && me2Ball.mod()<150*10 && fabs(Utils::Normalize(ball2Projection.dir() - ball.Vel().dir()))<0.1) {
            if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -PARAM::Math::PI/1.5), "Intercept", COLOR_WHITE);
            getballTask.player.pos = ballLine.projection(me.RawPos());
        }
        //普通情况，计算接球点
        else {
            if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -PARAM::Math::PI/1.5), "Intercept", COLOR_RED);
            CVector interPoint2Ball = ball.RawPos() - interPoint;
            if(me2Ball.mod() > 80*10) getballTask.player.angle = interPoint2Ball.dir();//面向球截球
            else getballTask.player.angle = me2Ball.dir();//面向球截球
            if ((fabs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) < PARAM::Math::PI / 4) ||
                (fabs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) < PARAM::Math::PI / 2 && me2Ball.mod() <= 40*10))//追在球屁股后面，且可能撞上球
                getballTask.player.pos = interPoint + (projection2Me / projection2Me.mod() * 40*10);//跑到球的侧面
            else
                getballTask.player.pos = interPoint;
        }
        if (Utils::InTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY)){
            getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY,ball.Vel().dir());
        }
        if (robotNum != goalieNumber && Utils::InOurPenaltyArea(getballTask.player.pos,PARAM::Vehicle::V2::PLAYER_SIZE))
            getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(ball.RawPos(),PARAM::Vehicle::V2::PLAYER_SIZE);
        setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle, getballTask.player.flag));
    }

    /******************static get ball**************/
    if (getBallMode == STATIC) {
        needdribble = true;
//        GDebugEngine::Instance()->gui_debug_line(me.RawPos(), me.RawPos()+Utils::Polar2Vector(99999, getballTask.player.angle), COLOR_YELLOW);
        if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -PARAM::Math::PI/1.5), "Static", COLOR_WHITE);
        getballTask.player.flag |= PlayerStatus::MIN_DSS; ///这种情况把dss开到最小，否则容易因为避障请不到球 by jsh 2019/6/27
        if(!ballplacement && (Utils::InOurPenaltyArea(ball.RawPos(), 0) || (me2Ball.mod() < 50*10 && fabs(Utils::Normalize(me.RawDir() - me2Ball.dir())) > PARAM::Math::PI/8))) {
            double me2BallDirDiff = Utils::Normalize(me2Ball.dir() - finalDir);
            bool isInDirectGetBallCircle = me.RawPos().dist(ball.RawPos()) < directGetBallDist && me.RawPos().dist(ball.RawPos()) > PARAM::Vehicle::V2::PLAYER_SIZE + 5 * 10;    //是否在直接冲上去拿球距离之内
            bool isGetBallDirReached = fabs(me2BallDirDiff) < directGetBallDirLimit;
            canGetBall = isInDirectGetBallCircle && isGetBallDirReached;     //重要布尔量:是否能直接上前拿球
            bool fraredGetball = fraredOn > 10;
            if (!canGetBall && me2Ball.mod() < 25*10 && !fraredGetball) needAvoidBall = true;
            else needAvoidBall = false;
            staticDir = getStaticDir(pVision, staticDir);

            if (needAvoidBall) {
                getballTask.player.angle = me2Ball.dir();
                if (fabs(me2BallDirDiff) > PARAM::Math::PI / 3) {
                    double avoidDir = Utils::Normalize(ball2Me.dir() + staticDir * PARAM::Math::PI / 4);
                    getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(30*10, avoidDir);
                } else {
                    double directDist = PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER + PARAM::Field::BALL_SIZE + 1;
                    if (fabs(me2BallDirDiff) < 0.2) {
                        getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(directDist - 5*10, Utils::Normalize(finalDir - PARAM::Math::PI));
                    } else {
                        getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(directDist, Utils::Normalize(finalDir - PARAM::Math::PI));
                    }
                }
                if (Utils::InTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY)){
                    getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY);
                }
                if (robotNum != goalieNumber && !ballplacement &&Utils::InOurPenaltyArea(getballTask.player.pos,AVOID_PENALTY)) {
                    getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(ball.RawPos(),AVOID_PENALTY);
                }
                setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle,
                                                  CVector(0,0),0,
                                                  400*10, 30,
                                                  300*10, 60, //max speed
                                                  getballTask.player.flag));
            } else {
                if (fabs(me2BallDirDiff) > PARAM::Math::PI / 2) {
                    double gotoDir = Utils::Normalize(finalDir + staticDir * PARAM::Math::PI * 3.5 / 5);
                    getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(40*10, gotoDir);
                } else if(fabs(me2BallDirDiff) <  15 * PARAM::Math::PI / 180) {
                    getballTask.player.pos = ball.RawPos();
                } else {
                    double directDist = PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER  + PARAM::Field::BALL_SIZE - 1.5*10;
                    getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(directDist, Utils::Normalize(finalDir - PARAM::Math::PI));
                }
                if (Utils::InTheirPenaltyArea(getballTask.player.pos,PARAM::Vehicle::V2::PLAYER_SIZE)){
                    getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,PARAM::Vehicle::V2::PLAYER_SIZE);
                }
                if (robotNum != goalieNumber &&  !ballplacement &&Utils::InOurPenaltyArea(getballTask.player.pos,AVOID_PENALTY))
                    getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(ball.RawPos(),AVOID_PENALTY);
                setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle, getballTask.player.flag));
            }
        }
        else{
            double angle = me2Ball.dir();
            CVector ball2target = targetPoint - ball.RawPos();
            if (ballplacement || !Utils::InTheirPenaltyArea(targetPoint, PARAM::Vehicle::V2::PLAYER_SIZE)) {
                getballTask.player.angle = ball2target.dir();//me2target.dir();
            }
            getballTask.player.flag |= PlayerStatus::DRIBBLING;
            needdribble = true;
            if(fraredOn < FraredBuffer){
                getballTask.player.pos = ball.RawPos()+Utils::Polar2Vector(-PARAM::Vehicle::V2::PLAYER_FRONT_TO_CENTER, me2Ball.dir());
                angle = fraredOn < 1 ? me2Ball.dir(): getballTask.player.angle;
            }else{
                angle = getballTask.player.angle;
                double bias_dir = Utils::Normalize(me2Ball.dir()+PARAM::Math::PI/3);
                if(fabs(Utils::Normalize(me.RawDir() - angle)) < PARAM::Math::PI/6) bias_dir = angle;//Utils::Normalize(me2Ball.dir()+PARAM::Math::PI/6);//angle;
//                getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(20, bias_dir);
                //红外触发达到一定程度时，原地持球
                getballTask.player.pos = me.Pos();
            }

            if (!ballplacement && Utils::InTheirPenaltyArea(getballTask.player.pos,PARAM::Vehicle::V2::PLAYER_SIZE)){
                getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,PARAM::Vehicle::V2::PLAYER_SIZE);
            }
            int goalieNumber = TaskMediator::Instance()->goalie();
            if (robotNum != goalieNumber && Utils::InOurPenaltyArea(getballTask.player.pos,AVOID_PENALTY))
                getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(ball.RawPos(),AVOID_PENALTY);
            setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, angle,
                                              CVector(0,0),0,            // final speed
                                              400*10, STATIC_ROTATE_ACC,    // max acc
                                              400*10, 200,    // max speed
                                               getballTask.player.flag));
        }
    }
    /******************chip get ball**************/
    if(getBallMode == CHIP && USE_CHIPSOLVER){
        if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -PARAM::Math::PI/1.5), "Chip", COLOR_ORANGE);
        CVector me2ball = ball.Pos() - me.RawPos();
        if(ZSkillUtils::instance()->canGetChipResult()){
            CVector second2first = ball.BestChipPredictPos() - ball.SecondChipPos();
            getballTask.player.angle = second2first.dir();
            getballTask.player.pos = ZSkillUtils::instance()->getChipInterPoint(robotNum);
        }
        else {
            getballTask.player.angle = me2ball.dir();
            getballTask.player.pos = ZSkillUtils::instance()->getChipInterPoint(robotNum);
        }
        CGeoPoint tempInterPoint = ZSkillUtils::instance()->getOurInterPoint(robotNum);

        if(CHIP_GETBALL_DEBUG){
            GDebugEngine::Instance()->gui_debug_arc( tempInterPoint,150,0,360,COLOR_CYAN);
            GDebugEngine::Instance()->gui_debug_arc( tempInterPoint,100,0,360,COLOR_CYAN);
            GDebugEngine::Instance()->gui_debug_arc( getballTask.player.pos,150,0,360,COLOR_ORANGE);
            GDebugEngine::Instance()->gui_debug_arc( getballTask.player.pos,100,0,360,COLOR_ORANGE);
        }
        if (Utils::InTheirPenaltyArea(getballTask.player.pos, AVOID_PENALTY)){
            double tmpdir;
            if(ZSkillUtils::instance()->canGetChipResult()){
                CVector rollvel(ball.chipKickVel().x()*secpos_coef,ball.chipKickVel().y()*secpos_coef);
                tmpdir = rollvel.dir();
            }
            else
                tmpdir = me2ball.dir();
            getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY,tmpdir);
        }
        if (Utils::InOurPenaltyArea(getballTask.player.pos, AVOID_PENALTY)){
            getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(getballTask.player.pos,AVOID_PENALTY);
        }
        getballTask.player.flag |= PlayerStatus::RUSH;
        if(IF_DEBUG) {
            GDebugEngine::Instance()->gui_debug_line(getballTask.player.pos,me.RawPos(),COLOR_ORANGE);
        }
        setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle, getballTask.player.flag));
    }

    //是否吸球
    if (needdribble && (me2Ball.mod() < 100*10 || frared)) { //球在我前方则吸球
        DribbleStatus::Instance()->setDribbleCommand(robotNum, 3);
    }
//    GDebugEngine::Instance()->gui_debug_line(me.RawPos(), me.RawPos()+Utils::Polar2Vector(99999, getballTask.player.angle), COLOR_BLACK);
    //是否射门
//    GDebugEngine::Instance()->gui_debug_line(me.RawPos(), me.RawPos()+Utils::Polar2Vector(1000, getballTask.player.angle), COLOR_PURPLE);
//    GDebugEngine::Instance()->gui_debug_line(me.RawPos(), me.RawPos()+Utils::Polar2Vector(1000, me.RawDir()), COLOR_GREEN);
//    GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(1.5*DEBUG_TEXT_HIGH, -PARAM::Math::PI/2), QString("KP: %1").arg(power).toLatin1(), COLOR_ORANGE);
//    if(power > 500*10) power = power - 0.8 * std::max(0.0, me.RawVel().mod()*cos(fabs(Utils::Normalize(me.RawDir() - me.RawVel().dir()))));
//    GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(2*DEBUG_TEXT_HIGH, -PARAM::Math::PI/2), QString("new KP: %1").arg(power).toLatin1(), COLOR_ORANGE);
//    GDebugEngine::Instance()->gui_debug_line(me.RawPos(), me.RawPos()+Utils::Polar2Vector(99999, getballTask.player.angle), COLOR_RED);
    if(kick_flag & PlayerStatus::FORCE_KICK) KickStatus::Instance()->setKick(robotNum, power);
//    qDebug()<<"fuck"<<power<<needkick<<fabs((me.ImuDir() - getballTask.player.angle))<<fabs(Utils::Normalize(me.ImuDir() - getballTask.player.angle))<<precision<<"frared: "<<fraredOn;
    bool isParamTest = FlatParamTest || ChipParamTest;
//    if (needkick && isParamTest && getBallMode != INTER && getBallMode != WAIT_TOUCH && getBallMode != INTER_TOUCH && fabs(Utils::Normalize(me.Dir() - getballTask.player.angle)) < PASS_ACCURACY*PARAM::Math::PI/180.0) {
//        KickStatus::Instance()->setKick(robotNum, power);
//    }
//    else {
        if (needkick && getBallMode != INTER && getBallMode != WAIT_TOUCH && getBallMode != INTER_TOUCH) {
                if(!isParamTest) power = KickRegulation::instance()->regulateCheck(robotNum, power, getballTask.player.angle, chip, precision*PARAM::Math::PI/180.0);
                if(power > 0.1)
                {
                    if(!chip) {
                        KickStatus::Instance()->setKick(robotNum, power);
                    }
                    else if(fraredOn >= 20) {
                        KickStatus::Instance()->setChipKick(robotNum, power);
                    }
                }
            }
        else if (needkick && (getBallMode == WAIT_TOUCH || getBallMode == INTER_TOUCH || lastGetBallMode == INTER_TOUCH) && fabs(Utils::Normalize(me.Dir() - getballTask.player.angle)) < TOUCH_ACCURACY*PARAM::Math::PI/180.0) {
            if(!chip) KickStatus::Instance()->setKick(robotNum, power);
            else if(fraredOn >= 20) KickStatus::Instance()->setChipKick(robotNum, power);
        }

//    }


    _lastCycle = pVision->getCycle();
    lastGetBallMode = getBallMode;
    CStatedTask::plan(pVision);
}

CPlayerCommand* CGetBallV4::execute(const CVisionModule* pVision) {
    if (subTask()) return subTask()->execute(pVision);
    return nullptr;
}

int CGetBallV4::getStaticDir(const CVisionModule* pVision, int staticDir) {
    const MobileVisionT& ball = pVision->ball();
    const int robotNum = task().executor;
    const PlayerVisionT& me = pVision->ourPlayer(robotNum);
    double ball2MeDir = (me.RawPos() - ball.RawPos()).dir();
    const CVector me2Target = targetPoint - me.RawPos();
    double finalDir = me2Target.dir();
    double tmp2FinalDirDiff = Utils::Normalize(ball2MeDir - finalDir);
    if (!staticDir) staticDir = tmp2FinalDirDiff > 0 ? 1 : -1;
    else {
        if (staticDir == 1) {
            if (tmp2FinalDirDiff < -0.5) staticDir = -1;
        } else if (tmp2FinalDirDiff > 0.5) staticDir = 1;
    }
    return staticDir;
}

void CGetBallV4::judgeMode(const CVisionModule * pVision) {
    const MobileVisionT& ball = pVision->ball();
    const int robotNum = task().executor;
    const PlayerVisionT& me = pVision->ourPlayer(robotNum);
    bool frared = RobotSensor::Instance()->IsInfraredOn(robotNum);
    const int kick_flag = task().player.kick_flag;
    /************** special judge *******************/
    if (kick_flag & PlayerStatus::NO_RUSH){
        getBallMode = STATIC;
        return;
    }
    if (ball.Vel().mod() < MIN_BALL_MOVING_VEL || (lastGetBallMode==STATIC && fraredOff < 5)) {
        getBallMode = rushMode ? RUSH : STATIC;
        if(! ball.Valid()&& (me.RawPos()-ball.RawPos()).mod()<80 && !frared){
            getBallMode = STATIC;
        }
        return;
    } else {
        getBallMode = INTER; //as default
    }
    if (isTouch) {//Touch优先级最高,进行一次判断，若不能touch，则isTouch为false
        getBallMode = WAIT_TOUCH;
        return;
    }

    if(USE_CHIPSOLVER){
        if(ZSkillUtils::instance()->isInFlyTime()){
            getBallMode = CHIP;
            return;
        }
    }
    /************** normal judge *******************/
//    interPoint = ZSkillUtils::instance()->getOurInterPoint(robotNum);
//    interTime = ZSkillUtils::instance()->getOurInterTime(robotNum);
//    hysteresisPredict(pVision, robotNum, interPoint, interTime, 0.1);
    interPoint = ZSkillUtils::instance()->getOurInterPoint(robotNum);
    interTime = ZSkillUtils::instance()->getOurInterTime(robotNum);
//    meanFilter(robotNum, interTime, interPoint);
//    if(IF_DEBUG){
////        for(int i=0; i<FILTER_NUM; i++){
////            GDebugEngine::Instance()->gui_debug_x(CGeoPoint(interPointXs[i], interPointYs[i]), COLOR_ORANGE);
////            GDebugEngine::Instance()->gui_debug_arc(CGeoPoint(interPointXs[i], interPointYs[i]), i, 0.0f, 360.0f, COLOR_ORANGE);
////        }
////        GDebugEngine::Instance()->gui_debug_x(interPoint, COLOR_YELLOW);
//        GDebugEngine::Instance()->gui_debug_arc(interPoint, 8*10, 0.0f, 360.0f, COLOR_YELLOW);
//    }

//    double interPointJump = sqrt((interPointXs[FILTER_NUM-1]-interPointXs[FILTER_NUM-2])*(interPointXs[FILTER_NUM-1]-interPointXs[FILTER_NUM-2])
//            + (interPointYs[FILTER_NUM-1]-interPointYs[FILTER_NUM-2])*(interPointYs[FILTER_NUM-1]-interPointYs[FILTER_NUM-2]));


//    if(interPointJump > 30*10)
////    if(Utils::IsInField(interPoint, 9))
//        lastInterState = true;
//    else lastInterState = false;

    CVector interPoint2target = targetPoint - interPoint;
//    GDebugEngine::Instance()->gui_debug_x(targetPoint);
//    GDebugEngine::Instance()->gui_debug_msg(targetPoint,"t",COLOR_GREEN);
//    GDebugEngine::Instance()->gui_debug_x(interPoint);
//    GDebugEngine::Instance()->gui_debug_msg(interPoint,"i",COLOR_GREEN);
//    GDebugEngine::Instance()->gui_debug_line(ball.Pos(),ball.Pos()+Utils::Polar2Vector(1000,ball.Vel().dir()),COLOR_GREEN);
    double ballBias = fabs(Utils::Normalize(ball.Vel().dir() - interPoint2target.dir()));
    double ballAngleDiff = fabs(Utils::Normalize(ball.Vel().dir() + PARAM::Math::PI - interPoint2target.dir()));
    if(fraredOn < 4 && ballAngleDiff < (NORMAL_TOUCH_DIR+10) * PARAM::Math::PI / 180) { // May touch
//        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,1000),QString("%1").arg(ballAngleDiff / PARAM::Math::PI * 180).toLatin1(),COLOR_GREEN);
        if((lastGetBallMode == INTER_TOUCH || lastGetBallMode == INTER) && ballAngleDiff > (NORMAL_TOUCH_DIR-10) * PARAM::Math::PI / 180){
            getBallMode = lastGetBallMode;
        }else{
            getBallMode = INTER_TOUCH;
        }
    } else if(ballBias < (NORMAL_CHASE_DIR / 180)*PARAM::Math::PI && !safeMode) { // May chase
        getBallMode = CHASE;
    }
    if(fabs(Utils::Normalize(ball.Vel().dir() - (ball.RawPos() - me.RawPos()).dir())) < RUSH_BALL_DIR *PARAM::Math::PI /180.0){
        getBallMode = RUSH;
        return;
    }
    if(lastGetBallMode==RUSH && (!ball.Valid() || (ball.RawPos() - me.RawPos()).mod() < 30*10)){
        getBallMode = RUSH;
    }
    return;
}

bool CGetBallV4::judgeShootMode(const CVisionModule * pVision) {
    const MobileVisionT& ball = pVision->ball();
    const int robotNum = task().executor;
    const PlayerVisionT& me = pVision->ourPlayer(robotNum);
    const CVector me2Target = targetPoint - me.RawPos();
    double finalDir = me2Target.dir();
    double ballVel2FinalDiff = Utils::Normalize(ball.Vel().dir() - finalDir);

    bool shootMode = fabs(ballVel2FinalDiff) < 0.5;
    return shootMode;
}

//todo put these function into skillutils!!!!!!!!!
bool CGetBallV4::predictedChipInterTime(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, CGeoPoint secondPoint, CVector3 chipKickVel,double firstChipTime,double RestChiptime, double responseTime) {
    const MobileVisionT ball = pVision->ball();//获得球
    const PlayerVisionT me = pVision->ourPlayer(robotNum);//获得车
    static const double ballAcc = FRICTION*3 / 4;//球减速度
    const CGeoPoint lastInterceptPoint = interceptPoint;
    CVector rollvel(ball.chipKickVel().x()*secpos_coef,ball.chipKickVel().y()*secpos_coef);
    if (!me.Valid()) {//车不存在
        interTime = 99999;
        interceptPoint = CGeoPoint(99999, 99999);
        return false;
    }
    if(rollvel.mod() < 300) {
        interceptPoint = secondPoint;//截球点
        interTime = predictedTime(me, secondPoint);//截球时间
        return true;
    }
    double ballArriveTime = 0;
    double meArriveTime = 9999;
    double testBallLength = 0;//球移动距离
    CGeoPoint testPoint = secondPoint;
    double testVel = rollvel.mod();
    double max_time = rollvel.mod() / ballAcc;
    CGeoLine ballLine(secondPoint, rollvel.dir());

    CGeoPoint ballLineProjection = ballLine.projection(me.Pos());
    CVector projection2me = me.Pos() - ballLineProjection;
    double width = projection2me.mod() < PARAM::Vehicle::V2::PLAYER_SIZE ? projection2me.mod() : PARAM::Vehicle::V2::PLAYER_SIZE;

    // test availability of last point
    bool useLastPoint = false;
    CGeoPoint last_point_ballLineProjection = ballLine.projection(lastInterceptPoint);
    auto dist = (secondPoint - last_point_ballLineProjection).mod();
    auto maxRollingDist = rollvel.mod2()/(2*ballAcc);
    auto res_time = 0.0;
    auto diffTime = 0.0;
    auto chipWaitTime = RestChiptime + firstChipTime* waittime_coef;
    if(dist < maxRollingDist){
        // a = ballAcc;b = -2*testVel;c = 2*dist; res = (-b-sqrt(b^2-4ac))/2a = ((V-sqrt(V2-2AD))/A)
        auto& V = testVel;
        auto& A = ballAcc;
        auto& D = dist;
        res_time = (V-sqrt(V*V-2*A*D))/A + chipWaitTime;
        auto arrTime = predictedTime(me, lastInterceptPoint);
        if(arrTime < 0.2) arrTime = 0;
        diffTime = arrTime+responseTime-res_time;
        if(diffTime < 0.8*DELTA_T && diffTime > -1.5*DELTA_T){
            useLastPoint = true;
            interceptPoint = last_point_ballLineProjection;
            interTime = std::max(arrTime,res_time);
        }
    }
    if(CHIP_GETBALL_DEBUG){
//        GDebugEngine::Instance()->gui_debug_x(last_point_ballLineProjection,0);
//        GDebugEngine::Instance()->gui_debug_msg(last_point_ballLineProjection,QString("dd: %1 %2 %3 %4 %5").arg(res_time,0,'f',2).arg(diffTime,0,'f',2).arg(useLastPoint ? "T" : "F").arg(dist,0,'f',2).arg(maxRollingDist,0,'f',2).toLatin1(),0,0,200);
//        GDebugEngine::Instance()->gui_debug_msg(last_point_ballLineProjection,"LP",8,0,500);
         GDebugEngine::Instance()->gui_debug_msg(last_point_ballLineProjection,QString("vel_x: %1 vel_y: %2").arg(rollvel.x()).arg(rollvel.y()).toLatin1(),COLOR_RED);
         GDebugEngine::Instance()->gui_debug_msg(last_point_ballLineProjection+CVector(0,-200),QString("resttime:%1 waittime:%2").arg(RestChiptime).arg(chipWaitTime).toLatin1(),COLOR_RED);

    }


    // old point not available, calculate new point using violence search
    if(!useLastPoint){
        for (ballArriveTime = 0; ballArriveTime < max_time; ballArriveTime += DELTA_T) {
            testVel = rollvel.mod() - ballAcc * ballArriveTime; //v_0-at
            testBallLength = PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER + (rollvel.mod() + testVel) * ballArriveTime / 2; //梯形法计算球移动距离
            testPoint = secondPoint + Utils::Polar2Vector(testBallLength, rollvel.dir());
            CVector me2testPoint = testPoint - me.RawPos();
            meArriveTime = predictedTime(me, testPoint + Utils::Polar2Vector(width, projection2me.dir()));//我到截球点的时间
            if(meArriveTime < 0.2) meArriveTime = 0;
            if(me.Vel().mod() < 1500 /*&& projection2me.mod() < 2*PARAM::Vehicle::V2::PLAYER_SIZE*/ && me2testPoint.mod() < 3*PARAM::Vehicle::V2::PLAYER_SIZE) meArriveTime = 0;
//            if(CHIP_GETBALL_DEBUG){
//                GDebugEngine::Instance()->gui_debug_x(testPoint,4);
//                GDebugEngine::Instance()->gui_debug_msg(testPoint,QString("t: %1 , %2").arg(meArriveTime,0,'f',2).arg(ballArriveTime,0,'f',2).toLatin1(),10,0,10);
//            }
            if (!Utils::IsInField(testPoint) || (meArriveTime + responseTime) < (ballArriveTime + chipWaitTime)) {
                break;
            }
        }
        interceptPoint = testPoint;
        interTime = predictedTime(me, interceptPoint);//截球时间
    }
//    // 避免快接到球的时候截球点突然往后跳

//    if (me.Pos().dist(ball.Pos()) < PARAM::Vehicle::V2::PLAYER_SIZE*std::max(5.0,testVel/500) && projection2me.mod() < 2*PARAM::Vehicle::V2::PLAYER_SIZE && fabs(Utils::Normalize(ball.Vel().dir() - (ballLineProjection-ball.Pos()).dir())) < PARAM::Math::PI/12){
//        interceptPoint = ballLineProjection;
//        interTime = predictedTime(me, interceptPoint);//截球时间
//        if(CHIP_GETBALL_DEBUG){
//            GDebugEngine::Instance()->gui_debug_x(interceptPoint,3);
//            GDebugEngine::Instance()->gui_debug_msg(interceptPoint,"NP",8,0,400);
//        }
//    }

    bool getTmpInterFlag = false;
    int priority = 0;
    CGeoPoint tmpInterPoint;
    // if enemy can intercept ball faster, go to enemy instead of ball
    double minDist = 999999;
    for(int i=0; i<PARAM::Field::MAX_PLAYER; i++){
        auto enemy = pVision->theirPlayer(i);
        if(!enemy.Valid()) continue;
        CVector enemy2ballLine = ballLine.projection(enemy.Pos()) - enemy.Pos();
        CVector ball2enemy = enemy.Pos() - secondPoint;
        CVector firstpos2enemy = enemy.Pos() - ball.BestChipPredictPos();
        // whether need to inter ball for blocking enemy if enemy is more suitable to get ball
        if(enemy2ballLine.mod() < 300){
            // the enemy is behind the second pos, make player directly to second pos, priority = 2
            if(fabs(Utils::Normalize(rollvel.dir() - ball2enemy.dir())) < PARAM::Math::PI/2){
                if(priority <= 2){
                    getTmpInterFlag = true;
                    // first required enemy, set tmp = interceptPoint
                    if(priority != 2)
                        tmpInterPoint = interceptPoint;
                    // once enemy's projection is more closer than interceptPoint, then make the player directly goto secondpos
                    if((ballLine.projection(enemy.Pos()) - secondPoint).mod() < (interceptPoint - secondPoint).mod()){
                        tmpInterPoint = secondPoint;
                    }
                    priority = 2;
                }
            }
            // the enemy is before the first pos, then make the player goto intercept point, priority = 1
            else if(fabs(Utils::Normalize(rollvel.dir() - firstpos2enemy.dir())) > PARAM::Math::PI/2){
                if(priority <= 1){
                    getTmpInterFlag = false;
                    tmpInterPoint = interceptPoint;
                    priority = 1;
                }
            }
            // the enemy is between the first pos and second pos
            else{
                CVector firstpos2ballpro = ballLine.projection(enemy.Pos()) - ball.BestChipPredictPos();
                double t = firstpos2ballpro.mod() / rollvel.mod();
                double meet_z = chipKickVel.z() * secpos_coef * t - 1.0 / 2 * 9810 * t * t;
                // if ball will hit the enemy, just make player move before enemy, priority = 3
                if(meet_z < PARAM::Vehicle::V2::PLAYER_HEIGHT){
                    if(priority <= 3){
                        getTmpInterFlag = true;
                        tmpInterPoint = enemy.Pos() + Utils::Polar2Vector(PARAM::Vehicle::V2::PLAYER_SIZE * 2, (ball.BestChipPredictPos() - enemy.Pos()).dir());
                        priority = 3;
                    }
                }
                // if ball will not hit the enemy, then make the player goto intercept point
                else{
                    if(priority <= 1){
                        getTmpInterFlag = false;
                        tmpInterPoint = interceptPoint;
                        priority = 1;
                    }
                }
            }
        }
        else {
            getTmpInterFlag = false;
        }

        if(enemy2ballLine.mod() < 1200 && (fabs(Utils::Normalize(rollvel.dir() - ball2enemy.dir()))<PARAM::Math::PI/6) && enemy.Vel().mod() < 2000 && (ball2enemy.mod() <rollvel.mod2()/FRICTION)){
            minDist = std::min(minDist, ball2enemy.mod());
        }
    }

    if(minDist<888888 && getTmpInterFlag == false){
        CGeoPoint new_interceptPoint = secondPoint + Utils::Polar2Vector(std::max(minDist-PARAM::Vehicle::V2::PLAYER_SIZE,0.0), rollvel.dir());
        double new_interTime = predictedTime(me, new_interceptPoint);
        if((secondPoint-new_interceptPoint).mod() < (secondPoint-interceptPoint).mod()){
            interTime = new_interTime;
            interceptPoint = new_interceptPoint;
        }
    }
    else if(getTmpInterFlag == true){
        interTime = predictedTime(me, tmpInterPoint);
        interceptPoint = tmpInterPoint;
    }

    return true;
}
//void CGetBallV4::meanFilter(int robotNum, double &interTime, CGeoPoint &interPoint){
//    for(int i=0; i < FILTER_NUM - 1; i++){
//        interTimes[i] = interTimes[i+1];
//        interPointXs[i] = interPointXs[i+1];
//        interPointYs[i] = interPointYs[i+1];
//    }

//    interTimes[FILTER_NUM-1] =interTime;
//    interPointXs[FILTER_NUM-1] = interPoint.x();
//    interPointYs[FILTER_NUM-1] = interPoint.y();

//    if(!inters.empty() && inters.size() >= FILTER_NUM) {
//        inters.pop_front();
//    }
//    interInfo inter;
//    inter.interTimes = interTime;
//    inter.interPointXs = interPoint.x();
//    inter.interPointYs = interPoint.y();
//    inters.push_back(inter);
//    inters.sort();
//    if(inters.size() > 5) {
//        inters.pop_back();
//        inters.pop_back();
//        inters.pop_front();
//        inters.pop_front();
//    } else if(inters.size() > 3) {
//        inters.pop_back();
//        inters.pop_front();
//    }

//    interTime = 0.0;
//    double x = 0.0, y = 0.0;
//    for(auto itor : inters) {
//        interTime += itor.interTimes;
//        x += itor.interPointXs;
//        y += itor.interPointYs;
//    }
//    interTime /= inters.size();
//    interPoint.setX(x / inters.size());
//    interPoint.setY(y / inters.size());
//}

//bool CGetBallV4::hysteresisPredict(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, double buffer) {
//    const MobileVisionT ball = pVision->ball();//获得球
//    const PlayerVisionT me = pVision->ourPlayer(robotNum);//获得车
//    if (!me.Valid()) {//车不存在
//        interTime = 99999;
//        interceptPoint = CGeoPoint(9999, 9999);
////        lastInterState[robotNum] = false;// not change state
//        return false;
//    }
//    if(ball.Vel().mod() < 40*10) {
//        interceptPoint = ball.Pos();//截球点
//        interTime = predictedTime(me, interceptPoint);//截球时间
//        //lastInterState[robotNum] = true;
//        return true;
//    }
//    double ballAcc = FRICTION / 2;//球减速度
//    double ballArriveTime = 0;
//    double meArriveTime = 9999;
//    double testBallLength = 0;//球移动距离
//    CGeoPoint testPoint = ball.Pos();
//    double testVel = ball.Vel().mod();
//    double max_time = ball.Vel().mod() / ballAcc;
//    CGeoLine ballLine(ball.Pos(), ball.Vel().dir());
//    CGeoPoint ballLineProjection = ballLine.projection(me.Pos());
//    CVector projection2me = me.Pos() - ballLineProjection;
////    double width = projection2me.mod() < PARAM::Vehicle::V2::PLAYER_SIZE ? projection2me.mod() : PARAM::Vehicle::V2::PLAYER_SIZE;
//    for (ballArriveTime = 0; ballArriveTime < max_time; ballArriveTime += 1.0 / (PARAM::Vision::FRAME_RATE * 2) ) {
//        testVel = ball.Vel().mod() - ballAcc * ballArriveTime; //v_0-at
//        testBallLength = (ball.Vel().mod() + testVel) * ballArriveTime / 2; //梯形法计算球移动距离
//        testPoint = ball.Pos() + Utils::Polar2Vector(testBallLength, ball.Vel().dir());
//        CVector me2testPoint = testPoint - me.RawPos();
//        meArriveTime = predictedTime(me, testPoint);//我到截球点的时间
//        if(meArriveTime < 0.3) meArriveTime = 0;
//        if(me.Vel().mod() < 100*10 && projection2me.mod() < 15*10 && me2testPoint.mod() < 15*10) meArriveTime = 0;
////        if(isBall2Me && projection2me.mod() < 10) meArriveTime = 0;
//        if (!Utils::IsInField(testPoint)){// ball out of field
//            //lastInterState[robotNum] = false;
//            break;
//        }
//        if (meArriveTime + buffer + responseTime< ballArriveTime){
//            //lastInterState[robotNum] = true;
//            break;
//        }
//        if (meArriveTime + buffer + responseTime> ballArriveTime && meArriveTime + responseTime< ballArriveTime){
//            if(lastInterState == true){
//                //lastInterState[robotNum] = true;
//                break;
//            }
//        }
//    }
//    interceptPoint = testPoint;//截球点
//    interTime = predictedTime(me, interceptPoint);//截球时间
//    return true;
//}
