#include "GotoPositionNew.h"
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
#include "PathPlanner.h"
#include <fstream>
#include "parammanager.h"
#include "BezierCurveNew.h"
#include "BezierMotion.h"
#include <RRTPathPlannerNew.h>
#include <iostream>
/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace {
/// debug switch
bool drawPathPoints = false;
bool drawPathLine = true;
bool drawViaPoints = false;

/// dealing with the shaking when approaching the target
const double DIST_REACH_CRITICAL = 2;	// [unit : cm]
const double SlowFactor = 0.5;
const double FastFactor = 1.2;

/// motion control parameters
double MAX_TRANSLATION_SPEED = 400;		// [unit : cm/s]
double MAX_TRANSLATION_ACC = 600;		// [unit : cm/s2]
double MAX_ROTATION_SPEED = 5;			// [unit : rad/s]
double MAX_ROTATION_ACC = 15;			// [unit : rad/s2]
double TRANSLATION_ACC_LIMIT = 1000;
double MAX_TRANSLATION_DEC = 650;

/// for goalie
double MAX_TRANSLATION_SPEED_GOALIE;
double MAX_TRANSLATION_ACC_GOALIE;
double MAX_TRANSLATION_DEC_GOALIE;
double MAX_ROTATION_ACC_GOALIE;
double MAX_ROTATION_SPEED_GOALIE;

/// for guards
double MAX_TRANSLATION_SPEED_BACK;
double MAX_TRANSLATION_ACC_BACK;
double MAX_TRANSLATION_DEC_BACK;
double MAX_ROTATION_ACC_BACK;
double MAX_ROTATION_SPEED_BACK;

/// control method parameter
int TASK_TARGET_COLOR = COLOR_CYAN;
ofstream carVisionVel("C://Users//gayty//Desktop//Sydney//carVisionVel.txt");

PlanType playType = RRT;

bool DRAW_OBSTACLES = false;
const double minVelConstr = 150;
const double thresholdL = 20;
const double thresholdS = 10;
const double swapPathCost = 40;
const double stopDecFactor = 3.0;
const double angleToContri = PARAM::Math::PI / 10.0;
const double onlyRotateScale = 2.5;

const double FRAME_PERIOD = 1.0 / PARAM::Vision::FRAME_RATE;
const double TEAMMATE_AVOID_DIST = PARAM::Vehicle::V2::PLAYER_SIZE + 4.0f;
const double OPP_AVOID_DIST = PARAM::Vehicle::V2::PLAYER_SIZE + 5.5f;
const double BALL_AVOID_DIST = PARAM::Field::BALL_SIZE + 5.0f;

bool openDebug = false;
bool lastIsDirect[PARAM::Field::MAX_PLAYER] { false };
double flatVelFactor[PARAM::Field::MAX_PLAYER];
CGeoPoint lastTarget[PARAM::Field::MAX_PLAYER];
vector<CGeoPoint> viaPoints[PARAM::Field::MAX_PLAYER];
vector<CGeoPoint> pathPoints[PARAM::Field::MAX_PLAYER];
stateNew nextState[PARAM::Field::MAX_PLAYER];
BezierController controller[PARAM::Field::MAX_PLAYER];
CRRTPathPlannerNew plannerNew[PARAM::Field::MAX_PLAYER];

inline int bezierMin(int a, int b) {
    return a > b ? b : a;
}
}
using namespace PARAM::Vehicle::V2;

