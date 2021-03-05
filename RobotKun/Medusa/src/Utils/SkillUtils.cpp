#include "SkillUtils.h"
#include "parammanager.h"
#include "GDebugEngine.h"
#include "staticparams.h"
#include <cmath>
#include "GoImmortalRush.h"
#include "RobotSensor.h"
#include "ballmodel.h"
#include <locale>
using namespace std;
namespace {
    bool DEBUG = false;
    bool DEBUG_DETAIL = false;
    double PASS_AVOID_DIST = 351.0;
    double SHOOT_AVOID_DIST = 180.0;
    double AGRESSIVE_SHOOT_AVOID_DIST = 90.0;
    constexpr double TIME_FOR_OUR = 0.3;
    constexpr double TIME_FOR_OUR_BOTH_KEEP = 0.1;
    constexpr double TIME_FOR_THEIR_BOTH_KEEP = -0.1;
    constexpr double TIME_FOR_THEIR = -0.3;
    constexpr double TIME_FOR_JUDGE_HOLDING = 0.5;
    const double DELTA_T = 8.0 / PARAM::Vision::FRAME_RATE;
    const double ourResponseTime = 0.2;
    double AVOID_PENALTY = 200;
    bool USE_CHIPSOLVER;
    bool CHIP_GETBALL_DEBUG;
    bool CHIP_INTER_DEBUG;
    double waittime_coef;
    double secpos_coef;
}
SkillUtils::SkillUtils():_isInFlyTime(false), firstChipState(false), secondChipState(false) {
    bool isSimulation;
    ZSS::ZParamManager::instance()->loadParam(isSimulation, "Alert/IsSimulation", false);
    if(isSimulation)
        ZSS::ZParamManager::instance()->loadParam(FRICTION, "AlertParam/Friction4Sim", 1520.0);
    else
        ZSS::ZParamManager::instance()->loadParam(FRICTION, "AlertParam/Friction4Real", 800.0);
    ZSS::ZParamManager::instance()->loadParam(PASS_AVOID_DIST, "Shoot/PassAvoidDist", 351.0);
    ZSS::ZParamManager::instance()->loadParam(SHOOT_AVOID_DIST, "Shoot/ShootAvoidDist", 180.0);
    ZSS::ZParamManager::instance()->loadParam(AGRESSIVE_SHOOT_AVOID_DIST, "Shoot/AgressiveShootAvoidDist", 90.0);
    ZSS::ZParamManager::instance()->loadParam(DEBUG, "Debug/InterTimeDisplay", false);
    ZSS::ZParamManager::instance()->loadParam(DEBUG_DETAIL,"Debug/InterTimeRobot0",false);
    ZSS::ZParamManager::instance()->loadParam(USE_CHIPSOLVER, "chipsolver/USE_CHIPSOLVER_IN_MEDUSA", false);
    ZSS::ZParamManager::instance()->loadParam(waittime_coef,"chipsolver/waittime_coef",0.2);
    ZSS::ZParamManager::instance()->loadParam(secpos_coef,"chipsolver/secpos_coef",0.5);
    ZSS::ZParamManager::instance()->loadParam(CHIP_GETBALL_DEBUG, "chipsolver/CHIP_GETBALL_DEBUG", false);
    ZSS::ZParamManager::instance()->loadParam(CHIP_INTER_DEBUG, "chipsolver/CHIP_INTER_DEBUG", false);
    ZSS::ZParamManager::instance()->loadParam(AVOID_PENALTY, "GetBall/AvoidPenalty", 20.0*10);
}
SkillUtils::~SkillUtils() {
}

