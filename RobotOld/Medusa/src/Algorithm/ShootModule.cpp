#include "ShootModule.h"
#include <ShootRangeList.h>
#include <RobotSensor.h>
#include "BallStatus.h"
#include "GDebugEngine.h"
#include "Compensate.h"
#include<fstream>
#include<sstream>
#include "Global.h"
#include "parammanager.h"
#include "kickregulation.h"
namespace {
    CGeoPoint Shoot2Goal;
    bool TEST_PASS;
    bool FORCE_SHOOT;
    bool VERBOSE_MODE = false;
    bool isDebug = false;
    double compensatevalue[100][50];
}

CShootModule::CShootModule( )
{
    reset();
    Shoot2Goal = CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0);
    ZSS::ZParamManager::instance()->loadParam(TEST_PASS, "Messi/NoShoot", false);
    ZSS::ZParamManager::instance()->loadParam(FORCE_SHOOT, "Messi/FORCE_SHOOT", true);
    ZSS::ZParamManager::instance()->loadParam(_needBallVel, "KickLimit/FlatKickMax", 630.0);
    ZSS::ZParamManager::instance()->loadParam(tolerance, "Shoot/Tolerance", 20);    //有待提升，即应该根据kicker位置，自适应tolerance
    ZSS::ZParamManager::instance()->loadParam(stepSize, "Shoot/StepSize", 1);
    ZSS::ZParamManager::instance()->loadParam(responseTime, "Shoot/ResponseTime", 0);
    stepSize = stepSize / 180 * Param::Math::PI;    //转换为弧度制
    const string path = Param::File::PlayBookPath;
    string fullname = path + COMPENSATE_FILE_NAME;
    ifstream infile(fullname.c_str());
    if (!infile) {
        cerr << "error opening file data"<< endl;
        exit(-1);
    }
    string line;
    int rowcount;
    int columncount;
    getline(infile,line);
    istringstream lineStream(line);
    lineStream>>rowcount>>columncount;
    for (int i =0;i<rowcount;i++){
        getline(infile,line);
        istringstream lineStream(line);
        for(int j=0;j<columncount;j++){
            lineStream>>compensatevalue[i][j];
        }
    }
}

void CShootModule::reset()
{
    _kick_valid = false;
    _is_compensated = false;

    _raw_kick_dir = 0.0;
    _compensate_value = 0.0;
    _real_kick_dir = 1.0;

    _kick_target = Shoot2Goal;
    _compensate_factor = 0.0;
}

bool CShootModule::generateShootDir(const int player)
{
    //进行重置
    reset();
    CVisionModule* pVision = vision;
    const MobileVisionT & ball = pVision->Ball();
    const PlayerVisionT & kicker = pVision->OurPlayer(player);
    const CGeoPoint & ballPos = ball.Pos();
    const CGeoPoint & kickerPos = kicker.Valid() ? kicker.RawPos() : kicker.Pos();
    CGeoPoint bestTarget;
    _kick_valid = generateBestTarget(pVision, bestTarget, ballPos);
    _kick_target = bestTarget;
    _raw_kick_dir  = (bestTarget- kickerPos).dir();
    _real_kick_dir = (bestTarget- kickerPos).dir();

    if(!_kick_valid) {
        GDebugEngine::Instance()->gui_debug_line(kickerPos, _kick_target, COLOR_BLUE);
        GDebugEngine::Instance()->gui_debug_x(_kick_target, COLOR_BLUE);
    }else {
        GDebugEngine::Instance()->gui_debug_line(kickerPos, _kick_target, COLOR_RED);
        GDebugEngine::Instance()->gui_debug_x(_kick_target, COLOR_RED);
    }

    if (VERBOSE_MODE) {
        GDebugEngine::Instance()->gui_debug_line(kickerPos,kickerPos+Utils::Polar2Vector(1000,_real_kick_dir),COLOR_CYAN);
        GDebugEngine::Instance()->gui_debug_line(kickerPos,kickerPos+Utils::Polar2Vector(1000,_raw_kick_dir),COLOR_WHITE);
        GDebugEngine::Instance()->gui_debug_line(kickerPos,kickerPos+Utils::Polar2Vector(1000,kicker.Dir()),COLOR_BLACK);
    }
    //******BestTarget已经确定******
    int compensateType = 0;
    if(RobotSensor::Instance()->fraredOn(player) > 5) {
        //吸的时间较长，应该用速度合成模型
        KickRegulation::instance()->regulate(player, _kick_target, _needBallVel, _real_kick_dir);
        compensateType = 1;
    }else {
        //吸的时间较短，查表方式补偿
        double ballspeed = ball.Vel().mod();

        double tempdir = (Utils::Normalize(Utils::Normalize(pVision->Ball().Vel().dir()+Param::Math::PI)-(_kick_target - pVision->OurPlayer(player).Pos()).dir()))*180/Param::Math::PI;
        int ratio = 0;
        if (tempdir>0){
            ratio = 1;
        }else{
            ratio = -1;
        }
        rawdir=abs((Utils::Normalize(Utils::Normalize(pVision->Ball().Vel().dir()+Param::Math::PI)-(_kick_target - pVision->OurPlayer(player).Pos()).dir()))*180/Param::Math::PI);
        // cout << rawdir << endl;
        if (rawdir > 70 && rawdir < 110){
            rawdir = 80;
        }

        shoot_speed = ballspeed;
        shoot_dir = rawdir;

        _compensate_value = Compensate::Instance()->checkCompensate(ballspeed,rawdir);

        if (pVision->Ball().Vel().mod()<50){
            _compensate_value = 0;
        }
        _real_kick_dir= Utils::Normalize(Utils::Normalize(ratio*_compensate_value*Param::Math::PI/180)+_raw_kick_dir);
        if(pVision->Ball().Vel().mod()<50){
            _real_kick_dir = _raw_kick_dir;
        }
        compensateType = 2;
    }
    GDebugEngine::Instance()->gui_debug_line(kickerPos, kickerPos+Utils::Polar2Vector(1000,_real_kick_dir),COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300, -100), QString("%1").arg(compensateType).toLatin1(), COLOR_RED);
    return getKickValid();
}