CGotoPositionNew::CGotoPositionNew() {
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_SPEED, "CGotoPositionV2/MNormalSpeed", 300);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_ACC, "CGotoPositionV2/MNormalAcc", 450);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_DEC, "CGotoPositionV2/MNormalDec", 450);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_SPEED_BACK, "CGotoPositionV2/MBackSpeed", 300);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_ACC_BACK, "CGotoPositionV2/MBackAcc", 450);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_DEC_BACK, "CGotoPositionV2/MBackDec", 450);

    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_ACC_BACK,"CGotoPositionV2/MBackRotateAcc",15);
    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_SPEED_BACK,"CGotoPositionV2/MBackRotateSpeed",15);

    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_SPEED_GOALIE, "CGotoPositionV2/MGoalieSpeed", 300);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_ACC_GOALIE, "CGotoPositionV2/MGoalieAcc", 450);
    ZSS::ZParamManager::instance()->loadParam(MAX_TRANSLATION_DEC_GOALIE, "CGotoPositionV2/MGoalieDec", 450);

    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_ACC_GOALIE,"CGotoPositionV2/MGoalieRotateAcc",15);
    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_SPEED_GOALIE,"CGotoPositionV2/MGoalieRotateSpeed",15);

    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_SPEED, "CGotoPositionV2/RotationSpeed", 15);
    ZSS::ZParamManager::instance()->loadParam(MAX_ROTATION_ACC, "CGotoPositionV2/RotationAcc", 15);
    ZSS::ZParamManager::instance()->loadParam(TRANSLATION_ACC_LIMIT, "CGotoPositionV2/AccLimit", 1400);
    ZSS::ZParamManager::instance()->loadParam(DRAW_OBSTACLES, "Debug/DrawObst", false);
}

void CGotoPositionNew::toStream(std::ostream& os) const {
    os << "Going to " << task().player.pos << " angle:" << task().player.angle;
}

void CGotoPositionNew::plan(const CVisionModule* pVision) {
    return ;
}

