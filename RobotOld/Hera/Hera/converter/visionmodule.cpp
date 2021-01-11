#include "visionmodule.h"
#include "montage.h"
#include "maintain.h"
#include "dealball.h"
#include "dealrobot.h"
#include "messages_robocup_ssl_wrapper.pb.h"
#include "parammanager.h"
#include <QElapsedTimer>
#include <QtDebug>

/**
 * @brief VisionModule consturctor
 * @param parent
 */
VisionModule::VisionModule(QObject *parent)
    : QObject(parent)
    , dealBall(this)
    , dealRobot(this)
    , cmaintain(this)
    , lastTouch(0)
{
    detectionBall = detectionFrame.mutable_balls();
    setCameraMatrix();
}

bool VisionModule::dealWithData() {
    dealBall.run();
    dealRobot.run();
    cmaintain.run();
//    ref_event* analyser = new ref_event();
//    analyser->run(maintain);
    toProtobuf();
    return true;
}

void VisionModule::parse(void * ptr, int size) {
    static SSL_WrapperPacket packet;
    ReceiveVisionMessage message;
    packet.ParseFromArray(ptr, size);
    if (packet.has_detection()) {
        const SSL_DetectionFrame& detection = packet.detection();
        message.camID = detection.camera_id();
//        std::cout << "message.camID : " <<message.camID<<std::endl;
        int ballSize = detection.balls_size();
        int blueSize = detection.robots_blue_size();
        int yellowSize = detection.robots_yellow_size();
//        std::cout << "detection yellow : " << yellowSize <<std::endl;
//        std::cout << "detection blue  : " << blueSize <<std::endl;
        for (int i = 0; i < ballSize; i++) {
            const SSL_DetectionBall& ball = detection.balls(i);
            if (globalsetting.inChosenArea(CGeoPoint(ball.x(), ball.y()))) {
                message.addBall(CGeoPoint(ball.x(), ball.y()));
            }
        }
        for (int i = 0; i < blueSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_blue(i);
//            std::cout << blueSize <<std::endl;
            if (globalsetting.inChosenArea(CGeoPoint(robot.x(), robot.y()))
                    && robot.robot_id() < PARAM::ROBOTNUM) {
                message.addRobot(BLUE, robot.robot_id(), CGeoPoint(robot.x(), robot.y()), robot.orientation());
            }
        }
        int debug = 0;
        for (int i = 0; i < yellowSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_yellow(i);
//            std::cout << yellowSize <<std::endl;
            if (globalsetting.inChosenArea(CGeoPoint(robot.x(), robot.y()))
                    && robot.robot_id() < PARAM::ROBOTNUM) {
                debug++;
                message.addRobot(YELLOW, robot.robot_id(), CGeoPoint(robot.x(), robot.y()), robot.orientation());
            }
        }
//        std::cout << "vision debug : " <<debug <<std::endl;
        camera[message.camID].push(message);
        cameraUpdate[message.camID] = true;
//        for (int i = 0; i < PARAM::CAMERA; i++) {
//            std::cout << cameraUpdate[i] <<std::endl;
//        }
//        std::cout << "///////////////////" <<std::endl;
//        std::cout << "message.ID  : " <<message.camID<<std::endl;
//        std::cout << collectNewVision() <<std::endl;
        dealWithData();
//        qDebug() << message.camID;
    }
    if (collectNewVision()) {
//        std::fill_n(cameraUpdate, PARAM::CAMERA, false);
//        std::cout << "fuck !!" <<std::endl;
        std::fill_n(cameraUpdate, PARAM::CAMERA -4 , false);//fix camera 4 for 2017 log
        dealWithData();
    }
}

/**
 * @brief judge if all needed camera vision is collected
 * @return
 */
bool VisionModule::collectNewVision() {
    for (int i = 0; i < PARAM::CAMERA; i++) {
        if (!cameraUpdate[i])
            return false;
    }
    return true;
}

void VisionModule::setCameraMatrix() {
     cameraMatrix[0].fillCenter(CGeoPoint(-3000,2250));
     cameraMatrix[1].fillCenter(CGeoPoint(3000,2250));
     cameraMatrix[2].fillCenter(CGeoPoint(-3000,-2250));
     cameraMatrix[3].fillCenter(CGeoPoint(3000,-2250));
     for (int i=0; i<PARAM::CAMERA;i++){
         cameraMatrix[i].leftedge.min=cameraMatrix[i].campos.x()-3300;
         cameraMatrix[i].leftedge.max=cameraMatrix[i].campos.x()-3000;
         cameraMatrix[i].rightedge.min=cameraMatrix[i].campos.x()+3300;
         cameraMatrix[i].rightedge.max=cameraMatrix[i].campos.x()+3000;
         cameraMatrix[i].downedge.min=cameraMatrix[i].campos.y()-2550;
         cameraMatrix[i].downedge.max=cameraMatrix[i].campos.y()-2250;
         cameraMatrix[i].upedge.min=cameraMatrix[i].campos.y()+2550;
         cameraMatrix[i].upedge.max=cameraMatrix[i].campos.y()+2250;
        }
    }