bool CShootModule::validShootPos(const CVisionModule *pVision, CGeoPoint shootPos, double shootVel, CGeoPoint target, double &interTime, const double responseTime, double ignoreCloseEnemyDist, bool ignoreTheirGoalie, bool ignoreTheirGuard, bool DEBUG_MODE) {
    bool isSimulation;
    double FRICTION;
    ZSS::ZParamManager::instance()->loadParam(isSimulation, "Alert/IsSimulation", false);
    if(isSimulation)
        ZSS::ZParamManager::instance()->loadParam(FRICTION, "AlertParam/Friction4Sim", 152.0);
    else
        ZSS::ZParamManager::instance()->loadParam(FRICTION, "AlertParam/Friction4Real", 80.0);
    static const double BALL_ACC = FRICTION / 2;
    constexpr double BALL_VEL_DECAY = 5.0 / 7.0;
    double AVOID_DIST = 4 * Param::Vehicle::V2::PLAYER_SIZE;
    const double DELTA_T = 5.0/75;
    if (Utils::InTheirPenaltyArea(target, 0)) AVOID_DIST = 2 * Param::Vehicle::V2::PLAYER_SIZE;

    const CVector passLine = target - shootPos;
    const double passLineDist = passLine.mod() - Param::Vehicle::V2::PLAYER_SIZE;
    const double passLineDir = passLine.dir();
    const double originVel = shootVel;
    if(DEBUG_MODE) GDebugEngine::Instance()->gui_debug_x(target, COLOR_CYAN);

    interTime = 999999;
    for(int robotNum = 1; robotNum <= Param::Field::MAX_PLAYER_NUM; robotNum++) {
        const PlayerVisionT& enemy = pVision->TheirPlayer(robotNum);//获得车（敌方车）
        bool isGoalie = Utils::InTheirPenaltyArea(enemy.Pos(), 0);
        if (!enemy.Valid()) continue;
        // 忽略距离近的防守车
        if(enemy.Pos().dist(shootPos) < ignoreCloseEnemyDist && !Utils::InTheirPenaltyArea(enemy.Pos(), 0)) continue;
        // 判断是否已经在球线上
        CGeoSegment ballLine(shootPos, target);
        if(ballLine.dist2Point(enemy.Pos()) < AVOID_DIST){
            interTime = -10; //对于被敌人挡住的点，统一取-10
            return false;   //相当于写死的距离AVOID_DIST,很粗暴
        }
        // 忽略对面的守门员
        if(ignoreTheirGoalie && Utils::InTheirPenaltyArea(enemy.Pos(), 0)) continue;
        // 忽略对面的后卫
        if(ignoreTheirGuard && Utils::InTheirPenaltyArea(enemy.Pos(), 30)) continue;
        // 初始化球速、加速度、球移动的距离、球初始位置
        CGeoPoint ballPos = shootPos;
        double ballVel = originVel;
        double ballMoveTime = 0;
        double ballMoveDist = 0;
        double oneWorstInterTime = 99999;
        while (true) {  //根据步长DELTA_T进行碰撞分析
            double oneInterTime = 9999;
            ballMoveTime += DELTA_T;
            ballVel = BALL_VEL_DECAY * originVel - BALL_ACC * ballMoveTime;
            if (ballVel < 0) {
                ballVel = 0;//球已静止
            }
            ballMoveDist = (ballVel + BALL_VEL_DECAY * originVel) * ballMoveTime / 2.0;
            //截球点坐标
            ballPos = shootPos + Utils::Polar2Vector(ballMoveDist, passLineDir);
            if(DEBUG_MODE) {
                static int cnt = 0;
                cnt++;
                if(cnt%30==0) GDebugEngine::Instance()->gui_debug_x(ballPos);
            }
            //该车无法在场内截球，跳出循环
            if ( Utils::OutOfField(ballPos) || ballMoveDist >= passLineDist) {
                break;
            }
            CVector adjustDir = enemy.Pos() - ballPos;
            CGeoPoint testPoint = ballPos + Utils::Polar2Vector(AVOID_DIST, adjustDir.dir());
            double enemyArriveTime = predictedTheirTime(enemy, testPoint);

            oneInterTime = enemyArriveTime + responseTime - ballMoveTime; //大于0拦不住，小于0拦得住
            if(oneInterTime < oneWorstInterTime) oneWorstInterTime = oneInterTime;
            // 判断截球点是否合理
            bool interPosValid = (!isGoalie && !Utils::InTheirPenaltyArea(ballPos, 0)) || (isGoalie && Utils::InTheirPenaltyArea(ballPos, 0));
            if(interPosValid && ballMoveDist < passLineDist && (ballVel < 1e-8 || oneInterTime < 0)){
                break;
            }
        }
        if(oneWorstInterTime <= interTime) interTime = oneWorstInterTime;
    }
    if(interTime <= 0) return false;
    return true;//for循环中所有车都不能截球，没有return false
}

