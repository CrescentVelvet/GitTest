#include "GotoPosition.h"
#include <utils.h>
#include "skill/Factory.h"
#include <CommandFactory.h>
#include <VisionModule.h>
#include <RobotCapability.h>
#include <sstream>
#include <TaskMediator.h>
#include <ControlModel.h>
#include <robot_power.h>
#include <DribbleStatus.h>
#include "PlayInterface.h"
#include <GDebugEngine.h>
#include "DynamicsSafetySearch.h"
#include "CMmotion.h"
#include <fstream>
#include "parammanager.h"
/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace{
    /// 调试开关
    bool DRAW_TARGET = false;
    bool DRAW_DEBUG_MSG = false;

    double MAX_DIST_NOT_STOP = 300;
    double MIN_DIST_NOT_STOP = 60;

    /// 用于解决到点晃动的问题
    const double DIST_REACH_CRITICAL = 2;	// [unit : cm]
    const double SlowFactor = 0.5;
    const double FastFactor = 1.2;

    /// 底层运动控制参数 ： 默认增大平动的控制性能
    double MAX_TRANSLATION_SPEED = 400;
    double MAX_TRANSLATION_ACC = 600;
    double MAX_ROTATION_SPEED = 5;
    double MAX_ROTATION_ACC = 15;
    double MAX_TRANSLATION_DEC = 650;

    const double TRANSLATION_ACC_LIMIT = 500;
    double TRANSLATION_SPEED_LIMIT = 350;
    const double TRANSLATION_ROTATE_ACC_LIMIT = 50;


    double stopBallAvoidDist = 50;
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

    /// 底层控制方法参数
    int TRAJECTORY_METHORD = 1;				// 默认使用 CMU 的轨迹控制
    int TASK_TARGET_COLOR = COLOR_CYAN;
    int SAO_ACTION;

    double Y_COMPENSATE_K;
    double Y_COMPENSATE_B;
}
using namespace Param::Vehicle::V2;

/// 构造函数 ： 参数初始化
CGotoPositionV2::CGotoPositionV2()
{
    ZSS::ZParamManager::instance()->loadParam(MAX_DIST_NOT_STOP,"CGotoPositionV2/MaxDistNotStop",60);
    ZSS::ZParamManager::instance()->loadParam(MIN_DIST_NOT_STOP,"CGotoPositionV2/MinDistNotStop",20);

    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_SPEED,"CGotoPositionV2/MNormalSpeed",300);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_ACC,"CGotoPositionV2/MNormalAcc",450);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_DEC,"CGotoPositionV2/MNormalDec",450);
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
    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_ACC,"CGotoPositionV2/RotationAcc",15);
    ZSS::ZParamManager::instance()->loadParam(DRAW_TARGET,"Debug/TargetPos",false);
    ZSS::ZParamManager::instance()->loadParam(TRAJECTORY_METHORD,"CGotoPositionV2/TrajectoryMethod",1);
    ZSS::ZParamManager::instance()->loadParam(SAO_ACTION,"Alert/SaoAction",0);
    ZSS::ZParamManager::instance()->loadParam(Y_COMPENSATE_K,"CGotoPositionV2/YCompensateK",0.0035);
    ZSS::ZParamManager::instance()->loadParam(Y_COMPENSATE_B,"CGotoPositionV2/YCompensateB",-0.1042);

    ZSS::ZParamManager::instance()->loadParam(TRANSLATION_SPEED_LIMIT,"CGotoPositionV2/NormalSpeedLimit",400); //下发速度最大限制，守门员、后卫除外
}

/// 输出流 ： 参数显示
void CGotoPositionV2::toStream(std::ostream& os) const
{
    os << "Going to " << task().player.pos<<" angle:"<<task().player.angle;
}

/// 规划入口
void CGotoPositionV2::plan(const CVisionModule* pVision)
{
    return ;
}

