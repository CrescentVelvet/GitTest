#include "ZBreak.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "RobotSensor.h"
#include "skill/Factory.h"
#include <utils.h>
#include "TaskMediator.h"
#include "SkillUtils.h"
#include "parammanager.h"
#include "BestPlayer.h"
#include "kickregulation.h"
#include "ShootModule.h"
namespace {
    bool VERBOSE;
    double SHOOT_ACCURACY;
    double PASS_ACCURACY;
    double MAX_ACC;
    double MAX_VEL;
    double MAX_ROT_ACC;
    double MAX_ROT_SPEED;
    double CHECK_OBSTCLE_DIST;
    double DRIBBLE_DIST;
    bool safeMode;
    enum {
        SIDEMOVE = 1,
        DRIBBLE = 2,
    };
    int maxFrared = 210;
    const double DEBUG_TEXT_HIGH = 23*10;
    const int MOD_NUM = 2;
    const int ANGEL_MOD[MOD_NUM+1] = {0,4,8};
    const int RADIUS = PARAM::Vehicle::V2::PLAYER_SIZE*1.4;
    CGeoPoint calc_point(const CVisionModule* pVision, const int vecNumber,const CGeoPoint&,const CGeoPoint&,const bool,bool&,bool&);
}

CZBreak::CZBreak():grabMode(DRIBBLE),last_mode(DRIBBLE){
    ZSS::ZParamManager::instance()->loadParam(VERBOSE,"Debug/ZBreak",false);
    ZSS::ZParamManager::instance()->loadParam(SHOOT_ACCURACY,"ZBreak/SHOOT_ACCURACY",5);
    ZSS::ZParamManager::instance()->loadParam(PASS_ACCURACY,"ZBreak/PASS_ACCURACY",3);
    ZSS::ZParamManager::instance()->loadParam(MAX_ACC,"ZBreak/MAX_ACC",350*10);
    ZSS::ZParamManager::instance()->loadParam(MAX_VEL,"ZBreak/MAX_VEL",150*10);
    ZSS::ZParamManager::instance()->loadParam(MAX_ROT_ACC,"ZBreak/MAX_ROT_ACC",20);
    ZSS::ZParamManager::instance()->loadParam(MAX_ROT_SPEED,"ZBreak/MAX_ROT_SPEED",20);
    ZSS::ZParamManager::instance()->loadParam(CHECK_OBSTCLE_DIST,"ZBreak/CHECK_OBSTCLE_DIST",150*10);
    ZSS::ZParamManager::instance()->loadParam(DRIBBLE_DIST,"ZBreak/DRIBBLE_DIST",70*10);
    ZSS::ZParamManager::instance()->loadParam(safeMode,"ZBreak/safeMode",false);
}

