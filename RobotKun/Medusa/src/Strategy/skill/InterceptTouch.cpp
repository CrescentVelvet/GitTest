#include "InterceptTouch.h"

#include <cmath>
#include <iostream>
#include <string>

#include "geometry.h"
#include "staticparams.h"
#include "utils.h"

#include "DribbleStatus.h"
#include "Factory.h"
#include "GDebugEngine.h"
#include "VisionModule.h"
#include "KickStatus.h"
#include "KickDirection.h"
#include "parammanager.h"
using namespace std;
using namespace PARAM;

namespace {

namespace Threshold {

const double MIN_DIFF_DIR = Math::PI / 18;  // 小于这个值就算方向正确
const double MIN_P2L_DIST = 5*10;              // 小于这个值就算到点
const double MAX_P2L_DIST = 10*10;             // 大于这个值就算没到点
const double MIN_INTERCEPT_DIST = 300*10;      // 能截到的最大横向距离 小于这个值就能截到
const double MAX_NEED_QUICK_RUN_ME_TO_TARGET_DIST = 20*10;    // 停止快速到点的车到目标点距离阈值 大于这个值就需要
const double MIN_BALL_MOVING_VEL = 40*10;      // 球速小于这个值就算不在滚

const int MAX_BUFFER_COUNTER_THRESHOLD = 2;
const int MIN_DRIBBLE_DIST_THRESHOLD = 30*10;  // 开吸球的距离阈值 小于这个值就开吸球
const int SAFE_DISTANCE = 30*10;
}

static CGeoPoint waitPos;
int FRICTION;
}

CInterceptTouch::CInterceptTouch() : buffer_counter_(0), last_cycle_(0) {
    bool IS_SIMULATION;
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION,"Alert/IsSimulation",false);
    if (IS_SIMULATION)
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Sim",800);
    else
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Real",1520);
}