bool SkillUtils::predictedInterTime(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, double responseTime) {
    const MobileVisionT ball = pVision->ball();//获得球
    const PlayerVisionT me = pVision->ourPlayer(robotNum);//获得车
    static const double ballAcc = FRICTION*3 / 4;//球减速度
    const CGeoPoint lastInterceptPoint = interceptPoint;
    if (!me.Valid()) {//车不存在
        interTime = 99999;
        interceptPoint = CGeoPoint(99999, 99999);
        return false;
    }
    if(ball.Vel().mod() < 300) {
        interceptPoint = ball.RawPos();//截球点
        interTime = predictedTime(me, interceptPoint);//截球时间
        return true;
    }
    double ballArriveTime = 0;
    double meArriveTime = 9999;
    double testBallLength = 0;//球移动距离
    CGeoPoint testPoint = ball.Pos();
    double testVel = ball.Vel().mod();
    double max_time = ball.Vel().mod() / ballAcc;
    CGeoLine ballLine(ball.Pos(), ball.Vel().dir());

    CGeoPoint ballLineProjection = ballLine.projection(me.Pos());
    CVector projection2me = me.Pos() - ballLineProjection;
    double width = projection2me.mod() < PARAM::Vehicle::V2::PLAYER_SIZE ? projection2me.mod() : PARAM::Vehicle::V2::PLAYER_SIZE;

    // test availability of last point
    bool useLastPoint = false;
    CGeoPoint last_point_ballLineProjection = ballLine.projection(lastInterceptPoint);
    auto dist = (ball.Pos() - last_point_ballLineProjection).mod();
    auto maxRollingDist = ball.Vel().mod2()/(2*ballAcc);
    auto res_time = 0.0;
    auto diffTime = 0.0;
    if(dist < maxRollingDist){
        // a = ballAcc;b = -2*testVel;c = 2*dist; res = (-b-sqrt(b^2-4ac))/2a = ((V-sqrt(V2-2AD))/A)
        auto& V = testVel;
        auto& A = ballAcc;
        auto& D = dist;
        res_time = (V-sqrt(V*V-2*A*D))/A;
        auto arrTime = predictedTime(me, lastInterceptPoint);
        if(arrTime < 0.2) arrTime = 0;
        diffTime = arrTime+responseTime-res_time;
        if(diffTime < 0.8*DELTA_T && diffTime > -1.5*DELTA_T){
            useLastPoint = true;
            interceptPoint = last_point_ballLineProjection;
            interTime = std::max(arrTime,res_time);
        }
    }
    if(DEBUG_DETAIL){
        GDebugEngine::Instance()->gui_debug_x(last_point_ballLineProjection,0);
        GDebugEngine::Instance()->gui_debug_msg(last_point_ballLineProjection,QString("dd: %1 %2 %3 %4 %5").arg(res_time,0,'f',2).arg(diffTime,0,'f',2).arg(useLastPoint ? "T" : "F").arg(dist,0,'f',2).arg(maxRollingDist,0,'f',2).toLatin1(),0,0,200);
        GDebugEngine::Instance()->gui_debug_msg(last_point_ballLineProjection,"LP",8,0,500);
    }


    // old point not available, calculate new point using violence search
    if(!useLastPoint){
        for (ballArriveTime = 0; ballArriveTime < max_time; ballArriveTime += DELTA_T) {
            testVel = ball.Vel().mod() - ballAcc * ballArriveTime; //v_0-at
            testBallLength = PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER + (ball.Vel().mod() + testVel) * ballArriveTime / 2; //梯形法计算球移动距离
            testPoint = ball.Pos() + Utils::Polar2Vector(testBallLength, ball.Vel().dir());
            CVector me2testPoint = testPoint - me.RawPos();
            meArriveTime = predictedTime(me, testPoint + Utils::Polar2Vector(width, projection2me.dir()));//我到截球点的时间
            if(meArriveTime < 0.2) meArriveTime = 0;
            if(me.Vel().mod() < 1500 /*&& projection2me.mod() < 2*PARAM::Vehicle::V2::PLAYER_SIZE*/ && me2testPoint.mod() < 3*PARAM::Vehicle::V2::PLAYER_SIZE) meArriveTime = 0;
            if(DEBUG_DETAIL){
                GDebugEngine::Instance()->gui_debug_x(testPoint,4);
                GDebugEngine::Instance()->gui_debug_msg(testPoint,QString("t: %1 , %2").arg(meArriveTime,0,'f',2).arg(ballArriveTime,0,'f',2).toLatin1(),10,0,10);
            }
            if (!Utils::IsInField(testPoint) || (meArriveTime + responseTime) < ballArriveTime) {
                break;
            }
        }
        interceptPoint = testPoint;
        interTime = predictedTime(me, interceptPoint);//截球时间
    }
    // 避免快接到球的时候截球点突然往后跳
    if (me.Pos().dist(ball.Pos()) < PARAM::Vehicle::V2::PLAYER_SIZE*std::max(5.0,testVel/500) && projection2me.mod() < 2*PARAM::Vehicle::V2::PLAYER_SIZE && fabs(Utils::Normalize(ball.Vel().dir() - (ballLineProjection-ball.Pos()).dir())) < PARAM::Math::PI/12){
        interceptPoint = ballLineProjection;
        interTime = predictedTime(me, interceptPoint);//截球时间
        if(DEBUG_DETAIL){
            GDebugEngine::Instance()->gui_debug_x(interceptPoint,3);
            GDebugEngine::Instance()->gui_debug_msg(interceptPoint,"NP",8,0,400);
        }
    }

    // if enemy can intercept ball faster, go to enemy instead of ball
    double minDist = 999999;
    for(int i=0; i<PARAM::Field::MAX_PLAYER; i++){
        auto enemy = pVision->theirPlayer(i);
        if(!enemy.Valid()) continue;
        CVector enemy2ballLine = ballLine.projection(enemy.Pos()) - enemy.Pos();
        CVector ball2enemy = enemy.Pos() - ball.RawPos();
        if(enemy2ballLine.mod() < 1200 && (fabs(Utils::Normalize(ball.Vel().dir() - (enemy.Pos()-ball.RawPos()).dir()))<PARAM::Math::PI/6) && enemy.Vel().mod() < 2000 && (ball2enemy.mod() < ball.Vel().mod2()/FRICTION)){
            minDist = std::min(minDist, ball2enemy.mod());
//            GDebugEngine::Instance()->gui_debug_arc(enemy.Pos(), 30, 0.0f, 360.0f, COLOR_GREEN);
//            GDebugEngine::Instance()->gui_debug_msg(enemy.Pos(), QString("dist: %1  dir: %2").arg(enemy2ballLine.mod()).arg(fabs(Utils::Normalize(ball.Vel().dir() - (enemy.Pos()-ball.RawPos()).dir())/PARAM::Math::PI*180)).toLatin1(), COLOR_WHITE);
        }
    }
    if(minDist<888888){
        CGeoPoint new_interceptPoint = ball.RawPos() + Utils::Polar2Vector(std::max(minDist-PARAM::Vehicle::V2::PLAYER_SIZE,0.0), ball.Vel().dir());
//        GDebugEngine::Instance()->gui_debug_x(new_interceptPoint, COLOR_PURPLE);
//        GDebugEngine::Instance()->gui_debug_arc(new_interceptPoint, 30, 0.0f, 360.0f, COLOR_PURPLE);
        double new_interTime = predictedTime(me, new_interceptPoint);
        if((ball.RawPos()-new_interceptPoint).mod() < (ball.RawPos()-interceptPoint).mod()){
            interTime = new_interTime;
            interceptPoint = new_interceptPoint;
        }
    }
    return true;
}

bool SkillUtils::predictedTheirInterTime(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, double responseTime) {
    const MobileVisionT& ball = pVision->ball();//获得球
    const PlayerVisionT& me = pVision->theirPlayer(robotNum);//获得车
    static const double AVOID_DIST = PARAM::Vehicle::V2::PLAYER_SIZE;
    static const double ballAcc = FRICTION / 2;//球减速度
    if (!me.Valid()) {//车不存在
        interTime = 99999;
        interceptPoint = CGeoPoint(99999, 99999);
        return false;
    }
    //double maxArriveTime = 5;//车最多移动时间
    double ballArriveTime = 0;
    double meArriveTime = 999999;
    double testBallLength = 0;//球移动距离
    CGeoPoint testPoint = ball.Pos();
    double testVel = ball.Vel().mod();
    double max_time = ball.Vel().mod() / ballAcc;
    for (ballArriveTime = 0; ballArriveTime < max_time; ballArriveTime += DELTA_T) {
        testVel = ball.Vel().mod() - ballAcc * ballArriveTime; //v_0-at
        testBallLength = (ball.Vel().mod() + testVel) * ballArriveTime / 2; //梯形法计算球移动距离
        testPoint = ball.Pos() + Utils::Polar2Vector(testBallLength, ball.Vel().dir()) + Utils::Polar2Vector(AVOID_DIST, (me.Pos() - testPoint).dir());
        meArriveTime = predictedTheirTime(me, testPoint);//我到截球点的时间
        CVector me2testPoint = testPoint - me.RawPos();
        if(me.Vel().mod() < 1500 && me2testPoint.mod() < 3*PARAM::Vehicle::V2::PLAYER_SIZE) meArriveTime = 0;
        if (!Utils::IsInField(testPoint) || (meArriveTime + responseTime) < ballArriveTime) {
            break;
        }
    }
    interceptPoint = testPoint;//截球点
    interTime = std::max(ballArriveTime,predictedTheirTime(me, interceptPoint));//截球时间
    return true;
}