CPlayerCommand* CGotoPositionNew::execute(const CVisionModule* pVision) {

    const int vecNumber = task().executor;
    const PlayerVisionT& me = pVision->ourPlayer(vecNumber);
    const double vecDir = me.Dir();
    playerFlag = task().player.flag;
    CGeoPoint targetPos = task().player.pos;
    CGeoPoint startPosForRRT = me.Pos();
    const bool isGoalie = vecNumber == TaskMediator::Instance()->goalie();
    const bool isBack = (vecNumber == TaskMediator::Instance()->leftBack())
                        || (vecNumber == TaskMediator::Instance()->rightBack());
    const double targetDir = task().player.angle;
    if (isnan(targetPos.x()) || isnan(targetPos.y())) {
        targetPos = me.Pos();
        cout << "Target Pos is NaN, vecNumber is : " << vecNumber << endl;
    }
    GDebugEngine::Instance()->gui_debug_x(task().player.pos, COLOR_RED);
    CCommandFactory* pCmdFactory = CmdFactory::Instance();
    PlayerCapabilityT capability = setCapability(pVision);

    double buffer = 3.5;

    if(isBack || isGoalie) {
        buffer = 0.0;
    }
    double avoidDist = PARAM::Vehicle::V2::PLAYER_SIZE + buffer;
    ObstaclesNew* obs = new ObstaclesNew(avoidDist);
    obs->addObs(pVision, task(), DRAW_OBSTACLES, OPP_AVOID_DIST, TEAMMATE_AVOID_DIST, BALL_AVOID_DIST, false);

    validateFinalTarget(targetPos, me.Pos(), avoidDist, isGoalie, isBack, *obs);
    if(!isBack) {
        validateStartPoint(startPosForRRT, me, obs, capability, avoidDist, isGoalie, isBack);
    }

    GDebugEngine::Instance()->gui_debug_arc(startPosForRRT, avoidDist, 0, 360, COLOR_ORANGE);
    GDebugEngine::Instance()->gui_debug_x(targetPos, COLOR_YELLOW);

    const CVector player2target = targetPos - me.Pos();
    const double dist = player2target.mod();

//    CVector realVel((me.RawPos().x() - meLast.RawPos().x()) * PARAM::Vision::FRAME_RATE, (me.RawPos().y() - meLast.RawPos().y()) * PARAM::Vision::FRAME_RATE);
    stateNew start(me.Pos(), vecDir, me.Vel(), me.RotVel());
    stateNew startForRRT(startPosForRRT, vecDir, me.Vel(), me.RotVel());
    stateNew target(targetPos, targetDir, task().player.vel, task().player.rotvel);

//    for(int i = 0; i < obs->getNum(); i++) {
//        if(obs->obs[i].getType() == OBS_RECTANGLE_NEW && obs->obs[i].getStart().x() < 0.0)
//            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0.0, 0.0), QString("%1").arg(obs->obs[i].check(CGeoSegment(me.Pos(), targetPos))).toLatin1());
//    }
//    cout << obs->getNum() << endl;

    bool newTarget = false;
    if(targetPos.dist(lastTarget[vecNumber]) > 1e-8) {
        newTarget = true;
        lastTarget[vecNumber] = targetPos;
    }

    if(obs->check(start.pos, target.pos)) {
        viaPoints[vecNumber].clear();
        pathPoints[vecNumber].clear();
        viaPoints[vecNumber].push_back(start.pos);
        viaPoints[vecNumber].push_back(target.pos);
        pathPoints[vecNumber].push_back(start.pos);
        pathPoints[vecNumber].push_back(target.pos);
        nextState[vecNumber] = target;
    }
    else {
        bool planned = false;
        if(isBack) {
            viaPoints[0].clear();
            viaPoints[0].push_back(startForRRT.pos);
            vector < CGeoPoint > viaBack = forBack(pVision, vecNumber, me.Pos(), targetPos);
//            if(viaBack.size() == 1 && (obs->check(startForRRT.pos, viaBack[0]) || obs->check(viaBack[0], targetPos))) {
//                viaPoints[0].push_back(viaBack[0]);
//                planned = true;
//            }
            for(auto itor : viaBack) {
                viaPoints[0].push_back(itor);
            }
            planned = !viaBack.empty();
            viaPoints[0].push_back(targetPos);
        }
        if(!planned) {
            if(!plannerNew[vecNumber].plannerInitted())
                plannerNew[vecNumber].initPlanner(400, 20, 15, 0.05, 0.55, 180.0);
            plannerNew[vecNumber].planPath(obs, startForRRT, target);
            vector < stateNew > path = plannerNew[vecNumber].getPathPoints();
            viaPoints[0].clear();
            if(path[0].pos.dist(me.Pos()) < 1e-8) {
                for(auto itor : path)
                    viaPoints[0].push_back(itor.pos);
            }
            else {
                CVector dir1 = path[0].pos - me.Pos();
                CVector dir2 = path[1].pos - path[0].pos;
                viaPoints[0].push_back(me.Pos());
                int firstIdx = 0;
                if(fabs(Utils::Normalize(dir1.dir() - dir2.dir())) < angleToContri) firstIdx = 1;
                while(firstIdx < path.size()) {
                    viaPoints[0].push_back(path[firstIdx].pos);
                    firstIdx++;
                }
            }
        }

        controller[0].initController(start, target, capability, viaPoints[0]);
        controller[0].planController();
        double zeroScore = controller[0].evaluatePath(start, obs, true);
        double tempScore = controller[vecNumber].evaluatePath(start, obs, false);

        if(newTarget || zeroScore > tempScore + swapPathCost) {
            controller[vecNumber] = controller[0];
            viaPoints[vecNumber].swap(viaPoints[0]);
            pathPoints[vecNumber] = controller[vecNumber].getFullGeoPathList();
        }
        double nextCurva = 0.0;
        nextState[vecNumber] = controller[vecNumber].getNextState(start, nextCurva);
    }

    drawPath(drawPathPoints, drawPathLine, drawViaPoints, vecNumber);
    if(openDebug) {
        GDebugEngine::Instance()->gui_debug_x(nextState[vecNumber].pos);
        GDebugEngine::Instance()->gui_debug_arc(me.Pos(), avoidDist, 0, 360, COLOR_RED);
    }

    CControlModel control;
    PlayerPoseT final;
    final.SetPos(nextState[vecNumber].pos);
    if(nextState[vecNumber].pos.dist(target.pos) > 1e-8 && !isBack) {
        final.SetDir(Utils::Normalize((nextState[vecNumber].pos - me.Pos()).dir()));
    }
    else {
        final.SetDir(targetDir);
    }
    final.SetVel(nextState[vecNumber].vel);
    final.SetRotVel(0.0);
    control.makeCmTrajectory(me, final, capability);

    // CVector globalVel = nextFrame.vel;
    CVector globalVel = control.getNextStep().Vel();
    if(openDebug) {
        GDebugEngine::Instance()->gui_debug_line(nextState[vecNumber].pos, nextState[vecNumber].pos + nextState[vecNumber].vel * 1000, COLOR_PURPLE);
        GDebugEngine::Instance()->gui_debug_line(me.Pos(), me.Pos() + globalVel * 1000, COLOR_BLUE);
    }

    int priority = 0;
    double usedtime = controller[vecNumber].getTime();
    if ((playerFlag & PlayerStatus::ALLOW_DSS)) {
        CVector tempVel = DynamicSafetySearch::Instance()->SafetySearch(vecNumber, globalVel, pVision, priority, target.pos, task().player.flag, usedtime, task().player.max_acceleration);
        if (WorldModel::Instance()->CurrentRefereeMsg() == "GameStop" && tempVel.mod() > 150) { // 不加这个在stop的时候车可能会冲出去
        } else {
//            globalVel = tempVel;
        }
    }

    double alpha = 1.0;
    if (dist <= DIST_REACH_CRITICAL) {
        alpha *= sqrt(dist / DIST_REACH_CRITICAL);
    }
    CVector localVel = (globalVel * alpha).rotate(-me.Dir());
    GDebugEngine::Instance()->gui_debug_msg(me.Pos(), QString("%1").arg(localVel.mod()).toLatin1());
    double rotVel = control.getNextStep().RotVel();
    if ((fabs(Utils::Normalize(target.orient - start.orient)) <= PARAM::Math::PI * 10.0 / 180)) {
        rotVel /= 2;
    }
    delete obs;

    const bool dribble = playerFlag & PlayerStatus::DRIBBLING;
    unsigned char dribblePower = DRIBBLE_DISABLED;

    if (dribble || task().player.needdribble) {
        dribblePower = DRIBBLE_NORAML;
    }
//    cout << "vel  " << localVel.mod() << endl;
//    return pCmdFactory->newCommand(CPlayerSpeedV2(vecNumber, localVel.x(), localVel.y(), rotVel, dribblePower));
        return pCmdFactory->newCommand(CPlayerSpeedV2(vecNumber, localVel.x(), localVel.y(), rotVel, task().player.use_dir, dribblePower));

//    return pCmdFactory->newCommand(CPlayerSpeedV2(vecNumber, 0, 0, 0, dribblePower));
}

