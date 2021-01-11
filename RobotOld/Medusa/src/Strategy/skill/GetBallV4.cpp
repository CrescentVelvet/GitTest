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
#include <algorithm>

namespace {
    double SHOOT_ACCURACY = 3.0;       //射门精度，角度制
    double TOUCH_ACCURACY = 6.0;       //Touch射门精度，角度制
    double NORMAL_CHASE_DIR = 45.0;   //根据球踢出后偏移的角度判断是否要chase
    double NORMAL_TOUCH_DIR = 60.0;   //根据球踢出后偏移的角度判断是否要touch
    const double MIN_BALL_MOVING_VEL = 70;  // 球速小于这个值就算不在滚
    double RUSH_BALL_DIR = 30;       //车到球向量与球速线夹角小于这个值就rush
    double AVOID_PENALTY = 20;

    double RUSH_ACC = 500.0;
    double RUSH_ROTATE_ACC = 40;
    double STATIC_ROTATE_ACC = 50;
    bool FORCE_AVOID = true;

    const double directGetBallDist = 35;               // 直接冲上去拿球的距离
    const double directGetBallDirLimit = Param::Math::PI / 6;
    const double touchDist = 150;					   //在touchDist内能截球则Touch
    int FraredBuffer = 30;

    double FRICTION;

    const int WAIT_TOUCH = 0;
    const int STATIC = 1;
    const int INTER_TOUCH = 2;
    const int INTER = 3;
    const int CHASE = 4;
    const int RUSH = 5;

    bool IF_DEBUG = true;
    const double DEBUG_TEXT_HIGH = 50;
    double responseTime = 0;

    bool IS_SIMULATION;
}

CGetBallV4::CGetBallV4() {
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION, "Alert/IsSimulation", false);
    if (IS_SIMULATION)
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Sim",80);
    else
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Real",152);

    ZSS::ZParamManager::instance()->loadParam(SHOOT_ACCURACY, "GetBall/ShootAccuracy", 3.0);
    ZSS::ZParamManager::instance()->loadParam(TOUCH_ACCURACY, "GetBall/TouchAccuracy", 6.0);
    ZSS::ZParamManager::instance()->loadParam(NORMAL_TOUCH_DIR, "GetBall/TouchAngle", 90.0);
    ZSS::ZParamManager::instance()->loadParam(NORMAL_CHASE_DIR, "GetBall/ChaseAngle", 45.0);
    ZSS::ZParamManager::instance()->loadParam(RUSH_BALL_DIR, "GetBall/RushBallAngle", 30.0);
    ZSS::ZParamManager::instance()->loadParam(FORCE_AVOID, "GetBall/ForceAvoid", true);
    ZSS::ZParamManager::instance()->loadParam(responseTime, "GetBall/ResponseTime", 0.15);
    ZSS::ZParamManager::instance()->loadParam(AVOID_PENALTY, "GetBall/AvoidPenalty", 20.0);
    ZSS::ZParamManager::instance()->loadParam(FraredBuffer, "GetBall/FraredBuffer", 30);

    _lastCycle = 0;
    canForwardShoot = false;
    getBallMode = STATIC;
    inters.clear();
}

void CGetBallV4::plan(const CVisionModule* pVision) {
    const int robotNum = task().executor;
    // new call
    if (pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1) {
        setState(BEGINNING);
        needAvoidBall = false;
        canGetBall = false;

        lastInterState = false;
//        hysteresisPredict(pVision, robotNum, interPoint, interTime, 0.1);
        interPoint = ZSkillUtils::instance()->getOurInterPoint(robotNum);
        interTime = ZSkillUtils::instance()->getOurInterTime(robotNum);
        for(int i=0; i<FILTER_NUM; i++){
            interTimes[i] = interTime;
            interPointXs[i] = interPoint.x();
            interPointYs[i] = interPoint.y();
        }
        inters.clear();
    }
    /******************视觉初步处理****************************/
    const MobileVisionT& ball = pVision->Ball();
    const PlayerVisionT& me = pVision->OurPlayer(robotNum);

    int power = task().player.rotdir;
    const CVector me2Ball = ball.RawPos() - me.RawPos();
    const CVector ball2Me = me.RawPos() - ball.RawPos();
    CGeoLine ballLine(ball.Pos(), ball.Vel().dir());
    ballLineProjection = ballLine.projection(me.RawPos() + Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, me.RawDir()));
    CVector ball2Projection = ballLineProjection - ball.Pos();
    targetPoint = task().player.pos;//目标点
