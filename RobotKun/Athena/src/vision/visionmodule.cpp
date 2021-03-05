#include "visionmodule.h"
#include "globaldata.h"
#include "montage.h"
#include "maintain.h"
#include "dealball.h"
#include "dealrobot.h"
#include "messages_robocup_ssl_wrapper.pb.h"
#include "field.h"
#include "parammanager.h"
#include "globalsettings.h"
#include "rec_recorder.h"
#include <QElapsedTimer>
#include "networkinterfaces.h"
#include <QtDebug>
#include "zautoshootfit.h"
#include "ballstate.h"

namespace {
auto zpm = ZSS::ZParamManager::instance();
auto vpm = ZSS::VParamManager::instance();
bool IS_SIMULATION = false; // 是否仿真
}

Semaphore chipsolverEvent;

/**
 * @brief CVisionModule consturctor
 * @param parent
 */
CVisionModule::CVisionModule(QObject *parent)
    : QObject(parent)
    , udpReceiveSocket()
    , udpSendSocket()
    , IF_EDGE_TEST(false)
    , interface(0){
    std::fill_n(GlobalData::instance()->cameraUpdate, PARAM::CAMERA, false);
    std::fill_n(GlobalData::instance()->cameraControl, PARAM::CAMERA, false);
    std::fill_n(GlobalData::instance()->processControl, 3, true);
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION, "Alert/IsSimulation", false);
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
    //qDebug() << "VisionPort : " << vision_port;
    udpReceiveSocket.bind(QHostAddress::AnyIPv4, vision_port, QUdpSocket::ShareAddress);
    udpReceiveSocket.joinMulticastGroup(QHostAddress(ZSS::SSL_ADDRESS),ZNetworkInterfaces::instance()->getFromIndex(interface));
    connect(&udpReceiveSocket, SIGNAL(readyRead()), this, SLOT(storeData()), Qt::DirectConnection);
    for (int i = 0; i < PARAM::CAMERA; i++) {
        ReceiveVisionMessage temp;
        GlobalData::instance()->camera[i].push(temp);
    }
}
void CVisionModule::setInterfaceIndex(const int index){ interface = index; }
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
        //qDebug()<<"saving data";
    }
    disconnect(&udpReceiveSocket);
    udpReceiveSocket.abort();
    udpSendSocket.abort();
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
void CVisionModule::storeData() {
    static QByteArray datagram;
    while (udpReceiveSocket.hasPendingDatagrams()) {
        datagram.resize(udpReceiveSocket.pendingDatagramSize());
        udpReceiveSocket.readDatagram(datagram.data(), datagram.size());
        parse((void*)datagram.data(), datagram.size());
    }
}
/**
 * @brief process data
 * @return
 */
bool CVisionModule::dealWithData() {
    counter++;
    edgeTest();
    DealBall::instance()->run();
    DealRobot::instance()->run();
    Maintain::instance()->run();

    AutoShootFit::instance()->run();

    //qDebug()<<GlobalData::instance()->processRobot[0].robot[1][10].angle;
    return true;
}
/**
 * @brief parse camera vision message
 * @param ptr
 * @param size
 */
void CVisionModule::parse(void * ptr, int size) {
    static SSL_WrapperPacket packet;
    ReceiveVisionMessage message;
    packet.ParseFromArray(ptr, size);
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
//            qDebug() << ball.z();
            if (GlobalSettings::instance()->inChosenArea(saoConvert(CGeoPoint(ball.x(), ball.y())))) {
                // use camera projection point instead of real pos in chip simulation
                // when chip, ball.z > 22
                if(IS_SIMULATION && ball.z() > 22){
                    double cam_x = GlobalData::instance()->cameraMatrix[detection.camera_id()].campos.x();
                    double cam_y = GlobalData::instance()->cameraMatrix[detection.camera_id()].campos.y();
                    double cam_z = GlobalData::instance()->cameraMatrix[detection.camera_id()].height;

                    CGeoPoint _ball = saoConvert(CGeoPoint(ball.x(), ball.y()));
                    double x_p = (cam_z * _ball.x() - ball.z() * cam_x) / (cam_z - ball.z());
                    double y_p = (cam_z * _ball.y() - ball.z() * cam_y) / (cam_z - ball.z());

                    message.addBall(CGeoPoint(x_p, y_p));
                }
                else{
                    message.addBall(saoConvert(CGeoPoint(ball.x(), ball.y())));
                }
            }
        }
        for (int i = 0; i < blueSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_blue(i);
            if (GlobalSettings::instance()->inChosenArea(saoConvert(CGeoPoint(robot.x(), robot.y())))
                    && robot.robot_id() < PARAM::ROBOTNUM) {
                message.setRobot(PARAM::BLUE, robot.robot_id(), saoConvert(CGeoPoint(robot.x(), robot.y())), saoConvert(robot.orientation()));
            }
        }
        for (int i = 0; i < yellowSize; i++) {
            const SSL_DetectionRobot& robot = detection.robots_yellow(i);
            if (GlobalSettings::instance()->inChosenArea(saoConvert(CGeoPoint(robot.x(), robot.y())))
                    && robot.robot_id() < PARAM::ROBOTNUM) {
                message.setRobot(PARAM::YELLOW, robot.robot_id(), saoConvert(CGeoPoint(robot.x(), robot.y())), saoConvert(robot.orientation()));
            }
        }
        GlobalData::instance()->camera[message.camID].push(message);
        GlobalData::instance()->cameraUpdate[message.camID] = true;
    }
    if (collectNewVision()) {
        checkCommand();
        dealWithData();
        udpSend();

        // another thread for chipsolver
        chipsolverEvent.Signal();

        ZRecRecorder::instance()->store();
        std::fill_n(GlobalData::instance()->cameraUpdate, PARAM::CAMERA, false);
        emit needDraw();
    }
}

