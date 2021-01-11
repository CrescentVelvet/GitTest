#include "SmartGotoPositionV3.h"
#include "skill/Factory.h"
#include <utils.h>
#include <TaskMediator.h>
#include "PlayInterface.h"
#include <GDebugEngine.h>
#include <RobotCapability.h>
#include <CommandFactory.h>
#include <RefereeBoxIf.h>
#include <ControlModel.h>
#include <iostream>
#include "parammanager.h"
#include <iostream>
#include <RRTPathPlannerNew.h>

namespace{

    bool DRAW_RRT_TREE = false;
    bool DRAW_PATH_POINT = true;
    bool DRAW_PENALTY_DEBUG_MSG = false;

    const double SlowFactor = 0.5;
    const double FastFactor = 1.2;
    /// 底层运动控制参数 ： 默认增大平动的控制性能
    double MAX_TRANSLATION_SPEED = 400;
    double MAX_TRANSLATION_ACC = 600;
    double MAX_ROTATION_SPEED = 5;
    double MAX_ROTATION_ACC = 15;
    double MAX_TRANSLATION_DEC = 450;

    const double TRANSLATION_ACC_LIMIT = 500;
    double TRANSLATION_SPEED_LIMIT = 350;
    const double TRANSLATION_ROTATE_ACC_LIMIT = 15;

    /// 守门员专用
    double MAX_TRANSLATION_SPEED_GOALIE;
    double MAX_TRANSLATION_ACC_GOALIE;
    double MAX_TRANSLATION_DEC_GOALIE;
    double MAX_ROTATION_ACC_GOALIE;
    double MAX_ROTATION_SPEED_GOALIE;

    /// 后卫专用
    double MAX_TRANSLATION_SPEED_BACK;
    double MAX_TRANSLATION_ACC_BACK;
    double MAX_TRANSLATION_DEC_BACK;
    double MAX_ROTATION_ACC_BACK;
    double MAX_ROTATION_SPEED_BACK;

    /// 避碰规划
    const double safeVelFactorFront = 1.0;
    const double safeVelFactorBack = 1.0;
    bool DRAW_OBSTACLES = false;

    double stopBallAvoidDist = 50;
    double ballPlacementDist = 60;

    /// related to rrt
    double startToRotateToTargetDirDist = 20;
    pair < vector < pair < stateNew, size_t > >, vector < pair < stateNew, size_t > > > tree[Param::Field::MAX_PLAYER + 1];
    CRRTPathPlannerNew planner[Param::Field::MAX_PLAYER * 2 + 1];
    vector < stateNew > viaPoint[Param::Field::MAX_PLAYER + 1];

    CGeoPoint lastPoint[Param::Field::MAX_PLAYER + 1];
    CGeoPoint lastFinalPoint[Param::Field::MAX_PLAYER + 1];
    const double TEAMMATE_AVOID_DIST = Param::Vehicle::V2::PLAYER_SIZE + 4.0f; // 2014/03/13 修改，为了减少stop的时候卡住的概率 yys
    const double OPP_AVOID_DIST = Param::Vehicle::V2::PLAYER_SIZE + 5.5f;
    const double BALL_AVOID_DIST = Param::Field::BALL_SIZE + 5.0f;
}
using namespace Param::Vehicle::V2;

CSmartGotoPositionV3::CSmartGotoPositionV3()
{
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_SPEED,"CGotoPositionV2/MNormalSpeed",335);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_ACC,"CGotoPositionV2/MNormalAcc",475);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_DEC,"CGotoPositionV2/MNormalDec",475);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_SPEED_BACK,"CGotoPositionV2/MBackSpeed",300);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_ACC_BACK,"CGotoPositionV2/MBackAcc",450);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_DEC_BACK,"CGotoPositionV2/MBackDec",450);

    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_ACC_BACK,"CGotoPositionV2/MBackRotateAcc",15);
    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_SPEED_BACK,"CGotoPositionV2/MBackRotateSpeed",15);

    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_SPEED_GOALIE,"CGotoPositionV2/MGoalieSpeed",300);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_ACC_GOALIE,"CGotoPositionV2/MGoalieAcc",450);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_DEC_GOALIE,"CGotoPositionV2/MGoalieDec",450);

    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_ACC_GOALIE,"CGotoPositionV2/MGoalieRotateAcc",15);
    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_SPEED_GOALIE,"CGotoPositionV2/MGoalieRotateSpeed",15);

    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_SPEED,"CGotoPositionV2/RotationSpeed",15);
    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_ACC,"CGotoPositionV2/RotationAcc",50);
    ZSS::ZParamManager::instance()->loadParam(DRAW_OBSTACLES, "Debug/DrawObst", false);
    ZSS::ZParamManager::instance()->loadParam(DRAW_RRT_TREE,"CGotoPositionV2/drawRrtTree",false);

    ZSS::ZParamManager::instance()->loadParam(TRANSLATION_SPEED_LIMIT,"CGotoPositionV2/NormalSpeedLimit",400); //下发速度最大限制，守门员、后卫除外
}

/// 输出流 ： 调试显示
void CSmartGotoPositionV3::toStream(std::ostream& os) const
{
    os << "Smart going to " << task().player.pos;
}