//    CVector me2target = targetPoint - me.Pos();
    waitPoint = CGeoPoint(task().player.kickpower, task().player.chipkickpower);//等待截球点
    int goalieNumber = PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie());
    // 传球精度控制
    double precision = task().player.kickprecision > 0 ? task().player.kickprecision : SHOOT_ACCURACY;

    const int kick_flag = task().player.kick_flag;
    needkick = kick_flag & PlayerStatus::KICK;//是否需要射出球
    chip = kick_flag & PlayerStatus::CHIP;//是否挑射
    needdribble = kick_flag & PlayerStatus::DRIBBLE;
    safeMode = kick_flag & PlayerStatus::SAFE;

    /*****************拿球Task初始化***************/
    TaskT getballTask(task());
//    if(!needkick){
//        getballTask.player.angle = me2Ball.dir(); //只截球，不射
//    }
//    else{
        bool canShoot = false;
        if(Utils::InTheirPenaltyArea(targetPoint, 0)){//shoot goal
            canShoot = ShootModule::Instance()->generateShootDir(robotNum);
            getballTask.player.angle = ShootModule::Instance()->getDir();
            power = 650;
        }
        else{//pass ball
            getballTask.player.angle = Compensate::Instance()->getKickDir(robotNum, targetPoint);
        }