/// 执行接口
CPlayerCommand* CGotoPositionV2::execute(const CVisionModule* pVision)
{
    /************************************************************************/
    /* 任务参数解析                                                         */
    /************************************************************************/
    const int vecNumber = task().executor;
    const PlayerVisionT& self = pVision->OurPlayer(vecNumber);
    const CGeoPoint& vecPos = self.Pos();							// 小车的位置
    CGeoPoint target = task().player.pos;							// 目标的位置
    playerFlag = task().player.flag;
    const bool needBreakRotate = (playerFlag & PlayerStatus::BREAK_THROUGH);
    nonZeroMode mode =  FAST;
    const bool isBack = (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->leftBack())) ||
                         (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->rightBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->singleBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->sideBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->defendMiddle()));

    const bool avoidBallCircle = (WorldModel::Instance()->CurrentRefereeMsg() == "GameStop") || (playerFlag & PlayerStatus::AVOID_STOP_BALL_CIRCLE);
    /************************************************************************/
    /* 修正非法目标点输入                                                   */
    /************************************************************************/
    if (isnan(target.x()) || isnan(target.y())) {
        target = self.Pos();
        cout << "Target Pos is NaN, vecNumber is : " << vecNumber << endl;
    }
    double arrivedDist = self.Vel().mod() * 0.12 + 0.1;

    //如果是后卫且距离禁区比较远，需要打开DSS避障，防止刚匹配的后卫发生碰撞
    //在禁区边的后卫允许撞车，否则容易被进球!!!
    if(isBack && !Utils::InOurPenaltyArea(vecPos, 40)) {
        playerFlag |= PlayerStatus::ALLOW_DSS;
    }

    double avoidLength = Param::Vehicle::V2::PLAYER_SIZE;
    if ((playerFlag & PlayerStatus::FREE_KICK)) {
        avoidLength += 20.0;
    }

    bool ignoreNotStop = false;
    target = avoidPenaltyArea(pVision, vecPos, target, avoidLength, vecNumber);

    // 记录当前的规划执行目标点
    GDebugEngine::Instance()->gui_debug_x(target, TASK_TARGET_COLOR);
    GDebugEngine::Instance()->gui_debug_line(self.Pos(), target, TASK_TARGET_COLOR);
    if(task().player.vel.mod() > 1e-8) {
        GDebugEngine::Instance()->gui_debug_line(target, target + task().player.vel / 10, COLOR_WHITE);
    }
    if(avoidBallCircle) {
        double buffer = Param::Vehicle::V2::PLAYER_SIZE;
        CGeoPoint ballPos = pVision->Ball().Pos();
        target = Utils::MakeOutOfCircle(ballPos, stopBallAvoidDist, target, buffer);
        if(self.Pos().dist(ballPos) < stopBallAvoidDist + buffer)
            target = Utils::MakeOutOfCircle(self.Pos(), stopBallAvoidDist, target, buffer);
    }
    const CVector player2target = target - vecPos;
    const double dist = player2target.mod();
    const bool dribble = playerFlag & PlayerStatus::DRIBBLING;
    unsigned char dribblePower = 0;

    /************************************************************************/
    /* 确定运动性能参数 确定只使用OmniAuto配置标签中的参数                       */
    /************************************************************************/
    /// 判断怎么走
    CCommandFactory* pCmdFactory = CmdFactory::Instance();					// 指向CommandFactoryV2的指针
    PlayerCapabilityT capability = setCapability(pVision);

    /************************************************************************/
    /* 确定初末状态 结合 选取的控制方式生成运动指令                  */
    /************************************************************************/
    /// 设定目标状态
    PlayerPoseT final;
    final.SetPos(target);
    final.SetDir((playerFlag & (PlayerStatus::POS_ONLY | PlayerStatus::TURN_AROUND_FRONT)) ? self.Dir() : task().player.angle);
    if(needBreakRotate) {
        double dir = ZSkillUtils::instance()->holdBallDir(pVision, vecNumber);
        if(dir < 1e4)
            final.SetDir(dir);
    }
    final.SetVel(task().player.vel);
    final.SetRotVel(task().player.rotvel);
    /// 调用控制方法
    CControlModel control;
    float usedtime = target.dist(self.Pos()) / capability.maxSpeed / 1.414;	// 单位：秒

    /// 进行轨迹生成并记录理想执行时间
    if (playerFlag & PlayerStatus::DO_NOT_STOP) { // 一般不会执行 ： cliffyin
        if (CMU_TRAJ == TRAJECTORY_METHORD) {		//CMU的运动控制方法，可以非零速度到点
            double distToTarget = player2target.mod();
            CVector taskVel = task().player.vel;
            if(taskVel.mod() < 1e-8) {
                CVector velUnit = player2target / player2target.mod();
                double v0 = self.Vel() * velUnit;
                double speed = 300.0;
                if(speed > 0.8 * sqrt(pow(v0, 2) + 2 * capability.maxAccel * distToTarget))
                    speed = 0.8 * sqrt(pow(v0, 2) + 2 * capability.maxAccel * distToTarget);
                if(distToTarget < 10) {
                    speed = 0.0;
                }
                ignoreNotStop = true;
                final.SetVel(velUnit * speed);
            }
            else {
                final.SetVel(taskVel);
            }
            control.makeCmTrajectory(self, final, capability, mode);
        }
        else {
            control.makeFastPath(self, final, capability);
            control.makeZeroFinalVelocityTheta(self, final, capability);
        }
    }
    else {																						// 真正执行部分 ： cliffyin
        int Current_Trajectory_Method = TRAJECTORY_METHORD;
        if (task().player.is_specify_ctrl_method) {// 指定的运动控制方式
            Current_Trajectory_Method = task().player.specified_ctrl_method;
        }
        switch (Current_Trajectory_Method) {
        case CMU_TRAJ:
            control.makeCmTrajectory(self, final, capability, mode);					// CMU 非零速到点
            break;
        case ZERO_FINAL:
            control.makeZeroFinalVelocityPath(self, final, capability);			// Bangbang 零速到点
            break;
        case ZERO_TRAP:
            control.makeTrapezoidalVelocityPath(self, final, capability);			// ZJUNlict 零速到点 : 存在问题，先不用
            break;
        case NONE_TRAP:
            control.makeNoneTrapezoidalVelocityPath(self, final, capability);	// ZJUNlict 非零速到点
            break;
        default:
            control.makeZeroFinalVelocityPath(self, final, capability);			// Bangbang 零速到点
            break;
        }
    }

    const double time_factor = 1.5;

    usedtime = expectedCMPathTime(self, final.Pos(), capability.maxAccel, capability.maxSpeed, time_factor);

    /************************************************************************/
    /* 调用动态避障模块（DSS），对轨迹生成模块运动指令进行避碰检查	*/
    /************************************************************************/
    // 获取轨迹生成模块在全局坐标系中的速度指令
    CVector globalVel = control.getNextStep().Vel();

    int priority = 0;
    if (playerFlag & PlayerStatus::ALLOW_DSS) {

        CVector tempVel = DynamicSafetySearch::Instance()->SafetySearch(vecNumber, globalVel, pVision, priority, target, task().player.flag, usedtime, task().player.max_acceleration);
        if (WorldModel::Instance()->CurrentRefereeMsg() == "GameStop" && tempVel.mod() > 150) { // 不加这个在stop的时候车可能会冲出去
        }
        else {
            globalVel = tempVel;
        }
    }
    if(DRAW_DEBUG_MSG) {
        GDebugEngine::Instance()->gui_debug_msg(self.Pos(), QString("originVel: (%1, %2)  %3").arg(self.Vel().x()).arg(self.Vel().y()).arg(self.Vel().mod()).toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(self.Pos() + CVector(0, 20), QString("finalVel: (%1, %2)").arg(globalVel.x()).arg(globalVel.y()).toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(self.Pos() + CVector(0, 40), QString("deltaVel: %1").arg(globalVel.mod() - self.Vel().mod()).toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(self.Pos() + CVector(0, 60), QString("acc: %1").arg(capability.maxAccel).toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(self.Pos() + CVector(0, 80), QString("targetVel: (%1, %2)").arg(task().player.vel.x()).arg(task().player.vel.y()).toLatin1());
    }

    /************************************************************************/
    /* 处理最后的指令                                                       */
    /************************************************************************/
    // 靠近系数调整，用于解决到点晃动的问题 [7/2/2011 cliffyin]
    double alpha = 1.0;
    if (dist <= DIST_REACH_CRITICAL) {
        alpha *= sqrt(dist / DIST_REACH_CRITICAL);
    }

    CVector localVel = (globalVel*alpha).rotate(-self.Dir());		// 车自己坐标系里面的速度

    if((playerFlag & PlayerStatus::DO_NOT_STOP) && player2target.mod() < arrivedDist && !ignoreNotStop)
        localVel = CVector(0.0, 0.0);

    double rotVel = control.getNextStep().RotVel();				// 旋转速度

    if ((fabs(Utils::Normalize(final.Dir() - self.Dir())) <= Param::Math::PI * 5 / 180)) {
        //用前者会小幅抖动，用后三句会很稳
        rotVel /= 2;
    }

    // 控球
    unsigned char set_power = DribbleStatus::Instance()->getDribbleCommand(vecNumber);

    if (set_power > 0) {
        dribblePower = set_power;
    }
    else {
        dribblePower = DRIBBLE_DISABLED;
    }

    if (dribble) {
        dribblePower = DRIBBLE_NORAML;
    }

    /// 生成并返回控制指令
    return pCmdFactory->newCommand(CPlayerSpeedV2(vecNumber, localVel.x(), localVel.y(), rotVel, dribblePower));
//        return pCmdFactory->newCommand(CPlayerSpeedV2(vecNumber, 0, 0, rotVel, dribblePower));
}

PlayerCapabilityT CGotoPositionV2::setCapability(const CVisionModule *pVision) {
    const int vecNumber = task().executor;
    const bool isGoalie = (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie()));
    const bool isBack = (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->leftBack())) ||
                         (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->rightBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->singleBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->sideBack())) ||
                        (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->defendMiddle()));

    const int playerFlag = task().player.flag;
    PlayerCapabilityT capability;

    // Traslation 确定运动参数
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
    if (task().player.max_acceleration > 1e-8) {
        capability.maxAccel = task().player.max_acceleration > TRANSLATION_ACC_LIMIT ? TRANSLATION_ACC_LIMIT : task().player.max_acceleration;
        if(isGoalie || isBack)
            capability.maxAccel = task().player.max_acceleration;
        capability.maxDec = capability.maxAccel;
    }
    if (task().player.max_speed > 1e-8) {
        capability.maxSpeed = task().player.max_speed > TRANSLATION_SPEED_LIMIT ? TRANSLATION_SPEED_LIMIT : task().player.max_speed;
    }

    if (task().player.max_rot_acceleration > 1e-8) {
        capability.maxAngularAccel = task().player.max_rot_acceleration > TRANSLATION_ROTATE_ACC_LIMIT ? TRANSLATION_ROTATE_ACC_LIMIT : task().player.max_rot_acceleration;
        capability.maxAngularDec = capability.maxAngularAccel;
    }

    if (task().player.max_rot_speed > 1e-8) {
        capability.maxAngularSpeed = task().player.max_rot_speed;
    }

    if (WorldModel::Instance()->CurrentRefereeMsg() == "GameStop") {
        capability.maxSpeed = 140;
    }
    return capability;
}