void CZBreak::plan(const CVisionModule* pVision) {
    if ((pVision->getCycle() - _lastCycle > PARAM::Vision::FRAME_RATE * 0.1)){
        grabMode = DRIBBLE;
        last_mode = DRIBBLE;
        move_point = CGeoPoint(-9999*10, -9999*10);
        dribblePoint = CGeoPoint(-9999*10,-9999*10);
        isDribble = false;
        fraredOn = 0;
        fraredOff = 0;
    }
    const MobileVisionT& ball = pVision->ball();
    const CGeoPoint passTarget = task().player.pos;
    int vecNumber = task().executor;
    const PlayerVisionT& me = pVision->ourPlayer(vecNumber);
    const int oppNum = ZSkillUtils::instance()->getTheirBestPlayer();
    const PlayerVisionT& enemy = pVision->theirPlayer(oppNum);
    double power = task().player.kickpower;
    const bool isChip = (task().player.kick_flag & PlayerStatus::CHIP);
    const bool setKick = (task().player.kick_flag & PlayerStatus::KICK);

    const CVector me2Ball = ball.Pos() - me.RawPos();
    const CVector me2enemy = enemy.RawPos() - me.RawPos();
    const CVector me2target = passTarget - me.RawPos();
//    double finalDir;
    CGeoPoint final_target = passTarget;
    bool canShoot = true;
    bool needBreakThrough = false;
    bool shootGoal = Utils::InTheirPenaltyArea(passTarget, 0);
    bool canShootGoal = false;
    if(shootGoal) {
        final_target = CGeoPoint(PARAM::Field::PITCH_LENGTH/2,0);
        canShootGoal = ShootModule::Instance()->generateBestTarget(pVision,final_target,ball.Pos());
    }

    // 传球精度控制
    double precision = shootGoal ? SHOOT_ACCURACY : PASS_ACCURACY;

    bool frared = RobotSensor::Instance()->IsInfraredOn(vecNumber);
    dribblePoint = RobotSensor::Instance()->dribblePoint(vecNumber);
    if (frared) {
        fraredOn = fraredOn >= maxFrared ? maxFrared : fraredOn + 1;
        if(fraredOn > 5) {
            fraredOff = 0;
            if(!isDribble){
//                dribblePoint = me.Pos(); // when dribble 5 frames, start to mark start point
                isDribble = true;   // not mark my point next time
            }
        }
    } else {
        fraredOff = fraredOff >= maxFrared ? maxFrared : fraredOff + 1;
        if(fraredOff > 5) {
            fraredOn = 0;
//            dribblePoint = me.Pos();//if lose ball  frames, dribblePoint become my position
            isDribble = false; //mark my point next time
        }
    }
//    if(fraredOn >= 5) KickRegulation::instance()->regulate(vecNumber, passTarget, power,finalDir, isChip);
    TaskT grabTask(task());
    /*********************** judge mode ********************/
    if(fraredOn > 50) {
        grabMode = SIDEMOVE;
    } else if(fraredOff > 7) {
        grabMode = DRIBBLE;
    }
    if(frared && last_mode == SIDEMOVE) grabMode = SIDEMOVE;

    //can see ball but has frared, sucking some robot's ass
    bool ballInfront = me2Ball.mod() < 20*10 && fabs(Utils::Normalize(me.Dir() - me2Ball.dir())) < PARAM::Math::PI/4;
    if (VERBOSE) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+CVector(0,40*10),QString("j:%1 %2 %3").arg(frared).arg(!ballInfront).arg(fabs(me.RotVel())).toLatin1());
    if(frared && me2enemy.mod() < 24*10 && ball.Valid() && !ballInfront){
        fraredOn  = 0;
        fraredOff = 0;
        frared = false;
        grabMode = DRIBBLE;
    }
    last_mode = grabMode;

    if(Utils::InOurPenaltyArea(passTarget,0))
        canShoot = false;
    /*********************** set subTask ********************/
    if(grabMode == SIDEMOVE) {
        if (VERBOSE) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -PARAM::Math::PI/1.5), "Side Move", COLOR_CYAN);
        move_point = calc_point(pVision, vecNumber, passTarget,dribblePoint,isChip,canShoot,needBreakThrough);
        if(VERBOSE) GDebugEngine::Instance()->gui_debug_arc(move_point, 8*10, 0.0, 360.0, COLOR_GREEN);
        if(VERBOSE) GDebugEngine::Instance()->gui_debug_line(dribblePoint, move_point, COLOR_ORANGE);

        grabTask.player.pos = move_point;
        grabTask.player.angle = (final_target-me.Pos()).dir();
        if(safeMode) {
            grabTask.player.pos = me.Pos();
            if(needBreakThrough) grabTask.player.angle = ZSkillUtils::instance()->holdBallDir(pVision, vecNumber);
        }
        if(Utils::InTheirPenaltyArea(grabTask.player.pos, 32*10) || Utils::InTheirPenaltyArea(me.Pos(), 32*10)) {
            grabTask.player.pos = Utils::MakeOutOfTheirPenaltyArea(grabTask.player.pos, 32*10);
        }
        if((shootGoal && canShootGoal) || !needBreakThrough){
            grabTask.player.flag = PlayerStatus::DRIBBLING;
        }else{
            grabTask.player.flag = PlayerStatus::DRIBBLING | PlayerStatus::BREAK_THROUGH;
        }
        grabTask.player.max_acceleration = MAX_ACC;
        grabTask.player.max_deceleration = MAX_ACC;
        grabTask.player.max_speed = MAX_VEL;
        grabTask.player.max_rot_acceleration = MAX_ROT_ACC;
        grabTask.player.max_rot_speed = MAX_ROT_SPEED;
        grabTask.player.path_plan_in_circle = true;
        grabTask.player.path_plan_circle_center = dribblePoint;
        grabTask.player.path_plan_circle_radius = 200*10;
        grabTask.player.use_dir = false;
        grabTask.player.need_rot_to_angle = canShoot && setKick;
        setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(grabTask));
        GDebugEngine::Instance()->gui_debug_arc(dribblePoint,100*10,0,360,COLOR_BLACK);
    }

    if(grabMode == DRIBBLE) {
        move_point = CGeoPoint(-9999*10, -9999*10);
        if (VERBOSE) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+ Utils::Polar2Vector(DEBUG_TEXT_HIGH, -PARAM::Math::PI/1.5), "Dribble", COLOR_CYAN);
        CGeoPoint target;
        double dir;
        if(ball.Valid()) target = ball.Pos() + Utils::Polar2Vector(5*10, enemy.Dir());
        else target = enemy.Pos() + Utils::Polar2Vector(5*10, enemy.Dir());
        if(ball.Valid()) dir = me2Ball.dir();
        else dir = me2enemy.dir();
        if(me2Ball.mod() > 50*10 || ball.Vel().mod() > 50*10)
            setSubTask(PlayerRole::makeItGetBallV4(vecNumber, PlayerStatus::RUSH|PlayerStatus::DRIBBLE|PlayerStatus::SAFE, target, CGeoPoint(999*10, 999*10), 0));
        else{
            if(!Utils::IsInField(target,PARAM::Vehicle::V2::PLAYER_SIZE+PARAM::Field::BALL_SIZE*2))
                target = Utils::MakeInField(target,PARAM::Vehicle::V2::PLAYER_SIZE+PARAM::Field::BALL_SIZE*2);
            setSubTask(PlayerRole::makeItGoto(vecNumber, target, dir, ballInfront ? PlayerStatus::NOT_AVOID_THEIR_VEHICLE : 0));
        }
    }
    /******************************/
    if (VERBOSE) GDebugEngine::Instance()->gui_debug_line(me.Pos(),final_target, COLOR_RED);
    auto vel_vertical_target = std::sin(me.Vel().dir()-me2target.dir())*me.Vel().mod();
    if(Utils::InOurPenaltyArea(passTarget,0))
        canShoot = false;
    if(Utils::InTheirPenaltyArea(passTarget, 0)){
//        canShoot = KickDirection::Instance()->getKickValid();
        power = ZSS::ZParamManager::instance()->value("KickLimit/FlatKickMax",630*10).toInt();
    }
    if (VERBOSE) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+CVector(0,50*10),QString("s:%1 %2 %3 %4").arg(canShoot).arg(vel_vertical_target).arg(fabs(me.RotVel())).arg(ballInfront).toLatin1(),COLOR_GREEN);

    power = KickRegulation::instance()->regulateCheck(vecNumber, power, (final_target-me.Pos()).dir(), isChip, precision*PARAM::Math::PI/180.0);
    if (VERBOSE) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+CVector(0,20*10),QString("ss:%1 %2 %3").arg(canShoot).arg(setKick).arg(power).toLatin1(),COLOR_GREEN);

    if (setKick && canShoot && power > 0.1) {

        bool enemyInFace = false;
        std::vector<CGeoPoint> enemy_points;
        for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++){
            auto test_enemy = pVision->theirPlayer(i);
            if(test_enemy.Valid() && (test_enemy.Pos()-me.RawPos()).mod() < PARAM::Vehicle::V2::PLAYER_SIZE * 2.2 && !Utils::InTheirPenaltyArea(test_enemy.Pos(),0)) enemy_points.push_back(test_enemy.Pos());
        }
        for(auto test_enemy : enemy_points){
            auto me2enemy = test_enemy - me.RawPos();
            if(std::abs(Utils::Normalize(me2enemy.dir() - me.ImuDir())) < 45 / 180 * PARAM::Math::PI){
                enemyInFace = true;
                break;
            }
        }

        if(!isChip && !enemyInFace) KickStatus::Instance()->setKick(vecNumber, power);
        else if(fraredOn >= 20) KickStatus::Instance()->setChipKick(vecNumber, power);
        if (VERBOSE) GDebugEngine::Instance()->gui_debug_msg(me.Pos()+CVector(0,70*10),QString("s! : %1").arg(power).toLatin1(),COLOR_WHITE);
    }
    DribbleStatus::Instance()->setDribbleCommand(vecNumber,3);
    _lastCycle = pVision->getCycle();
    return CStatedTask::plan(pVision);
}

