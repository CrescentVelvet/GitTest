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
#include "SkillUtils.h"
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
    Shoot2Goal = CGeoPoint(PARAM::Field::PITCH_LENGTH / 2, 0);
    ZSS::ZParamManager::instance()->loadParam(TEST_PASS, "Messi/NoShoot", false);
    ZSS::ZParamManager::instance()->loadParam(FORCE_SHOOT, "Messi/FORCE_SHOOT", true);
    ZSS::ZParamManager::instance()->loadParam(_needBallVel, "KickLimit/FlatKickMax", 6300.0);
    ZSS::ZParamManager::instance()->loadParam(tolerance, "Shoot/Tolerance", 200);    //有待提升，即应该根据kicker位置，自适应tolerance
    ZSS::ZParamManager::instance()->loadParam(stepSize, "Shoot/StepSize", 1);
    ZSS::ZParamManager::instance()->loadParam(responseTime, "Shoot/ResponseTime", 0);
    stepSize = stepSize / 180 * PARAM::Math::PI;    //转换为弧度制
    const string path = PARAM::File::PlayBookPath;
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
    const MobileVisionT & ball = pVision->ball();
    const PlayerVisionT & kicker = pVision->ourPlayer(player);
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

        double tempdir = (Utils::Normalize(Utils::Normalize(pVision->ball().Vel().dir()+PARAM::Math::PI)-(_kick_target - pVision->ourPlayer(player).Pos()).dir()))*180/PARAM::Math::PI;
        int ratio = 0;
        if (tempdir>0){
            ratio = 1;
        }else{
            ratio = -1;
        }
        rawdir=abs((Utils::Normalize(Utils::Normalize(pVision->ball().Vel().dir()+PARAM::Math::PI)-(_kick_target - pVision->ourPlayer(player).Pos()).dir()))*180/PARAM::Math::PI);
        // cout << rawdir << endl;
        if (rawdir > 70 && rawdir < 110){
            rawdir = 80;
        }

        shoot_speed = ballspeed;
        shoot_dir = rawdir;

        _compensate_value = Compensate::Instance()->checkCompensate(ballspeed,rawdir);

        if (pVision->ball().Vel().mod()<500){
            _compensate_value = 0;
        }
        _real_kick_dir= Utils::Normalize(Utils::Normalize(ratio*_compensate_value*PARAM::Math::PI/180)+_raw_kick_dir);
        if(pVision->ball().Vel().mod()<500){
            _real_kick_dir = _raw_kick_dir;
        }
        compensateType = 2;
    }
    GDebugEngine::Instance()->gui_debug_line(kickerPos, kickerPos+Utils::Polar2Vector(1000,_real_kick_dir),COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(300, -100), QString("%1").arg(compensateType).toLatin1(), COLOR_RED);
    return getKickValid();
}

bool CShootModule::canShoot(const CVisionModule *pVision, CGeoPoint shootPos)
{
    if(TEST_PASS)
        return false;
    //ZPass在禁区附近强制射门
//    if(Utils::InTheirPenaltyArea(shootPos, 3*PARAM::Vehicle::V2::PLAYER_SIZE) &&
//            shootPos.x() <= PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH)
//        return true;
    // 只剩不多于1辆进攻车时，强开射门.
    if (FORCE_SHOOT && pVision->getValidNum() <= 3)
        return true;
    // 在后场不射门
    if (shootPos.x() < 0)
        return false;

    //进行一般判断，出现可行小球路经即return true，否则return false
    CGeoPoint bestTarget = CGeoPoint(PARAM::Field::PITCH_LENGTH / 2, 0);
    return generateBestTarget(pVision, bestTarget, shootPos);
    //return ShootModule::Instance()->GenerateShootDir(robotNum);
}

bool CShootModule::generateBestTarget(const CVisionModule *pVision, CGeoPoint &bestTarget, const CGeoPoint& pos)
{
    CGeoPoint startPos;
    if (std::abs(pos.x()) > 16000) {
        startPos = pVision->ball().Pos();
    }else {
        startPos = pos;
    }
    bestTarget = CGeoPoint(PARAM::Field::PITCH_LENGTH / 2, 0);
    const CGeoPoint leftPost(PARAM::Field::PITCH_LENGTH / 2, -PARAM::Field::GOAL_WIDTH / 2 + tolerance); // 左门柱
    const CGeoPoint rightPost(PARAM::Field::PITCH_LENGTH / 2, PARAM::Field::GOAL_WIDTH / 2 - 0.5 * tolerance); // 右门柱 此处不使用tolerance是因为进行stepsize时会自动留有余量，而左门柱是起点
    const CGeoLine bottomLine = CGeoLine(leftPost, rightPost);
    const double leftPostAngle = (leftPost - startPos).dir();   //vector.dir()都是弧度制
    const double rightPostAngle = (rightPost - startPos).dir();
    const double AngleRange = abs(leftPostAngle - rightPostAngle);
    if(AngleRange < stepSize)
        return false;
    double bestInterTime = -99999;
    CGeoPoint lastTarget = leftPost;
    for(int i = 0;  i * stepSize <= AngleRange; i++) {
        double tempDir = leftPostAngle + (leftPostAngle > rightPostAngle ? - i * stepSize : i * stepSize);
        CGeoLine tempLine = CGeoLine(startPos, startPos + Utils::Polar2Vector(10000, tempDir));
        CGeoLineLineIntersection intersect = CGeoLineLineIntersection(tempLine,bottomLine);
        if(intersect.Intersectant()) {
            CGeoPoint tempTarget = intersect.IntersectPoint();
            double interTime, interTime1, interTime2;
            ZSkillUtils::instance()->validShootPos(pVision, startPos, 6000, lastTarget, interTime1, responseTime);
            ZSkillUtils::instance()->validShootPos(pVision, startPos, 6000, tempTarget, interTime2, responseTime);
            //validShootPos(pVision, startPos, 6000, lastTarget, interTime1, responseTime);
            //validShootPos(pVision, startPos, 6000, tempTarget, interTime2, responseTime);
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
    if(bestInterTime < 0)
        return false;
    else
        return true;
}