//    }

    double finalDir = getballTask.player.angle;

    if(robotNum == goalieNumber || !FORCE_AVOID || (me2Ball.mod() < 100 && me.Vel().mod() < 200)){
        //nothing
    } else {
        getballTask.player.flag |= PlayerStatus::ALLOW_DSS;
        GDebugEngine::Instance()->gui_debug_arc(me.Pos(), 20, 0.0f, 360.0f, COLOR_YELLOW);
    }
    if (ZSkillUtils::instance()->IsInField(waitPoint)) {//若下发了wait的点，则尽量用touch
        isTouch = true;
    }

    bool frared = RobotSensor::Instance()->IsInfraredOn(robotNum);
    fraredOn = RobotSensor::Instance()->fraredOn(robotNum);
    fraredOff = RobotSensor::Instance()->fraredOff(robotNum);

    /********* 判断方式 **********/
    judgeMode(pVision);
    /*****************决策执行********************/
    /*************** Touch **************/
    if (getBallMode == WAIT_TOUCH) {
        CGeoLine ballVelLine(ball.Pos(), ball.Vel().dir());//球线
        double perpendicularDir = Utils::Normalize(ball.Vel().dir() + Param::Math::PI / 2);//垂直球线的方向
        CGeoLine perpLineAcrossMyPos(waitPoint, perpendicularDir);//过截球点的垂线
        CGeoPoint projectionPos = CGeoLineLineIntersection(ballVelLine, perpLineAcrossMyPos).IntersectPoint();//垂线与球线的交点
        double ballDist = (ball.RawPos() - projectionPos).mod();
        bool isBallMovingToWaitPoint = ball.Vel().mod() > 30;//球是30否在动
        bool canInterceptBall = false;
        //在touch半径内，且离场地边界9cm内，且球向waitpoint移动
        if (ZSkillUtils::instance()->IsInFieldV2(projectionPos, Param::Vehicle::V2::PLAYER_SIZE) && projectionPos.dist(waitPoint) < touchDist && isBallMovingToWaitPoint) {
            double meArriveTime = predictedTime(me, projectionPos + Utils::Polar2Vector(-Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, getballTask.player.angle));
            if((me.RawPos() - (projectionPos + Utils::Polar2Vector(-Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, getballTask.player.angle))).mod() < 5) meArriveTime = 0;
            double ballArriveTime = 0;
            if (ball.Vel().mod2() / FRICTION > ballDist) {//球能到
                ballArriveTime = (ball.Vel().mod() - sqrt(ball.Vel().mod2() - FRICTION * ballDist)) / (FRICTION/2);
                if (meArriveTime < ballArriveTime) canInterceptBall = true;
            }
        }
        CVector interPoint2target =  targetPoint - projectionPos;
        double ballBias = fabs(Utils::Normalize(ball.Vel().dir() - interPoint2target.dir()));
        //加特判，不要用屁股截球
        if (canInterceptBall && Param::Math::PI - ballBias > 90 * Param::Math::PI / 180){
            waitPoint = CGeoPoint(9999,9999);
            isTouch = false;
            judgeMode(pVision);
        }
        else if (canInterceptBall) {
            if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Wait Touch", COLOR_YELLOW);
            getballTask.player.pos = projectionPos + Utils::Polar2Vector(-Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, getballTask.player.angle);
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
        if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Rush", COLOR_ORANGE);
        double angle = (ball.RawPos() - me.RawPos()).dir();
        if(!ball.Valid()) angle = (interPoint-me.RawPos()).dir();
        CVector ball2target = targetPoint - ball.RawPos();
        if (!Utils::InTheirPenaltyArea(targetPoint, Param::Vehicle::V2::PLAYER_SIZE)) {
            getballTask.player.angle = ball2target.dir();
        }
        getballTask.player.flag |= PlayerStatus::DRIBBLING;
        needdribble = true;
        if(fraredOn < 5){
            CVector vel = CVector(0,0);
            if(ball.Valid()) getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(2, angle);
            else getballTask.player.pos = interPoint + Utils::Polar2Vector(2, (interPoint-me.RawPos()).dir());
//            if(ball.Vel().mod() < 200) vel = Utils::Polar2Vector(300, ball.Vel().dir());
//            else vel = CVector(ball.VelX()*2, ball.VelY()*2);
            vel = Utils::Polar2Vector(300, ball.Vel().dir());
            if(Utils::InTheirPenaltyArea(getballTask.player.pos, 80) || Utils::InOurPenaltyArea(getballTask.player.pos, 80)) vel = Utils::Polar2Vector(100, ball.Vel().dir());

            if(Utils::OutOfField(getballTask.player.pos,Param::Vehicle::V2::PLAYER_SIZE) || Utils::OutOfField(ball.RawPos(),Param::Vehicle::V2::PLAYER_SIZE)){
                Utils::MakeInField(getballTask.player.pos,Param::Vehicle::V2::PLAYER_SIZE);
                vel = CVector(0,0);
            }
            if (Utils::InTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY)){
                getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY,ball.Vel().dir());
                vel = CVector(0,0);
            }
            if (robotNum != goalieNumber && Utils::InOurPenaltyArea(getballTask.player.pos,Param::Vehicle::V2::PLAYER_SIZE)){
                getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(ball.RawPos(),Param::Vehicle::V2::PLAYER_SIZE);
                vel = CVector(0,0);
            }
            setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, angle, vel,0, RUSH_ACC, RUSH_ROTATE_ACC, 400, 15,getballTask.player.flag));
        }else{
            angle = getballTask.player.angle;
            double bias_dir = Utils::Normalize(me2Ball.dir()+Param::Math::PI/3);
            if(fabs(Utils::Normalize(me.RawDir() - angle)) < Param::Math::PI/6) bias_dir = angle;//Utils::Normalize(me2Ball.dir()+Param::Math::PI/6);//angle;
            getballTask.player.pos = ball.Pos() + Utils::Polar2Vector(20, bias_dir);
            if (Utils::InTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY)){
                getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY,ball.Vel().dir());
            }
            int goalieNumber = PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie());
            if (robotNum != goalieNumber && Utils::InOurPenaltyArea(getballTask.player.pos,Param::Vehicle::V2::PLAYER_SIZE))
                getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(ball.RawPos(),Param::Vehicle::V2::PLAYER_SIZE);
            //红外触发达到一定程度时，原地持球
            getballTask.player.pos = me.Pos();
            setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, angle,
                                              CVector(0,0),0,
                                              500, 50,
                                              500, 50,
                                              getballTask.player.flag));
        }
    }

    /***************** inter touch *****************/
    if(getBallMode == INTER_TOUCH) {
        if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Inter Touch", COLOR_CYAN);
        CVector projection2Me = me.RawPos() - ballLineProjection;
        getballTask.player.pos = interPoint + Utils::Polar2Vector(-Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, getballTask.player.angle);
        if (fabs(Utils::Normalize(ball2Me.dir() -  ball.Vel().dir())) < 30 * Param::Math::PI / 180 && (ball.RawPos() - me.RawPos()).mod() < 30 + ball.Vel().mod() * 0.25 && fabs(Utils::Normalize(ball2Projection.dir() - ball.Vel().dir())) < 0.1) {
            getballTask.player.pos = ballLineProjection + Utils::Polar2Vector(-Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, getballTask.player.angle);
        }
        if ((fabs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) < Param::Math::PI / 4) ||
                (fabs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) < Param::Math::PI / 2 && me2Ball.mod() <= 40)){//追在球屁股后面，且可能撞上球
            getballTask.player.pos = getballTask.player.pos + (projection2Me / projection2Me.mod() * 40);//跑到球的侧面
        }
        if (Utils::InTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY)){
            getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY,ball.Vel().dir());
        }
        if(me2Ball.mod() < 100){
            setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle,
                                              CVector(0,0),0,
                                              450, 30,
                                              300, 15,
                                              getballTask.player.flag));
        }
        else{
            setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle,getballTask.player.flag));
        }
    }
    /*************** chase kick **************/
    if (getBallMode == CHASE) {
        if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Chase Kick", COLOR_RED);
        canForwardShoot = judgeShootMode(pVision);
        setSubTask(PlayerRole::makeItZChaseKick(robotNum, getballTask.player.angle));
    }

    /************inter get ball************/
    if (getBallMode == INTER) {
        CVector projection2Me = me.RawPos() - ballLineProjection;
        getballTask.player.angle = me2Ball.dir();//面向球截球
        getballTask.player.flag |= PlayerStatus::MIN_DSS; ///这种情况把dss开到最小，否则容易因为避障请不到球 by jsh 2019/6/27
        if (me.RawPos().dist(ballLineProjection) < 50 && me2Ball.mod()<150 && fabs(Utils::Normalize(ball2Projection.dir() - ball.Vel().dir()))<0.1) {
            if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Intercept", COLOR_WHITE);
            getballTask.player.pos = ballLine.projection(me.RawPos());
        }
        //普通情况，计算接球点
        else {
            if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Intercept", COLOR_RED);
            CVector interPoint2Ball = ball.RawPos() - interPoint;
            if(me2Ball.mod() > 80) getballTask.player.angle = interPoint2Ball.dir();//面向球截球
            else getballTask.player.angle = me2Ball.dir();//面向球截球
            if ((fabs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) < Param::Math::PI / 4) ||
                (fabs(Utils::Normalize(me2Ball.dir() - ball.Vel().dir())) < Param::Math::PI / 2 && me2Ball.mod() <= 40))//追在球屁股后面，且可能撞上球
                getballTask.player.pos = interPoint + (projection2Me / projection2Me.mod() * 40);//跑到球的侧面
            else
                getballTask.player.pos = interPoint;
        }
        if (Utils::InTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY)){
            getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY,ball.Vel().dir());
        }
        if (robotNum != goalieNumber && Utils::InOurPenaltyArea(getballTask.player.pos,Param::Vehicle::V2::PLAYER_SIZE))
            getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(ball.RawPos(),Param::Vehicle::V2::PLAYER_SIZE);
        setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle, getballTask.player.flag));
    }

    /******************static get ball**************/
    if (getBallMode == STATIC) {
        needdribble = true;
        if(IF_DEBUG) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Static", COLOR_WHITE);
        getballTask.player.flag |= PlayerStatus::MIN_DSS; ///这种情况把dss开到最小，否则容易因为避障请不到球 by jsh 2019/6/27
        if(Utils::InOurPenaltyArea(ball.RawPos(), 0) || (me2Ball.mod() < 50 && fabs(Utils::Normalize(me.RawDir() - me2Ball.dir())) > Param::Math::PI/8)) {
            double me2BallDirDiff = Utils::Normalize(me2Ball.dir() - finalDir);
            bool isInDirectGetBallCircle = me.RawPos().dist(ball.RawPos()) < directGetBallDist && me.RawPos().dist(ball.RawPos()) > Param::Vehicle::V2::PLAYER_SIZE + 5;    //是否在直接冲上去拿球距离之内
            bool isGetBallDirReached = fabs(me2BallDirDiff) < directGetBallDirLimit;
            canGetBall = isInDirectGetBallCircle && isGetBallDirReached;     //重要布尔量:是否能直接上前拿球
            bool fraredGetball = fraredOn > 10;
            if (!canGetBall && me2Ball.mod() < 25 && !fraredGetball) needAvoidBall = true;
            else needAvoidBall = false;
            staticDir = getStaticDir(pVision, staticDir);

            if (needAvoidBall) {
                getballTask.player.angle = me2Ball.dir();
                if (fabs(me2BallDirDiff) > Param::Math::PI / 3) {
                    double avoidDir = Utils::Normalize(ball2Me.dir() + staticDir * Param::Math::PI / 4);
                    getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(30, avoidDir);
                } else {
                    double directDist = Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER + Param::Field::BALL_SIZE + 1;
                    if (fabs(me2BallDirDiff) < 0.2) {
                        getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(directDist - 5, Utils::Normalize(finalDir - Param::Math::PI));
                    } else {
                        getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(directDist, Utils::Normalize(finalDir - Param::Math::PI));
                    }
                }
                if (Utils::InTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY)){
                    getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,AVOID_PENALTY);
                }
                if (robotNum != goalieNumber && Utils::InOurPenaltyArea(getballTask.player.pos,AVOID_PENALTY))
                    getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(ball.RawPos(),AVOID_PENALTY);
                setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle,
                                                  CVector(0,0),0,
                                                  400, 30,
                                                  300, 15, //max speed
                                                  getballTask.player.flag));
            } else {
                if (fabs(me2BallDirDiff) > Param::Math::PI / 2) {
                    double gotoDir = Utils::Normalize(finalDir + staticDir * Param::Math::PI * 3.5 / 5);
                    getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(40, gotoDir);
                } else if(fabs(me2BallDirDiff) <  15 * Param::Math::PI / 180) {
                    getballTask.player.pos = ball.RawPos();
                } else {
                    double directDist = Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER  + Param::Field::BALL_SIZE - 1.5;
                    getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(directDist, Utils::Normalize(finalDir - Param::Math::PI));
                }
                if (Utils::InTheirPenaltyArea(getballTask.player.pos,Param::Vehicle::V2::PLAYER_SIZE)){
                    getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,Param::Vehicle::V2::PLAYER_SIZE);
                }
                if (robotNum != goalieNumber && Utils::InOurPenaltyArea(getballTask.player.pos,AVOID_PENALTY))
                    getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(ball.RawPos(),AVOID_PENALTY);
                setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, getballTask.player.angle, getballTask.player.flag));
            }
        }
        else{
            double angle = me2Ball.dir();
            CVector ball2target = targetPoint - ball.RawPos();
            if (!Utils::InTheirPenaltyArea(targetPoint, Param::Vehicle::V2::PLAYER_SIZE)) {
                getballTask.player.angle = ball2target.dir();//me2target.dir();
            }
            getballTask.player.flag |= PlayerStatus::DRIBBLING;
            needdribble = true;
            if(fraredOn < FraredBuffer){
                getballTask.player.pos = ball.RawPos();// + Utils::Polar2Vector(1, me2Ball.dir());
            }else{
                angle = getballTask.player.angle;
                double bias_dir = Utils::Normalize(me2Ball.dir()+Param::Math::PI/3);
                if(fabs(Utils::Normalize(me.RawDir() - angle)) < Param::Math::PI/6) bias_dir = angle;//Utils::Normalize(me2Ball.dir()+Param::Math::PI/6);//angle;
//                getballTask.player.pos = ball.RawPos() + Utils::Polar2Vector(20, bias_dir);
                //红外触发达到一定程度时，原地持球
                getballTask.player.pos = me.Pos();
            }

            if (Utils::InTheirPenaltyArea(getballTask.player.pos,Param::Vehicle::V2::PLAYER_SIZE)){
                getballTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(getballTask.player.pos,Param::Vehicle::V2::PLAYER_SIZE);
            }
            int goalieNumber = PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie());
            if (robotNum != goalieNumber && Utils::InOurPenaltyArea(getballTask.player.pos,AVOID_PENALTY))
                getballTask.player.pos = Utils::MakeOutOfOurPenaltyArea(ball.RawPos(),AVOID_PENALTY);
            setSubTask(PlayerRole::makeItGoto(robotNum, getballTask.player.pos, angle,
                                              CVector(0,0),0,            // final speed
                                              400, STATIC_ROTATE_ACC,    // max acc
                                              400, 50,    // max speed
                                               getballTask.player.flag));
        }
    }

    //是否吸球
    if (needdribble && (me2Ball.mod() < 100 || frared)) { //球在我前方则吸球
        DribbleStatus::Instance()->setDribbleCommand(robotNum, 3);
    }
    //是否射门
