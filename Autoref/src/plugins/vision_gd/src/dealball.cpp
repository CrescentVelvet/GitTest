#include "dealball.h"
#include "globaldata.h"
#include "staticparams.h"
#include "parammanager.h"
#include "matrix2d.h"
#include <iostream>
#include "ballstate.h"
namespace {
auto zpm = ZSS::ZParamManager::instance();
long long min(long long a, long long b ) {
    if (a < b) return a;
    else return b;
}

const int MAX_BALL_PER_FRAME = 200;
const int MIN_FILT_DIST = 150;
}
/**
 * @brief CDealball
 */
CDealBall::CDealBall() {
    zpm->loadParam(minBelieveFrame, "AlertFusion/ballMinBelieveFrame", 10);
    upPossible = 1.0 / minBelieveFrame;
    downPossible = 0.05;
    lastBall.pos.setX(0);
    lastBall.pos.setY(0);
}

//bool CDealBall::ballNearVechile(Ball curentBall, double dist) {
//    bool answer = false;
//    ReceiveVisionMessage result = GlobalData::instance()->maintain[-1];
//    for (int color = 0; color < PARAM::TEAMS; color++) {
//        for (int i = 0; i < PARAM::ROBOTNUM; i++) {
//            if (result.robot[color][i].pos.dist(curentBall.pos) <= dist) {
//                answer = true;
//                break;
//            }
//        }
//    }
//    return answer;
//}

double CDealBall::calculateWeight(int camID, CGeoPoint ballPos) {
    SingleCamera camera = GlobalData::instance()->cameraMatrix[camID];
    if (ballPos.x() > camera.leftedge.max && ballPos.x() < camera.rightedge.max &&
            ballPos.y() > camera.downedge.max && ballPos.y() < camera.upedge.max)
        return 1;
    else if (ballPos.x() < camera.leftedge.max && ballPos.x() > camera.leftedge.min)
        return abs(ballPos.x() - camera.leftedge.min) / abs(camera.leftedge.max - camera.leftedge.min);
    else if (ballPos.x() > camera.rightedge.max && ballPos.x() < camera.rightedge.min)
        return abs(ballPos.x() - camera.rightedge.min) / abs(camera.rightedge.max - camera.rightedge.min);
    else if (ballPos.y() < camera.downedge.max && ballPos.y() > camera.downedge.min)
        return abs(ballPos.y() - camera.downedge.min) / abs(camera.downedge.max - camera.downedge.min);
    else if (ballPos.y() > camera.upedge.max && ballPos.y() < camera.upedge.min)
        return abs(ballPos.y() - camera.upedge.min) / abs(camera.upedge.max - camera.upedge.min);
    else return 1e-8;//to deal with can see out of border situation
}


void CDealBall::mergeBall() {
    int i, j;
    actualBallNum = 0;
    for ( i = 0; i < origin.ballSize; i++) {
        bool found = false;
        for (j = 0; j < actualBallNum; j++) {
            for (int id = 0; id < PARAM::CAMERA; id++)
                if (origin.ball[i].pos.dist(ballSequence[j][id].pos) < PARAM::BALLMERGEDISTANCE) {
                    found = true;
                    break;
                }
            if (found) break;
        }
        if (found) ballSequence[j][origin.ball[i].cameraID].fill(origin.ball[i]);
        else ballSequence[actualBallNum++][origin.ball[i].cameraID].fill(origin.ball[i]);
    }
    if (PARAM::DEBUG) std::cout << "Actually have " << actualBallNum << " balls.\n";

    result.init();
    if (!origin.isBallValid && GlobalData::instance()->maintain[-1].ball[0].ball_state_machine == State::_struggle)
        result.addBall(GlobalData::instance()->maintain[-1].ball[0]);
    else {
        for (i = 0; i < actualBallNum; i++) {
            double weight = 0;
            CGeoPoint average(0, 0);
            for(j = 0; j < PARAM::CAMERA; j++) {
                if (ballSequence[i][j].pos.x() < 88888 && ballSequence[i][j].pos.y() < 88888) {
                    SingleCamera camera = GlobalData::instance()->cameraMatrix[j];
                    double _weight;
                    _weight = calculateWeight(j, ballSequence[i][j].pos);
                    _weight = std::pow(ballSequence[i][j].pos.dist(GlobalData::instance()->cameraMatrix[camera.id].campos) / 1000.0, -2.0);
                    weight += _weight;
                    average.setX(average.x() + ballSequence[i][j].pos.x() * _weight);
                    average.setY(average.y() + ballSequence[i][j].pos.y() * _weight);
                }
            }
            if (weight > 1e-4) result.addBall(average.x() / weight, average.y() / weight);
            if (PARAM::DEBUG) std::cout << "have merged NO. " << i << " ball with" << average << " " << weight << "\n";
        }
    }
}

void CDealBall::init() {
    origin.init();
    for(int i = 0; i < PARAM::BALLNUM; i++)
        for(int j = 0; j < PARAM::CAMERA; j++)
            ballSequence[i][j].fill(99999,99999);
    _cycle = GlobalData::instance()->processBall.cycle() + 1;
    for (int i = 0; i < PARAM::CAMERA; i++) {
        if (GlobalData::instance()->cameraUpdate[i]) {
            for(int j = 0; j < GlobalData::instance()->camera[i][0].ballSize; j++) {
                Ball currentball = GlobalData::instance()->camera[i][0].ball[j];
                origin.addBall(GlobalData::instance()->camera[i][0].ball[j].pos.x(),
                               GlobalData::instance()->camera[i][0].ball[j].pos.y(), 0, i);
            }
        }
    }
    if (origin.ballSize > 0) origin.isBallValid = true;
    if (PARAM::DEBUG) std::cout << "Origin vision has " << origin.ballSize << " balls.\n";
}


