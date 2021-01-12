#include "PenaltyDef2017V2.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include "WorldModel.h"
#include "BestPlayer.h"
#include "GoaliePosV1.h"
#include "KickStatus.h"
#include "AdvanceBallV2.h"
#include "ChipBallJudge.h"
#include "BallSpeedModel.h"
#include "SkillUtils.h"
namespace {
	const bool debug = false;

	CGeoPoint ourGate;
	CGeoPoint ourGateLeft;
	CGeoPoint ourGateRight;

	const double MAX_ACC = 500;
	const double MAX_RUSH_DIST = 200;
	const double goalBuffer = 5;
}

CPenaltyDef2017V2::CPenaltyDef2017V2()
{
	ourGate = CGeoPoint(-PARAM::Field::PITCH_LENGTH / 2, 0);
	ourGateLeft = CGeoPoint(-PARAM::Field::PITCH_LENGTH / 2, -(PARAM::Field::GOAL_WIDTH / 2 - goalBuffer));
	ourGateRight = CGeoPoint(-PARAM::Field::PITCH_LENGTH / 2, PARAM::Field::GOAL_WIDTH / 2 - goalBuffer);
	_lastCycle = 0;
	_rushFlag = 0;
	_turnFlag = 0;
}

void CPenaltyDef2017V2::plan(const CVisionModule* pVision)
{
	if (pVision->getCycle() - _lastCycle > PARAM::Vision::FRAME_RATE * 0.1)
	{
		_rushFlag = 0;
		_turnFlag = 0;
	}

	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->ourPlayer(robotNum);
	const MobileVisionT& ball = pVision->ball();
	const int enemyNum = ZSkillUtils::instance()->getTheirBestPlayer();
	const PlayerVisionT& enemy = pVision->theirPlayer(enemyNum);
	TaskT myTask(task());

	_rushFlag = (ourGate - me.Pos()).mod() > MAX_RUSH_DIST ? 1 : 0;

	double compenDist = 0;
	CGeoLine enemyDirLine(enemy.Pos(), enemy.Dir());
	CGeoLine gateLine(CGeoPoint(-PARAM::Field::PITCH_LENGTH / 2, 0), PARAM::Math::PI / 2);
	CGeoLineLineIntersection gateInter(enemyDirLine, gateLine);
	CVector enemy2Ball(ball.Pos().x() - enemy.Pos().x(), ball.Pos().y() - enemy.Pos().y());
	CGeoLine gate2BallLine;
	if (fabs(gateInter.IntersectPoint().y()) < PARAM::Field::GOAL_WIDTH / 2 + goalBuffer) {
		_case = 1;
		gate2BallLine = CGeoLine(ball.Pos(), enemy.Dir());
	}
	else {
		_case = 2;
		compenDist = -(fabs(enemy.Pos().y()) < 36 ? fabs(enemy.Pos().y()) : 36) * gateInter.IntersectPoint().y() / fabs(gateInter.IntersectPoint().y());
		gate2BallLine = CGeoLine(ball.Pos(), ourGate + Utils::Polar2Vector(compenDist, -PARAM::Math::PI / 2));
	}

	CGeoLineLineIntersection gate2BallInter(gate2BallLine, gateLine);
	if (fabs(gate2BallInter.IntersectPoint().y()) > 180) gate2BallLine = CGeoLine(enemy.Pos(), ourGate);
	CGeoCirlce gateCircle(ourGate, MAX_RUSH_DIST);
	CGeoLineCircleIntersection taskInter(gate2BallLine, gateCircle);
	myTask.player.pos = taskInter.point1().x() > taskInter.point2().x() ? taskInter.point1() : taskInter.point2();
	myTask.player.angle = (ball.Pos() - me.Pos()).dir();

	if (isBallShoot(pVision) && fabs(ball.Vel().dir() - (me.Pos() - ball.Pos()).dir()) < 0.05) {
		CGeoPoint taskPoint = taskInter.point1().x() > -PARAM::Field::PITCH_LENGTH / 2 ? taskInter.point1() : taskInter.point2();
		myTask.player.pos = ball.Vel().dir() - (me.Pos() - ball.Pos()).dir() > 0 ? CGeoPoint(me.Pos().x(), 9999) : CGeoPoint(me.Pos().x(), -9999);
	}
	if ((ball.Pos() - ourGate).mod() + 20 < (me.Pos() - ball.Pos()).mod()) {
		myTask.player.pos = enemy.Pos() + Utils::Polar2Vector(PARAM::Vehicle::V2::PLAYER_SIZE * 3, enemy2Ball.dir());
	}
	KickStatus::Instance()->setKick(robotNum, 9999);
	setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));
	if (debug) cout << _case << gate2BallInter.IntersectPoint() << myTask.player.pos << endl;

	CStatedTask::plan(pVision);
	_lastCycle = pVision->getCycle();
}

CPlayerCommand* CPenaltyDef2017V2::execute(const CVisionModule* pVision)
{
	if (subTask()) {
		return subTask()->execute(pVision);
	}
	return NULL;
}

bool CPenaltyDef2017V2::isBallShoot(const CVisionModule * pVision)
{
	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->ourPlayer(robotNum);
	const MobileVisionT& ball = pVision->ball();
	const int enemyNum = ZSkillUtils::instance()->getTheirBestPlayer();
	const PlayerVisionT& enemy = pVision->theirPlayer(enemyNum);

	CVector enemy2Ball(ball.Pos().x() - enemy.Pos().x(), ball.Pos().y() - enemy.Pos().y());
	bool ballDirFrontOpp = (ball.Vel().dir() - enemy2Ball.dir()) < PARAM::Math::PI; //角度条件
	bool ballDistFrontOpp = (enemy2Ball.mod() > 25); //距离条件
	CGeoLine enemyFaceLine(enemy.Pos(), enemy.Dir());
	CGeoLine ballVelLine(ball.Pos(), ball.Vel().dir());
	CGeoLine goalLine(CGeoPoint(PARAM::Field::PITCH_LENGTH / 2, 0), PARAM::Math::PI / 2);
	CGeoLineLineIntersection taskInter(enemyFaceLine, goalLine);
	CGeoLineLineIntersection ballVelInter(ballVelLine, goalLine);
	bool isFaceOurDoor = fabs(taskInter.IntersectPoint().y()) < PARAM::Field::GOAL_WIDTH / 2 - goalBuffer; //脸朝向门内
	bool isVelEnough = ball.Vel().mod() > 300 && ball.Vel().x() < 0; //速度条件

	if (debug) cout << "dist:" << ballDistFrontOpp << " dir:" << ballDirFrontOpp << " vel:" << (ball.Vel().mod() > 150) 
		<< " face:" << isFaceOurDoor << " " << (ballDistFrontOpp && ballDirFrontOpp && isVelEnough) 
		<< " " << (fabs(ball.Vel().dir() - (me.Pos() - ball.Pos()).dir()) < 0.05) << " " << ball.Vel().mod() << " ";

	return ballDistFrontOpp && ballDirFrontOpp && isVelEnough;
}
