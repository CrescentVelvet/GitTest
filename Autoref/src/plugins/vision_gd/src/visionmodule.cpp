#include "visionmodule.h"
#include "globaldata.h"
#include "montage.h"
#include "maintain.h"
#include "dealball.h"
#include "dealrobot.h"
#include "messages_robocup_ssl_wrapper.pb.h"
#include "parammanager.h"
#include "setthreadname.h"
#include <QElapsedTimer>
#include <QtDebug>

namespace {
auto zpm = ZSS::ZParamManager::instance();
auto vpm = ZSS::VParamManager::instance();
}
/**
 * @brief CVisionModule consturctor
 * @param parent
 */
CVisionModule::CVisionModule()
    : IF_EDGE_TEST(false)
    , ZSPlugin("Vision_GD"){
    std::fill_n(GlobalData::instance()->cameraUpdate, PARAM::CAMERA, false);
    std::fill_n(GlobalData::instance()->cameraControl, PARAM::CAMERA, true);
    std::fill_n(GlobalData::instance()->processControl, 3, true);
    declare_receive("ssl_vision",false);
    declare_publish("zss_vision");
}
CVisionModule::~CVisionModule(){}
void CVisionModule::parseCameraMode(int mode){
    for(int i=0;i<PARAM::CAMERA;i++){
        GlobalData::instance()->cameraControl[i] = ((mode&(1<<i)) != 0);
    }
    for(int i=0;i<PARAM::CAMERA;i++){
        std::cout << GlobalData::instance()->cameraControl[i] << " ";
    }
    std::cout << std::endl;
}
/**
 * @brief connect UDP for receive vision
 * @param real
 */
void CVisionModule::setIfEdgeTest(bool isEdgeTest) {
    IF_EDGE_TEST = isEdgeTest;
}
bool CVisionModule::showIfEdgeTest() {
    return IF_EDGE_TEST;
}
void CVisionModule::udpSocketConnect(bool real) {
    real ? zpm->loadParam(vision_port, "AlertPorts/Vision4Real", 10005) : zpm->loadParam(vision_port, "AlertPorts/Vision4Sim", 10020);
    zpm->loadParam(saoAction, "Alert/SaoAction", 0);
    GlobalData::instance()->setCameraMatrix(real);
    qDebug() << "VisionPort : " << vision_port;
    for (int i = 0; i < PARAM::CAMERA; i++) {
        ReceiveVisionMessage temp;
        GlobalData::instance()->camera[i].push(temp);
    }
}
/**
 * @brief disconnect UDP
 */
void CVisionModule::udpSocketDisconnect() {
    if (IF_EDGE_TEST) {
        for (int i = 0; i < PARAM::CAMERA; i++) {
            SingleCamera& currentCamera = GlobalData::instance()->cameraMatrix[i];
            vpm->changeParam("Camera" + QString::number(i) + "Leftmin", currentCamera.leftedge.min);
            vpm->changeParam("Camera" + QString::number(i) + "Leftmax", currentCamera.leftedge.max);
            vpm->changeParam("Camera" + QString::number(i) + "Rightmin", currentCamera.rightedge.min);
            vpm->changeParam("Camera" + QString::number(i) + "Rightmax", currentCamera.rightedge.max);
            vpm->changeParam("Camera" + QString::number(i) + "Upmin", currentCamera.upedge.min);
            vpm->changeParam("Camera" + QString::number(i) + "Upmax", currentCamera.upedge.max);
            vpm->changeParam("Camera" + QString::number(i) + "Downmin", currentCamera.downedge.min);
            vpm->changeParam("Camera" + QString::number(i) + "Downmax", currentCamera.downedge.max);
        }
    }
}
/**
 * @brief convert between different field size
 * @param originPoint
 * @return
 */
