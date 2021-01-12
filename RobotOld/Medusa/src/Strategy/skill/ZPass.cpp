#include "ZPass.h"
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
#include "kickregulation.h"

namespace {
    const int verbose = true;
    double SHOOT_ACCURACY = 1; //1度，不允许调大
    double CAN_SHHOT_ANGLE = 3; //控制射门是否保守
    double MAX_ACC = 250*10;
    double MOVE_SPEED = 200*10;  //最大移动速度
    double MAX_ROT_ACC = 50;
    double MAX_ROT_SPEED = 15;
    double MAX_TURN_DIR = 150 * Param::Math::PI / 180;
    int FraredBuffer = 30;
    int MAX_TURN_CNT = 180;
    enum {
        TURN = 1,
        SIDEMOVE = 2,
        DRIBBLE = 3,
    };
    int grabMode = DRIBBLE;
    int last_mode = DRIBBLE;
    int fraredOn;
    int fraredOff;
    CGeoPoint dribblePoint;
    bool isDribble = false;
    int maxFrared = 999;
    double AVOID_PENALTY = Param::Vehicle::V2::PLAYER_SIZE*3.5;
    bool isTurning[Param::Field::MAX_PLAYER + 1];
    int turnWay = 2;
//    bool returnPoint = false;
    const double DEBUG_TEXT_HIGH = 30*10;
//    const double DIR_NUM = 60;
    const int MOD_NUM = 1;
    CGeoPoint move_point = CGeoPoint(-9999*10, -9999*10);
    bool test_canShoot = false;
    int turnCnt = 0;
    double TEST_DIR[6] = {/*-2*Param::Math::PI/3,*/ -7*Param::Math::PI/12, -Param::Math::PI/2, -5*Param::Math::PI/12, /*-Param::Math::PI/3,*/
                            /*2*Param::Math::PI/3,*/  7*Param::Math::PI/12,  Param::Math::PI/2,  5*Param::Math::PI/12,  /*Param::Math::PI/3*/};
}

CGeoPoint calc_point(const CVisionModule* pVision, const int vecNumber, double &finalDir, bool &canShoot);

CZPass::CZPass(){
    ZSS::ZParamManager::instance()->loadParam(FraredBuffer, "GetBall/FraredBuffer", 30);
    ZSS::ZParamManager::instance()->loadParam(MAX_ACC, "ZPass/MAX_ACC", 250*10);
    ZSS::ZParamManager::instance()->loadParam(MOVE_SPEED, "ZPass/MOVE_SPEED", 200*10);
    ZSS::ZParamManager::instance()->loadParam(MAX_ROT_ACC, "ZPass/MAX_ROT_ACC", 50);
    ZSS::ZParamManager::instance()->loadParam(MAX_ROT_SPEED, "ZPass/MAX_ROT_SPEED", 15);
    ZSS::ZParamManager::instance()->loadParam(MAX_TURN_CNT, "ZPass/MAX_TURN_CNT", 180);
}

void CZPass::plan(const CVisionModule* pVision) {
    if (pVision->getCycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1) {
        setState(BEGINNING);
        grabMode = DRIBBLE;
        last_mode = DRIBBLE;
        isDribble = false;
        fraredOn = 0;
        fraredOff = 0;
        turnCnt = 0;
    }
    const MobileVisionT& ball = pVision->ball();
    const CGeoPoint passTarget = task().player.pos;
    int vecNumber = task().executor;
    const PlayerVisionT& me = pVision->ourPlayer(vecNumber);
    oppNum = ZSkillUtils::instance()->getTheirBestPlayer();
    const PlayerVisionT& enemy = pVision->theirPlayer(oppNum);
    double power = task().player.kickpower;
    const bool isChip = (task().player.kick_flag & PlayerStatus::CHIP);
    // 传球精度控制
//    double precision = task().player.kickprecision > 0 ? task().player.kickprecision : SHOOT_ACCURACY;

    CVector me2Ball = ball.RawPos() - me.RawPos();
    CVector me2enemy = enemy.Pos() - me.RawPos();
    CVector me2target = passTarget - me.RawPos();

    double delta = fabs(Utils::Normalize(me2enemy.dir() - me.RawDir()));

    double finalDir;
    double theta = 0;
    bool canShoot = false;
    if(Utils::InTheirPenaltyArea(passTarget, 0)) {
        theta = KickDirection::Instance()->GenerateShootDir(vecNumber, me.RawPos()+Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, me.RawDir()));
        finalDir = KickDirection::Instance()->getRealKickDir();
//        canShoot = KickDirection::Instance()->getKickValid();
        if(fabs(theta) > CAN_SHHOT_ANGLE * Param::Math::PI / 180) canShoot = true;
        else canShoot = false;
    } else {
        finalDir = me2target.dir();
        canShoot = true;
    }
    KickRegulation::instance()->regulate(vecNumber, passTarget, power, finalDir, isChip);