/// 规划接口
void CSmartGotoPositionV3::plan(const CVisionModule* pVision)
{

    if ((lastFinalPoint[task().executor] - task().player.pos).mod() > 5) {
        lastPoint[task().executor] = task().player.pos;
        lastFinalPoint[task().executor] = task().player.pos;
    }
    /************************************************************************/
    /* 任务参数解析                                                         */
    /************************************************************************/
    playerFlag = task().player.flag;
    const bool rec = task().player.needdribble;
    const int vecNumber = task().executor;
    const PlayerVisionT& self = pVision->OurPlayer(vecNumber);
    CGeoPoint myPos = self.Pos();
    CGeoPoint ballPos = pVision->Ball().Pos();
    double ballplacementCanDirect = true;

//    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, 0), QString("SmartGoto!!!").toLatin1());

    const bool isGoalie = vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie());

    const bool isBack = (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->leftBack())) ||
                         (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->rightBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->singleBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->sideBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->defendMiddle()));
    //需要躲避球周围半径为50cm的圆
    bool avoidBallCircle = (WorldModel::Instance()->CurrentRefereeMsg() == "GameStop") || (playerFlag & PlayerStatus::AVOID_STOP_BALL_CIRCLE);
    //需要躲避对方放球的椭圆区域
    bool avoidTheirBallPlacementArea = (WorldModel::Instance()->CurrentRefereeMsg() == "TheirBallPlacement") || (playerFlag & PlayerStatus::AVOID_THEIR_BALLPLACEMENT_AREA);
    CGeoPoint finalTargetPos = task().player.pos;
    const bool shrinkTheirPenalty = Utils::InTheirPenaltyArea(finalTargetPos, 40) || Utils::InTheirPenaltyArea(myPos, 40);
    //设置运行参数
    _capability = setCapability(pVision);
    //zbreak仅在圈内规划
    const bool needBreakRotate = (playerFlag & PlayerStatus::BREAK_THROUGH);
    bool onlyPlanInCircle = task().player.path_plan_in_circle;
    const CGeoPoint planCircleCenter = task().player.path_plan_circle_center;
    const double planCircleRadius = task().player.path_plan_circle_radius;
    //仅在自己半场规划
    bool avoidHalfField = (playerFlag & PlayerStatus::AVOID_HALF_FIELD);

    if(onlyPlanInCircle && Utils::OutOfField(planCircleCenter, -40)) {
        cout << "Oh shit!!! PathPlan circle center not initialized!!! ---SmartGotoPosition.cpp" << endl;
        onlyPlanInCircle = false;
    }  else if(onlyPlanInCircle && planCircleRadius < 1e-8) {
        cout << "Oh shit!!! PathPlan circle radius is too small!!! ---SmartGotoPosition.cpp" << endl;
        onlyPlanInCircle = false;
    }

    /************************************************************************/
    /* 游戏环节、自身状态解析                                                  */
    /************************************************************************/

    //自身的避障范围： 车半径 + buffer
    double buffer = 1.5;
    if (self.Vel().mod() < 200) {
        buffer = 1.5;
    }
    else if (self.Vel().mod() < 300) {
        buffer = 1.5 + (self.Vel().mod() - 200) / 100;
    }
    else {
        buffer = 2.5;
    }
    if ((self.Pos() - finalTargetPos).mod() < 100) {
        buffer /= 2;
    }
    if (isGoalie ||
            (isBack && Utils::InOurPenaltyArea(myPos, 40)) ||
            WorldModel::Instance()->CurrentRefereeMsg() == "OurTimeout") {
        buffer = 0;
    }
    if(playerFlag & PlayerStatus::BREAK_THROUGH) {
        buffer = 5.5;
    }
    double avoidLength = Param::Vehicle::V2::PLAYER_SIZE + buffer;
    ObstaclesNew obsNew(avoidLength);
    //如果是zbreak，就让躲避距离小一点
    if(!(playerFlag & PlayerStatus::BREAK_THROUGH)) {
        obsNew.addObs(pVision, task(), DRAW_OBSTACLES, OPP_AVOID_DIST, TEAMMATE_AVOID_DIST, BALL_AVOID_DIST, shrinkTheirPenalty);
    }
    else {
        obsNew.addObs(pVision, task(), DRAW_OBSTACLES, Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 5.0, Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE + 5.0, Param::Field::BALL_SIZE, shrinkTheirPenalty);
    }
    //对方放球需要避开球到放球点之间半径为50cm的圈
    if(avoidTheirBallPlacementArea) {
        CGeoPoint segP1 = pVision->Ball().Pos(), segP2(RefereeBoxInterface::Instance()->getBallPlacementPosX(), RefereeBoxInterface::Instance()->getBallPlacementPosY());
        bool addObs;
        finalTargetPos = finalPointBallPlacement(myPos, finalTargetPos, segP1, segP2, avoidLength, obsNew, addObs);
//        if(addObs) {
//            obsNew.addLongCircle(segP1, segP2, CVector(0.0, 0.0), ballPlacementDist, OBS_LONG_CIRCLE_NEW, false, true);
//        }
        obsNew.drawLongCircle(segP1, segP2, ballPlacementDist);
    }
    //我方的ballplacement且为非放球车， 需要避开放球点
    if(WorldModel::Instance()->CurrentRefereeMsg() == "OurBallPlacement" &&
            !(playerFlag & PlayerStatus::NOT_AVOID_PENALTY)) {
        const double avoidRadius = 30.0;
        CGeoPoint ballPlacePoint(RefereeBoxInterface::Instance()->getBallPlacementPosX(), RefereeBoxInterface::Instance()->getBallPlacementPosY());
        if(ballPlacePoint.dist(self.Pos()) < avoidRadius)
            finalTargetPos = Utils::MakeOutOfCircle(ballPlacePoint, avoidRadius, self.Pos(), avoidLength);
    }

    //处理无效目标点:在禁区内、在车身内、在场地外
    if(!avoidTheirBallPlacementArea)
        validateFinalTarget(finalTargetPos, pVision, shrinkTheirPenalty, avoidLength, isGoalie);

    //躲避stop状态中的球圈
    if(avoidBallCircle) {
        CGeoPoint ballPos = pVision->Ball().Pos();
        finalTargetPos = Utils::MakeOutOfCircle(ballPos, stopBallAvoidDist, finalTargetPos, avoidLength);
    }

    //处理非零速时的末速度，避免车因为非零速冲进禁区或冲出场外
    CVector velNew = /*validateFinalVel(isGoalie, self.Pos(), finalTargetPos, task().player.vel)*/task().player.vel;

    //处理规划起始点，防止rrt规划失败
    validateStartPoint(myPos, avoidLength, isGoalie, obsNew);

    //debug信息输出： 初始目标点， 真实自身位置， 处理过的目标点， 处理过的自身位置
    if(DRAW_PATH_POINT) {
        GDebugEngine::Instance()->gui_debug_x(task().player.pos, COLOR_RED);
        GDebugEngine::Instance()->gui_debug_arc(self.Pos(), avoidLength,0,360,1);
        GDebugEngine::Instance()->gui_debug_x(finalTargetPos, COLOR_YELLOW);
        GDebugEngine::Instance()->gui_debug_x(myPos, COLOR_PURPLE);
    }

    // 初始点， 如果规划失败则直接冲向目标点
    CGeoPoint middlePoint = finalTargetPos;

    //设置新的子任务
    TaskT newTask(task());
    newTask.player.pos = finalTargetPos;
    newTask.player.vel = velNew;

    //如果是zbreak规划状态，且距离最终目标点很近，就将该状态取消，朝目标点转
    if(self.Pos().dist(finalTargetPos) < startToRotateToTargetDirDist && needBreakRotate)
        newTask.player.flag &= (~PlayerStatus::BREAK_THROUGH);

    double arrivedDist = self.Vel().mod() * 0.1 + 5; //判断是否到达中间点，让整条路径更加连贯

    /************************************************************************/
    /* 开始规划路径点                                                        */
    /************************************************************************/
    stateNew startNew, targetNew;
    startNew.pos = myPos;
    targetNew.pos = finalTargetPos;
    //第一种情况：可以直接到目标点(预处理后、预处理前)
    if (obsNew.check(startNew.pos, targetNew.pos) ||
            obsNew.check(self.Pos(), targetNew.pos) ||
            self.Pos().dist(finalTargetPos) < Param::Vehicle::V2::PLAYER_SIZE * 2) { // the robot can directly head to the final pos
        middlePoint = finalTargetPos;
    }
    //第二种情况：上次规划的中间点仍然可以用，且还没有到达中间点
    else if (ballplacementCanDirect && obsNew.check(startNew.pos, lastPoint[vecNumber]) && (lastPoint[vecNumber] - self.Pos()).mod() > arrivedDist) { // keep the pos last cycle
        middlePoint = lastPoint[vecNumber];
    }
    //第三种：其它，则重新规划
    else {
        //需要只在自己半场规划，防止kick off的时候犯规
        if(avoidHalfField && myPos.x() < -Param::Vehicle::V2::PLAYER_SIZE) {
            planner[vecNumber].initPlanner(250, 15, 20, 0.05, 0.55, 9.0, avoidHalfField);
            planner[vecNumber].planPath(&obsNew, startNew, targetNew);
            viaPoint[vecNumber] = planner[vecNumber].getPathPoints();
        }
        //zbreak的时候，需要只在一个圈内进行路径规划，防止dribbling犯规
        else if(onlyPlanInCircle) {
            planner[vecNumber + Param::Field::MAX_PLAYER_NUM].initPlanner(100, 15, 20, 0.05, 0.0, Param::Vehicle::V2::PLAYER_SIZE / 2.0, false, onlyPlanInCircle, planCircleCenter, planCircleRadius);
            planner[vecNumber + Param::Field::MAX_PLAYER_NUM].planPath(&obsNew, startNew, targetNew);
            viaPoint[vecNumber] = planner[vecNumber + Param::Field::MAX_PLAYER_NUM].getPathPoints();
        }
        //一般情况下的rrt规划
        else {
            planner[vecNumber].initPlanner(250, 15, 20, 0.05, 0.55, 9.0);
            planner[vecNumber].planPath(&obsNew, startNew, targetNew);
            viaPoint[vecNumber] = planner[vecNumber].getPathPoints();
        }
        //规划成功的情况则给中间点赋值，一般都是有中间点的
        if(viaPoint[vecNumber].size() > 2) {
            middlePoint = viaPoint[vecNumber][1].pos;
        }
    }

    //画出debug信息，看规划范围是否正确
    if(DRAW_RRT_TREE) {
        if(onlyPlanInCircle)
            tree[vecNumber] = planner[vecNumber + Param::Field::MAX_PLAYER_NUM].getNodes();
        else
            tree[vecNumber] = planner[vecNumber].getNodes();
        GDebugEngine::Instance()->gui_debug_msg(middlePoint, QString("%1").arg(viaPoint[vecNumber].size()).toLatin1());
        for(size_t i = 0; viaPoint[vecNumber].size() > 0 &&  i < viaPoint[vecNumber].size() - 1; i++) {
            GDebugEngine::Instance()->gui_debug_line(viaPoint[vecNumber][i].pos, viaPoint[vecNumber][i + 1].pos, COLOR_CYAN);
        }

        for(size_t i = 1; !tree[vecNumber].first.empty() && i < tree[vecNumber].first.size(); i+=1) {
            GDebugEngine::Instance()->gui_debug_line(tree[vecNumber].first[i].first.pos, tree[vecNumber].first[tree[vecNumber].first[i].second].first.pos, COLOR_GREEN);
            GDebugEngine::Instance()->gui_debug_x(tree[vecNumber].first[i].first.pos, COLOR_GREEN);
        }
        for(size_t i = 1; !tree[vecNumber].second.empty() && i < tree[vecNumber].second.size(); i++) {
            GDebugEngine::Instance()->gui_debug_line(tree[vecNumber].second[i].first.pos, tree[vecNumber].second[tree[vecNumber].second[i].second].first.pos, COLOR_YELLOW);
            GDebugEngine::Instance()->gui_debug_x(tree[vecNumber].second[i].first.pos, COLOR_YELLOW);
        }
    }
    //记录中间点，作为下一次规划基础
    lastPoint[vecNumber] = middlePoint;
    if(!isGoalie && !(playerFlag & PlayerStatus::NOT_AVOID_PENALTY)) {
        middlePoint = dealPlanFail(self.Pos(), middlePoint, avoidLength, shrinkTheirPenalty);
    }

    if (rec) DribbleStatus::Instance()->setDribbleCommand(vecNumber,2); //吸球指令

    newTask.player.pos = middlePoint;

    //零速到达中间点，非零速只有在可以直接到时才执行
    if(middlePoint.dist(task().player.pos) > 1e-8) {
        newTask.player.vel = CVector(0.0, 0.0);
    }

    setSubTask(TaskFactoryV2::Instance()->GotoPosition(newTask));

    _lastCycle = pVision->Cycle();
    CPlayerTask::plan(pVision);
    return ;
}