PlayerCapabilityT CGotoPositionNew::setCapability(const CVisionModule* pVision) {
    const int vecNumber = task().executor;
    const int playerFlag = task().player.flag;
    PlayerCapabilityT capability;

    // Traslation
    if (vecNumber == TaskMediator::Instance()->goalie()) {
        capability.maxSpeed = MAX_TRANSLATION_SPEED_GOALIE;
        capability.maxAccel = MAX_TRANSLATION_ACC_GOALIE;
        capability.maxDec = MAX_TRANSLATION_DEC_GOALIE;
        // Rotation	  确定转动运动参数
        capability.maxAngularSpeed = MAX_ROTATION_SPEED_GOALIE;
        capability.maxAngularAccel = MAX_ROTATION_ACC_GOALIE;
        capability.maxAngularDec = MAX_ROTATION_ACC_GOALIE;
    } else if (TaskMediator::Instance()->leftBack() != 0 && vecNumber == TaskMediator::Instance()->leftBack()
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
    } else {
        capability.maxSpeed = MAX_TRANSLATION_SPEED;
        capability.maxAccel = MAX_TRANSLATION_ACC;
        capability.maxDec = MAX_TRANSLATION_DEC;
        // Rotation	  确定转动运动参数
        capability.maxAngularSpeed = MAX_ROTATION_SPEED;
        capability.maxAngularAccel = MAX_ROTATION_ACC;
        capability.maxAngularDec = MAX_ROTATION_ACC;
    }


    if (task().player.max_acceleration > 1) {
        capability.maxAccel = task().player.max_acceleration > TRANSLATION_ACC_LIMIT ? TRANSLATION_ACC_LIMIT : task().player.max_acceleration;
        capability.maxDec = capability.maxAccel;
    }
    if (WorldModel::Instance()->CurrentRefereeMsg() == "gameStop") {
        const MobileVisionT ball = pVision->ball();
        if (ball.Pos().x() < -240 && abs(ball.Pos().y()) > 150) {
            capability.maxSpeed = 100;
        } else {
            capability.maxSpeed = 150;
        }
    }
    return capability;
}