//    GDebugEngine::Instance()->gui_debug_line(me.Pos(), me.Pos() + CVector(500, 0).rotate(finalDir), COLOR_GREEN);

    if (isTurning[vecNumber] == true) {
        if(! Utils::InTheirPenaltyArea(passTarget, 0)) {
            isTurning[vecNumber] = false;
        }
    }
    bool frared = RobotSensor::Instance()->IsInfraredOn(vecNumber);
    if (frared) {
        fraredOn = fraredOn >= maxFrared ? maxFrared : fraredOn + 1;
        fraredOff = 0;
        if(fraredOn > 5 && !isDribble) {
            dribblePoint = me.RawPos(); // when dribble 5 frames, start to mark start point
            isDribble = true;   // not mark my point next time
        }
    } else {
        fraredOn = 0;
        fraredOff = fraredOff >= maxFrared ? maxFrared : fraredOff + 1;
        if(fraredOff > 10) {
            dribblePoint = me.RawPos();//if lose ball  frames, dribblePoint become my position
            isDribble = false; //mark my point next time
        }
    }
    TaskT grabTask(task());
    /*********************** judge mode ********************/
    if(fraredOn > FraredBuffer && !canShoot) {
        if(delta < MAX_TURN_DIR && turnCnt < MAX_TURN_CNT) grabMode = TURN;
        else grabMode = SIDEMOVE;
    } else{
        grabMode = DRIBBLE;
    }