void CInterceptTouch::plan(const CVisionModule* pVision) {
  using namespace Threshold;

	if ( pVision->getCycle() - last_cycle_ > 6 )
		setState(BEGINNING);

  const int runner = task().executor;
  if (task().player.pos.x() > 9990 && task().player.pos.y() >= 9990) {
    if (state() == BEGINNING)
      waitPos = pVision->ourPlayer(runner).RawPos();
  } else {
    waitPos = task().player.pos;
  }
  double touchDir = task().player.angle;
  const double power = task().player.kickpower;
  const double bufferDist = task().player.chipkickpower;
  const bool useChip = task().player.ischipkick;
  const bool testMode = task().ball.front;
  const CGeoPoint target = CGeoPoint(task().player.speed_x, task().player.speed_y);//add by Wang in 2018/06/18
  const PlayerVisionT& me = pVision->ourPlayer(runner);
  const MobileVisionT& ball = pVision->ball();
  const CGeoPoint& myPos = pVision->ourPlayer(runner).Pos();
  const CGeoPoint& ballPos = ball.RawPos();
  const double ballVelDir = ball.Vel().dir();
  
  // 判断球是否向我而来
  const double ballVelMod = ball.Vel().mod();
  const double meToBallDir = (myPos - ballPos).dir();
  const double diffBetweenBallVelDirAndMeToBallDir = fabs(Utils::Normalize(ballVelDir - meToBallDir));
  //cout << ' ' << ballVelDir << ' ' <<  meToBallDir << ' ' << diffBetweenBallVelDirAndMeToBallDir << endl;
  bool isBallMovingToMe = false;
  if (ballVelMod > MIN_BALL_MOVING_VEL && diffBetweenBallVelDirAndMeToBallDir < Math::PI / 2)
    isBallMovingToMe = true;

  // 判断是否能截到球
  const CGeoLine ballVelLine(ballPos, ballVelDir);
  const double perpendicularDir = Utils::Normalize(ballVelDir + Math::PI / 2);
  const CGeoLine perpLineAcrossMyPos(waitPos, perpendicularDir);
  const CGeoPoint projectionPos = CGeoLineLineIntersection(ballVelLine, perpLineAcrossMyPos).IntersectPoint();
  const double meToBallVelLineDist = projectionPos.dist(myPos);

//  auto ballDir = ball.Vel().dir();
//  auto rawBallPos = ball.RawPos();
  const double distMe2Ball = (me.Pos() - ball.RawPos()).mod();
  bool canInterceptBall = false;
  if (ball.Valid() && isBallMovingToMe) {
    if (projectionPos.x() > -Field::PITCH_LENGTH / 2 && projectionPos.x() < Field::PITCH_LENGTH / 2 &&
        projectionPos.y() > -Field::PITCH_WIDTH / 2  && projectionPos.y() < Field::PITCH_WIDTH / 2) {
      if (meToBallVelLineDist <= MIN_INTERCEPT_DIST)
        canInterceptBall = true;
    }
  }

  const double instancePrediction = ballVelMod*ballVelMod / FRICTION;
  bool ballArriveMe = instancePrediction - distMe2Ball > SAFE_DISTANCE && isBallMovingToMe;
  // 判断状态跳转
  switch (state()) {
    case BEGINNING: {
      if (canInterceptBall){
        //cout << "can InterceptBall" << endl;
        setState(GOTO_PROJ_POS);
      }
      else{
        //cout << "can not intercept" << endl;
        setState(SIMPLY_GOTO_WAIT_POS);
      }
    } break;

    case SIMPLY_GOTO_WAIT_POS: {
      if (canInterceptBall)
        setState(GOTO_PROJ_POS);
    } break;

    case GOTO_PROJ_POS: {
      if (canInterceptBall == false) {
        setState(SIMPLY_GOTO_WAIT_POS);
        buffer_counter_ = 0;
      } else {
        if (meToBallVelLineDist < MIN_P2L_DIST)
          ++buffer_counter_;
        else
          buffer_counter_ = 0;
        if (buffer_counter_ == MAX_BUFFER_COUNTER_THRESHOLD) {
          setState(INTERCEPT);
          buffer_counter_ = 0;
        }
      }
    } break;
	case SAVE: {
        if (ballArriveMe)
            if(canInterceptBall)
                setState(GOTO_PROJ_POS);
            else
                setState(SIMPLY_GOTO_WAIT_POS);
          break;
      }
  }

  if (!testMode && !ballArriveMe)
	  setState(SAVE);
  // 朝向对了就强开射门
  const double myDir = me.Dir();
//  cout << "[InterceptTouch.cpp] touchDir: " << touchDir
//       << ", meDir: " << myDir
//       << endl;
  //modify by Wang in 2018/06/18
//  if(!Utils::InTheirPenaltyArea(target, 0)){//not their goal
////      cout << "not their goal"<<endl;
//      double diffBetweenMyDirAndTouchDir = fabs(Utils::Normalize(myDir - touchDir));
//      if (diffBetweenMyDirAndTouchDir < MIN_DIFF_DIR) {
//        if (useChip)
//          KickStatus::Instance()->setChipKick(runner, power);
//        else
//          KickStatus::Instance()->setKick(runner, power);
////        cout << "[InterceptTouch.cpp] test if" << endl;
//      }
//  }
//  else{//shoot their goal with compensate & generate shoot dir
////      cout << "their goal !!!!!"<<endl;
//      KickDirection::Instance()->GenerateShootDir(runner, myPos);
//      double finalDir = KickDirection::Instance()->getRealKickDir();//KickDirection::Instance()->getRawKickDir();
//      touchDir = finalDir;
//      double diffBetweenMyDirAndTouchDir = fabs(Utils::Normalize(myDir - finalDir));
//      if (diffBetweenMyDirAndTouchDir < MIN_DIFF_DIR){
    KickStatus::Instance()->setKick(runner, power);
////        cout << "[InterceptTouch.cpp] test else if" << endl;
//      }
//  }
//  double finalDir = touchDir;
  // 根据状态执行
  string msg;
  const double meToBallDist = myPos.dist(ballPos);
  CGeoPoint runPos;
  switch (state()) {
    case GOTO_PROJ_POS: {
      runPos = projectionPos + Utils::Polar2Vector(bufferDist, touchDir + Math::PI);
      // 需要快速到点
      const double myVelMod = me.Vel().mod();
      const double ballToTargetDist = runPos.dist(ballPos);
      const double ballToTargetTime = ballToTargetDist / (ballVelMod + 0.1);
      const double meToTargetDist = runPos.dist(myPos);
      const double meToTargetTime = meToTargetDist / (myVelMod + 0.1);
      if (ballVelMod > 300*10 && ballToTargetTime < meToTargetTime && meToTargetDist >= MAX_NEED_QUICK_RUN_ME_TO_TARGET_DIST) {
        const double meToTargetDir = (runPos - myPos).dir();
        runPos = runPos + Utils::Polar2Vector(meToTargetDist * meToTargetTime / ballToTargetTime, meToTargetDir);
        //GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-120, 0), "QUICK_RUN");
      }
      //GDebugEngine::Instance()->gui_debug_line(myPos + Utils::Polar2Vector(100, perpendicularDir + Math::PI), myPos + Utils::Polar2Vector(100, perpendicularDir));
	  setSubTask(PlayerRole::makeItGoto(runner, runPos, touchDir));
	  // cout << runPos << endl;
	  msg = "GOTO_PROJ_POS";
    } break;
    case INTERCEPT: {
      const CGeoLine perpLineAcrossWaitPos(waitPos, perpendicularDir);
      const CGeoPoint interceptPos = CGeoLineLineIntersection(perpLineAcrossWaitPos, ballVelLine).IntersectPoint();
      //GDebugEngine::Instance()->gui_debug_line(waitPos + Utils::Polar2Vector(100, perpendicularDir + Math::PI), waitPos + Utils::Polar2Vector(100, perpendicularDir));
      runPos = interceptPos + Utils::Polar2Vector(bufferDist, touchDir + Math::PI);
	  setSubTask(PlayerRole::makeItGoto(runner, runPos, touchDir));
      msg = "INTERCEPT";
    } break;
    default:
    case SIMPLY_GOTO_WAIT_POS: {
      runPos = waitPos;// + Utils::Polar2Vector(bufferDist, touchDir + Math::PI);
	  setSubTask(PlayerRole::makeItGoto(runner, runPos, touchDir));
      msg = "SIMPLY_GOTO_WAIT_POS";
    } break;
	case SAVE: {
        msg = "SAVE";
        CGeoPoint target = CGeoPoint(PARAM::Field::PITCH_LENGTH/2,0);
        setSubTask(PlayerRole::makeItGetBallV4(runner, PlayerStatus::KICK, target, CGeoPoint(-9999,-9999), 630*10));
	}
  }

  

  // 如果球快进嘴了就吸球
  //if (meToBallDist < MIN_DRIBBLE_DIST_THRESHOLD)
	 // DribbleStatus::Instance()->setDribbleCommand(runner, DRIBBLE_POWER);

  GDebugEngine::Instance()->gui_debug_line(ballPos, ballPos + Utils::Polar2Vector(500*10, ballVelDir),COLOR_GREEN);
  GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-300*10, 0), msg.c_str(),COLOR_GREEN);
  last_cycle_ = pVision->getCycle();
  CPlayerTask::plan(pVision);
}

CPlayerCommand* CInterceptTouch::execute(const CVisionModule* pVision) {
  if (subTask())
    return subTask()->execute(pVision);
  return NULL;
}