void CVisionModule::checkCommand() {
    for (int team = 0; team < PARAM::TEAMS; team++) {
        auto commands = ZCommunicator::instance()->getCommands(team);
        if (commands.valid) {
            GlobalData::instance()->robotCommand[team].push(commands);
            GlobalData::instance()->commandMissingFrame[team] = 0;
        } else {
            commands = RobotCommands();
            GlobalData::instance()->robotCommand[team].push(commands);
            GlobalData::instance()->commandMissingFrame[team] >= 20 ?
            GlobalData::instance()->commandMissingFrame[team] = 20 :
            GlobalData::instance()->commandMissingFrame[team]++;
        }
    }
    ZCommunicator::instance()->clearCommands();
}

void  CVisionModule::udpSend() {
    //udp start
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
        if(result.ball[0].ball_state_machine == State::_chip_pass){
            detectionBall->set_chip_predict_x(GlobalData::instance()->maintain[0].ball[0].predict_pos.x());
            detectionBall->set_chip_predict_y(GlobalData::instance()->maintain[0].ball[0].predict_pos.y());
            detectionBall->set_chip_best_predict_x(GlobalData::instance()->maintain[0].ball[0].best_predict_pos.x());
            detectionBall->set_chip_best_predict_y(GlobalData::instance()->maintain[0].ball[0].best_predict_pos.y());
            detectionBall->set_real_x(GlobalData::instance()->maintain[0].ball[0].real_pos.x());
            detectionBall->set_real_y(GlobalData::instance()->maintain[0].ball[0].real_pos.y());
            detectionBall->set_real_z(GlobalData::instance()->maintain[0].ball[0].real_pos.z());
            detectionBall->set_chip_vel_x(GlobalData::instance()->maintain[0].ball[0].chip_velocity.x());
            detectionBall->set_chip_vel_y(GlobalData::instance()->maintain[0].ball[0].chip_velocity.y());
            detectionBall->set_chip_vel_z(GlobalData::instance()->maintain[0].ball[0].chip_velocity.z());
            detectionBall->set_chip_second_pos_x(GlobalData::instance()->maintain[0].ball[0].chip_second_pos.x());
            detectionBall->set_chip_second_pos_y(GlobalData::instance()->maintain[0].ball[0].chip_second_pos.y());
            detectionBall->set_chip_fly_time(GlobalData::instance()->maintain[0].ball[0].chip_fly_time);
            detectionBall->set_chip_rest_fly_time(GlobalData::instance()->maintain[0].ball[0].chip_rest_time);
        }
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
            if(GlobalData::instance()->robotInformation[team][id].imuavailable){
                robot->set_imu_dir(GlobalData::instance()->robotInformation[team][id].imuDir);
                robot->set_imu_rotate_vel(GlobalData::instance()->robotInformation[team][id].imuRotateDir);
                GlobalData::instance()->robotInformation[team][id].imuavailable = false;
            }
            else {
                robot->set_imu_dir(result.robot[team][id].angle);
                robot->set_imu_rotate_vel(result.robot[team][id].rotateVel);
            }
//            if(result.robot[team][id].valid!=true) qDebug()<<"a error of valid in maintain";
            robot->set_raw_vel_x(result.robot[team][id].raw_vel.x());
            robot->set_raw_vel_y(result.robot[team][id].raw_vel.y());
            robot->set_raw_rotate_vel(result.robot[team][id].rawRotateVel);
        }
    }
    int size = detectionFrame.ByteSize();
    QByteArray buffer(size, 0);
    detectionFrame.SerializeToArray(buffer.data(), buffer.size());
    GlobalData::instance()->ctrlCMutex.lock();
    bool sw = GlobalData::instance()->ctrlC;
    GlobalData::instance()->ctrlCMutex.unlock();
    if(!sw) { //需要发两个端口以供对打
        udpSendSocket.writeDatagram(buffer.data(), buffer.size(), QHostAddress("127.0.0.1"), ZSS::Athena::VISION_SEND[0]);
        udpSendSocket.writeDatagram(buffer.data(), buffer.size(), QHostAddress("127.0.0.1"), ZSS::Athena::VISION_SEND[1]);
        udpSendSocket.writeDatagram(buffer.data(), buffer.size(), QHostAddress("127.0.0.1"), ZSS::Athena::SEND_TO_PYTHON);
    }
    detectionFrame.clear_robots_blue();
    detectionFrame.clear_robots_yellow();
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
    //qDebug()<<"edge test!";
    for (int i = 0; i < PARAM::CAMERA; i++) {
        if (GlobalData::instance()->cameraControl[i] == true) {
            //qDebug()<<"camera"<<i<<" is testing!";
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