PlayerCapabilityT CSmartGotoPositionV3::setCapability(const CVisionModule* pVision) {
    const int vecNumber = task().executor;
    const bool isGoalie = (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie()));
    const bool isBack = (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->leftBack())) ||
                         (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->rightBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->singleBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->sideBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->defendMiddle()));
    const CGeoPoint mePos = pVision->OurPlayer(vecNumber).Pos();
    const int playerFlag = task().player.flag;
    PlayerCapabilityT capability;

    // Traslation 确定平动运动参数
    if (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())) {
        capability.maxSpeed = MAX_TRANSLATION_SPEED_GOALIE;
        capability.maxAccel = MAX_TRANSLATION_ACC_GOALIE;
        capability.maxDec = MAX_TRANSLATION_DEC_GOALIE;
        // Rotation	  确定转动运动参数
        capability.maxAngularSpeed = MAX_ROTATION_SPEED_GOALIE;
        capability.maxAngularAccel = MAX_ROTATION_ACC_GOALIE;
        capability.maxAngularDec = MAX_ROTATION_ACC_GOALIE;
    }
    else if (TaskMediator::Instance()->leftBack() != 0 && vecNumber == TaskMediator::Instance()->leftBack()
        || TaskMediator::Instance()->rightBack() != 0 && vecNumber == TaskMediator::Instance()->rightBack()
        || TaskMediator::Instance()->singleBack() != 0 && vecNumber == TaskMediator::Instance()->singleBack()
        || TaskMediator::Instance()->sideBack() != 0 && vecNumber == TaskMediator::Instance()->sideBack()) {
        capability.maxSpeed = MAX_TRANSLATION_SPEED_BACK;
        capability.maxAccel = MAX_TRANSLATION_ACC_BACK;
        capability.maxDec = MAX_TRANSLATION_DEC_BACK;
        // Rotation	  确定转动运动参数
        capability.maxAngularSpeed = MAX_ROTATION_SPEED_BACK;
        capability.maxAngularAccel = MAX_ROTATION_ACC_BACK;
        capability.maxAngularDec = MAX_ROTATION_ACC_BACK;
    }
    else {
        capability.maxSpeed = MAX_TRANSLATION_SPEED;
        capability.maxAccel = MAX_TRANSLATION_ACC;
        capability.maxDec = MAX_TRANSLATION_DEC;
        // Rotation	  确定转动运动参数
        capability.maxAngularSpeed = MAX_ROTATION_SPEED;
        capability.maxAngularAccel = MAX_ROTATION_ACC;
        capability.maxAngularDec = MAX_ROTATION_ACC;
    }

    if (playerFlag & PlayerStatus::SLOWLY) {
        capability.maxSpeed = 140;
        capability.maxAccel *= SlowFactor;
        capability.maxDec *= SlowFactor;
        capability.maxAngularSpeed *= SlowFactor;
        capability.maxAngularAccel *= SlowFactor;
        capability.maxAngularDec *= SlowFactor;
    }
    if (playerFlag & PlayerStatus::QUICKLY
        || vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())) {
        capability.maxSpeed *= FastFactor;
        capability.maxAccel *= FastFactor;
        capability.maxDec *= FastFactor;
        capability.maxAngularSpeed *= FastFactor;
        capability.maxAngularAccel *= FastFactor;
        capability.maxAngularDec *= FastFactor;
    }

    if (playerFlag & PlayerStatus::QUICKLY
        || vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())) {
    }

    //指定加速度上限；
    //后卫在离禁区比较远的时候，一律作为普通车处理，不允许指定加速度
    if (task().player.max_acceleration > 1e-8 && !(isBack && !Utils::InOurPenaltyArea(mePos, 40))) {
        capability.maxAccel = task().player.max_acceleration > TRANSLATION_ACC_LIMIT ? TRANSLATION_ACC_LIMIT : task().player.max_acceleration;
        if(isGoalie || isBack) {
            capability.maxAccel = task().player.max_acceleration;
        }
        capability.maxDec = capability.maxAccel;
    }
    //指定速度上限
    if (task().player.max_speed > 1e-8) {
        capability.maxSpeed = task().player.max_speed > TRANSLATION_SPEED_LIMIT ? TRANSLATION_SPEED_LIMIT : task().player.max_speed;
    }
    //最大转动加速度上限
    if (task().player.max_rot_acceleration > 1e-8) {
        capability.maxAngularAccel = task().player.max_rot_acceleration > TRANSLATION_ROTATE_ACC_LIMIT ? TRANSLATION_ROTATE_ACC_LIMIT : task().player.max_rot_acceleration;
        capability.maxAngularDec = capability.maxAngularAccel;
    }
    //最大转速上限----其实没什么用
    if (task().player.max_rot_speed > 1e-8) {
        capability.maxAngularSpeed = task().player.max_rot_speed;
    }
    //GameStop状态不能超速
    if (WorldModel::Instance()->CurrentRefereeMsg() == "GameStop") {
        capability.maxSpeed = 140;
    }

    return capability;
}