void SkillUtils::judgeChipResult(const CVisionModule* pVision){
//    if(pVision->ball().ChipBallState() && Utils::IsInField(pVision->ball().BestChipPredictPos())){
//        firstChipState = true;
//    }

    static int chip_cnt = 0;
//    static bool chip_first_flag = false; // is in first period before ball firstly hit the ground or not
    if(pVision->ball().ChipBallState() && Utils::IsInField(pVision->ball().SecondChipPos())){
//        chip_first_flag = true;
        firstChipState = true;
        _isInFlyTime = true;
        chip_cnt = static_cast<int>(pVision->ball().ChipFlyTime() * waittime_coef * PARAM::Vision::FRAME_RATE);
    } else {
        if(firstChipState){
//            chip_first_flag = false;
            firstChipState = false;
        }
        if(--chip_cnt > 0){
            secondChipState = true;
            _isInFlyTime = true;
        }
        else {
            chip_cnt = 0;
            secondChipState = false;
            _isInFlyTime = false;
        }
    }
}

bool SkillUtils::isInFlyTime(){
    return _isInFlyTime;
}

bool SkillUtils::canGetChipResult(){
    return firstChipState;
}

bool SkillUtils::predictedChipInterTime(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, CGeoPoint secondPoint, CVector3 chipKickVel,double firstChipTime,double RestChiptime, double responseTime) {
    // if in second chip, chipsolver's result is invalid, remain the chip result in first chip(not change)
    // when is out of chip, all the result(inter point and time) will be clear
    if(secondChipState)
        return true;
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

    if(Utils::InTheirPenaltyArea(interceptPoint, AVOID_PENALTY)){
        interceptPoint = Utils::MakeOutOfTheirPenaltyArea(interceptPoint, AVOID_PENALTY, rollvel.dir());
    }
    if(Utils::InOurPenaltyArea(interceptPoint, AVOID_PENALTY)){
        interceptPoint = Utils::MakeOutOfOurPenaltyArea(interceptPoint, AVOID_PENALTY,rollvel.dir());
    }

    interTime = predictedTime(me, interceptPoint);

    int RGBvalue[8]= {25467101,252157154,249205173,200200169,131175155,22215683,222211140};
    if(CHIP_INTER_DEBUG){
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(me.Pos().x(), me.Pos().y()+400), QString("priority %1").arg(priority).toLatin1(),COLOR_USE_RGB,RGBvalue[robotNum]);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(me.Pos().x(), me.Pos().y()+300), QString("interT %1").arg(interTime).toLatin1(),COLOR_USE_RGB,RGBvalue[robotNum]);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(me.Pos().x(), me.Pos().y()+200), QString("interP x:%1 y:%2").arg(interceptPoint.x()).arg(interceptPoint.y()).arg(interTime).toLatin1(),COLOR_USE_RGB,RGBvalue[robotNum]);
    }

    return true;
}

void SkillUtils::resetChipInterMessage(){
    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++){
        chipInterPoint[i].fill(99999, 99999);
        chipInterTime[i] = 99999;
    }
}

double SkillUtils::getChipInterTime(int num){
    return chipInterTime[num];
}

CGeoPoint SkillUtils::getChipInterPoint(int num){
    return chipInterPoint[num];
}

CGeoPoint SkillUtils::predictedTheirInterPoint(CGeoPoint enemy, CGeoPoint ball) {
    static const double ballAcc = FRICTION / 2;//球减速度
    constexpr double carAcc = 4000;
    constexpr double maxBallSpeed = 6500;
    double dist = (enemy - ball).mod();
    double delta = maxBallSpeed * maxBallSpeed + 2 * (carAcc - ballAcc) * dist;
    double t = (std::sqrt(delta) - maxBallSpeed) / (carAcc - ballAcc);
    double enemy2point = 0.5 * carAcc * t * t;
    return enemy + Utils::Polar2Vector(enemy2point, (ball - enemy).dir());
}

bool SkillUtils::IsBallReachable(double ballVel, double length, double friction) {
    double max_dist = ballVel * ballVel / (2 * friction);
    if (length < max_dist) return true;
    else return false;
}

void SkillUtils::generateInterInformation(const CVisionModule* pVision) {
    for(int robotNum = 0; robotNum < PARAM::Field::MAX_PLAYER; robotNum++) {
        predictedInterTime(pVision, robotNum, ourInterPoint[robotNum], ourInterTime[robotNum], ourResponseTime);
        predictedTheirInterTime(pVision, robotNum, theirInterPoint[robotNum], theirInterTime[robotNum], 0);

        if(USE_CHIPSOLVER){
            // if in first chip, chipsolver's result is valid, update the data
            // if in second chip, chipsolver's result is invalid, remain the chip result in first chip(not change)
            // when is out of chip, all the result(inter point and time) will be clear
            if(_isInFlyTime)
                predictedChipInterTime(pVision, robotNum, chipInterPoint[robotNum], chipInterTime[robotNum], pVision->ball().SecondChipPos(), pVision->ball().chipKickVel(), pVision->ball().ChipFlyTime(), pVision->ball().ChipRestFlyTime(), ourResponseTime);
            else
                resetChipInterMessage();
        }
    }
    return;
}

double SkillUtils::getOurInterTime(int num) {
    return ourInterTime[num];
}

double SkillUtils::getTheirInterTime(int num) {
    return theirInterTime[num];
}

CGeoPoint SkillUtils::getOurInterPoint(int num) {
    return ourInterPoint[num];
}

CGeoPoint SkillUtils::getTheirInterPoint(int num) {
    return theirInterPoint[num];
}

bool SkillUtils::isSafeShoot(const CVisionModule* pVision, double ballVel, CGeoPoint target) {
    const MobileVisionT ball = pVision->ball();//获得球
    const CGeoPoint ballPos = ball.Pos();
    const CVector ball2target = target - ballPos;
    double ballAcc = FRICTION / 2;//球减速度
    for(int robotNum = 0; robotNum < PARAM::Field::MAX_PLAYER; robotNum++) {
        PlayerVisionT me = pVision->theirPlayer(robotNum);//获得车
        if (!me.Valid()) {//车不存在
            continue;//无车，查看下一辆
        }
        double ballArriveTime = 0;
        double meArriveTime = 99999;
        double testBallLength = 0;//球移动距离
        CGeoPoint testPoint(ballPos.x(), ballPos.y());
        double testVel = ballVel;
        while (true) {
            ballArriveTime += DELTA_T;
            testVel = ballVel - ballAcc * ballArriveTime; //v_0-at
            if (testVel < 0) testVel = 0;//球已静止
            testBallLength = (ballVel + testVel) * ballArriveTime / 2; //梯形法计算球移动距离
            testPoint.setX(ballPos.x() + testBallLength * std::cos(ball2target.dir()));//截球点坐标
            testPoint.setY(ballPos.y() + testBallLength * std::sin(ball2target.dir()));
            meArriveTime = predictedTime(me, testPoint);//对方车到截球点的时间
            if (!Utils::IsInField(testPoint)) {
                break;//该车无法在场内截球，跳出循环
            }
            if (meArriveTime < ballArriveTime) {//该车可在场内截球
                if(testBallLength < ball2target.mod())
                    return false;//能在球线上截到球
                else break;//不能在球线上截到球，跳出循环
            }
        }
    }
    return true;//for循环中所有车都不能截球，没有return false
}