void CDealBall::choseBall() {
    double dis = 99999;
    int id = -1;
    for (int i = 0; i < result.ballSize; i++) {
        if (result.ball[i].pos.dist(lastBall.pos) < dis) {
            dis = result.ball[i].pos.dist(lastBall.pos);
            id = i;
            if (PARAM::DEBUG) std::cout << " the dis=" << dis << std::endl;
        }
    }

    //possiblity
    if (lastBall.pos.dist(result.ball[id].pos) < min(2000, MAX_BALL_PER_FRAME * (_cycle - lastFrame))) {
        lastBall = result.ball[id];
        lastFrame = _cycle;
        lastPossible = 1;
        currentPossible = 0;
    } else if(currentBall.pos.dist(result.ball[id].pos) < min(2000, MAX_BALL_PER_FRAME * (_cycle - currentFrame))) {
        currentBall = result.ball[id];
        currentFrame = _cycle;
        currentPossible += upPossible;
        lastPossible -= downPossible;
    } else {
        currentBall = result.ball[id];
        currentFrame = _cycle;
        currentPossible = upPossible;
        lastPossible -= downPossible;
    }
    result.init();
    if (lastPossible >= currentPossible)
        result.addBall(lastBall);
    else {
        result.addBall(currentBall);
        lastBall = currentBall;
        lastFrame = currentFrame;
        lastPossible = currentPossible;
        currentPossible = 0;
    }
}

void CDealBall::run() {
    init();
    if (origin.isBallValid) {
        mergeBall();
        choseBall();
    } else {
        CVector lastVel = GlobalData::instance()->maintain[0].ball[0].velocity / ZSS::Athena::FRAME_RATE;
        CVector ballTravel ;
        if (GlobalData::instance()->maintain[0].ball[0].velocity.mod() >= ZSS::Athena::V_SWITCH)
            ballTravel = lastVel +  lastVel.unit() * (ZSS::Athena::ACC_SLIDE / ZSS::Athena::FRAME_RATE / ZSS::Athena::FRAME_RATE / 2 ) ;
        else if (GlobalData::instance()->maintain[0].ball[0].velocity.mod() <= 1e-8)
            ballTravel = CVector(0, 0);
        else
            ballTravel = lastVel +  lastVel.unit() * (ZSS::Athena::ACC_ROLL / ZSS::Athena::FRAME_RATE / ZSS::Athena::FRAME_RATE / 2 );

        result.addBall(GlobalData::instance()->maintain[0].ball[0].pos + ballTravel);
        lastBall = GlobalData::instance()->maintain[0].ball[0];
        lastPossible = 1;
    }
    result.isBallValid = origin.isBallValid;

    GlobalData::instance()->processBall.push(result);
}

void CDealBall::updateVel(const Matrix2d& tempMatrix, ReceiveVisionMessage& result) {

    // 1.进行Kalman滤波，估计球的位置以及球速
    CGeoPoint filtPoint (tempMatrix(0, 0), tempMatrix(1, 0));
    CVector ballVel(tempMatrix(2, 0)* ZSS::Athena::FRAME_RATE, tempMatrix(3, 0)*ZSS::Athena::FRAME_RATE);
    ballVel = ballVel / lostFrame;
    if (ballVel.mod()>7500) ballVel= ballVel.unit()*7500;
    // 2.延时补偿，根据延时帧率将位置和速度进行修正
    for( int i = 0; i < ZSS::Athena::TOTAL_LATED_FRAME; ++i ) {
        //thisCycle.SetPos(thisCycle.Pos() + thisCycle.Vel() / ZSS::Athena::FRAME_RATE);
        CVector uniVec = ballVel / (ballVel.mod() + 1.0);
        if ( ballVel.mod() > ZSS::Athena::BALL_DELC_CHANGE_POINT )
            ballVel = ( uniVec * ( ballVel.mod() - ZSS::Athena::BALL_FAST_DEC / ZSS::Athena::FRAME_RATE ));
        else if ( ballVel.mod() > 100 )
            ballVel = ( uniVec * ( ballVel.mod() - ZSS::Athena::BALL_SLOW_DEC / ZSS::Athena::FRAME_RATE ));
        else {
            ballVel = (CVector(0, 0));
        }
    }
    // 3.估计球速大小及方向，特别是方向
    if(lastPossible > 0.3) { //MAYBE FIX
        // 计算球速方向:
        double movingDirection = ballVel.dir();
        _lastValidDir = movingDirection;
        ballVel.setVector(ballVel.mod()*std::cos(movingDirection), ballVel.mod()*std::sin(movingDirection));
    } else {
        double predictDir = _lastValidDir;
        double velMod = ballVel.mod();
        if (velMod < 0.1) {
            velMod = 0.1;
        }
        ballVel.setVector(velMod * std::cos(predictDir), velMod * std::sin(predictDir));
    }
    result.ball[0].velocity = ballVel;

    if(origin.isBallValid) lostFrame = 1;
    else lostFrame ++;
}