//    GDebugEngine::Instance()->gui_debug_line(me.RawPos(), me.RawPos()+Utils::Polar2Vector(1000, getballTask.player.angle), COLOR_PURPLE);
//    GDebugEngine::Instance()->gui_debug_line(me.RawPos(), me.RawPos()+Utils::Polar2Vector(1000, me.RawDir()), COLOR_GREEN);
//    GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(1.5*DEBUG_TEXT_HIGH, -Param::Math::PI/2), QString("KP: %1").arg(power).toLatin1(), COLOR_ORANGE);
    if(power > 500) power = power - 0.8 * std::max(0.0, me.RawVel().mod()*cos(fabs(Utils::Normalize(me.RawDir() - me.RawVel().dir()))));
//    GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(2*DEBUG_TEXT_HIGH, -Param::Math::PI/2), QString("new KP: %1").arg(power).toLatin1(), COLOR_ORANGE);
    if (needkick && fabs(Utils::Normalize(me.RawDir() - getballTask.player.angle)) < precision*Param::Math::PI/180.0 && getBallMode != INTER) {
        if(!chip) KickStatus::Instance()->setKick(robotNum, power);
        else if(fraredOn >= 20) KickStatus::Instance()->setChipKick(robotNum, power);
    }
    else if (needkick && (getBallMode == WAIT_TOUCH || getBallMode == INTER_TOUCH) && fabs(Utils::Normalize(me.RawDir() - getballTask.player.angle)) < TOUCH_ACCURACY*Param::Math::PI/180.0) {
        if(!chip) KickStatus::Instance()->setKick(robotNum, power);
        else if(fraredOn >= 20) KickStatus::Instance()->setChipKick(robotNum, power);
    }

    _lastCycle = pVision->Cycle();
    lastGetBallMode = getBallMode;
    CStatedTask::plan(pVision);
}