CGeoPoint CVisionModule::saoConvert(CGeoPoint originPoint) {
    CGeoPoint result;
    switch (saoAction) {
    case 0:
        //default
        result.setX(originPoint.x());
        result.setY(originPoint.y());
        break;
    case 1:
        //convert small field to half big field
        result.setX(originPoint.y() + 3000);
        result.setY(-originPoint.x());
        break;
    case 2:
        //convert small field to whole big field
        result.setX(originPoint.x() * 3 / 2);
        result.setY(originPoint.y() * 3 / 2);
        break;
    default:
        result.setX(originPoint.x());
        result.setY(originPoint.y());
        break;
    }
    return result;
}
/**
 * @brief convert angle when convert small field to half big field
 * @param direction
 * @return
 */
double CVisionModule::saoConvert(double direction) {
    if (saoAction == 1)
        return (direction - 3.1415926 / 2);
    else
        return direction;
}
/**
 * @brief CVisionModule::storeData
 */
void CVisionModule::run(){
    SetThreadName("VisionModule");
    ZSData vision;
    SSL_WrapperPacket packet;
    while(response_to_control() != CONTROL_NEED_EXIT){
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        receive("ssl_vision",vision);
        packet.ParseFromArray(vision.data(), vision.size());
        parse(packet);
    }
    qDebug() << "VisionModule:run() safe exit!";
}
/**
 * @brief process data
 * @return
 */
bool CVisionModule::dealWithData() {
    counter++;
    if (IF_EDGE_TEST) edgeTest();
    DealBall::instance()->run();
    DealRobot::instance()->run();
    Maintain::instance()->run();
    return true;
}
/**
 * @brief parse camera vision message
 * @param ptr
 * @param size
 */
void CVisionModule::parse(const SSL_WrapperPacket& packet) {
    ReceiveVisionMessage message;
    if (packet.has_detection()) {
        const SSL_DetectionFrame& detection = packet.detection();
        message.camID = detection.camera_id();
        if (message.camID >= PARAM::CAMERA || message.camID < 0) {
//            qDebug() << "get invalid camera id : " << message.camID;
            message.camID = 0;
        }
        int ballSize = detection.balls_size();
        int blueSize = detection.robots_blue_size();
        int yellowSize = detection.robots_yellow_size();
        for (int i = 0; i < ballSize; i++) {
            const SSL_DetectionBall& ball = detection.balls(i);
            message.addBall(saoConvert(CGeoPoint(ball.x(), ball.y())));
        }
        for (int i = 0; i < blueSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_blue(i);
            if (robot.robot_id() < PARAM::ROBOTNUM) {
                message.setRobot(PARAM::BLUE, robot.robot_id(), saoConvert(CGeoPoint(robot.x(), robot.y())), saoConvert(robot.orientation()));
            }
        }
        for (int i = 0; i < yellowSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_yellow(i);
            if (robot.robot_id() < PARAM::ROBOTNUM) {
                message.setRobot(PARAM::YELLOW, robot.robot_id(), saoConvert(CGeoPoint(robot.x(), robot.y())), saoConvert(robot.orientation()));
            }
        }
        GlobalData::instance()->camera[message.camID].push(message);
        GlobalData::instance()->cameraUpdate[message.camID] = true;
    }
    if (collectNewVision()) {
        dealWithData();
        send();
        std::fill_n(GlobalData::instance()->cameraUpdate, PARAM::CAMERA, false);
//        publish("zss_vision_signal");
    }
}