//处理无效目标点:在禁区内、在车身内、在场地外

void CSmartGotoPositionV3::validateFinalTarget(CGeoPoint& finalTarget,  const CVisionModule* pVision, bool shrinkTheirPenalty, double avoidLength, bool isGoalie) {
    //free kick的时候需要多避对方禁区20cm
    double theirPenaltyAvoidLength = avoidLength;
    if(playerFlag & PlayerStatus::FREE_KICK) {
        theirPenaltyAvoidLength += 20.0;
    }
    else if(!shrinkTheirPenalty) {
        theirPenaltyAvoidLength += Param::Vehicle::V2::PLAYER_SIZE;
    }
    //界外和禁区内的点调整进来
    if(!(playerFlag & PlayerStatus::NOT_AVOID_PENALTY)) {
        finalTarget = Utils::MakeInField(finalTarget, -avoidLength * 2);
        if(!isGoalie && Utils::InOurPenaltyArea(finalTarget, avoidLength))
            finalTarget = Utils::MakeOutOfOurPenaltyArea(finalTarget, avoidLength);
        if(Utils::InTheirPenaltyArea(finalTarget, theirPenaltyAvoidLength)) {
            finalTarget = Utils::MakeOutOfTheirPenaltyArea(finalTarget, theirPenaltyAvoidLength);
        }
    }
//    for(int i = 0; i < Param::Field::MAX_PLAYER_NUM; i++) {
////        if(pVision->TheirPlayer(i).Valid()) {
////            GDebugEngine::Instance()->gui_debug_msg(pVision->TheirPlayer(i).Pos(), QString("valid").toLatin1());
////        } else {
////            GDebugEngine::Instance()->gui_debug_msg(pVision->TheirPlayer(i).Pos(), QString("not valid").toLatin1());
////        }

//        if(!pVision->TheirPlayer(i).Valid()) continue;
//        const PlayerVisionT& opp = pVision->TheirPlayer(i);
//        double adjustDist = Param::Vehicle::V2::PLAYER_SIZE;
//        while(Utils::InTheirPenaltyArea(opp.Pos(), Param::Vehicle::V2::PLAYER_SIZE * 1.8) &&
//                finalTarget.dist(opp.Pos()) < Param::Vehicle::V2::PLAYER_SIZE * 2.3) {
//            finalTarget = Utils::MakeOutOfTheirPenaltyArea(finalTarget, adjustDist);
//            adjustDist += 5.0;
//        }
//    }
}