void CGotoPositionNew::validateFinalTarget(CGeoPoint &finalTarget, CGeoPoint myPos, double avoidLength, bool isGoalie, bool isBack, ObstaclesNew& obs) {
    if(!(playerFlag & PlayerStatus::NOT_AVOID_PENALTY)) {
        finalTarget = Utils::MakeInField(finalTarget, -avoidLength * 2);
        if(!isGoalie && Utils::InOurPenaltyArea(finalTarget, avoidLength))
            finalTarget = Utils::MakeOutOfOurPenaltyArea(finalTarget, avoidLength);
        if(Utils::InTheirPenaltyArea(finalTarget, avoidLength))
            finalTarget = Utils::MakeOutOfTheirPenaltyArea(finalTarget, avoidLength);
    }

    int obsNum = obs.getNum();

    for(int i = 0; i < obsNum; i++) {
        if(obs.obs[i].getType() != OBS_RECTANGLE_NEW && !obs.obs[i].check(finalTarget)) {
            if(obs.obs[i].getType() == OBS_CIRCLE_NEW) {
                if(finalTarget.dist(myPos) > (avoidLength + obs.obs[i].getRadius()) * 1.2) {
                    finalTarget = Utils::MakeOutOfCircle(obs.obs[i].getStart(), obs.obs[i].getRadius(), finalTarget, avoidLength, isBack, myPos);
                } else {
                    finalTarget = Utils::MakeOutOfCircle(obs.obs[i].getStart(), PARAM::Vehicle::V2::PLAYER_FRONT_TO_CENTER, finalTarget, PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER, isBack, myPos);
                }
            }
            else {
                if(finalTarget.dist(myPos) > (avoidLength + obs.obs[i].getRadius()) * 1.2){
                    finalTarget = Utils::MakeOutOfLongCircle(obs.obs[i].getStart(), obs.obs[i].getEnd(), obs.obs[i].getRadius(), finalTarget, avoidLength);
                } else {
                    finalTarget = Utils::MakeOutOfLongCircle(obs.obs[i].getStart(), obs.obs[i].getEnd(), PARAM::Vehicle::V2::PLAYER_FRONT_TO_CENTER, finalTarget, PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER);
                }
            }
        }
    }
}