CPlayerCommand* CGetBallV4::execute(const CVisionModule* pVision) {
    if (subTask()) return subTask()->execute(pVision);
    return nullptr;
}

int CGetBallV4::getStaticDir(const CVisionModule* pVision, int staticDir) {
    const MobileVisionT& ball = pVision->Ball();
    const int robotNum = task().executor;
    const PlayerVisionT& me = pVision->OurPlayer(robotNum);
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
    const MobileVisionT& ball = pVision->Ball();
    const int robotNum = task().executor;
    const PlayerVisionT& me = pVision->OurPlayer(robotNum);

    /************** special judge *******************/
    if (ball.Vel().mod() < MIN_BALL_MOVING_VEL || (lastGetBallMode==STATIC && fraredOff < 5)) {
        getBallMode = STATIC;
        return;
    } else {
        getBallMode = INTER; //as default
    }
    if (isTouch) {//Touch优先级最高,进行一次判断，若不能touch，则isTouch为false
        getBallMode = WAIT_TOUCH;
        return;
    }
    /************** normal judge *******************/
//    interPoint = ZSkillUtils::instance()->getOurInterPoint(robotNum);
//    interTime = ZSkillUtils::instance()->getOurInterTime(robotNum);
//    hysteresisPredict(pVision, robotNum, interPoint, interTime, 0.1);
    interPoint = ZSkillUtils::instance()->getOurInterPoint(robotNum);
    interTime = ZSkillUtils::instance()->getOurInterTime(robotNum);
    meanFilter(robotNum, interTime, interPoint);
    if(IF_DEBUG){
        for(int i=0; i<FILTER_NUM; i++){
            GDebugEngine::Instance()->gui_debug_x(CGeoPoint(interPointXs[i], interPointYs[i]), COLOR_ORANGE);
            GDebugEngine::Instance()->gui_debug_arc(CGeoPoint(interPointXs[i], interPointYs[i]), i, 0.0f, 360.0f, COLOR_ORANGE);
        }
        GDebugEngine::Instance()->gui_debug_x(interPoint, COLOR_YELLOW);
        GDebugEngine::Instance()->gui_debug_arc(interPoint, 8, 0.0f, 360.0f, COLOR_YELLOW);
    }

    double interPointJump = sqrt((interPointXs[FILTER_NUM-1]-interPointXs[FILTER_NUM-2])*(interPointXs[FILTER_NUM-1]-interPointXs[FILTER_NUM-2])
            + (interPointYs[FILTER_NUM-1]-interPointYs[FILTER_NUM-2])*(interPointYs[FILTER_NUM-1]-interPointYs[FILTER_NUM-2]));


    if(interPointJump > 30)
//    if(ZSkillUtils::instance()->IsInField(interPoint, 9))
        lastInterState = true;
    else lastInterState = false;

    CVector interPoint2target = targetPoint - interPoint;
    double ballBias = fabs(Utils::Normalize(ball.Vel().dir() - interPoint2target.dir()));

    if(fraredOff > 3 && Param::Math::PI - ballBias < NORMAL_TOUCH_DIR * Param::Math::PI / 180) { // May touch
        getBallMode = INTER_TOUCH;
    } else if(ballBias < (NORMAL_CHASE_DIR / 180)*Param::Math::PI && !safeMode) { // May chase
        getBallMode = CHASE;
    }
    if(fabs(Utils::Normalize(ball.Vel().dir() - (ball.RawPos() - me.RawPos()).dir())) < RUSH_BALL_DIR *Param::Math::PI /180.0){
        getBallMode = RUSH;
        return;
    }
    if(lastGetBallMode==RUSH && (!ball.Valid() || (ball.RawPos() - me.RawPos()).mod() < 30)) getBallMode = RUSH;
    return;
}

