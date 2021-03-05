#include "SlowGetBall.h"
#include "GDebugEngine.h"
#include <QDebug>
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <DribbleStatus.h>
#include <RobotSensor.h>
#include <CommandFactory.h>
#include <KickStatus.h>
using namespace std;
namespace{
	enum slow_get_ball_state
	{
		S_GETBALL = 1,
		S_GOTOWARD,
		S_RUSH,
		S_STOP
	};

	//>需要用到的常量
    const double newVehicleBuffer = 6;              // 小嘴巴机器人PLAYER_FRONT_TO_CENTER补偿
	const int dribblePower =3;// 1;                        // 控球力度
    const double low_speed = 100;//100                       // 最后上前的小速度
    const double stopDist = 30; //30
    const double getBallOverDist = PARAM::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer + PARAM::Field::BALL_SIZE + stopDist - 25;
	const int steadyCycle = 10;                         // 确定控制住球的帧数
	static CGeoPoint startPoint = CGeoPoint(0,0);
	//>开关量
    bool Verbose = true;                              // 调试模式

	//用到的静态变量
    bool trueBallCatched = false;
	int catchBallCount = 0;
	int actionCount = 0;
	int cntJumpOutGoTowards = 0;//从GO_TOWARD状态跳出去的计数器
}

CSlowGetBall::CSlowGetBall()
{
	_lastCycle = 0;
}