bool SkillUtils::validShootPos(const CVisionModule *pVision, CGeoPoint shootPos, double shootVel, CGeoPoint target, double &interTime, const double responseTime, double ignoreCloseEnemyDist, bool ignoreTheirGoalie, bool ignoreTheirGuard, bool DEBUG_MODE) {
    double AVOID_DIST = PASS_AVOID_DIST;//3.9 * PARAM::Vehicle::V2::PLAYER_SIZE;
    if (Utils::InTheirPenaltyArea(target, 0)) {
        AVOID_DIST = SHOOT_AVOID_DIST;
        if(Utils::InTheirPenaltyArea(shootPos, 800))
            AVOID_DIST = AGRESSIVE_SHOOT_AVOID_DIST;
    }


    const CVector passLine = target - shootPos;
    const double passLineDist = passLine.mod() - PARAM::Vehicle::V2::PLAYER_SIZE;
    const double passLineDir = passLine.dir();
    const CVector vecVel = Utils::Polar2Vector(shootVel, passLineDir);
    if(DEBUG_MODE) GDebugEngine::Instance()->gui_debug_x(target, COLOR_CYAN);
    interTime = 999999; //敌人到达时间减去球到达时间
    for(int robotNum = 0; robotNum < PARAM::Field::MAX_PLAYER; robotNum++) {
        const PlayerVisionT& enemy = pVision->theirPlayer(robotNum);//获得车
        bool isGoalie = Utils::InTheirPenaltyArea(enemy.Pos(), 0);
        if (!enemy.Valid()) continue;
        // 忽略距离近的防守车
        if(enemy.Pos().dist(shootPos) < ignoreCloseEnemyDist && !Utils::InTheirPenaltyArea(enemy.Pos(), 0)) continue;
        // 判断是否已经在球线上
        CGeoSegment ballLine(shootPos, target);
        if(ballLine.dist2Point(enemy.Pos()) < AVOID_DIST){
            interTime = -10; //对于被敌人挡住的点，统一取-10
            return false;
        }
        // 忽略对面的守门员
        if(ignoreTheirGoalie && Utils::InTheirPenaltyArea(enemy.Pos(), 0)) continue;
        // 忽略对面的后卫
        if(ignoreTheirGuard && Utils::InTheirPenaltyArea(enemy.Pos(), 300)) continue;
        // 初始化球
        double ballMoveTime = 0;
        double oneWorstInterTime = 99999;
        while (true) {
            double oneInterTime = 9999;
            ballMoveTime += DELTA_T;
            CGeoPoint ballPos = BallModel::instance()->flatPos(shootPos, vecVel, ballMoveTime);
            double ballMoveDist = BallModel::instance()->flatMoveDist(vecVel, ballMoveTime);
            double ballVel = BallModel::instance()->flatMoveVel(shootVel, ballMoveTime);
            //截球点坐标
            ballPos = shootPos + Utils::Polar2Vector(ballMoveDist, passLineDir);
            if(DEBUG_MODE) {
                static int cnt = 0;
                cnt++;
                if(cnt%30==0) GDebugEngine::Instance()->gui_debug_x(ballPos);
            }
            //该车无法在场内截球，跳出循环
            if (!Utils::IsInField(ballPos) || ballMoveDist >= passLineDist) {
                break;
            }
            CVector adjustDir = enemy.Pos() - ballPos;
            CGeoPoint testPoint = ballPos + Utils::Polar2Vector(AVOID_DIST, adjustDir.dir());
            double enemyArriveTime = predictedTheirTime(enemy, testPoint);
            //更新最差时间
            oneInterTime = enemyArriveTime + responseTime - ballMoveTime; //大于0拦不住，小于0拦得住
            if(oneInterTime < oneWorstInterTime)
                oneWorstInterTime = oneInterTime;

            // 判断截球点是否合理
            bool interPosValid = (!isGoalie && !Utils::InTheirPenaltyArea(ballPos, 0)) || (isGoalie && Utils::InTheirPenaltyArea(ballPos, 0));
            if(interPosValid && ballMoveDist < passLineDist && (ballVel < 1e-8 ||oneInterTime < 0)){
                if(DEBUG_MODE) GDebugEngine::Instance()->gui_debug_msg(ballPos, QString("%1 %2").arg(enemyArriveTime).arg(ballMoveTime).toLatin1(), COLOR_CYAN);
                break;
                //return false;
            }
            if(!interPosValid && ballVel < 1e-8)
                break;
                //return false
        }
        if(oneWorstInterTime < interTime)
            interTime = oneWorstInterTime;
    }
    if(interTime < 0)
        return false;
    return true;//for循环中所有车都不能截球，没有return false
}

