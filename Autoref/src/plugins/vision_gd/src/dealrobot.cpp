#include "dealrobot.h"
#include "globaldata.h"
#include "staticparams.h"
#include "parammanager.h"
#include "matrix2d.h"
#include <iostream>
#include <qdebug.h>
namespace {
auto zpm = ZSS::ZParamManager::instance();
int DIFF_VECHILE_MAX_DIFF = 600;//1000
float MAX_SPEED = 5000;
// 由于图像的一次事故，帧率和系统延时改变了
const double TOTAL_MOV_LATED_FRAME = 3.5f; //平移的延时(原来为4.2)
const int NUM_MOV_LATENCY_FRAMES = static_cast<int>(TOTAL_MOV_LATED_FRAME);
const float MOV_LATENCY_FRACTION  = TOTAL_MOV_LATED_FRAME - static_cast<float>(NUM_MOV_LATENCY_FRAMES);

const double TOTAL_DIR_LATED_FRAME = 2.0f; //转动的延时
const int NUM_DIR_LATENCY_FRAMES = static_cast<int>(TOTAL_DIR_LATED_FRAME);
const float DIR_LATENCY_FRACTION  = TOTAL_DIR_LATED_FRAME - static_cast<float>(NUM_DIR_LATENCY_FRAMES);

const double STEP_POSSIBILITY = 0.1;
double DECIDE_POSSIBILITY = 0.0;
double OUR_DOWN_POSSIBILITY = 0.0;
double THEIR_DOWN_POSSIBILITY = 0.0;
}

CDealRobot::CDealRobot() {
    zpm->loadParam(fieldWidth, "field/canvasWidth", 13200);
    zpm->loadParam(fieldHeight, "field/canvasHeight", 9900);
    zpm->loadParam(minBelieveFrame, "AlertFusion/carMinBelieveFrame", 3);
    zpm->loadParam(theirMaxLostFrame, "AlertFusion/theirCarMaxLostFrame", 15);
    zpm->loadParam(ourMaxLostFrame, "AlertFusion/ourCarMaxLostFrame", 150);
    DECIDE_POSSIBILITY = minBelieveFrame * STEP_POSSIBILITY;
    OUR_DOWN_POSSIBILITY = (1.0 - DECIDE_POSSIBILITY) / ourMaxLostFrame;
    THEIR_DOWN_POSSIBILITY = (1.0 - DECIDE_POSSIBILITY) / theirMaxLostFrame;
}

bool CDealRobot::isOnField(CGeoPoint p) {
    if (p.x() < fieldWidth / 2 && p.x() > -fieldWidth / 2 && p.y() < fieldHeight / 2 && p.y() > -fieldHeight / 2)
        return true;
    return false;
}

double CDealRobot::calculateWeight(int camID, CGeoPoint roboPos) {
    SingleCamera camera = GlobalData::instance()->cameraMatrix[camID];
    if (roboPos.x() > camera.leftedge.max && roboPos.x() < camera.rightedge.max &&
            roboPos.y() > camera.downedge.max && roboPos.y() < camera.upedge.max)
        return 1;
    else if (roboPos.x() < camera.leftedge.max && roboPos.x() > camera.leftedge.min)
        return abs(roboPos.x() - camera.leftedge.min) / abs(camera.leftedge.max - camera.leftedge.min);
    else if (roboPos.x() > camera.rightedge.max && roboPos.x() < camera.rightedge.min)
        return abs(roboPos.x() - camera.rightedge.min) / abs(camera.rightedge.max - camera.rightedge.min);
    else if (roboPos.y() < camera.downedge.max && roboPos.y() > camera.downedge.min)
        return abs(roboPos.y() - camera.downedge.min) / abs(camera.downedge.max - camera.downedge.min);
    else if (roboPos.y() > camera.upedge.max && roboPos.y() < camera.upedge.min)
        return abs(roboPos.y() - camera.upedge.min) / abs(camera.upedge.max - camera.upedge.min);
    else return 0.0001;//to deal with can see out of border situation
}