void CSlowGetBall::plan(const CVisionModule* pVision)
{
	//第一次进入动作输出
	static bool isIn = true;
	if (Verbose)
	{
		if (isIn == true)
		{
			setState(BEGINNING);
			//cout << endl << "get into SLOW_GET_BALL !" << endl;
			isIn = false;
		}
	}
	//动作中断时间过长，状态重置
	if ( pVision->getCycle() - _lastCycle > PARAM::Vision::FRAME_RATE * 0.1 ){
		//cout << "Clear: get into SLOW_GET_BALL !" << endl;
		setState(BEGINNING);
		catchBallCount = 0;
		trueBallCatched = false;
		actionCount = 0;
	}
	KickStatus::Instance()->clearAll();
	//视觉初步处理
	const MobileVisionT& ball = pVision->ball();
	const int robotNum = task().executor;
    const CGeoPoint target = task().player.pos;
    int kick_flag = task().player.kick_flag;
	const int flags = task().player.flag;
	const PlayerVisionT& me = pVision->ourPlayer(robotNum);
	const CGeoPoint myhead = me.Pos()+Utils::Polar2Vector(PARAM::Vehicle::V2::PLAYER_FRONT_TO_CENTER + newVehicleBuffer,me.Dir());
	const double finalDir = task().player.angle;
	const CGeoLine myheadLine = CGeoLine(myhead,Utils::Normalize(me.Dir() + PARAM::Math::PI/2.0));
	const CGeoPoint ball2myheadLine_ProjPoint = myheadLine.projection(ball.Pos());
	
	//状态判断模块
    bool isBallFrontOfMyhead = myhead.dist(ball2myheadLine_ProjPoint) < 35.0 ? true : false;                            //2.0
	bool dirReached = fabs(Utils::Normalize(finalDir - me.Dir())) < PARAM::Math::PI*7/180.0 ? true : false;  //PARAM::Math::PI*5/180.0
    bool speedReached = me.RawVel().mod() < 100.0 ? true : false;                                                           //5.0
    bool isInfraredOn = RobotSensor::Instance()->IsInfraredOn(robotNum);                              //getBallOverDist + 2.0
    bool getballComplete = isBallFrontOfMyhead && dirReached && speedReached && isInfraredOn;   //判断是否跳转到S_GOTOWARD状态
	bool fraredOn = false;
	bool ballControled = false;
	if (RobotSensor::Instance()->IsInfraredOn(robotNum))
	{
		fraredOn = true;
	}	
	if (RobotSensor::Instance()->isBallControled(robotNum))
	{
		ballControled = true;
	}
	bool ballCatched = fraredOn /*&& ballControled*/;
	if (S_GOTOWARD == getState())                    // 当判断ballCatched若干帧后才认定ballCatched
	{
		if (ballCatched)
		{
			catchBallCount++;
		} else catchBallCount = 0;
		if (catchBallCount > steadyCycle)
		{
			trueBallCatched = true;
		}
	}
	static int fraredLongOff = 0;
	if (!fraredOn)
	{
		fraredLongOff++;
	} else fraredLongOff = 0;

	//调试输出
    if (Verbose)
	{
        cout << "check kick status : !!!!!!!!!!! " << (flags & PlayerStatus::FORCE_KICK) << endl;
        //cout<<isBallFrontOfMyhead<<dirReached<<speedReached<<isInfraredOn<<"   getBallComplete: "<<getballComplete<<endl;
		//cout<<fraredOn<<ballControled<<"   ballCatched: "<<ballCatched<<"   catchBallCount : "<<catchBallCount<<"   trueCatchBall: "<<trueBallCatched<<endl<<endl;
	}

	//状态机管理模块
	if (BEGINNING == getState())
	{
		setState(S_GETBALL);
		actionCount = 0;
	}
	else if (S_GETBALL == getState())
	{
        if (getballComplete /*|| isBallFrontOfMyhead && actionCount > 300*/)//************TODO解决不上前问题，此处先强硬处理
		{
			setState(S_GOTOWARD);
			cntJumpOutGoTowards = 0;
			actionCount = 0;
			startPoint = ball.Pos();
		} else if (actionCount > 60)
		{
			setState(BEGINNING);
			actionCount = 0;
		}
    }
	else if (S_GOTOWARD == getState())
	{
		//cout<<trueBallCatched<<"  "<<startPoint.dist(me.Pos())<<endl;
		if (trueBallCatched) //|| startPoint.dist(me.Pos()) < 3)//6 //没有上传的时候 用视觉补偿
		{
			//if(!trueBallCatched){
			//	cout<<"use!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
			//}
			setState(S_STOP);
			actionCount = 0;
			catchBallCount = 0;
			trueBallCatched = false;

		} else{
			if (actionCount > 150 && !fraredOn || !dirReached || !isBallFrontOfMyhead)//如果在向前进途中，折回拿球状态，注意与actionCount所比较的值有没有问题
			{
				if (++cntJumpOutGoTowards > 5)
				{				
					setState(BEGINNING);
					actionCount = 0;
				}
			}else{
				cntJumpOutGoTowards = 0;
			}
		} 
	} else if (S_STOP == getState())
	{
		/*if (startPoint.dist(ball.Pos()) > 5)
		{
			setState(S_RUSH);
			actionCount = 0;
		}
        else*/ if (me.Pos().dist(ball.Pos()) > 500 /* || fraredLongOff > 30*/)
		{
			setState(BEGINNING);
			actionCount = 0;
			catchBallCount = 0;
			trueBallCatched = false;
		} 
	} else if (S_RUSH == getState())
	{
        if (me.Pos().dist(ball.Pos()) > 500)
		{
			setState(BEGINNING);
			actionCount = 0;
			catchBallCount = 0;
			trueBallCatched = false;
		}
	}
	if (Verbose)
	{
		if (S_GETBALL == getState())
		{
			cout<<"S_GETBALL"<<endl;
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1700, -1500), QString("S_GETBALL dist speed dir reached: %1 %2 %3 dist2ball: %4").arg(isInfraredOn).arg(speedReached).arg(dirReached).arg(me.Pos().dist(ball.Pos())).toLatin1(),COLOR_CYAN);
		} else if (S_GOTOWARD == getState())
		{
			cout<<"S_GOTOWARD"<<endl;
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1700, -1500), QString("S_GOTOWARD IS TRUEBALLCATCHED: %1 CatchBall Cnt: %2").arg(trueBallCatched).arg(catchBallCount).toLatin1(),COLOR_CYAN);
		} else if (S_STOP == getState())
		{
			cout<<"S_STOP"<<endl;
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1700, -1500), "S_STOP",COLOR_CYAN);
		} else if (S_RUSH == getState())
		{
			cout<<"S_RUSH"<<endl;
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1700, -1500), "S_RUSH",COLOR_CYAN);
		}
	}
	//状态执行
	int myState = getState();
	double x_speed = low_speed * cos(finalDir);
	double y_speed = low_speed * sin(finalDir);
    // ad trick in sydney
    if(RobotSensor::Instance()->fraredOn(robotNum)>5)
        x_speed = y_speed = 0;

	//开吸球
    DribbleStatus::Instance()->setDribbleCommand(robotNum,3);
	KickStatus::Instance()->setBothKick(robotNum,0,0);
    QString state_output = "null state";
	switch (myState)
	{
	case S_GETBALL:
		actionCount++;
        kick_flag = kick_flag | PlayerStatus::NOT_AVOID_PENALTY | PlayerStatus::DRIBBLING;
        state_output = "S_GETBALL";
        setSubTask(PlayerRole::makeItGetBallV4(robotNum,kick_flag, me.Pos()+Utils::Polar2Vector(300,finalDir),CGeoPoint(99999,99999),0));
//        setSubTask(PlayerRole::makeItNoneTrajGetBall(robotNum,finalDir,CVector(0,0),flags|PlayerStatus::NOT_AVOID_PENALTY,stopDist, CMU_TRAJ));
        break;
	case S_GOTOWARD:
		actionCount++;
        state_output = "S_GOTOWARD";
        setSubTask(PlayerRole::makeItRun(robotNum,x_speed,y_speed,0,PlayerStatus::DRIBBLING|PlayerStatus::NOT_AVOID_PENALTY));
		break;
	case S_STOP:
        state_output = "S_STOP";
        setSubTask(PlayerRole::makeItRun(robotNum,0.0,0.0,0.0,PlayerStatus::DRIBBLING|PlayerStatus::NOT_AVOID_PENALTY));
		break;
	case S_RUSH:
        state_output = "S_RUSH";
        setSubTask(PlayerRole::makeItNoneTrajGetBall(robotNum,finalDir,CVector(0,0),flags|PlayerStatus::NOT_AVOID_PENALTY,-2));
		break;
	default: break;
	}
    if(Verbose){
        GDebugEngine::Instance()->gui_debug_msg(me.Pos()+CVector(200,200),state_output.toLatin1(),10,255255255,300);
    }
	//限制actionCount
	if (actionCount > 500)
	{
		actionCount = 500;
	}

	_lastCycle = pVision->getCycle();
	CStatedTask::plan(pVision);
}

CPlayerCommand* CSlowGetBall::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}	
	return NULL;
}