//    else{
//        grabMode = SIDEMOVE;
//    }
    if(frared && last_mode == SIDEMOVE) grabMode = SIDEMOVE;
    //can see ball but has frared, sucking some robot's ass
    if(frared && ball.Valid() && !(me2Ball.mod() < 20 && fabs(Utils::Normalize(me.Dir() - me2Ball.dir())) < Param::Math::PI/6)){
        fraredOn  = 0;
        frared = false;
        grabMode = DRIBBLE;
    }
    last_mode = grabMode;
    /*********************** set subTask ********************/
    if(grabMode == TURN) {
        turnCnt++;
        move_point = CGeoPoint(-9999*10, -9999*10);
        if (verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Turn", COLOR_CYAN);
        if(isTurning[vecNumber] == false) {
            turnWay = me2target.dir() > 0 ? 4 : 2;
            isTurning[vecNumber] = true;
        }
        setSubTask(PlayerRole::makeItOpenSpeedCircle(vecNumber, 15*10, turnWay, Param::Math::PI, 1, /*Param::Math::PI / 2*/0));
    }

    if(grabMode == SIDEMOVE) {
        turnCnt = 0;
        if (verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Side Move", COLOR_CYAN);
        double test_finalDir=0;
        if(Utils::OutOfField(move_point,9*10)) move_point = calc_point(pVision, vecNumber, test_finalDir, test_canShoot);
        CVector me2move_point = move_point - me.RawPos();
        if(me2move_point.mod() < 10*10){
            auto last_move_point = move_point;
            move_point = calc_point(pVision, vecNumber, test_finalDir, test_canShoot);
            CVector last2now = last_move_point - move_point;
            if(last2now.mod() < 20*10) move_point = dribblePoint;
            if (verbose) GDebugEngine::Instance()->gui_debug_arc(move_point, 8*10, 0.0, 360.0, COLOR_GREEN);
            test_canShoot = true;
        }
        if(!test_canShoot){// no where to go
            if (verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, Param::Math::PI/2), "NO WHERE TO GO", COLOR_RED);
            double movingWay = ball.RawPos().y() > 0 ? -1.0:1.0;
            move_point = dribblePoint + Utils::Polar2Vector(100*10, Utils::Normalize(me2target.dir() + movingWay*Param::Math::PI/2));
//            if(Utils::OutOfField(move_point, 9)) move_point = Utils::MakeInField(move_point,9);
            if(Utils::OutOfField(move_point,9*10)) move_point = dribblePoint + Utils::Polar2Vector(100*10, Utils::Normalize(me2target.dir() - movingWay*Param::Math::PI/2));
            if(Utils::InTheirPenaltyArea(move_point, AVOID_PENALTY)) move_point = Utils::MakeOutOfTheirPenaltyArea(move_point,AVOID_PENALTY);
            if (verbose) GDebugEngine::Instance()->gui_debug_arc(move_point, 8*10, 0.0, 360.0, COLOR_YELLOW);
        }
        if(verbose) GDebugEngine::Instance()->gui_debug_line(dribblePoint, move_point, COLOR_ORANGE);

        grabTask.player.pos = move_point;
        grabTask.player.angle = finalDir;
        if(Utils::InTheirPenaltyArea(grabTask.player.pos, AVOID_PENALTY) || Utils::InTheirPenaltyArea(me.RawPos(), AVOID_PENALTY)) {
            grabTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(grabTask.player.pos, AVOID_PENALTY);
        }
        grabTask.player.flag = PlayerStatus::DRIBBLING;
        grabTask.player.max_speed = MOVE_SPEED;
        grabTask.player.max_acceleration = MAX_ACC;
        grabTask.player.max_deceleration = MAX_ACC;
        grabTask.player.max_rot_acceleration = MAX_ROT_ACC;
        grabTask.player.max_rot_speed = MAX_ROT_SPEED;
        setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(grabTask));
    }

    if(grabMode == DRIBBLE) {
        turnCnt = 0;
        move_point = CGeoPoint(-9999*10, -9999*10);
        if (verbose) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -Param::Math::PI/1.5), "Dribble", COLOR_CYAN);
        CGeoPoint target;
        double dir;
        if(ball.Valid()) target = ball.RawPos();// + Utils::Polar2Vector(5, enemy.Dir());
        else target = enemy.Pos() + Utils::Polar2Vector(5*10, enemy.Dir());
        if(ball.Valid()) dir = me2Ball.dir();
        else dir = me2enemy.dir();
        if(me2Ball.mod() > 50*10 || ball.Vel().mod() > 50*10 || canShoot)
            setSubTask(PlayerRole::makeItGetBallV4(vecNumber, PlayerStatus::KICK+PlayerStatus::SAFE +PlayerStatus::DRIBBLE, task().player.pos, CGeoPoint(999*10, 999*10), 0));
        else{
            if(Utils::OutOfField(target,Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE*2))
                target = Utils::MakeInField(target,Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE*2);
            if(Utils::InTheirPenaltyArea(target, AVOID_PENALTY))
                target = Utils::MakeOutOfTheirPenaltyArea(target, AVOID_PENALTY);
            setSubTask(PlayerRole::makeItGoto(vecNumber, target, dir/*, PlayerStatus::NOT_AVOID_THEIR_VEHICLE*/));
        }
    }
    /****************** other ***************/
    if (me2Ball.mod() < 50*10) {
        DribbleStatus::Instance()->setDribbleCommand(vecNumber, 3);
    }
    /******************************/