void CDealRobot::init() {
    result.init();
    for (int roboId = 0; roboId < PARAM::ROBOTNUM; roboId++) {
        for (int color = 0; color < PARAM::TEAMS; color++) {
            for (int camId = 0; camId < PARAM::CAMERA; camId++) {
                robotSeqence[color][roboId][camId].init();
            }
        }
    }
    for (int i = 0; i < PARAM::CAMERA; i++) {
        if(GlobalData::instance()->cameraUpdate[i]) {
            for (int color = 0; color < PARAM::TEAMS; color++) {
                for (int robotID = 0; robotID < PARAM::ROBOTNUM; robotID++) {
                    Robot robot = GlobalData::instance()->camera[i][0].robot[color][robotID];
                    if (GlobalData::instance()->robotPossible[color][robotID] < DECIDE_POSSIBILITY)
                        //当这是新车的时候
                        robotSeqence[color][robotID][i] = robot;
                    else if  (lastRobot[color][robotID].pos.dist(robot.pos) < DIFF_VECHILE_MAX_DIFF)
                        robotSeqence[color][robotID][i] = robot;
                }
            }
        }
    }
//    for (int color = 0; color < PARAM::TEAMS; color++) {
//        for (int i = 0; i < PARAM::ROBOTNUM; i++) {
//            sortTemp[color][i].init();
//        }
//    }
}

void CDealRobot::mergeRobot() {
    for (int color = 0; color < PARAM::TEAMS; color++) {
        for (int roboId = 0; roboId < PARAM::ROBOTNUM; roboId++) {
            bool isFound = false;
            double teamWeight = 0;
            CGeoPoint teamAverage(0, 0);
            double teamAngle = 0;
            for (int camId = 0; camId < PARAM::CAMERA; camId++) {
                double _weight = 0;
                if(robotSeqence[color][roboId][camId].pos.x() < 88888 && robotSeqence[color][roboId][camId].pos.y() < 88888) {
                    isFound = true;
                    _weight = calculateWeight(camId, robotSeqence[color][roboId][camId].pos);
                    teamWeight += _weight;
                    teamAverage.setX(teamAverage.x() + robotSeqence[color][roboId][camId].pos.x() * _weight);
                    teamAverage.setY(teamAverage.y() + robotSeqence[color][roboId][camId].pos.y() * _weight);
                    teamAngle = robotSeqence[color][roboId][camId].angle;
                    break;
                }
            }
            if (isFound) {
                Robot ave(teamAverage.x() / teamWeight, teamAverage.y() / teamWeight, teamAngle, false);
                result.setRobot(color, roboId, ave);
            }
        }
    }
//    if (PARAM::DEBUG) std::cout << "have found " << result.robotSize[PARAM::BLUE] << "blue car.\t" << result.robotSize[PARAM::YELLOW] << std::endl;
}

void CDealRobot::setPossibility() {
    for (int color = 0; color < PARAM::TEAMS; color ++) {
        for (int id = 0; id < PARAM::ROBOTNUM; id++) {
            bool found = false;
            lastRobot[color][id].valid = false;
            if ((isOnField(result.robot[color][id].pos))
                && (GlobalData::instance()->robotPossible[color][id] < DECIDE_POSSIBILITY
                || result.robot[color][id].pos.dist(lastRobot[color][id].pos) < DIFF_VECHILE_MAX_DIFF)) {
                lastRobot[color][id] = result.robot[color][id];
                found = true;
            }
            if (found)  GlobalData::instance()->robotPossible[color][id] += STEP_POSSIBILITY;
            else { // 没看到车，猜测一个原始位置
                lastRobot[color][id].pos = (lastRobot[color][id].pos + lastRobot[color][id].velocity / ZSS::Athena::FRAME_RATE);
                lastRobot[color][id].angle = lastRobot[color][id].angle + lastRobot[color][id].rotateVel / ZSS::Athena::FRAME_RATE;
                if (GlobalData::instance()->robotPossible[color][id] >= DECIDE_POSSIBILITY)
                    if (GlobalData::instance()->commandMissingFrame[color] >= 20)
                        GlobalData::instance()->robotPossible[color][id] -= THEIR_DOWN_POSSIBILITY;
                    else
                        GlobalData::instance()->robotPossible[color][id] -= OUR_DOWN_POSSIBILITY;
                else GlobalData::instance()->robotPossible[color][id] -= DECIDE_POSSIBILITY;
            }

            if (GlobalData::instance()->robotPossible[color][id] > 1.0) GlobalData::instance()->robotPossible[color][id] = 1.0;
            if (GlobalData::instance()->robotPossible[color][id] < 0.0) GlobalData::instance()->robotPossible[color][id] = 0.0;

            if(GlobalData::instance()->robotPossible[color][id] >= DECIDE_POSSIBILITY) {
                lastRobot[color][id].valid = true;
                result.setRobot(color, id, lastRobot[color][id]);
            }
        }
    }
}