bool SkillUtils::validChipPos(const CVisionModule *pVision, CGeoPoint shootPos, double shootVel, CGeoPoint target, const double responseTime, /*double ignoreCloseEnemyDist, */bool ignoreTheirGuard, bool DEBUG_MODE) {
    const double CHIP_FIRST_ANGLE = 54.29 / 180.0 * PARAM::Math::PI;
    const double CHIP_SECOND_ANGLE = 45.59 / 180.0 * PARAM::Math::PI;
    constexpr double CHIP_LENGTH_RATIO = 1.266;
    constexpr double CHIP_VEL_RATIO = 0.6372;
    constexpr double G = 9.8;
    static const double BALL_ACC = FRICTION / 2;
    static const double AVOID_DIST = 2 * PARAM::Vehicle::V2::PLAYER_SIZE;

    const CVector passLine = target - shootPos;
    const double passLineDist = passLine.mod() - PARAM::Vehicle::V2::PLAYER_SIZE;
    const double passLineDir = passLine.dir();

    double chipLength1 = shootVel / 1000;
    double chipTime1 = sqrt(2.0 * chipLength1 * tan(CHIP_FIRST_ANGLE) / G);
    double chipLength2 = (CHIP_LENGTH_RATIO - 1) * chipLength1;
    double chipTime2 = sqrt(2 * chipLength2 * tan(CHIP_SECOND_ANGLE) / G);
    chipLength1 *= 1000;
    chipLength2 *= 1000;
    CGeoPoint startPos = shootPos + Utils::Polar2Vector(chipLength1 + chipLength2, passLineDir);
    double startTime = chipTime1 + chipTime2;
    double startVel = pow(chipTime1 * 1000 * G / (2 * sin(CHIP_FIRST_ANGLE)), 2) * CHIP_VEL_RATIO / 9800;
    double startDist = startPos.dist(shootPos);
    if(DEBUG_MODE){
        GDebugEngine::Instance()->gui_debug_x(target);
        GDebugEngine::Instance()->gui_debug_line(shootPos, shootPos + Utils::Polar2Vector(chipLength1, passLineDir), COLOR_BLUE);
        GDebugEngine::Instance()->gui_debug_line(shootPos + Utils::Polar2Vector(chipLength1, passLineDir), shootPos + Utils::Polar2Vector(chipLength1+chipLength2, passLineDir), COLOR_GREEN);
        GDebugEngine::Instance()->gui_debug_msg(target, QString::number(shootVel).toLatin1());
    }

    for(int robotNum = 0; robotNum < PARAM::Field::MAX_PLAYER; robotNum++) {
        const PlayerVisionT& me = pVision->theirPlayer(robotNum);//获得车
        if (!me.Valid()) continue;
//        // 忽略距离近的防守车
//        if(me.Pos().dist(shootPos) < ignoreCloseEnemyDist) continue;
        // 判断是否已经在球线上
        CGeoSegment ballLine(startPos, target);
        if(ballLine.dist2Point(me.Pos()) < AVOID_DIST)
            return false;
        // 忽略对面的后卫
        if(ignoreTheirGuard && Utils::InTheirPenaltyArea(me.Pos(), 300)) continue;
        // 初始化球速、加速度、球移动的距离、球初始位置
        CGeoPoint ballPos = startPos;
        double ballVel = startVel;
        double ballMoveTime = startTime;
        double ballMoveDist = startDist;
        while (true) {
            ballMoveTime += DELTA_T;
            ballVel = startVel - BALL_ACC * (ballMoveTime - startTime);
            if (ballVel < 0) {
                ballVel = 0;//球已静止
            }
            ballMoveDist = startDist + (ballVel + startVel) / 2 * (ballMoveTime - startTime);
            //截球点坐标
            ballPos = shootPos + Utils::Polar2Vector(ballMoveDist, passLineDir);
            if(DEBUG_MODE) GDebugEngine::Instance()->gui_debug_x(ballPos);
            //该车无法在场内截球，跳出循环
            if (!Utils::IsInField(ballPos) || ballMoveDist >= passLineDist) {
                break;
            }
            CVector adjustDir = me.Pos() - ballPos;
            ballPos = ballPos + Utils::Polar2Vector(AVOID_DIST, adjustDir.dir());
            double meArriveTime = predictedTheirTime(me, ballPos);
            if(!Utils::InTheirPenaltyArea(ballPos, 0) && ballMoveDist < passLineDist && (ballVel < 1e-8 || meArriveTime + responseTime < ballMoveTime)){
                return false;
            }
        }
    }
    return true;//for循环中所有车都不能截球，没有return false
}

void SkillUtils::generatePredictInformation(const CVisionModule* pVision) {
    CGeoPoint ball = pVision->ball().Pos();//获得球
    double ballAcc = FRICTION;//球减速度
    double carAcc = 5000;
    double maxBallSpeed = 6500;
    for(int enemyNum = 0; enemyNum < PARAM::Field::MAX_PLAYER; enemyNum++) {
        PlayerVisionT enemy = pVision->theirPlayer(enemyNum);//获得车
        if (!enemy.Valid()) {//车不存在
            predictTheirInterTime[enemyNum] = 99999;
            predictTheirInterPoint[enemyNum] = CGeoPoint(99999, 99999);
            continue;
        }
        CGeoPoint enemyPoint = enemy.Pos();
        double dist = (enemyPoint - ball).mod();
        double delta = maxBallSpeed * maxBallSpeed + 2 * (carAcc - ballAcc) * dist;
        double t = (std::sqrt(delta) - maxBallSpeed) / (carAcc - ballAcc);
        double enemy2point = 0.5 * carAcc * t * t;
        predictTheirInterTime[enemyNum] = t;
        predictTheirInterPoint[enemyNum] = enemyPoint + Utils::Polar2Vector(enemy2point, (ball - enemyPoint).dir());
    }
}

double SkillUtils::getPredictTime(int num) {
    return predictTheirInterTime[num];
}

CGeoPoint SkillUtils::getPredictPoint(int num) {
    return predictTheirInterPoint[num];
}