//处理规划起始点，防止rrt规划失败
void CSmartGotoPositionV3::validateStartPoint(CGeoPoint &startPoint, double avoidLength, bool isGoalie, ObstaclesNew &obs) {

    if(!(playerFlag & PlayerStatus::NOT_AVOID_PENALTY)) {
        if(!isGoalie && Utils::InOurPenaltyArea(startPoint, avoidLength))
            startPoint = Utils::MakeOutOfOurPenaltyArea(startPoint, avoidLength);
        if(Utils::InTheirPenaltyArea(startPoint, avoidLength)) {
            if(playerFlag & PlayerStatus::FREE_KICK)
                startPoint = Utils::MakeOutOfTheirPenaltyArea(startPoint, 3 * avoidLength);
            else
                startPoint = Utils::MakeOutOfTheirPenaltyArea(startPoint, avoidLength);
        }
    }
    CGeoPoint originStartPoint = startPoint;
    CVector adjustVec(0, 0);
    int obsNum = obs.getNum(), i = 0;
    for(i = 0; i < obsNum; i++) {
        if(!obs.obs[i].check(startPoint)) {

            if(obs.obs[i].getType() == OBS_CIRCLE_NEW) {
                adjustVec = adjustVec + Utils::Polar2Vector((avoidLength + obs.obs[i].getRadius() - obs.obs[i].getStart().dist(startPoint)), (startPoint - obs.obs[i].getStart()).dir());
            } else if(obs.obs[i].getType() == OBS_LONG_CIRCLE_NEW) {
                CGeoPoint nearPoint = obs.obs[i].getStart().dist(startPoint) > obs.obs[i].getEnd().dist(startPoint) ? obs.obs[i].getEnd() : obs.obs[i].getStart();
                adjustVec = adjustVec + Utils::Polar2Vector(avoidLength + obs.obs[i].getRadius() - nearPoint.dist(startPoint), (startPoint - nearPoint).dir());
            }
        }
    }
    double adjustStep = 9.0;
    adjustVec = adjustVec / adjustVec.mod();
    i = 0;
    while(!obs.check(startPoint) && i < 10) {
        startPoint = startPoint + adjustVec * adjustStep;
        i++;
    }
    if(i > 10)
        startPoint = originStartPoint;
}


//考虑到点后的速度对末位置的漂移影响； 速度在垂直于轨迹线方向对初始位置的影响
CVector CSmartGotoPositionV3::validateFinalVel(const bool isGoalie, const CGeoPoint& startPos,const CGeoPoint &targetPos, const CVector &targetVel) {

    if(targetVel.mod() < 1e-8) return CVector(0.0, 0.0);
    double distToStop, velMod, adjustStep, factorFront, factorBack, distToTarget, maxFinalSpeedAccordingToDist;
    CGeoPoint stopPosFront, stopPosBack;
    CVector velNew = targetVel;
    CVector velDir = targetVel / targetVel.mod();
    distToTarget = startPos.dist(targetPos);


    if(distToTarget < 1e-8 || _capability.maxAccel < 1e-8)
        return CVector(0.0, 0.0);
    maxFinalSpeedAccordingToDist = sqrt(2 * _capability.maxAccel * distToTarget * 0.5);
    if(maxFinalSpeedAccordingToDist > _capability.maxSpeed)
        maxFinalSpeedAccordingToDist = _capability.maxSpeed;

    if(targetVel.mod() > maxFinalSpeedAccordingToDist)
        velNew = velDir * maxFinalSpeedAccordingToDist;
    velMod = velNew.mod();
    factorFront = safeVelFactorFront;
    factorBack = safeVelFactorBack;
    adjustStep = 5;

    if(_capability.maxDec < 1e-8) return CVector(0.0, 0.0);
    distToStop = pow(targetVel.mod(), 2) / (2.0 * _capability.maxDec);
    stopPosFront = targetPos + velDir * distToStop * factorFront;
    stopPosBack = targetPos + velDir * (-distToStop) * factorBack;

    while(Utils::OutOfField(stopPosFront, Param::Vehicle::V2::PLAYER_SIZE) ||
          Utils::InTheirPenaltyArea(stopPosFront, Param::Vehicle::V2::PLAYER_SIZE) ||
          (!isGoalie && Utils::InOurPenaltyArea(stopPosFront, Param::Vehicle::V2::PLAYER_SIZE)) ||
          Utils::OutOfField(stopPosBack, Param::Vehicle::V2::PLAYER_SIZE) ||
            Utils::InTheirPenaltyArea(stopPosBack, Param::Vehicle::V2::PLAYER_SIZE) ||
            (!isGoalie && Utils::InOurPenaltyArea(stopPosBack, Param::Vehicle::V2::PLAYER_SIZE))) {
        velMod -= adjustStep;
        if(velMod < 0) return CVector(0.0, 0.0);
        velNew = velDir * velMod;
        if(_capability.maxSpeed > 1e-8 && safeVelFactorFront > 1.0)
            factorFront = velNew.mod() * (safeVelFactorFront - 1.0) / _capability.maxSpeed + 1.0;
        if(_capability.maxSpeed > 1e-8 && safeVelFactorBack > 1.0)
            factorBack = velNew.mod() * (safeVelFactorBack - 1.0) / _capability.maxSpeed + 1.0;
        factorFront = (factorFront >= 1.0) ? factorFront : 1.0;
        factorBack = (factorBack >= 1.0) ? factorBack : 1.0;
        if(_capability.maxDec < 1e-8) return CVector(0.0, 0.0);
        distToStop = pow(velNew.mod(), 2) / (2.0 * _capability.maxDec);
        stopPosFront = targetPos + velDir * distToStop * factorFront;
        stopPosBack = targetPos + velDir * (-distToStop) * factorBack;
    }
    return velNew;
}