bool CGetBallV4::judgeShootMode(const CVisionModule * pVision) {
    const MobileVisionT& ball = pVision->Ball();
    const int robotNum = task().executor;
    const PlayerVisionT& me = pVision->OurPlayer(robotNum);
    const CVector me2Target = targetPoint - me.RawPos();
    double finalDir = me2Target.dir();
    double ballVel2FinalDiff = Utils::Normalize(ball.Vel().dir() - finalDir);

    bool shootMode = fabs(ballVel2FinalDiff) < 0.5;
    return shootMode;
}

void CGetBallV4::meanFilter(int robotNum, double &interTime, CGeoPoint &interPoint){
    robotNum = robotNum - 1;
    for(int i=0; i < FILTER_NUM - 1; i++){
        interTimes[i] = interTimes[i+1];
        interPointXs[i] = interPointXs[i+1];
        interPointYs[i] = interPointYs[i+1];
    }

    interTimes[FILTER_NUM-1] =interTime;
    interPointXs[FILTER_NUM-1] = interPoint.x();
    interPointYs[FILTER_NUM-1] = interPoint.y();

    if(!inters.empty() && inters.size() >= FILTER_NUM) {
        inters.pop_front();
    }
    interInfo inter;
    inter.interTimes = interTime;
    inter.interPointXs = interPoint.x();
    inter.interPointYs = interPoint.y();
    inters.push_back(inter);
    inters.sort();
    if(inters.size() > 5) {
        inters.pop_back();
        inters.pop_back();
        inters.pop_front();
        inters.pop_front();
    } else if(inters.size() > 3) {
        inters.pop_back();
        inters.pop_front();
    }

    interTime = 0.0;
    double x = 0.0, y = 0.0;
    for(auto itor : inters) {
        interTime += itor.interTimes;
        x += itor.interPointXs;
        y += itor.interPointYs;
    }
    interTime /= inters.size();
    interPoint.setX(x / inters.size());
    interPoint.setY(y / inters.size());
}