CGeoPoint CGotoPositionV2::avoidPenaltyArea(const CVisionModule* pVision, const CGeoPoint &startPos, const CGeoPoint &targetPos, const double avoidLength, const int vecNumber) {
    CGeoPoint target = targetPos;
    const int goalieNum = PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie());
    const bool isGoalie = (vecNumber == goalieNum);

    if (!(playerFlag & PlayerStatus::NOT_AVOID_PENALTY) && !isGoalie) target = Utils::MakeInField(target, -avoidLength);

    bool isMeInOurPenaltyArea = Utils::InOurPenaltyArea(startPos, Param::Vehicle::V2::PLAYER_SIZE);
    bool isTargetInOurPenaltyArea = Utils::InOurPenaltyArea(target, Param::Vehicle::V2::PLAYER_SIZE);
    if (!(playerFlag & PlayerStatus::NOT_AVOID_PENALTY) && !isGoalie ) {	// 非自动放球状态下，非门将队员在禁区里面 : 离开禁区
        if (isMeInOurPenaltyArea) {//我在禁区
            target = Utils::MakeOutOfOurPenaltyArea(startPos, avoidLength * 1.5);
        }
        else if (isTargetInOurPenaltyArea) {//目标在禁区
                double extra_out_dist = avoidLength * 1.5;
                while (extra_out_dist < 100) {
                    target = Utils::MakeOutOfOurPenaltyArea(target, extra_out_dist);
                    bool checkOk = true;
                    for (int teammate = 1; teammate <= Param::Field::MAX_PLAYER_NUM; teammate++) {
                        if (teammate != vecNumber) {
                            if (pVision->OurPlayer(teammate).Pos().dist(target) < avoidLength) {
                                checkOk = false;
                                break;
                            }
                        }
                    }
                    if (checkOk == true)
                        break;
                    extra_out_dist += avoidLength;
                }
        }
    }
     // 所有车都不能进对方禁区
    bool isMeInTheirPenaltyArea = Utils::InTheirPenaltyArea(startPos, avoidLength);
    bool isTargetInTheirPenaltyArea = Utils::InTheirPenaltyArea(target, avoidLength);
    if (!(playerFlag & PlayerStatus::NOT_AVOID_PENALTY)){      //非放球状态下
        if (isMeInTheirPenaltyArea == true) {
            target = Utils::MakeOutOfTheirPenaltyArea(startPos, avoidLength * 1.5);
        }
        //modified by Wang in 2018/4/3
        else if (isTargetInTheirPenaltyArea) {
            double extraOutDist = avoidLength * 1.5;
            while (extraOutDist < 100) {
                target = Utils::MakeOutOfTheirPenaltyArea(target, extraOutDist);
                bool checkOk = true;
                for (int teammate = 1; teammate <= Param::Field::MAX_PLAYER_NUM; teammate++) {
                    if (teammate != vecNumber) {
                        if (pVision->OurPlayer(teammate).Pos().dist(target) < avoidLength) {
                            checkOk = false;
                            break;
                        }
                    }
                }
                if (checkOk == true)
                    break;
                extraOutDist += avoidLength;
            }
        }
    }
    return target;
}