CPlayerCommand* CZBreak::execute(const CVisionModule* pVision) {
    if (subTask()) {
        return subTask()->execute(pVision);
    }
    return nullptr;
}
namespace{
CGeoPoint calc_point(const CVisionModule* pVision, const int vecNumber,const CGeoPoint& target,const CGeoPoint& dribblePoint,const bool isChip,bool& canShoot,bool& needBreakThrough){
    canShoot = false;
    needBreakThrough = false;
    const PlayerVisionT& me = pVision->ourPlayer(vecNumber);
    double max_straight_dist = -9999*10;
    double to_target_dist = 9999*10;
    auto move_point = me.RawPos();
    std::vector<CGeoPoint> enemy_points;
    CVector me2target = target - dribblePoint;
    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++){
        auto test_enemy = pVision->theirPlayer(i);
        if(test_enemy.Valid() && (test_enemy.Pos()-dribblePoint).mod()<CHECK_OBSTCLE_DIST && !Utils::InTheirPenaltyArea(test_enemy.Pos(),0)) enemy_points.push_back(test_enemy.Pos());
    }
//    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++){
//        if(i==vecNumber) continue;
//        auto test_enemy = pVision->ourPlayer(i);
//        if(test_enemy.Valid() && (test_enemy.Pos()-dribblePoint).mod()<CHECK_OBSTCLE_DIST) enemy_points.push_back(test_enemy.Pos());//, PARAM::Vehicle::V2::PLAYER_SIZE*1.5)
//    }
    for(auto test_enemy : enemy_points){
        auto me2enemy = test_enemy - move_point;
        if(me2enemy.mod() < 60*10 && fabs(Utils::Normalize(me.Dir() - me2enemy.dir())) < PARAM::Math::PI/3){
            needBreakThrough = true;
            break;
        }
    }
    if(VERBOSE){
        for(auto test_enemy : enemy_points) GDebugEngine::Instance()->gui_debug_arc(test_enemy,RADIUS,0.0f,360.0f,COLOR_BLACK);
    }
    for(int j=MOD_NUM; j>0; j--){
        for(int i=-ANGEL_MOD[j]; i<ANGEL_MOD[j]; i++){
            bool temp_canShoot = true;
            double temp_max_straight_dist = 9999*10;
            CVector vec = Utils::Polar2Vector(double(j*DRIBBLE_DIST/MOD_NUM), Utils::Normalize(me2target.dir() + i*PARAM::Math::PI/ANGEL_MOD[j]));
            CGeoPoint test_point = dribblePoint + vec;
            if(!Utils::IsInField(test_point, 18*10)) test_point = Utils::MakeInField(test_point,18*10);
            if(Utils::InTheirPenaltyArea(test_point, 30*10)) test_point = Utils::MakeOutOfTheirPenaltyArea(test_point,30*10);
            if (VERBOSE) GDebugEngine::Instance()->gui_debug_x(test_point, COLOR_PURPLE);
            auto test_seg = CGeoSegment(test_point, target);
            double temp_to_target_dist = (test_point - target).mod();
            for(auto test_enemy : enemy_points){
                auto projection = test_seg.projection(test_enemy);
                auto projection_dist = (projection-test_enemy).mod();
                auto to_projection_dist = (projection-move_point).mod();
                auto straight_dist = (test_enemy - test_point).mod();
                if(isChip){
                    if((test_seg.IsPointOnLineOnSegment(projection)&&projection_dist<RADIUS)&&to_projection_dist<RADIUS){
                        if (VERBOSE) GDebugEngine::Instance()->gui_debug_line(dribblePoint, test_point, COLOR_YELLOW);
                        GDebugEngine::Instance()->gui_debug_line(test_enemy, test_point, COLOR_GREEN);
                        temp_canShoot = false;
                    }
                }else{
                    if((test_seg.IsPointOnLineOnSegment(projection)&&projection_dist<std::max(straight_dist*0.3,1.0*RADIUS))||straight_dist < RADIUS){
                        if (VERBOSE) GDebugEngine::Instance()->gui_debug_line(dribblePoint, test_point, COLOR_YELLOW);
                        if (VERBOSE) GDebugEngine::Instance()->gui_debug_line(test_enemy, test_point, COLOR_GREEN);
                        temp_canShoot = false;
                    }
                }
                temp_max_straight_dist = std::min(temp_max_straight_dist,straight_dist);
            }
            if(temp_canShoot)
                GDebugEngine::Instance()->gui_debug_arc(test_point,4*10,0,360,COLOR_BLUE);
//            GDebugEngine::Instance()->gui_debug_x(test_point,COLOR_PURPLE);
//            GDebugEngine::Instance()->gui_debug_msg(test_point+vec+CVector(0,0),QString("s:%1").arg(canShoot?1:0).toLatin1(),COLOR_BLUE);
//            GDebugEngine::Instance()->gui_debug_msg(test_point+vec+CVector(0,20),QString("ttd:%1,msd:%2").arg((int)temp_to_target_dist).arg((int)temp_max_straight_dist).toLatin1(),COLOR_CYAN);

            if(fabs(temp_max_straight_dist-9999*10)<0.1)temp_max_straight_dist = -9999*10;
            // update
            if(canShoot){
                if(temp_canShoot){
                    if(max_straight_dist - temp_max_straight_dist < -0.1*10){
                        max_straight_dist = temp_max_straight_dist;
                        move_point = test_point;
                        to_target_dist = temp_to_target_dist;
                    }else if(fabs(max_straight_dist - temp_max_straight_dist)<0.01*10 && to_target_dist > temp_to_target_dist){
                        max_straight_dist = temp_max_straight_dist;
                        move_point = test_point;
                        to_target_dist = temp_to_target_dist;
                    }
                }
            }else{
                if(temp_canShoot){
                    canShoot = true;
                    max_straight_dist = temp_max_straight_dist;
                    move_point = test_point;
                    to_target_dist = temp_to_target_dist;
                }else if(max_straight_dist - temp_max_straight_dist < -0.1*10){
                    canShoot = false;
                    max_straight_dist = temp_max_straight_dist;
                    move_point = test_point;
                    to_target_dist = temp_to_target_dist;
                }else if(fabs(max_straight_dist - temp_max_straight_dist)<0.01*10 && to_target_dist > temp_to_target_dist){
                    canShoot = false;
                    max_straight_dist = temp_max_straight_dist;
                    move_point = test_point;
                    to_target_dist = temp_to_target_dist;
                }
            }
        }
    }
    canShoot = true;
    if(canShoot){
        auto ball = pVision->ball();
        CGeoPoint test_point = ball.Valid() ? ball.RawPos() : ball.Pos();
        auto test_seg = CGeoSegment(test_point, target);
        for(auto test_enemy : enemy_points){
            auto projection = test_seg.projection(test_enemy);
            auto projection_dist = (projection-test_enemy).mod();
            auto to_projection_dist = (projection-move_point).mod();
            auto straight_dist = (test_enemy - test_point).mod();
            if(isChip){
                if((test_seg.IsPointOnLineOnSegment(projection)&&projection_dist<RADIUS)&&to_projection_dist<RADIUS){
                    if (VERBOSE) GDebugEngine::Instance()->gui_debug_line(dribblePoint, test_point, COLOR_CYAN);
                    if (VERBOSE) GDebugEngine::Instance()->gui_debug_line(test_enemy, test_point, COLOR_CYAN);
                    canShoot = false;
                    break;
                }
            }else{
                if((test_seg.IsPointOnLineOnSegment(projection)&&projection_dist<std::max(straight_dist*0.2,1.0*RADIUS))){
                    if (VERBOSE) {
                        GDebugEngine::Instance()->gui_debug_arc(test_enemy,RADIUS,0.0f,360.0f,COLOR_CYAN);
                        GDebugEngine::Instance()->gui_debug_line(projection,test_enemy,COLOR_CYAN);
                    }
                    canShoot = false;
                    break;
                }
            }
        }
    }

    return move_point;
}
}