bool CGotoPositionNew::validateStartPoint(CGeoPoint &startPos, const PlayerVisionT &me, ObstaclesNew *obst, const PlayerCapabilityT& capability, double avoidLength, bool isGoalie, bool isBack) {

    CGeoPoint myPos = me.Pos();
    if(WorldModel::Instance()->CurrentRefereeMsg() != "OurPenaltyKick" && WorldModel::Instance()->CurrentRefereeMsg() != "OurBallPlacement") {
        if(!isGoalie && Utils::InOurPenaltyArea(startPos, avoidLength))
            startPos = Utils::MakeOutOfOurPenaltyArea(startPos, avoidLength);
        if(Utils::InTheirPenaltyArea(startPos, avoidLength))
            startPos = Utils::MakeOutOfTheirPenaltyArea(startPos, avoidLength);
    }

    int obsNum = obst->getNum();
    for(int i = 0; i < obsNum; i++) {
        if(obst->obs[i].getType() != OBS_RECTANGLE_NEW && !obst->obs[i].check(startPos)) {
            if(obst->obs[i].getType() == OBS_CIRCLE_NEW) {
                if(startPos.dist(myPos) > (avoidLength + obst->obs[i].getRadius()) * 1.2) {
                    startPos = Utils::MakeOutOfCircle(obst->obs[i].getStart(), obst->obs[i].getRadius(),  startPos, avoidLength);
                } else {
                    startPos = Utils::MakeOutOfCircle(obst->obs[i].getStart(), PARAM::Vehicle::V2::PLAYER_FRONT_TO_CENTER, startPos, PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER);
                }
            }
            else {
                if(startPos.dist(myPos) > (avoidLength + obst->obs[i].getRadius()) * 1.2){
                    startPos = Utils::MakeOutOfLongCircle(obst->obs[i].getStart(), obst->obs[i].getEnd(), obst->obs[i].getRadius(), startPos, avoidLength);
                } else {
                    startPos = Utils::MakeOutOfLongCircle(obst->obs[i].getStart(), obst->obs[i].getEnd(), PARAM::Vehicle::V2::PLAYER_FRONT_TO_CENTER, startPos, PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER);
                }
            }
        }
    }
    if(me.Vel().mod() < 20 || isBack) return true;
    CVector meVel = me.Vel();
    double distToStop = pow(meVel.mod(), 2) / (2 * capability.maxDec);
    if(me.Vel().mod() > 250)
        distToStop *= 2.0;
    CVector adjustDir = meVel / meVel.mod();
    CGeoPoint startPosNew = startPos + adjustDir * distToStop ;

    if(!obst->check(startPos, startPosNew)) return false;
    if(me.Vel().mod() > minVelConstr)
        startPos = startPosNew;
    return true;
}

