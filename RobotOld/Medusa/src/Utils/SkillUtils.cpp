#include "SkillUtils.h"
#include "parammanager.h"
#include "GDebugEngine.h"
#include "param.h"
#include <cmath>
#include "GoImmortalRush.h"
#include "ballmodel.h"
using namespace std;
namespace {
    bool DEBUG = false;
    constexpr double TIME_FOR_OUR = 0.3;
    constexpr double TIME_FOR_OUR_BOTH_KEEP = 0.1;
    constexpr double TIME_FOR_THEIR_BOTH_KEEP = -0.1;
    constexpr double TIME_FOR_THEIR = -0.3;
    constexpr double TIME_FOR_JUDGE_HOLDING = 0.5;
    const double DELTA_T = 10.0 / Param::Vision::FRAME_RATE;
}
SkillUtils::SkillUtils() {
    bool isSimulation;
    ZSS::ZParamManager::instance()->loadParam(isSimulation, "Alert/IsSimulation", false);
    if(isSimulation)
        ZSS::ZParamManager::instance()->loadParam(FRICTION, "AlertParam/Friction4Sim", 1520.0);
    else
        ZSS::ZParamManager::instance()->loadParam(FRICTION, "AlertParam/Friction4Real", 800.0);
    ZSS::ZParamManager::instance()->loadParam(DEBUG, "Debug/InterTimeDisplay", false);
}
SkillUtils::~SkillUtils() {
}