void  CVisionModule::send() {
    static ZSData data;
    auto detectionBall = detectionFrame.mutable_balls();
    ReceiveVisionMessage result = GlobalData::instance()->maintain[0];
    if (result.ballSize > 0) {
        detectionBall->set_x(result.ball[0].pos.x());
        if (std::fabs(result.ball[0].pos.y()) < 1e-4) detectionBall->set_y(float(0.1));
        else detectionBall->set_y(result.ball[0].pos.y());//to fix a role match bug 2018.6.15
        CVector TransferVel(result.ball[0].velocity.x(), result.ball[0].velocity.y());
        detectionBall->set_vel_x(TransferVel.x());
        detectionBall->set_vel_y(TransferVel.y());
        detectionBall->set_valid(result.isBallValid);
        detectionBall->set_last_touch(GlobalData::instance()->lastTouch);
        detectionBall->set_ball_state(result.ball[0].ball_state_machine);
        detectionBall->set_raw_x(GlobalData::instance()->processBall[0].ball[0].pos.x());
        detectionBall->set_raw_y(GlobalData::instance()->processBall[0].ball[0].pos.y());
        detectionBall->set_chip_predict_x(GlobalData::instance()->maintain[0].ball[0].predict_pos.x());
        detectionBall->set_chip_predict_y(GlobalData::instance()->maintain[0].ball[0].predict_pos.y());
    } else {
        detectionBall->set_valid(false);
        detectionBall->set_x(99999);
        detectionBall->set_y(99999);
    }

    for (int team = 0; team < PARAM::TEAMS; team++) {
//        for (int i = 0; i < result.robotSize[team]; i++) {
//            if (i == PARAM::SENDROBOTNUM) break; //for sending MAX 8 car possible
        for(int id = 0;id< PARAM::ROBOTNUM;id++){//change by lzx
            Vision_DetectionRobot* robot;
            if (team == 0 ) robot = detectionFrame.add_robots_blue();
            else robot = detectionFrame.add_robots_yellow();
            robot->set_x(result.robot[team][id].pos.x());
            robot->set_y(result.robot[team][id].pos.y());
            robot->set_orientation(result.robot[team][id].angle);
            robot->set_robot_id(id);
            CVector TransferVel(result.robot[team][id].velocity.x(),
                                result.robot[team][id].velocity.y());
            robot->set_vel_x(TransferVel.x());
            robot->set_vel_y(TransferVel.y());
            robot->set_rotate_vel(result.robot[team][id].rotateVel);
            robot->set_accelerate_x(result.robot[team][id].accelerate.x());
            robot->set_accelerate_y(result.robot[team][id].accelerate.y());
            robot->set_raw_x(GlobalData::instance()->processRobot[0].robot[team][id].pos.x());
            robot->set_raw_y(GlobalData::instance()->processRobot[0].robot[team][id].pos.y());
            robot->set_raw_orientation(GlobalData::instance()->processRobot[0].robot[team][id].angle);
            robot->set_valid(result.robot[team][id].valid);
//            if(result.robot[team][id].valid!=true) qDebug()<<"a error of valid in maintain";
            robot->set_raw_vel_x(result.robot[team][id].raw_vel.x());
            robot->set_raw_vel_y(result.robot[team][id].raw_vel.y());
            robot->set_raw_rotate_vel(result.robot[team][id].rawRotateVel);
        }
    }
    for(int i = 0; i < PARAM::CAMERA; i++) {
        auto result = GlobalData::instance()->camera[i][0];
        for(int i=0;i<result.ballSize;i++) {
            auto* detectionBall = detectionFrame.add_origin_balls();
            detectionBall->set_x(result.ball[i].pos.x());
            detectionBall->set_y(result.ball[i].pos.y());
            detectionBall->set_valid(true);
            detectionBall->set_raw_x(result.ball[i].pos.x());
            detectionBall->set_raw_y(result.ball[i].pos.y());
        }
        for (int team = 0; team < PARAM::TEAMS; team++) {
            for(int id = 0;id< PARAM::ROBOTNUM;id++){
                auto robot = result.robot[team][id];
                if(robot.valid){
                    auto* msg = team == 0 ? detectionFrame.add_origin_robots_blue() : detectionFrame.add_origin_robots_yellow();
                    msg->set_x(robot.pos.x());
                    msg->set_y(robot.pos.y());
                    msg->set_orientation(robot.angle);
                    msg->set_robot_id(id);
                    msg->set_raw_x(robot.pos.x());
                    msg->set_raw_y(robot.pos.y());
                    msg->set_valid(true);
                }
            }
        }
    }
    int size = detectionFrame.ByteSize();
    data.resize(size);
    detectionFrame.SerializeToArray(data.ptr(), size);
    publish("zss_vision",data);
    detectionFrame.clear_robots_blue();
    detectionFrame.clear_robots_yellow();
    detectionFrame.clear_origin_balls();
    detectionFrame.clear_origin_robots_blue();
    detectionFrame.clear_origin_robots_yellow();
}