bool CShootModule::canShoot(const CVisionModule *pVision, CGeoPoint shootPos)
{
    // 只剩不多于1辆进攻车时，强开射门
    if (FORCE_SHOOT && pVision->getValidNum() <= 3) {
        return true;
    }
    // 在后场不射门
    if (shootPos.x() < 0) {
        return false;
    }

    //进行一般判断，出现可行小球路经即return true，否则return false
    CGeoPoint bestTarget = CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0);
    return generateBestTarget(pVision, bestTarget, shootPos);
    //return ShootModule::Instance()->GenerateShootDir(robotNum);
}

bool CShootModule::generateBestTarget(const CVisionModule *pVision, CGeoPoint &bestTarget, const CGeoPoint pos)
{
    CGeoPoint startPos;
    if (std::abs(pos.x()) > 1600) {
        startPos = pVision->Ball().Pos();
    }else {
        startPos = pos;
    }
    bestTarget = CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0);
    const CGeoPoint leftPost(Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2 + tolerance); // 左门柱
    const CGeoPoint rightPost(Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2 - 0.5 * tolerance); // 右门柱 此处不使用tolerance是因为进行stepsize时会自动留有余量，而左门柱是起点
    const CGeoLine bottomLine = CGeoLine(leftPost, rightPost);
    const double leftPostAngle = (leftPost - startPos).dir();   //vector.dir()都是弧度制
    const double rightPostAngle = (rightPost - startPos).dir();
    const double AngleRange = abs(leftPostAngle - rightPostAngle);
    double bestInterTime = -99999;
    CGeoPoint lastTarget = leftPost;
    for(int i = 0;  i * stepSize <= AngleRange; i++) {
        double tempDir = leftPostAngle + (leftPostAngle > rightPostAngle ? - i * stepSize : i * stepSize);
        CGeoLine tempLine = CGeoLine(startPos, startPos + Utils::Polar2Vector(1000, tempDir));
        CGeoLineLineIntersection intersect = CGeoLineLineIntersection(tempLine,bottomLine);
        if(intersect.Intersectant()) {
            CGeoPoint tempTarget = intersect.IntersectPoint();
            double interTime, interTime1, interTime2;
            validShootPos(pVision, startPos, 600, lastTarget, interTime1, responseTime);
            validShootPos(pVision, startPos, 600, tempTarget, interTime2, responseTime);
                interTime = (interTime1 + interTime2) / 2;
                if(isDebug) {
                    GDebugEngine::Instance()->gui_debug_x(tempTarget, COLOR_BLACK);
                    GDebugEngine::Instance()->gui_debug_msg(tempTarget, QString("%1").arg(interTime2).toLatin1(), COLOR_RED);
                }
                if(interTime > bestInterTime) {
                    bestInterTime = interTime;
                    bestTarget = CGeoPoint((lastTarget.x() + tempTarget.x()) / 2, (lastTarget.y() + tempTarget.y()) / 2);
                }
            lastTarget = tempTarget;
        }
    }
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,0), QString("%1").arg(bestInterTime).toLatin1(), COLOR_RED);
    if(bestInterTime < 0) {
        //bestTarget = CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0);
        return false;
    }else {
        return true;
    }
}