void CDealRobot::updateVel(ReceiveVisionMessage& result) {
    for (int team = 0; team < PARAM::TEAMS; team ++) {
        //   for (int i = 0; i < validNum[team]; i++) {
        //   Robot & robot = result.robot[team][i];
           for(int i = 0;i < PARAM::ROBOTNUM; i++){
               Robot& robot = result.robot[team][i];
               if(!robot.valid) continue;//change by lzx
               //位置滤波
               auto & playerPosVel = _kalmanFilter[team][i].update(robot.pos.x(), robot.pos.y());
               CGeoPoint filtPoint (playerPosVel(0, 0), playerPosVel(1, 0));
               CVector PlayVel(playerPosVel(2, 0), playerPosVel(3, 0));

               bool side = false;
               //朝向滤波
               auto &playerRotVel = _dirFilter[team][i].update(std::cos(robot.angle), std::sin(robot.angle));
               double filterDir = std::atan2(playerRotVel(1, 0), playerRotVel(0, 0));
               double rotVel = playerRotVel(2, 0) * std::cos(90 * 3.1416 / 180 + filterDir)
                               + playerRotVel(3, 0) * std::sin(90 * 3.1416 / 180 + filterDir);

               robot.angle = filterDir;
               robot.rotateVel = rotVel * ZSS::Athena::FRAME_RATE;
               ZSS::ZParamManager::instance()->loadParam(side, "ZAlert/IsRight");
               robot.pos = filtPoint;
               robot.velocity = PlayVel * ZSS::Athena::FRAME_RATE;
               robot.raw_vel = robot.velocity;
               robot.rawRotateVel = robot.rotateVel;
               //我方位置朝向修正，根据medusa回传的速度信息
               if (GlobalData::instance()->commandMissingFrame[team] < 20) {
                   //小数部分
                   auto command = GlobalData::instance()->robotCommand[team][0 - NUM_MOV_LATENCY_FRAMES].robotSpeed[i];
                   CVector robot_travel = CVector(command.vx, command.vy) * MOV_LATENCY_FRACTION / double(ZSS::Athena::FRAME_RATE);

                   robot.angle += command.vr * DIR_LATENCY_FRACTION / ZSS::Athena::FRAME_RATE ;
                   //整数部分
                   for (int frame = NUM_MOV_LATENCY_FRAMES - 1; frame >= 0; frame--) {
                       command = GlobalData::instance()->robotCommand[team][0 - frame].robotSpeed[i];
                       robot_travel = robot_travel + CVector(command.vx, command.vy) / ZSS::Athena::FRAME_RATE;
       //                qDebug() << "after" << robot.pos.x() << " " << robot.pos.y();
                   }
                   for (int frame = NUM_DIR_LATENCY_FRAMES - 1; frame >= 0 ; frame--) {
                       command = GlobalData::instance()->robotCommand[team][0 - frame].robotSpeed[i];
                       robot.angle += command.vr  / ZSS::Athena::FRAME_RATE ;
       //                qDebug() << "after" << robot.pos.x() << " " << robot.pos.y();
                   }

                   robot_travel = robot_travel.rotate(robot.angle);
                   robot.pos = robot.pos + robot_travel;

                   robot.rotateVel = command.vr;
                   robot.velocity = CVector(GlobalData::instance()->robotCommand[team][0].robotSpeed[i].vx,
                                            GlobalData::instance()->robotCommand[team][0].robotSpeed[i].vy);
                   robot.velocity = robot.velocity.rotate(robot.angle);
                   if (robot.velocity.mod() > 10000)
                       qDebug() << "fuck!!!" << robot.velocity.mod();
                   //FIX IT

               }

       //        for (int j = 0; j < PARAM::ROBOTNUM; j++) {
       //            if (robot.id == GlobalData::instance()->maintain[0].robot[team][j].id ) {

       //                robot.accelerate = (robot.velocity - GlobalData::instance()->maintain[-2].robot[team][j].velocity) / 3 * ZSS::Athena::FRAME_RATE ;
       ////                if (abs(robot.accelerate.mod()) > 600) robot.accelerate = CVector(0, 0);
       //            }
       //        }
               robot.accelerate = (robot.velocity - GlobalData::instance()->maintain[-2].robot[team][i].velocity) / 3 * ZSS::Athena::FRAME_RATE ;//change by lzx
               lastRobot[team][i].velocity = robot.velocity;
               lastRobot[team][i].rotateVel = robot.rotateVel;
           }
    }

}

void CDealRobot::run() {
    init();
    mergeRobot();
    setPossibility();
    GlobalData::instance()->processRobot.push(result);
}