//单独处理禁区有关的规划失败问题
CGeoPoint CSmartGotoPositionV3::dealPlanFail(CGeoPoint startPoint, CGeoPoint nextPoint, double avoidLength, bool shrinkTheirPenalty) {

    CGeoRectangle theirPenalty(CGeoPoint(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH, -Param::Field::PENALTY_AREA_WIDTH/2), CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::PENALTY_AREA_WIDTH / 2));
    CGeoRectangle ourPenalty(CGeoPoint(-Param::Field::PITCH_LENGTH / 2, -Param::Field::PENALTY_AREA_WIDTH / 2), CGeoPoint(-Param::Field::PITCH_LENGTH / 2 + Param::Field::PENALTY_AREA_DEPTH, Param::Field::PENALTY_AREA_WIDTH / 2));
    CGeoLine pathLine(startPoint, nextPoint);
    if(!shrinkTheirPenalty) {
        theirPenalty = CGeoRectangle(CGeoPoint(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH - Param::Vehicle::V2::PLAYER_SIZE, -Param::Field::PENALTY_AREA_WIDTH/2 - Param::Vehicle::V2::PLAYER_SIZE), CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::PENALTY_AREA_WIDTH / 2 + Param::Vehicle::V2::PLAYER_SIZE));
    }
    CGeoLineRectangleIntersection theirInter(pathLine, theirPenalty);
    CGeoLineRectangleIntersection ourInter(pathLine, ourPenalty);
    CGeoPoint nextPointNew = nextPoint;
    CGeoPoint candidate11 = CGeoPoint(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH - avoidLength - OPP_AVOID_DIST, Param::Field::PENALTY_AREA_WIDTH / 2 + avoidLength + OPP_AVOID_DIST),
            candidate12  = CGeoPoint(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH - avoidLength - OPP_AVOID_DIST, Param::Field::PENALTY_AREA_WIDTH / 2 - avoidLength - OPP_AVOID_DIST),
            candidate21 = CGeoPoint(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH - avoidLength - OPP_AVOID_DIST, -Param::Field::PENALTY_AREA_WIDTH / 2 - avoidLength - OPP_AVOID_DIST),
            candidate22 = CGeoPoint(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH - avoidLength - OPP_AVOID_DIST, -Param::Field::PENALTY_AREA_WIDTH / 2 + avoidLength + OPP_AVOID_DIST),
            candidate31 = CGeoPoint(-Param::Field::PITCH_LENGTH / 2 + Param::Field::PENALTY_AREA_DEPTH + avoidLength + OPP_AVOID_DIST, Param::Field::PENALTY_AREA_WIDTH / 2 + avoidLength + OPP_AVOID_DIST),
            candidate32 = CGeoPoint(-Param::Field::PITCH_LENGTH / 2 + Param::Field::PENALTY_AREA_DEPTH + avoidLength + OPP_AVOID_DIST, Param::Field::PENALTY_AREA_WIDTH / 2 - avoidLength - OPP_AVOID_DIST),
            candidate41 = CGeoPoint(-Param::Field::PITCH_LENGTH / 2 + Param::Field::PENALTY_AREA_DEPTH + avoidLength + OPP_AVOID_DIST, -Param::Field::PENALTY_AREA_WIDTH / 2 - avoidLength - OPP_AVOID_DIST),
            candidate42 = CGeoPoint(-Param::Field::PITCH_LENGTH / 2 + Param::Field::PENALTY_AREA_DEPTH + avoidLength + OPP_AVOID_DIST, -Param::Field::PENALTY_AREA_WIDTH / 2 + avoidLength + OPP_AVOID_DIST);
    if(DRAW_PENALTY_DEBUG_MSG) {
        GDebugEngine::Instance()->gui_debug_x(candidate11);
        GDebugEngine::Instance()->gui_debug_x(candidate21);
        GDebugEngine::Instance()->gui_debug_x(candidate31);
        GDebugEngine::Instance()->gui_debug_x(candidate41);
        GDebugEngine::Instance()->gui_debug_x(candidate12);
        GDebugEngine::Instance()->gui_debug_x(candidate22);
        GDebugEngine::Instance()->gui_debug_x(candidate32);
        GDebugEngine::Instance()->gui_debug_x(candidate42);
        GDebugEngine::Instance()->gui_debug_msg(candidate11, QString("11").toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(candidate12, QString("12").toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(candidate21, QString("21").toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(candidate22, QString("22").toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(candidate31, QString("31").toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(candidate32, QString("32").toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(candidate41, QString("41").toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(candidate42, QString("42").toLatin1());
    }


    if(theirInter.intersectant() || ourInter.intersectant()) {
        if(!((startPoint.y() > Param::Field::PENALTY_AREA_WIDTH / 2 && nextPoint.y() > Param::Field::PENALTY_AREA_WIDTH / 2 ) ||
             (startPoint.y() < -Param::Field::PENALTY_AREA_WIDTH / 2 && nextPoint.y() < -Param::Field::PENALTY_AREA_WIDTH / 2) ||
             (startPoint.y() < Param::Field::PENALTY_AREA_WIDTH / 2 && startPoint.y() > -Param::Field::PENALTY_AREA_WIDTH / 2 && nextPoint.y() < Param::Field::PENALTY_AREA_WIDTH / 2 && nextPoint.y() > -Param::Field::PENALTY_AREA_WIDTH / 2))) {

            if(startPoint.x() > Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH) {
                if(startPoint.y() > Param::Field::PENALTY_AREA_WIDTH / 2) {
                    nextPointNew = candidate11;
                    if(nextPoint.dist(candidate12) < nextPoint.dist(candidate12) && startPoint.dist(candidate11) < 5)
                        nextPointNew = candidate12;
                } else {
                    nextPointNew = candidate21;
                    if(nextPoint.dist(candidate22) < nextPoint.dist(candidate22) && startPoint.dist(candidate21) < 5)
                        nextPointNew = candidate22;
                }
            }
            else if(startPoint.x() < -Param::Field::PITCH_LENGTH / 2 + Param::Field::PENALTY_AREA_DEPTH) {
                if(startPoint.y() > Param::Field::PENALTY_AREA_WIDTH / 2) {
                    nextPointNew = candidate31;
                    if(nextPoint.dist(candidate32) < nextPoint.dist(candidate32) && startPoint.dist(candidate31) < 5)
                        nextPointNew = candidate32;
                }
                else {
                    nextPointNew = candidate41;
                    if(nextPoint.dist(candidate42) < nextPoint.dist(candidate42) && startPoint.dist(candidate41) < 5)
                        nextPointNew = candidate42;
                }
            }
            else if(nextPoint.x() > Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH) {
                if(nextPoint.y() > Param::Field::PENALTY_AREA_WIDTH / 2) {
                    nextPointNew = candidate12;
                    if(nextPoint.dist(candidate11) < nextPoint.dist(candidate12) && startPoint.dist(candidate12) < 5)
                        nextPointNew = candidate11;
                }
                else {
                    nextPointNew = candidate22;
                    if(nextPoint.dist(candidate21) < nextPoint.dist(candidate22) && startPoint.dist(candidate22) < 5)
                        nextPointNew = candidate21;
                }
            }
            else if(nextPoint.x() < -Param::Field::PITCH_LENGTH / 2 + Param::Field::PENALTY_AREA_DEPTH) {
                if(nextPoint.y() > Param::Field::PENALTY_AREA_WIDTH / 2) {
                    nextPointNew = candidate32;
                    if(nextPoint.dist(candidate31) < nextPoint.dist(candidate32) && startPoint.dist(candidate32) < 5)
                        nextPointNew = candidate31;
                }
                else {
                    nextPointNew = candidate42;
                    if(nextPoint.dist(candidate41) < nextPoint.dist(candidate42) && startPoint.dist(candidate42) < 5)
                        nextPointNew = candidate41;
                }
            }
        }
    }

    return nextPointNew;
}

CGeoPoint CSmartGotoPositionV3::finalPointBallPlacement(const CGeoPoint &startPoint, const CGeoPoint &target, const CGeoPoint &segPoint1, const CGeoPoint &segPoint2, const double avoidLength, ObstaclesNew& obst, bool& canDirect) {
    CGeoPoint targetNew = target;
    CGeoPoint projectionPoint = CGeoPoint(1e8, 1e8);
    CGeoSegment obsSegment(segPoint1, segPoint2);
    CGeoSegment pathLine(startPoint, target);
    double obsRadius = ballPlacementDist;
    const double CAN_PASS_ANGLE = Param::Math::PI / 9;
    CVector segVec = segPoint1 - segPoint2;
    CVector adjustDir = segVec.rotate(Param::Math::PI / 2).unit();
    double adjustStep = 5.0;
    double avoidBuffer = 60;
    canDirect = true;

    ///情况1：自己和目标都在区域内，把自己移出区域并作为新的目标点；
    if(obsSegment.dist2Point(startPoint) < obsRadius + avoidLength /*&&
            obsSegment.dist2Point(target) < obsRadius + avoidLength*/) {
        GDebugEngine::Instance()->gui_debug_msg(startPoint, QString("Me In, Target In").toLatin1(), COLOR_ORANGE);
        targetNew = startPoint;
        projectionPoint = startPoint;
        while (obsSegment.dist2Point(targetNew) < obsRadius + avoidLength) {
            targetNew = targetNew + adjustDir * adjustStep;
        }
        if(targetNew.dist(startPoint) > obsRadius + avoidLength) {
            adjustDir = -adjustDir;
            targetNew = targetNew + (adjustDir) * (2 * (obsRadius + avoidLength + adjustStep));
        }
        targetNew = targetNew + adjustDir * avoidBuffer;
        CGeoPoint originTargetNew = targetNew;
        double adjustBuffer = avoidBuffer;
        while(Utils::OutOfField(targetNew, 0) ||
                Utils::InOurPenaltyArea(targetNew, avoidLength) ||
                Utils::InTheirPenaltyArea(targetNew, avoidLength)) {
            targetNew = targetNew + (adjustDir) * (2 * (obsRadius + avoidLength + adjustStep + adjustBuffer));
            adjustBuffer -= 10;
            if(adjustBuffer < 10) break;
        }

        adjustDir = -adjustDir;
        adjustBuffer = avoidBuffer;
        targetNew = originTargetNew;
        while(Utils::OutOfField(targetNew, 0) ||
                Utils::InOurPenaltyArea(targetNew, avoidLength) ||
                Utils::InTheirPenaltyArea(targetNew, avoidLength)) {
            targetNew = targetNew + (adjustDir) * (2 * (obsRadius + avoidLength + adjustStep + adjustBuffer));
            adjustBuffer -= 10;
            if(adjustBuffer < 10) break;
        }
//        targetNew = targetNew + adjustDir * avoidBuffer;
        if(avoidBuffer < 10)
            targetNew = CGeoPoint(0.0, 0.0);
        canDirect = false;
    }

    ///情况2： 自己在区域外，目标在区域内，将目标移动到己方区域边界
    else if(obsSegment.dist2Point(startPoint) > obsRadius + avoidLength &&
            obsSegment.dist2Segment(pathLine) < obsRadius + avoidLength) {
        GDebugEngine::Instance()->gui_debug_msg(startPoint, QString("Me Out, Target In").toLatin1(), COLOR_ORANGE);
        targetNew = target;
        projectionPoint = startPoint;
        targetNew = startPoint;
//        while(obsSegment.dist2Point(targetNew) < obsRadius + avoidLength)
//            targetNew = targetNew + adjustDir * adjustStep;
//        if(obsSegment.dist2Segment(CGeoSegment(startPoint, targetNew)) < obsRadius + avoidLength) {
//            adjustDir = -adjustDir;
//            targetNew = targetNew + (adjustDir) * (2 * (obsRadius + avoidLength + adjustStep));
//        }
//        targetNew = targetNew + adjustDir * avoidBuffer;
//        if(Utils::OutOfField(targetNew, avoidLength) || Utils::InOurPenaltyArea(targetNew, avoidLength) || Utils::InTheirPenaltyArea(targetNew, avoidLength)) {
//            adjustDir = -adjustDir;
//            targetNew = targetNew + (adjustDir) * (2 * (obsRadius + avoidLength + adjustStep) + avoidBuffer);
//        }
//        targetNew = targetNew + adjustDir * avoidBuffer;
        canDirect = false;
    }

    ///情况3：自己和目标点都在区域外，且分居区域两侧，将自己尽可能向靠近目标点的区域外同侧移动
    else if(obsSegment.dist2Point(startPoint) > obsRadius + avoidLength && obsSegment.dist2Point(target) > obsRadius + avoidLength && obsSegment.dist2Segment(CGeoSegment(startPoint, target)) < obsRadius + avoidLength) {
        GDebugEngine::Instance()->gui_debug_msg(startPoint, QString("Me Out, Target Out").toLatin1(), 0);
        targetNew = startPoint;
        projectionPoint = target;
        CVector adjustDir2 = segVec.unit();
        CVector me2Target = target - startPoint;
        if(fabs(fabs(Utils::Normalize(me2Target.dir() - adjustDir2.dir())) - Param::Math::PI / 2) < CAN_PASS_ANGLE) return target;
        if(adjustDir2 * me2Target < 0)
            adjustDir2 = -adjustDir2;
        while(me2Target * adjustDir2 > 0) {
            targetNew = targetNew + adjustDir2 * adjustStep;
            me2Target = target - targetNew;
        }
        if(obsSegment.dist2Point(targetNew) > obsRadius + avoidLength + adjustStep) {
            if(obsSegment.dist2Point(targetNew + adjustDir) < obsSegment.dist2Point(targetNew)) {
                targetNew = targetNew + adjustDir * (obsSegment.dist2Point(targetNew) - obsRadius - avoidLength - adjustStep);
            }
            else {
                targetNew = targetNew + (-adjustDir) * (obsSegment.dist2Point(targetNew) - obsRadius - avoidLength - adjustStep);
            }
        }

    }

    ///情况4：自己在区域内，目标在区域外，新目标点为目标点向自己方向移动的区域边界对应点
    else if(obsSegment.dist2Point(startPoint) < obsRadius + avoidLength && obsSegment.dist2Point(target) > obsRadius + avoidLength) {
        GDebugEngine::Instance()->gui_debug_msg(startPoint, QString("Me In, Target Out").toLatin1(), 0);
        targetNew = target;
        projectionPoint = startPoint;
        CVector adjustDir2 = segVec.unit();
        CVector target2Me = startPoint - targetNew;
        if(fabs(fabs(Utils::Normalize(target2Me.dir() - adjustDir2.dir())) - Param::Math::PI / 2) < CAN_PASS_ANGLE) return target;
        if(adjustDir2 * target2Me < 0)
            adjustDir2 = -adjustDir2;
        while(target2Me * adjustDir2 > 0) {
            targetNew = targetNew + adjustDir2 * adjustStep;
            target2Me = startPoint - targetNew;
        }
        if(obsSegment.dist2Point(targetNew) > obsRadius + avoidLength + adjustStep) {
            if(obsSegment.dist2Point(targetNew + adjustDir) < obsSegment.dist2Point(targetNew))
                targetNew = targetNew + adjustDir * (obsSegment.dist2Point(targetNew) - obsRadius - avoidLength - adjustStep);
            else
                targetNew = targetNew + (-adjustDir) * (obsSegment.dist2Point(targetNew) - obsRadius - avoidLength - adjustStep);
        }
    }
    ///如果前面初步调整得到的目标点在障碍物中，则进一步搜索
//    if(!obst.check(targetNew)) {
//        CGeoPoint originTargetNew = targetNew;
//        CVector xAdjustDir = segVec.unit();
//        CVector yAdjustDir;
//        if(!Utils::OutOfField(projectionPoint, -20))
//            yAdjustDir = (targetNew - projectionPoint).unit();
//        else
//            yAdjustDir = adjustDir;
//        if(obsSegment.dist2Point(targetNew) > obsSegment.dist2Point(targetNew + yAdjustDir))
//            yAdjustDir = -adjustDir;
//        double totalAvoidDist = 0.0;
//        double maxAvoidDist = 100;
//        adjustStep = 9.0;
//        bool reversed = false;
//        while(totalAvoidDist < maxAvoidDist && !obst.check(targetNew)) {
//            targetNew = targetNew + xAdjustDir * adjustStep;
//            CVector testVec = targetNew - projectionPoint;
//            if(fabs(fabs(Utils::Normalize(testVec.dir() - xAdjustDir.dir())) - Param::Math::PI / 2) > CAN_PASS_ANGLE / 1.5) {
//                if(!reversed) {
//                    xAdjustDir = -xAdjustDir;
//                    targetNew = originTargetNew;
//                    reversed = true;
//                } else {
//                    targetNew = originTargetNew = originTargetNew + yAdjustDir * adjustStep;
//                    totalAvoidDist += adjustStep;
//                    reversed = false;
//                }
//            }
//        }
//        if(!obst.check(targetNew) && obsSegment.dist2Point(startPoint) > obsRadius + avoidLength)
//            targetNew = startPoint;
//    }

    return targetNew;
}