/**
 * @brief judge if all needed camera vision is collected
 * @return
 */
bool CVisionModule::collectNewVision() {
    for (int i = 0; i < PARAM::CAMERA; i++) {
        if (GlobalData::instance()->cameraControl[i] && !GlobalData::instance()->cameraUpdate[i])
            return false;
    }
    return true;
}
/**
 * @brief filed edgeTest
 */
void CVisionModule::edgeTest() {
    for (int i = 0; i < PARAM::CAMERA; i++) {
        if (GlobalData::instance()->cameraControl[i] == true) {
            SingleCamera& currentCamera = GlobalData::instance()->cameraMatrix[i];
            for(int j = 0; j < GlobalData::instance()->camera[i][0].ballSize; j++) {
                Ball currentball = GlobalData::instance()->camera[i][0].ball[j];
                if (currentball.pos.x() < currentCamera.leftedge.min) currentCamera.leftedge.min = currentball.pos.x();
                if (currentball.pos.x() > currentCamera.rightedge.min) currentCamera.rightedge.min = currentball.pos.x();
                if (currentball.pos.y() > currentCamera.upedge.min) currentCamera.upedge.min = currentball.pos.y();
                if (currentball.pos.y() < currentCamera.downedge.min) currentCamera.downedge.min = currentball.pos.y();
            }
        }
    }
    for (int i = 0; i < PARAM::CAMERA; i++) {
        if (GlobalData::instance()->cameraControl[i] == true) {
            SingleCamera& currentCamera = GlobalData::instance()->cameraMatrix[i];
            currentCamera.leftedge.max = currentCamera.leftedge.min;
            currentCamera.rightedge.max = currentCamera.rightedge.min;
            currentCamera.upedge.max = currentCamera.upedge.min;
            currentCamera.downedge.max = currentCamera.downedge.min;
            for (int j = 0; j < PARAM::CAMERA; j++)
                if(GlobalData::instance()->cameraControl[j] == true && i != j) {
                    SingleCamera otherCamera = GlobalData::instance()->cameraMatrix[j];
                    if (currentCamera.leftedge.max < otherCamera.rightedge.min && currentCamera.campos.x() > otherCamera.rightedge.min)
                        currentCamera.leftedge.max = otherCamera.rightedge.min;
                    if (currentCamera.rightedge.max > otherCamera.leftedge.min && currentCamera.campos.x() < otherCamera.leftedge.min)
                        currentCamera.rightedge.max = otherCamera.leftedge.min;
                    if (currentCamera.upedge.max > otherCamera.downedge.min && currentCamera.campos.y() < otherCamera.downedge.min)
                        currentCamera.upedge.max = otherCamera.downedge.min;
                    if (currentCamera.downedge.max < otherCamera.upedge.min && currentCamera.campos.y() > otherCamera.upedge.min)
                        currentCamera.downedge.max = otherCamera.upedge.min;
                }
        }
    }
}
/**
 * @brief getFPS for front
 * @return
 */
quint16 CVisionModule::getFPS() {
    static QElapsedTimer timer;
    static bool ifStart = false;
    static quint64 lastCount;
    static quint16 result;
    if (!ifStart) {
        ifStart = true;
        timer.start();
        lastCount = counter;
        return 0;
    }
    result = (counter - lastCount) * 1000.0 / timer.restart();
    lastCount = counter;
    return result;
}