void VisionModule::toProtobuf(){
    ReceiveVisionMessage result = maintain[0];
    ref_event* analyser = new ref_event();
    analyser->run(maintain);
    if (result.ballSize > 0) {
        detectionBall->set_x(result.ball[0].pos.x());
        detectionBall->set_raw_x(result.ball[0].pos.x());
        if (result.ball[0].pos.y() == 0) {
            detectionBall->set_y(float(0.1));
            detectionBall->set_raw_y(float(0.1));
        }else{
            detectionBall->set_raw_y(result.ball[0].pos.y());
            detectionBall->set_y(result.ball[0].pos.y());
        }//to fix a role match bug 2018.6.15

        CVector TransferVel(result.ball[0].velocity.x(), result.ball[0].velocity.y());
        detectionBall->set_vel_x(TransferVel.x());
        detectionBall->set_vel_y(TransferVel.y());
        detectionBall->set_valid(dealBall.getValid());
        detectionBall->set_last_touch(lastTouch);
        detectionBall->set_ball_state(ballStateMachine);
    } else {
        detectionBall->set_x(-32767);
        detectionBall->set_y(-32767);
        detectionBall->set_raw_x(-32767);
        detectionBall->set_raw_y(-32767);
    }
    for (int i = 0; i < result.robotSize[PARAM::BLUE]; i++) {
        if (i == PARAM::SENDROBOTNUM) break; //for sending MAX 8 car possible
        detectionRobot[PARAM::BLUE][i] = detectionFrame.add_robots_blue();
        detectionRobot[PARAM::BLUE][i]->set_x(result.robot[PARAM::BLUE][i].pos.x());
        detectionRobot[PARAM::BLUE][i]->set_y(result.robot[PARAM::BLUE][i].pos.y());
        detectionRobot[PARAM::BLUE][i]->set_raw_x(result.robot[PARAM::BLUE][i].pos.x());
        detectionRobot[PARAM::BLUE][i]->set_raw_y(result.robot[PARAM::BLUE][i].pos.y());
        detectionRobot[PARAM::BLUE][i]->set_orientation(result.robot[PARAM::BLUE][i].angle);
        detectionRobot[PARAM::BLUE][i]->set_robot_id(result.robot[PARAM::BLUE][i].id);
        detectionRobot[PARAM::BLUE][i]->set_confidence(1);
        CVector TransferVel(result.robot[PARAM::BLUE][i].velocity.x(), result.robot[PARAM::BLUE][i].velocity.y());
        detectionRobot[PARAM::BLUE][i]->set_vel_x(TransferVel.x());
        detectionRobot[PARAM::BLUE][i]->set_vel_y(TransferVel.y());
        detectionRobot[PARAM::BLUE][i]->set_raw_vel_x(TransferVel.x());
        detectionRobot[PARAM::BLUE][i]->set_raw_vel_y(TransferVel.y());
        detectionRobot[PARAM::BLUE][i]->set_rotate_vel(result.robot[PARAM::BLUE][i].rotateVel);
        detectionRobot[PARAM::BLUE][i]->set_valid(true);
    }
    for (int i = 0; i < result.robotSize[PARAM::YELLOW]; i++) {
        if (i == PARAM::SENDROBOTNUM) break;
        detectionRobot[PARAM::YELLOW][i] = detectionFrame.add_robots_yellow();
        detectionRobot[PARAM::YELLOW][i]->set_x(result.robot[PARAM::YELLOW][i].pos.x());
        detectionRobot[PARAM::YELLOW][i]->set_y(result.robot[PARAM::YELLOW][i].pos.y());
        detectionRobot[PARAM::YELLOW][i]->set_raw_x(result.robot[PARAM::YELLOW][i].pos.x());
        detectionRobot[PARAM::YELLOW][i]->set_raw_y(result.robot[PARAM::YELLOW][i].pos.y());
        detectionRobot[PARAM::YELLOW][i]->set_orientation(result.robot[PARAM::YELLOW][i].angle);
        detectionRobot[PARAM::YELLOW][i]->set_robot_id(result.robot[PARAM::YELLOW][i].id);
        detectionRobot[PARAM::YELLOW][i]->set_confidence(1);
        CVector TransferVel(result.robot[PARAM::YELLOW][i].velocity.x(), result.robot[PARAM::YELLOW][i].velocity.y());
        detectionRobot[PARAM::YELLOW][i]->set_vel_x(TransferVel.x());
        detectionRobot[PARAM::YELLOW][i]->set_vel_y(TransferVel.y());
        detectionRobot[PARAM::YELLOW][i]->set_raw_vel_x(TransferVel.x());
        detectionRobot[PARAM::YELLOW][i]->set_raw_vel_y(TransferVel.y());
        detectionRobot[PARAM::YELLOW][i]->set_rotate_vel(result.robot[PARAM::YELLOW][i].rotateVel);
        detectionRobot[PARAM::YELLOW][i]->set_valid(true);
    }
    int size = detectionFrame.ByteSize();
    QByteArray buffer(size, 0);
    detectionFrame.SerializeToArray(buffer.data(), buffer.size());

    //在这儿发送protubuf包
//    if (flag == 0) {
//        lw_v.write(buffer);
//    } else if (flag == 1) {
//        ns.udpSend(buffer);
//    }
    //发送完毕

    detectionFrame.clear_robots_blue();
    detectionFrame.clear_robots_yellow();

//    ref_event analyser;
//    analyser.run(maintain);

//    ZSS::Protocol::Debug_Msgs msgs;
//    ZSS::Protocol::Debug_Msg* ball_msg = msgs.add_msgs();
//    ball_msg->set_type(ZSS::Protocol::Debug_Msg_Debug_Type_TEXT);
//    ball_msg->set_color(ZSS::Protocol::Debug_Msg_Color_YELLOW);
//    ZSS::Protocol::Debug_Text* ball_debugText = ball_msg->mutable_text();
//    ZSS::Protocol::Point* ball_locate = new ZSS::Protocol::Point();
//    ball_locate->set_x(-480);
//    ball_locate->set_y(480);
//    ball_debugText->set_allocated_pos(ball_locate);
//    std::string ball_text;
//    switch(analyser->getState()){
//    case 0 : ball_text = "nor";
//        break;
//    case 1 : ball_text = "out_of_field";
//        break;
//    case 2 : ball_text = "goal";
//        break;
//    case 3 : ball_text = "yellow_collision";
//        break;
//    case 4 : ball_text = "blue_collision";
//        break;
//    case 5 : ball_text = "overspeed";
//        break;
//    case 6 : ball_text = "blue_kick_ball";
//        break;
//    }

//    setDebugMsg(-480,480,ball_text);

//    ball_debugText->set_text(ball_text);

//    ZSS::Protocol::Debug_Msg* robot_msg = msgs.add_msgs();
//    robot_msg->set_type(ZSS::Protocol::Debug_Msg_Debug_Type_TEXT);
//    robot_msg->set_color(ZSS::Protocol::Debug_Msg_Color_YELLOW);
//    ZSS::Protocol::Debug_Text* robot_debugText = robot_msg->mutable_text();
//    ZSS::Protocol::Point* robot_locate = new ZSS::Protocol::Point();
//    robot_locate->set_x(-480);
//    robot_locate->set_y(415);
//    robot_debugText->set_allocated_pos(robot_locate);
//    std::string robot_text;
//    switch(analyser->getRobotState()){
//    case 0 : robot_text = "Run";
//        break;
//    case 1 : robot_text = "blue_kick_ball";
//        break;
//    case 2 : robot_text = "yellow_kick_ball";
//        break;
//    }

//    setDebugMsg(-480,415,robot_text);
////    robot_debugText->set_text(robot_text);
//    int id = analyser->lastTouchID();

//    setDebugMsg(-220,435,std::to_string(id));


//    int size = msgs.ByteSize();
//    QByteArray buffer(size , 0);
//    msgs.SerializePartialToArray(buffer.data(), buffer.size());

//    int sent_size = 0;
//    sent_size = sendSocket.writeDatagram(buffer,buffer.size(),QHostAddress(ZSS::LOCAL_ADDRESS),30002);

//    msgs.clear_msgs();
}

void VisionModule::setDebugMsg(double pos_x , double pos_y ,std::string text){
    ZSS::Protocol::Debug_Msg* msg = msgs.add_msgs();
    msg->set_type(ZSS::Protocol::Debug_Msg_Debug_Type_TEXT);
    msg->set_color(ZSS::Protocol::Debug_Msg_Color_YELLOW);
    ZSS::Protocol::Debug_Text* debugText = msg->mutable_text();
    ZSS::Protocol::Point* locate = new ZSS::Protocol::Point();
    locate->set_x(pos_x);
    locate->set_y(pos_y);
    debugText->set_allocated_pos(locate);

    debugText->set_text(text);
}