bool SkillUtils::predictedInterTime(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, double responseTime) {
    const MobileVisionT ball = pVision->ball();//获得球
    const PlayerVisionT me = pVision->ourPlayer(robotNum);//获得车
    static const double ballAcc = FRICTION / 2;//球减速度
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
    double width = projection2me.mod() < Param::Vehicle::V2::PLAYER_SIZE ? projection2me.mod() : Param::Vehicle::V2::PLAYER_SIZE;
    for (ballArriveTime = 0; ballArriveTime < max_time; ballArriveTime += DELTA_T) {
        testVel = ball.Vel().mod() - ballAcc * ballArriveTime; //v_0-at
        testBallLength = Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER + (ball.Vel().mod() + testVel) * ballArriveTime / 2; //梯形法计算球移动距离
        testPoint = ball.Pos() + Utils::Polar2Vector(testBallLength, ball.Vel().dir());
        CVector me2testPoint = testPoint - me.RawPos();
        meArriveTime = predictedTime(me, testPoint + Utils::Polar2Vector(width, projection2me.dir()));//我到截球点的时间
        if(meArriveTime < 0.2) meArriveTime = 0;
        if(me.Vel().mod() < 1500 /*&& projection2me.mod() < 2*Param::Vehicle::V2::PLAYER_SIZE*/ && me2testPoint.mod() < 3*Param::Vehicle::V2::PLAYER_SIZE) meArriveTime = 0;
//        if(me.Pos().dist(testPoint) < Param::Vehicle::V2::PLAYER_SIZE*2 && meArriveTime < 0.3) meArriveTime = 0;
        if (!IsInField(testPoint) || (meArriveTime + responseTime) < ballArriveTime) {
            break;
        }
    }
    interceptPoint = testPoint;//截球点
    interTime = predictedTime(me, interceptPoint);//截球时间

    double minDist = 999999;
    for(int i=0; i<Param::Field::MAX_PLAYER_NUM; i++){
        auto enemy = pVision->theirPlayer(i+1);
        if(!enemy.Valid()) continue;
        CVector enemy2ballLine = ballLine.projection(enemy.Pos()) - enemy.Pos();
        CVector ball2enemy = enemy.Pos() - ball.RawPos();
        if(enemy2ballLine.mod() < 1200 && (fabs(Utils::Normalize(ball.Vel().dir() - (enemy.Pos()-ball.RawPos()).dir()))<Param::Math::PI/6) && enemy.Vel().mod() < 2000 && (ball2enemy.mod() < ball.Vel().mod2()/FRICTION)){
            minDist = std::min(minDist, ball2enemy.mod());
//            GDebugEngine::Instance()->gui_debug_arc(enemy.Pos(), 30, 0.0f, 360.0f, COLOR_GREEN);
//            GDebugEngine::Instance()->gui_debug_msg(enemy.Pos(), QString("dist: %1  dir: %2").arg(enemy2ballLine.mod()).arg(fabs(Utils::Normalize(ball.Vel().dir() - (enemy.Pos()-ball.RawPos()).dir())/Param::Math::PI*180)).toLatin1(), COLOR_WHITE);
        }
    }
    if(minDist<888888){
        CGeoPoint new_interceptPoint = ball.RawPos() + Utils::Polar2Vector(std::max(minDist-Param::Vehicle::V2::PLAYER_SIZE,0.0), ball.Vel().dir());
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
    static const double AVOID_DIST = Param::Vehicle::V2::PLAYER_SIZE;
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
        if(me.Vel().mod() < 1500 && me2testPoint.mod() < 3*Param::Vehicle::V2::PLAYER_SIZE) meArriveTime = 0;
        if (!IsInField(testPoint) || (meArriveTime + responseTime) < ballArriveTime) {
            break;
        }
    }
    interceptPoint = testPoint;//截球点
    interTime = predictedTheirTime(me, interceptPoint);//截球时间
    return true;
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

bool SkillUtils::IsInField(const CGeoPoint p, double buffer) {
    return (p.x() > buffer - Param::Field::PITCH_LENGTH / 2 && p.x() < Param::Field::PITCH_LENGTH / 2 - buffer &&
            p.y() > buffer - Param::Field::PITCH_WIDTH / 2 && p.y() < Param::Field::PITCH_WIDTH / 2 - buffer);
}

bool SkillUtils::IsInFieldV2(const CGeoPoint p, double buffer) {
    return (IsInField(p, buffer) && !Utils::InOurPenaltyArea(p, buffer) && !Utils::InTheirPenaltyArea(p, buffer));
}

bool SkillUtils::IsBallReachable(double ballVel, double length, double friction) {
    double max_dist = ballVel * ballVel / (2 * friction);
    if (length < max_dist) return true;
    else return false;
}

void SkillUtils::generateInterInformation(const CVisionModule* pVision) {
    for(int robotNum = 1; robotNum <= Param::Field::MAX_PLAYER_NUM; robotNum++) {
        predictedInterTime(pVision, robotNum, ourInterPoint[robotNum], ourInterTime[robotNum], 0.15);
        predictedTheirInterTime(pVision, robotNum, theirInterPoint[robotNum], theirInterTime[robotNum], 0);
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
    for(int robotNum = 1; robotNum <= Param::Field::MAX_PLAYER_NUM; robotNum++) {
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
            if (!IsInField(testPoint)) {
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

bool SkillUtils::validShootPos(const CVisionModule *pVision, CGeoPoint shootPos, double shootVel, CGeoPoint target, const double responseTime, double ignoreCloseEnemyDist, bool ignoreTheirGoalie, bool ignoreTheirGuard, bool DEBUG_MODE) {
    double AVOID_DIST = 4 * Param::Vehicle::V2::PLAYER_SIZE;
    if (Utils::InTheirPenaltyArea(target, 0)) AVOID_DIST = 2 * Param::Vehicle::V2::PLAYER_SIZE;

    const CVector passLine = target - shootPos;
    const double passLineDist = passLine.mod() - Param::Vehicle::V2::PLAYER_SIZE;
    const double passLineDir = passLine.dir();
    const CVector vecVel = Utils::Polar2Vector(shootVel, passLineDir);
    if(DEBUG_MODE) GDebugEngine::Instance()->gui_debug_x(target, COLOR_CYAN);

    for(int robotNum = 1; robotNum <= Param::Field::MAX_PLAYER_NUM; robotNum++) {
        const PlayerVisionT& me = pVision->theirPlayer(robotNum);//获得车
        bool isGoalie = Utils::InTheirPenaltyArea(me.Pos(), 0);
        if (!me.Valid()) continue;
        // 忽略距离近的防守车
        if(me.Pos().dist(shootPos) < ignoreCloseEnemyDist && !Utils::InTheirPenaltyArea(me.Pos(), 0)) continue;
        // 判断是否已经在球线上
        CGeoSegment ballLine(shootPos, target);
        if(ballLine.dist2Point(me.Pos()) < AVOID_DIST)
            return false;
        // 忽略对面的守门员
        if(ignoreTheirGoalie && Utils::InTheirPenaltyArea(me.Pos(), 0)) continue;
        // 忽略对面的后卫
        if(ignoreTheirGuard && Utils::InTheirPenaltyArea(me.Pos(), 300)) continue;
        // 初始化球
        double ballMoveTime = 0;
        while (true) {
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
            if (!IsInField(ballPos) || ballMoveDist >= passLineDist) {
                break;
            }
            CVector adjustDir = me.Pos() - ballPos;
            CGeoPoint testPoint = ballPos + Utils::Polar2Vector(AVOID_DIST, adjustDir.dir());
            double meArriveTime = predictedTheirTime(me, testPoint);
            // 判断截球点是否合理
            bool interPosValid = (!isGoalie && !Utils::InTheirPenaltyArea(ballPos, 0)) || (isGoalie && Utils::InTheirPenaltyArea(ballPos, 0));
            if(interPosValid && ballMoveDist < passLineDist && (ballVel < 1e-8 || meArriveTime + responseTime < ballMoveTime)){
                if(DEBUG_MODE) GDebugEngine::Instance()->gui_debug_msg(ballPos, QString("%1 %2").arg(meArriveTime).arg(ballMoveTime).toLatin1(), COLOR_CYAN);
                return false;
            }
            if(!interPosValid && ballVel < 1e-8) return false;
        }
    }
    return true;//for循环中所有车都不能截球，没有return false
}

bool SkillUtils::validChipPos(const CVisionModule *pVision, CGeoPoint shootPos, double shootVel, CGeoPoint target, const double responseTime, /*double ignoreCloseEnemyDist, */bool ignoreTheirGuard, bool DEBUG_MODE) {
    const double CHIP_FIRST_ANGLE = 54.29 / 180.0 * Param::Math::PI;
    const double CHIP_SECOND_ANGLE = 45.59 / 180.0 * Param::Math::PI;
    constexpr double CHIP_LENGTH_RATIO = 1.266;
    constexpr double CHIP_VEL_RATIO = 0.6372;
    constexpr double G = 9.8;
    static const double BALL_ACC = FRICTION / 2;
    static const double AVOID_DIST = 4 * Param::Vehicle::V2::PLAYER_SIZE;

    const CVector passLine = target - shootPos;
    const double passLineDist = passLine.mod() - Param::Vehicle::V2::PLAYER_SIZE;
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

    for(int robotNum = 1; robotNum <= Param::Field::MAX_PLAYER_NUM; robotNum++) {
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
            if (!IsInField(ballPos) || ballMoveDist >= passLineDist) {
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
    for(int enemyNum = 1; enemyNum < Param::Field::MAX_PLAYER_NUM + 1; enemyNum++) {
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
    generateInterInformation(pVision);
    generatePredictInformation(pVision);
    calculateBallBelongs(pVision);
    updateTheirGoalie(pVision);
    if(DEBUG) {
        for(int i = 1; i <= Param::Field::MAX_PLAYER_NUM; i++) {
            GDebugEngine::Instance()->gui_debug_x(theirInterPoint[i]);
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(2000, 3000 - i * 300), QString("%1 %2").arg(i).arg(theirInterTime[i]).toLatin1());
            GDebugEngine::Instance()->gui_debug_x(ourInterPoint[i], COLOR_WHITE);
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-2000, 3000 - i * 300), QString("%1 %2").arg(i).arg(ourInterTime[i]).toLatin1(), COLOR_WHITE);
        }
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-2000, -2000), QString("OurBest : %1").arg(ourBestInterRobot).toLatin1(), COLOR_WHITE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(2000, -2000), QString("TheirBest : %1").arg(theirBestInterRobot).toLatin1());
    }
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, Param::Field::PITCH_WIDTH / 2), ZBallState::toStr[ballState].c_str(), COLOR_CYAN);
}
void SkillUtils::calculateBallBelongs(const CVisionModule* pVision) {
    for(int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
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


    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1500, Param::Field::PITCH_WIDTH / 2), QString("OurBest %1 TheirBest %2").arg(ourBestInterRobot - 1).arg(theirBestInterRobot - 1).toLatin1());
//    if(bothHolding)
//        ballState = ZBallState::BothHolding;
//    else if(ourHolding)
//        ballState = ZBallState::OurHolding;
//    else if(theirHolding)
//        ballState = ZBallState::TheirHolding;

}