bool CGetBallV4::hysteresisPredict(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, double buffer) {
    const MobileVisionT ball = pVision->Ball();//获得球
    const PlayerVisionT me = pVision->OurPlayer(robotNum);//获得车
    if (!me.Valid()) {//车不存在
        interTime = 99999;
        interceptPoint = CGeoPoint(999, 999);
//        lastInterState[robotNum] = false;// not change state
        return false;
    }
    if(ball.Vel().mod() < 40) {
        interceptPoint = ball.Pos();//截球点
        interTime = predictedTime(me, interceptPoint);//截球时间
        //lastInterState[robotNum] = true;
        return true;
    }
    double ballAcc = FRICTION / 2;//球减速度
    double ballArriveTime = 0;
    double meArriveTime = 9999;
    double testBallLength = 0;//球移动距离
    CGeoPoint testPoint = ball.Pos();
    double testVel = ball.Vel().mod();
    double max_time = ball.Vel().mod() / ballAcc;
    CGeoLine ballLine(ball.Pos(), ball.Vel().dir());
    CGeoPoint ballLineProjection = ballLine.projection(me.Pos());
    CVector projection2me = me.Pos() - ballLineProjection;
//    double width = projection2me.mod() < Param::Vehicle::V2::PLAYER_SIZE ? projection2me.mod() : Param::Vehicle::V2::PLAYER_SIZE;
    for (ballArriveTime = 0; ballArriveTime < max_time; ballArriveTime += 1.0 / (Param::Vision::FRAME_RATE * 2) ) {
        testVel = ball.Vel().mod() - ballAcc * ballArriveTime; //v_0-at
        testBallLength = (ball.Vel().mod() + testVel) * ballArriveTime / 2; //梯形法计算球移动距离
        testPoint = ball.Pos() + Utils::Polar2Vector(testBallLength, ball.Vel().dir());
        CVector me2testPoint = testPoint - me.RawPos();
        meArriveTime = predictedTime(me, testPoint);//我到截球点的时间
        if(meArriveTime < 0.3) meArriveTime = 0;
        if(me.Vel().mod() < 100 && projection2me.mod() < 15 && me2testPoint.mod() < 15) meArriveTime = 0;
//        if(isBall2Me && projection2me.mod() < 10) meArriveTime = 0;
        if (!ZSkillUtils::instance()->IsInField(testPoint)){// ball out of field
            //lastInterState[robotNum] = false;
            break;
        }
        if (meArriveTime + buffer + responseTime< ballArriveTime){
            //lastInterState[robotNum] = true;
            break;
        }
        if (meArriveTime + buffer + responseTime> ballArriveTime && meArriveTime + responseTime< ballArriveTime){
            if(lastInterState == true){
                //lastInterState[robotNum] = true;
                break;
            }
        }
    }
    interceptPoint = testPoint;//截球点
    interTime = predictedTime(me, interceptPoint);//截球时间
    return true;
}