vector < CGeoPoint > CGotoPositionNew::forBack(const CVisionModule* _pVision, const int vecNum, const CGeoPoint &startPos, const CGeoPoint &targetPos) {
    vector < CGeoPoint > result;
    result.clear();
    CGeoPoint anotherBack;
    CGeoPoint mePos = _pVision->ourPlayer(vecNum).Pos();
    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        if(i != vecNum && (i == TaskMediator::Instance()->leftBack() ||
            i == TaskMediator::Instance()->rightBack()) &&
                _pVision->ourPlayer(i).Valid()) {
            anotherBack = _pVision->ourPlayer(i).Pos();
            break;
        }
    }
    CVector me2another = anotherBack - mePos;
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0.0, 0.0), QString("%1").arg(me2another.mod()).toLatin1(), COLOR_GREEN);
    CGeoPoint candidate1(-455, -135), candidate2(-455, 135),
            candidateOut1(-450, -160), candidateOut2(-450, 160);
    if(startPos.x() < -PARAM::Field::PITCH_LENGTH / 2 + PARAM::Field::PENALTY_AREA_DEPTH + PARAM::Vehicle::V2::PLAYER_SIZE &&
            targetPos.x() < -PARAM::Field::PITCH_LENGTH / 2 + PARAM::Field::PENALTY_AREA_DEPTH + PARAM::Vehicle::V2::PLAYER_SIZE) {
        if(startPos.y() * targetPos.y() < 0.0) {
            if(startPos.y() < 0) {
                if(fabs(Utils::Normalize(me2another.dir() - (candidate1 - mePos).dir())) > PARAM::Math::PI / 2 || me2another.mod() > 4.0 * PARAM::Vehicle::V2::PLAYER_SIZE) {
                    result.push_back(candidate1);
                    result.push_back(candidate2);
                } else {
                    result.push_back(CGeoPoint(startPos.x(), -160));
                    result.push_back(candidateOut1);
                    result.push_back(candidateOut2);
                    result.push_back(CGeoPoint(targetPos.x(), 160));
                }
            } else {
                if(fabs(Utils::Normalize(me2another.dir() - (candidate2 - mePos).dir())) > PARAM::Math::PI / 2 || me2another.mod() > 4.0 * PARAM::Vehicle::V2::PLAYER_SIZE) {
                    result.push_back(candidate2);
                    result.push_back(candidate1);
                } else {
                    result.push_back(CGeoPoint(startPos.x(), 160));
                    result.push_back(candidateOut2);
                    result.push_back(candidateOut1);
                    result.push_back(CGeoPoint(targetPos.x(), -160));
                }
            }
        }
    }
    else if(!(startPos.x() > -PARAM::Field::PITCH_LENGTH / 2 + PARAM::Field::PENALTY_AREA_DEPTH + PARAM::Vehicle::V2::PLAYER_SIZE &&
                targetPos.x() > -PARAM::Field::PITCH_LENGTH / 2 + PARAM::Field::PENALTY_AREA_DEPTH + PARAM::Vehicle::V2::PLAYER_SIZE)) {
        if(startPos.x() < -PARAM::Field::PITCH_LENGTH / 2 + PARAM::Field::PENALTY_AREA_DEPTH + PARAM::Vehicle::V2::PLAYER_SIZE) {
            if(startPos.y() > 0) {
                if(fabs(Utils::Normalize(me2another.dir() - (candidate2 - mePos).dir())) > PARAM::Math::PI / 2 || me2another.mod() > 4.0 * PARAM::Vehicle::V2::PLAYER_SIZE)
                    result.push_back(candidate2);
                else {
                    result.push_back(CGeoPoint(startPos.x(), 160));
                    result.push_back(candidateOut2);
                    result.push_back(CGeoPoint(-PARAM::Field::PITCH_LENGTH / 2 + 160, targetPos.y()));
                }
            }
            else {
                if(fabs(Utils::Normalize(me2another.dir() - (candidate1 - mePos).dir())) > PARAM::Math::PI / 2 || me2another.mod() > 4.0 * PARAM::Vehicle::V2::PLAYER_SIZE)
                    result.push_back(candidate1);
                else {
                    result.push_back(CGeoPoint(startPos.x(), -160));
                    result.push_back(candidateOut1);
                    result.push_back(CGeoPoint(-PARAM::Field::PITCH_LENGTH / 2 + 160, targetPos.y()));
                }
            }
        } else {
            if(targetPos.y() > 0) {
                if(fabs(Utils::Normalize(me2another.dir() - (candidate2 - mePos).dir())) > PARAM::Math::PI / 2 || me2another.mod() > 4.0 * PARAM::Vehicle::V2::PLAYER_SIZE)
                    result.push_back(candidate2);
                else {
                    result.push_back(CGeoPoint(-PARAM::Field::PITCH_LENGTH / 2 + 160, startPos.y()));
                    result.push_back(candidateOut2);
                    result.push_back(CGeoPoint(targetPos.x(), 160));
                }
            }
            else {
                if(fabs(Utils::Normalize(me2another.dir() - (candidate1 - mePos).dir())) > PARAM::Math::PI / 2 || me2another.mod() > 4.0 * PARAM::Vehicle::V2::PLAYER_SIZE)
                    result.push_back(candidate1);
                else {
                    result.push_back(CGeoPoint(-PARAM::Field::PITCH_LENGTH / 2 + 160, startPos.y()));
                    result.push_back(candidateOut1);
                    result.push_back(CGeoPoint(targetPos.x(), -160));
                }
            }
        }
    }
    return result;
}

void CGotoPositionNew::drawPath(bool drawPathPoints, bool drawPathLine, bool drawViaPoints, int vecNumber) {
    int i = 0;
    for(i = 0; drawPathLine && !pathPoints[vecNumber].empty() && i < pathPoints[vecNumber].size() - 1; i++) {
        GDebugEngine::Instance()->gui_debug_line(pathPoints[vecNumber][i], pathPoints[vecNumber][i + 1], COLOR_YELLOW);
    }

    for(i = 0; drawPathPoints && i < pathPoints[vecNumber].size() - 1; i++) {
        GDebugEngine::Instance()->gui_debug_x(pathPoints[vecNumber][i], COLOR_YELLOW);
    }
    for(i = 0; drawViaPoints && i < viaPoints[vecNumber].size(); i++) {
        GDebugEngine::Instance()->gui_debug_x(viaPoints[vecNumber][i], COLOR_GREEN);
    }
}