void SkillUtils::run(const CVisionModule* pVision) {
    _lastCycle = pVision->getCycle();
    ourBestInterRobot = theirBestInterRobot = theirGoalie = 1;
    if(USE_CHIPSOLVER)
        judgeChipResult(pVision);
    generateInterInformation(pVision);
    generatePredictInformation(pVision);
    calculateBallBelongs(pVision);
    updateTheirGoalie(pVision);
    if(DEBUG) {
        for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
            GDebugEngine::Instance()->gui_debug_x(theirInterPoint[i]);
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(2000, 3000 - i * 300), QString("%1 %2").arg(i).arg(theirInterTime[i]).toLatin1());
            GDebugEngine::Instance()->gui_debug_x(ourInterPoint[i], COLOR_WHITE);
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-2000, 3000 - i * 300), QString("%1 %2").arg(i).arg(ourInterTime[i]).toLatin1(), COLOR_WHITE);
        }
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-2000, -2000), QString("OurBest : %1").arg(ourBestInterRobot).toLatin1(), COLOR_WHITE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(2000, -2000), QString("TheirBest : %1").arg(theirBestInterRobot).toLatin1());
    }
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, PARAM::Field::PITCH_WIDTH / 2), ZBallState::toStr[ballState].c_str(), COLOR_CYAN);
}
void SkillUtils::calculateBallBelongs(const CVisionModule* pVision) {
    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        if(pVision->ourPlayer(i).Valid() && Utils::InOurPenaltyArea(CGeoPoint(pVision->ourPlayer(i).Pos().x(), pVision->ourPlayer(i).Pos().y()), 200))
            continue; //排除我方后卫
        if(ourInterTime[i] < ourInterTime[ourBestInterRobot])
            ourBestInterRobot = i;
        if(theirInterTime[i] < theirInterTime[theirBestInterRobot])
            theirBestInterRobot = i;
    }
    double ourTimeAdvantage = (theirInterTime[theirBestInterRobot] - ourInterTime[ourBestInterRobot]);
    bool our = ourTimeAdvantage > TIME_FOR_OUR;
    bool their = ourTimeAdvantage < TIME_FOR_THEIR;
    bool ourKeep = !our && ourTimeAdvantage > TIME_FOR_OUR_BOTH_KEEP;
    bool theirKeep = !their && ourTimeAdvantage < TIME_FOR_THEIR_BOTH_KEEP;
    //cout << "Time : ourTimeAdvantage : " << ourTimeAdvantage << endl;

    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        const MobileVisionT ball = pVision->ball();
        auto mine = pVision->ourPlayer(i);
        if (!pVision->ourPlayer(i).Valid()) continue;
        if (RobotSensor::Instance()->IsInfraredOn(i)){
            ourBestInterRobot = i;
            ballState = ZBallState::Our;
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1500, PARAM::Field::PITCH_WIDTH / 2), QString("dhz 222 OurBest %1 TheirBest %2").arg(ourBestInterRobot).arg(theirBestInterRobot).toLatin1());
            return;
        }
    }

    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        const MobileVisionT ball = pVision->ball();
        auto mine = pVision->ourPlayer(i);

        if (!pVision->ourPlayer(i).Valid()) continue;

        auto ourDist = (ball.Pos() - mine.Pos()).mod();
        auto ourDir = (ball.Pos() - mine.Pos()).dir();
        auto ourDirError = fabs(mine.Dir() - ourDir);
        if (ourDist < 150 && ourDirError < PARAM::Math::PI / 4){
            ourBestInterRobot = i;
            ballState = ZBallState::Our;
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1500, PARAM::Field::PITCH_WIDTH / 2), QString("dhz 333 OurBest %1 TheirBest %2").arg(ourBestInterRobot).arg(theirBestInterRobot).toLatin1());
            return;
        }
    }

    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
           auto enemy = pVision->theirPlayer(i);
           const MobileVisionT ball = pVision->ball();
           if (!pVision->theirPlayer(i).Valid()) continue;

           auto dist = (ball.Pos() - enemy.Pos()).mod();
           auto dir = (ball.Pos() - enemy.Pos()).dir();
           auto dir_error = fabs(enemy.Dir() - dir);
           if (dist < 150 && dir_error < PARAM::Math::PI / 4){
               ballState = ZBallState::Their;
               theirBestInterRobot = i;
               GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1500, PARAM::Field::PITCH_WIDTH / 2), QString("dhz 111 OurBest %1 TheirBest %2").arg(ourBestInterRobot).arg(theirBestInterRobot).toLatin1());
               return;
           }
    }

    if(our)
        ballState = ZBallState::Our;
    else if(their)
        ballState = ZBallState::Their;
    else if(ourKeep)
        ballState = (ballState == ZBallState::Our ? ZBallState::Our : ZBallState::Both);
    else if(theirKeep)
        ballState = (ballState == ZBallState::Their ? ZBallState::Their : ZBallState::Both);
    else
        ballState = ZBallState::Both;

    if(ballState == ZBallState::Our && ourInterTime[ourBestInterRobot] < TIME_FOR_JUDGE_HOLDING)
        ballState = ZBallState::OurHolding;
    if(ballState == ZBallState::Their && theirInterTime[theirBestInterRobot] < TIME_FOR_JUDGE_HOLDING)
        ballState = ZBallState::TheirHolding;
    if(ballState == ZBallState::Both && (ourInterTime[ourBestInterRobot] < TIME_FOR_JUDGE_HOLDING || theirInterTime[theirBestInterRobot] < TIME_FOR_JUDGE_HOLDING))
        ballState = ZBallState::BothHolding;


    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1500, PARAM::Field::PITCH_WIDTH / 2), QString("OurBest %1 TheirBest %2").arg(ourBestInterRobot).arg(theirBestInterRobot).toLatin1());
//    if(bothHolding)
//        ballState = ZBallState::BothHolding;
//    else if(ourHolding)
//        ballState = ZBallState::OurHolding;
//    else if(theirHolding)
//        ballState = ZBallState::TheirHolding;

}

void SkillUtils::updateTheirGoalie(const CVisionModule* pVision) {
    const static CGeoPoint GOAL_CENTER(PARAM::Field::PITCH_LENGTH/2,0);
    double min_dist = 999999;
    bool in_penalty = false;
    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        auto enemy_pos = pVision->theirPlayer(i).Pos();
        auto dist = (GOAL_CENTER-enemy_pos).mod();
        if(Utils::InTheirPenaltyArea(enemy_pos,0)){
            dist -= 1000;//by wangzai
        }
        if(min_dist > dist){
            theirGoalie = i;
            min_dist = dist;
        }
    }
}

double SkillUtils::getOurBestInterTime() {
    return getOurInterTime(ourBestInterRobot);
}

double SkillUtils::getTheirBestInterTime() {
    return getTheirInterTime(theirBestInterRobot);
}

int SkillUtils::getOurBestPlayer() {
    return ourBestInterRobot;
}

int SkillUtils::getTheirBestPlayer() {
    return theirBestInterRobot;
}
int SkillUtils::getTheirGoalie(){
    return theirGoalie;
}
// CANNOT USE!!!!!!!
// CANNOT USE!!!!!!!
// CANNOT USE!!!!!!!
double SkillUtils::getImmortalRushTime(const CVisionModule *pVision, int robotNum, CGeoPoint targetPos, double targetDir) {
    PlayerVisionT me = pVision->ourPlayer(robotNum);
    int mode = 0;
    if ((me.Pos() - targetPos).mod() < 150) { //almost arrived
        mode = 3;
    } else if ((me.Pos() - targetPos).mod() > 200 && Utils::Normalize(me.Dir() - (me.Pos() - targetPos).dir()) > 0.5) {
        mode = 1;
    } else {
        mode = 2;
    }
    return calcImmortalTime(pVision, robotNum, targetPos, targetDir, mode);
}