//    qDebug() << fabs(Utils::Normalize(me.Dir() - finalDir))/Param::Math::PI *180 <<"\t"<< width/Param::Math::PI *180 <<"\t"<< canShoot;
    if (verbose) GDebugEngine::Instance()->gui_debug_line(me.Pos(), me.Pos() + Utils::Polar2Vector(1000*10, finalDir), COLOR_RED);
    if (canShoot && fabs(Utils::Normalize(me.RawDir() - finalDir)) < SHOOT_ACCURACY*Param::Math::PI/180.0) {
//        qDebug() << "Shoot !";
        if(!isChip) KickStatus::Instance()->setKick(vecNumber, power);
        else KickStatus::Instance()->setChipKick(vecNumber, power);
    }
    _lastCycle = pVision->getCycle();
    return CStatedTask::plan(pVision);
}

CPlayerCommand* CZPass::execute(const CVisionModule* pVision) {
    if (subTask()) {
        return subTask()->execute(pVision);
    }
    return nullptr;
}

CGeoPoint calc_point(const CVisionModule* pVision, const int vecNumber, double &finalDir, bool &canShoot){
    const PlayerVisionT& me = pVision->ourPlayer(vecNumber);
    double maxRange = -9999*10;
    auto move_point = me.RawPos();
    std::vector<CGeoCirlce> enemy_circles;
    CGeoPoint theirGoal = CGeoPoint(Param::Field::PITCH_LENGTH/2, 0);
    CVector me2theirGoal = theirGoal - me.RawPos();
    for(int i = 1; i <= Param::Field::MAX_PLAYER; i++){
        auto test_enemy = pVision->theirPlayer(i);
        if(test_enemy.Valid()) enemy_circles.push_back(CGeoCirlce(test_enemy.Pos(), Param::Vehicle::V2::PLAYER_SIZE*1.5));
    }
    for(int i = 1; i <= Param::Field::MAX_PLAYER; i++){
        if(i==vecNumber) continue;
        auto test_enemy = pVision->ourPlayer(i);
        if(test_enemy.Valid()) enemy_circles.push_back(CGeoCirlce(test_enemy.Pos(), Param::Vehicle::V2::PLAYER_SIZE*1.5));
    }
    for(int i=0; i<6; i++){
        for(int j=MOD_NUM; j>0; j--){
            double temp_finalDir = 0;
            bool temp_canShoot = false;
            bool intersectant = false;
            CVector vec = Utils::Polar2Vector(double(j*90.0/MOD_NUM), Utils::Normalize(me2theirGoal.dir() + TEST_DIR[i]));
            CGeoPoint test_point = dribblePoint + vec;
            if(Utils::OutOfField(test_point, 8*10)) test_point = Utils::MakeInField(test_point,8*10);
            if(Utils::InTheirPenaltyArea(test_point, AVOID_PENALTY)) test_point = Utils::MakeOutOfTheirPenaltyArea(test_point,AVOID_PENALTY);
            if (verbose) GDebugEngine::Instance()->gui_debug_line(dribblePoint, test_point, COLOR_PURPLE);
            auto test_seg = CGeoSegment(dribblePoint, test_point);
            for(CGeoCirlce test_circle : enemy_circles){
                auto seg2circle = CGeoSegmentCircleIntersection(test_seg, test_circle);
                if(seg2circle.intersectant()){
                    if (verbose) GDebugEngine::Instance()->gui_debug_line(dribblePoint, test_point, COLOR_YELLOW);
                    if (verbose) GDebugEngine::Instance()->gui_debug_line(test_circle.Center(), test_point, COLOR_GREEN);
                    intersectant = true;
                }
            }
            if(intersectant){
                intersectant = false;
                break;
            }
            double temp_maxRange = KickDirection::Instance()->GenerateShootDir(vecNumber, test_point);
            temp_finalDir = KickDirection::Instance()->getRealKickDir();
            temp_canShoot = KickDirection::Instance()->getKickValid();
            if(temp_maxRange > maxRange){
                maxRange = temp_maxRange;
                finalDir = temp_finalDir;
                canShoot = temp_canShoot;
                move_point = test_point;
            }
        }
    }
    return move_point;
}