void SkillUtils::updateTheirGoalie(const CVisionModule* pVision) {
    const static CGeoPoint GOAL_CENTER(Param::Field::PITCH_LENGTH/2,0);
    double min_dist = 999999;
    bool in_penalty = false;
    for(int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
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
//        safetyDistL = Param::Vehicle::V2::PLAYER_SIZE * (markingVel * D2T_Direc / vMax * 8); //前向安全距离
//        safetyDistS = Param::Vehicle::V2::PLAYER_SIZE * (markingVel * D2T_Direc / vMax * 8); //侧向安全距离
//    }
//    else {
    safetyDistL = safetyDistS = Param::Vehicle::V2::PLAYER_SIZE * 4;
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
        if(direc2.mod() > Param::Vehicle::V2::PLAYER_SIZE * 4)
            runPoint = markingPos + direc2 / direc2.mod() * Param::Vehicle::V2::PLAYER_SIZE * 16;
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
    for(int i=1; i<=Param::Field::MAX_PLAYER_NUM; i++){
        if(!pVision->theirPlayer(i).Valid()) continue;
        if(pVision->theirPlayer(i).Pos().dist(me.Pos()) > DIS_THRESHOLD) continue;
        const PlayerVisionT& enemy = pVision->theirPlayer(i);
        CVector enemy2me = me.Pos() - enemy.Pos();
        double targetAngle = enemy2me.dir()/* > 0 ? 2*Param::Math::PI - enemy2me.dir() : -1*enemy2me.dir()*/;
        finalAngle += targetAngle/enemy2me.mod();
        coeff += 1/enemy2me.mod();
    }
    if(finalAngle == 0) return 1e8;

    finalAngle /= coeff;
    // 计算最佳距离
    double anotherAngle = finalAngle < Param::Math::PI ? finalAngle + Param::Math::PI : finalAngle - Param::Math::PI;
    double diff1 = 0, diff2 = 0;
    for(int i=1; i<=Param::Field::MAX_PLAYER_NUM; i++){
        if(!pVision->theirPlayer(i).Valid()) continue;
        if(pVision->theirPlayer(i).Pos().dist(me.Pos()) > DIS_THRESHOLD) continue;
        const PlayerVisionT& enemy = pVision->theirPlayer(i);
        CVector enemy2me = me.Pos() - enemy.Pos();
        double targetAngle = enemy2me.dir()/* > 0 ? 2*Param::Math::PI - enemy2me.dir() : -1*enemy2me.dir()*/;
        double d_angle1 = fabs(targetAngle-finalAngle) < Param::Math::PI ? fabs(targetAngle-finalAngle) : 2*Param::Math::PI - fabs(targetAngle-finalAngle);
        diff1 += d_angle1/enemy2me.mod();
        double d_angle2 = abs(targetAngle-anotherAngle) < Param::Math::PI ? fabs(targetAngle-anotherAngle) : 2*Param::Math::PI - fabs(targetAngle-anotherAngle);
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

    double width = projection2me.mod() < Param::Vehicle::V2::PLAYER_SIZE ? projection2me.mod() : Param::Vehicle::V2::PLAYER_SIZE;
    for (ballArriveTime = 0; ballArriveTime < max_time; ballArriveTime += DELTA_T ) {
        testVel = ballVel.mod() - ballAcc * ballArriveTime; //v_0-at
        testBallLength = Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER + (ballVel.mod() + testVel) * ballArriveTime / 2; //梯形法计算球移动距离
        testPoint = ballPos + Utils::Polar2Vector(testBallLength, ballVel.dir());
        meArriveTime = predictedTime(me, testPoint + Utils::Polar2Vector(width, projection2me.dir()));//我到截球点的时间
        if(meArriveTime < 0.15) meArriveTime = 0;
        if (!IsInField(testPoint) || (meArriveTime + responseTime) < ballArriveTime) {
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
        GDebugEngine::Instance()->gui_debug_arc(interceptPoint, interDist, 0.0f, 360.0f, COLOR_ORANGE);
//        GDebugEngine::Instance()->gui_debug_line(me.Pos(),interceptPoint,COLOR_YELLOW);
//        GDebugEngine::Instance()->gui_debug_line(CGeoPoint(-600,0),interceptPoint,COLOR_YELLOW);
    }
    if (interDist < Param::Vehicle::V2::PLAYER_SIZE * 2 + 2)
        interDist = Param::Vehicle::V2::PLAYER_SIZE * 2 + 2;

    CVector point2goal = Utils::Polar2Vector(interDist, (CGeoPoint(-Param::Field::PITCH_LENGTH/2, 0)- interceptPoint).dir());
    CGeoPoint markingPos = interceptPoint + point2goal;
    if(! enemy.Valid()){
        markingPos = CGeoPoint(-150*10 - 30*10*pri, 0);
        return markingPos;
    }
    auto inter2ball = ball.RawPos() - interceptPoint;
    bool noPoint = false;
    bool havePoint = false;
    double sign = Utils::Normalize(point2goal.dir() - inter2ball.dir()) > 0 ? -1 : 1;
    for (double test_dist = point2goal.mod(); test_dist > Param::Vehicle::V2::PLAYER_SIZE * 2; test_dist -= point2goal.mod()/10){
        if(havePoint) break;
        int cnt = 0;
        for(double test_dir = point2goal.dir(); cnt < 9; test_dir = Utils::Normalize(test_dir - sign*Utils::Normalize(point2goal.dir() - inter2ball.dir())/10)){
            cnt ++;
            if(havePoint) break;
            auto test_point = interceptPoint + Utils::Polar2Vector(test_dist, test_dir);
            if(Utils::InOurPenaltyArea(test_point, Param::Vehicle::V2::PLAYER_SIZE) || Utils::InTheirPenaltyArea(markingPos, Param::Vehicle::V2::PLAYER_SIZE)) continue;
            if(Utils::OutOfField(test_point, Param::Vehicle::V2::PLAYER_SIZE)) continue;

            if(VERBOSE_MODE) GDebugEngine::Instance()->gui_debug_x(test_point, COLOR_WHITE);
            if(VERBOSE_MODE) GDebugEngine::Instance()->gui_debug_arc(test_point, 5*10, 0.0f, 360.0f, COLOR_WHITE);
            noPoint = false;
            for(int i =0; i<Param::Field::MAX_PLAYER_NUM; i++){
                if((i+1)==robotNum) continue;
                if(!pVision->ourPlayer(i+1).Valid()) continue;
                if((test_point - pVision->ourPlayer(i+1).RawPos()).mod() < Param::Vehicle::V2::PLAYER_SIZE * 2 + 2*10){
                    noPoint = true;
                    if(VERBOSE_MODE) GDebugEngine::Instance()->gui_debug_arc(pVision->ourPlayer(i+1).RawPos(), 10*10, 0.0f, 360.0f, COLOR_CYAN);
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
    if (Utils::OutOfField(markingPos, Param::Vehicle::V2::PLAYER_SIZE * 2))
        markingPos = Utils::MakeInField(markingPos, Param::Vehicle::V2::PLAYER_SIZE * 2);
    //防止车进入禁区
    if (Utils::InOurPenaltyArea(markingPos, Param::Vehicle::V2::PLAYER_SIZE * 2)){
        CGeoLine markingLine = CGeoLine(markingPos, enemy.Pos());
        double buffer = Param::Vehicle::V2::PLAYER_SIZE;
        CGeoRectangle penalty= CGeoRectangle(CGeoPoint(-Param::Field::PITCH_LENGTH / 2, -Param::Field::PENALTY_AREA_WIDTH / 2 - buffer), CGeoPoint(-Param::Field::PITCH_LENGTH / 2 + Param::Field::PENALTY_AREA_DEPTH+buffer, Param::Field::PENALTY_AREA_WIDTH / 2+buffer));
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