CGeoPoint SkillUtils::getMarkingPoint (CGeoPoint markingPos, CVector markingVel, double aMax, double dMax, double aRotateMax, double vMax, CGeoPoint protectPos) {
    if(markingVel.mod() == 0) {
        CVector defDirec = protectPos - markingPos;
        CGeoPoint runPoint = markingPos + defDirec * 0.3;
        return runPoint;
    }
    CVector adjustDirec = protectPos - markingPos;
    CVector D2T_Direc = adjustDirec / adjustDirec.mod();

    double safetyDistL, safetyDistS;
//    if(markingVel * D2T_Direc > 0) {
//        safetyDistL = PARAM::Vehicle::V2::PLAYER_SIZE * (markingVel * D2T_Direc / vMax * 8); //前向安全距离
//        safetyDistS = PARAM::Vehicle::V2::PLAYER_SIZE * (markingVel * D2T_Direc / vMax * 8); //侧向安全距离
//    }
//    else {
    safetyDistL = safetyDistS = PARAM::Vehicle::V2::PLAYER_SIZE * 4;
//    }

    double stopDist = markingVel.mod2() / (2 * dMax);
    double stopTime = markingVel.mod() / dMax;
    double vM = markingVel.mod() + aMax * stopTime;
    double accDist;
    if(vM < vMax) {
        accDist = markingVel.mod() * stopTime + 0.5 * aMax * pow(stopTime, 2);
    } else
        accDist = (vMax * vMax - markingVel.mod2()) / (2 * aMax) + vMax * (stopTime - (vMax - vM) / aMax);

    CVector velDirec = markingVel / markingVel.mod();
    CVector direc1(velDirec.y(), -velDirec.x());
    CVector direc2 = -direc1;
    double R = markingVel.mod2() / aRotateMax;
    double theta1 = markingVel.mod() * stopTime / R;

    CVector direcToTarget = protectPos - markingPos;
    CGeoPoint stopPoint = markingPos + velDirec * stopDist;
    CGeoPoint appPoint = stopPoint + direcToTarget * 0.3;
    CGeoPoint accPoint = markingPos + velDirec * accDist;
    CGeoPoint leftPoint = markingPos + direc1 * R * (1 - cos(theta1)) + velDirec * R * sin(theta1);
    CGeoPoint rightPoint = markingPos + direc2 * R * (1 - cos(theta1)) + velDirec * R * sin(theta1);

    CGeoPoint runPoint;
    if(appPoint.dist(protectPos) < leftPoint.dist(protectPos)) {
        runPoint = appPoint.dist(protectPos) < rightPoint.dist(protectPos) ? stopPoint : rightPoint;
    } else {
        runPoint = leftPoint.dist(protectPos) < rightPoint.dist(protectPos) ? leftPoint : rightPoint;
    }
//根据不同的跑位点情况对跑位点进行安全距离修正；
    if(runPoint == stopPoint && runPoint.dist(markingPos) < safetyDistL)
        runPoint = runPoint + adjustDirec / adjustDirec.mod() * safetyDistL;
    else if(runPoint.dist(markingPos) < safetyDistS)
        runPoint = runPoint + adjustDirec / adjustDirec.mod() * safetyDistS;

    //处理被防守车往反方向运动的情况
    direc1 = runPoint - markingPos;
    direc2 = protectPos - markingPos;
    if(direc1 * direc2 / (direc1.mod() * direc2.mod()) < 0) {
        if(direc2.mod() > PARAM::Vehicle::V2::PLAYER_SIZE * 4)
            runPoint = markingPos + direc2 / direc2.mod() * PARAM::Vehicle::V2::PLAYER_SIZE * 16;
        else
            runPoint = markingPos + direc2 * 0.5;
    }

    return runPoint;
}

double SkillUtils::holdBallDir(const CVisionModule *pVision, int robotNum){
    static const int DIS_THRESHOLD = 800;

    // 计算多人包夹时的角度
    double finalAngle = 0;
    double coeff = 0;
    const PlayerVisionT& me = pVision->ourPlayer(robotNum);
    for(int i=0; i<PARAM::Field::MAX_PLAYER; i++){
        if(!pVision->theirPlayer(i).Valid()) continue;
        if(pVision->theirPlayer(i).Pos().dist(me.Pos()) > DIS_THRESHOLD) continue;
        const PlayerVisionT& enemy = pVision->theirPlayer(i);
        CVector enemy2me = me.Pos() - enemy.Pos();
        double targetAngle = enemy2me.dir()/* > 0 ? 2*PARAM::Math::PI - enemy2me.dir() : -1*enemy2me.dir()*/;
        finalAngle += targetAngle/enemy2me.mod();
        coeff += 1/enemy2me.mod();
    }
    if(std::fabs(finalAngle) < 1e-4) return 1e8;

    finalAngle /= coeff;
    // 计算最佳距离
    double anotherAngle = finalAngle < PARAM::Math::PI ? finalAngle + PARAM::Math::PI : finalAngle - PARAM::Math::PI;
    double diff1 = 0, diff2 = 0;
    for(int i=0; i<PARAM::Field::MAX_PLAYER; i++){
        if(!pVision->theirPlayer(i).Valid()) continue;
        if(pVision->theirPlayer(i).Pos().dist(me.Pos()) > DIS_THRESHOLD) continue;
        const PlayerVisionT& enemy = pVision->theirPlayer(i);
        CVector enemy2me = me.Pos() - enemy.Pos();
        double targetAngle = enemy2me.dir()/* > 0 ? 2*PARAM::Math::PI - enemy2me.dir() : -1*enemy2me.dir()*/;
        double d_angle1 = fabs(targetAngle-finalAngle) < PARAM::Math::PI ? fabs(targetAngle-finalAngle) : 2*PARAM::Math::PI - fabs(targetAngle-finalAngle);
        diff1 += d_angle1/enemy2me.mod();
        double d_angle2 = abs(targetAngle-anotherAngle) < PARAM::Math::PI ? fabs(targetAngle-anotherAngle) : 2*PARAM::Math::PI - fabs(targetAngle-anotherAngle);
        diff2 += d_angle2/enemy2me.mod();
    }
    if(diff1 > diff2)finalAngle = anotherAngle;
    return finalAngle;
}

bool SkillUtils::predictedInterTimeV2(const CVector ballVel, const CGeoPoint ballPos, const PlayerVisionT me, CGeoPoint& interceptPoint, double& interTime, double responseTime) {
    double ballAcc = FRICTION / 2;//球减速度
    double ballArriveTime = 0;
    double meArriveTime = 99999;
    double testBallLength = 0;//球移动距离
    CGeoPoint testPoint = ballPos;
    double testVel = ballVel.mod();
    double max_time = ballVel.mod() / ballAcc;
    CGeoLine ballLine(ballPos, ballVel.dir());
    CGeoPoint ballLineProjection = ballLine.projection(me.Pos());
    CVector projection2me = me.Pos() - ballLineProjection;

    double width = projection2me.mod() < PARAM::Vehicle::V2::PLAYER_SIZE ? projection2me.mod() : PARAM::Vehicle::V2::PLAYER_SIZE;
    for (ballArriveTime = 0; ballArriveTime < max_time; ballArriveTime += DELTA_T ) {
        testVel = ballVel.mod() - ballAcc * ballArriveTime; //v_0-at
        testBallLength = PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER + (ballVel.mod() + testVel) * ballArriveTime / 2; //梯形法计算球移动距离
        testPoint = ballPos + Utils::Polar2Vector(testBallLength, ballVel.dir());
        meArriveTime = predictedTime(me, testPoint + Utils::Polar2Vector(width, projection2me.dir()));//我到截球点的时间
        if(meArriveTime < 0.15) meArriveTime = 0;
        if (!Utils::IsInField(testPoint) || (meArriveTime + responseTime) < ballArriveTime) {
            break;
        }
    }
    interceptPoint = testPoint;//截球点
    interTime = predictedTime(me, interceptPoint);//截球时间
    return true;
}

CGeoPoint SkillUtils::getZMarkingPos(const CVisionModule* pVision, const int robotNum, const int enemyNum, const int pri, const int flag){
    constexpr bool VERBOSE_MODE = false;
    constexpr double MARKING_DIST_RATE = 0.8;
    bool half_field_mode = flag & PlayerStatus::AVOID_HALF_FIELD;
    const PlayerVisionT& enemy = pVision->theirPlayer(enemyNum);
    const MobileVisionT& ball = pVision->ball();

    CGeoPoint interceptPoint = predictedTheirInterPoint(enemy.Pos(), ball.RawPos());
    double interDist = MARKING_DIST_RATE * (enemy.Pos() - interceptPoint).mod();
    if(VERBOSE_MODE){
        GDebugEngine::Instance()->gui_debug_arc(interceptPoint, interDist, 0.0, 360.0, COLOR_ORANGE);
//        GDebugEngine::Instance()->gui_debug_line(me.Pos(),interceptPoint,COLOR_YELLOW);
//        GDebugEngine::Instance()->gui_debug_line(CGeoPoint(-600,0),interceptPoint,COLOR_YELLOW);
    }
    if (interDist < PARAM::Vehicle::V2::PLAYER_SIZE * 2 + 2)
        interDist = PARAM::Vehicle::V2::PLAYER_SIZE * 2 + 2;

    CVector point2goal = Utils::Polar2Vector(interDist, (CGeoPoint(-PARAM::Field::PITCH_LENGTH/2, 0)- interceptPoint).dir());
    CGeoPoint markingPos = interceptPoint + point2goal;
    if(! enemy.Valid()){
        markingPos = CGeoPoint(-150*10 - 30*10*pri, 0);
        return markingPos;
    }
    auto inter2ball = ball.RawPos() - interceptPoint;
    bool noPoint = false;
    bool havePoint = false;
    double sign = Utils::Normalize(point2goal.dir() - inter2ball.dir()) > 0 ? -1 : 1;
    for (double test_dist = point2goal.mod(); test_dist > PARAM::Vehicle::V2::PLAYER_SIZE * 2; test_dist -= point2goal.mod()/10){
        if(havePoint) break;
        int cnt = 0;
        for(double test_dir = point2goal.dir(); cnt < 9; test_dir = Utils::Normalize(test_dir - sign*Utils::Normalize(point2goal.dir() - inter2ball.dir())/10)){
            cnt ++;
            if(havePoint) break;
            auto test_point = interceptPoint + Utils::Polar2Vector(test_dist, test_dir);
            if(Utils::InOurPenaltyArea(test_point, PARAM::Vehicle::V2::PLAYER_SIZE) || Utils::InTheirPenaltyArea(markingPos, PARAM::Vehicle::V2::PLAYER_SIZE)) continue;
            if(!Utils::IsInField(test_point, PARAM::Vehicle::V2::PLAYER_SIZE)) continue;

            if(VERBOSE_MODE) GDebugEngine::Instance()->gui_debug_x(test_point, COLOR_WHITE);
            if(VERBOSE_MODE) GDebugEngine::Instance()->gui_debug_arc(test_point, 5*10, 0.0f, 360.0f, COLOR_WHITE);
            noPoint = false;
            for(int i =0; i<PARAM::Field::MAX_PLAYER; i++){
                if((i)==robotNum) continue;
                if(!pVision->ourPlayer(i).Valid()) continue;
                if((test_point - pVision->ourPlayer(i).RawPos()).mod() < PARAM::Vehicle::V2::PLAYER_SIZE * 2 + 2*10){
                    noPoint = true;
                    if(VERBOSE_MODE) GDebugEngine::Instance()->gui_debug_arc(pVision->ourPlayer(i).RawPos(), 10*10, 0.0f, 360.0f, COLOR_CYAN);
                    break;
                }
            }
            if(!noPoint){
                markingPos = test_point;
                noPoint = false;
                havePoint = true;
                break;
            }
        }
    }
    if(VERBOSE_MODE){
        GDebugEngine::Instance()->gui_debug_x(markingPos, COLOR_PURPLE);
        GDebugEngine::Instance()->gui_debug_arc(markingPos, 8, 0.0f, 360.0f, COLOR_PURPLE);
    }
    //防止车出场
    if (!Utils::IsInField(markingPos, PARAM::Vehicle::V2::PLAYER_SIZE * 2))
        markingPos = Utils::MakeInField(markingPos, PARAM::Vehicle::V2::PLAYER_SIZE * 2);
    //防止车进入禁区
    if (Utils::InOurPenaltyArea(markingPos, PARAM::Vehicle::V2::PLAYER_SIZE * 2)){
        CGeoLine markingLine = CGeoLine(markingPos, enemy.Pos());
        double buffer = PARAM::Vehicle::V2::PLAYER_SIZE;
        CGeoRectangle penalty= CGeoRectangle(CGeoPoint(-PARAM::Field::PITCH_LENGTH / 2, -PARAM::Field::PENALTY_AREA_WIDTH / 2 - buffer), CGeoPoint(-PARAM::Field::PITCH_LENGTH / 2 + PARAM::Field::PENALTY_AREA_DEPTH+buffer, PARAM::Field::PENALTY_AREA_WIDTH / 2+buffer));
        CGeoLineRectangleIntersection intersection = CGeoLineRectangleIntersection(markingLine, penalty);
        auto point1 = intersection.point1();
        auto point2 = intersection.point2();
        double p1_l = (point1 - enemy.Pos()).mod();
        double p2_l = (point2 - enemy.Pos()).mod();
        if(p1_l < p2_l){
            if(fabs(point1.x()) > 1e-4  && fabs(point1.y()) > 1e-4) markingPos = point1;
            else markingPos = Utils::MakeOutOfOurPenaltyArea(markingPos, buffer);
        }
        else{
            if(fabs(point2.x()) > 1e-4 && fabs(point2.y()) > 1e-4) markingPos = point2;
            else markingPos = Utils::MakeOutOfOurPenaltyArea(markingPos, buffer);
        }
    }
    //防止禁区前车挤车,绕前marking
    if ((markingPos - enemy.Pos()).mod() < 23 * 10)
        markingPos = enemy.Pos() + Utils::Polar2Vector(20 * 10, (ball.Pos()- enemy.Pos()).dir());
    if(VERBOSE_MODE){
        GDebugEngine::Instance()->gui_debug_line(enemy.Pos(),ball.Pos(),COLOR_CYAN);
        GDebugEngine::Instance()->gui_debug_line(enemy.Pos(),CGeoPoint(-600 * 10,0),COLOR_CYAN);
    }
    if(half_field_mode){
        if(markingPos.x() > -15 * 10) markingPos.setX(-15 * 10);
    }
    return markingPos;
}
