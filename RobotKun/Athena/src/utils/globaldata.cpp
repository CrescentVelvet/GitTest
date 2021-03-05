#include "globaldata.h"
#include "parammanager.h"
#include "staticparams.h"
#include "actionmodule.h"
#include <QDateTime>
#include <chrono>
#include <thread>
namespace  {
bool IMUDebug;
auto vpm = ZSS::VParamManager::instance();
auto zpm = ZSS::ZParamManager::instance();
auto ipm = ZSS::IParamManager::instance();
//std::thread* antiThread = nullptr;
ZSS::Protocol::Robots_Command command;
double Normalize(double angle) {
    const double M_2PI = PARAM::Math::PI * 2;
    while( angle > PARAM::Math::PI ) {
        angle -= M_2PI;
    }

    while( angle <= -PARAM::Math::PI ) {
        angle += M_2PI;
    }
    return angle;
}
}
CGlobalData::CGlobalData(): ctrlC(false)
    , maintain(ZSS::Athena::Vision::MAINTAIN_STORE_BUFFER)
    , processBall(ZSS::Athena::Vision::BALL_STORE_BUFFER)
    , processRobot(ZSS::Athena::Vision::ROBOT_STORE_BUFFER)
    , cleancount(0)
    , MedusaOn(false) {
    QString key = "";
    for(int i = 0; i < PARAM::CAMERA; i++) {
        cameraMatrix[i].id = i;
    }
    imuend = false;
    for(int i = 0; i<PARAM::TEAMS; i++) {
        imucleaning[i] = false;
    }
}
CGeoPoint CGlobalData::saoConvert(CGeoPoint originPoint) {
    CGeoPoint result;
    switch (saoAction) {
    case 0:
        result.setX(originPoint.x());
        result.setY(originPoint.y());
        break;
    case 1:
        result.setX(originPoint.y() + 3000);
        result.setY(-originPoint.x());
        break;
    case 2:
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

void CGlobalData::imuClean(int team) {
    imucleaning[team] = true;
    zpm->loadParam(IMUDebug, "IMU/IMUDebug", false);
    if(team) {
        ZSS::ZActionModule::instance()->connectRadio(team, 6);
    }
    else {
        ZSS::ZActionModule::instance()->connectRadio(team, 8);
    }
    while(!imuend){
        int i;
        if(IMUDebug) qDebug()<<"normal imu clean"<<team;
        for(i=0;i<PARAM::ROBOTNUM;i++) {
            if(robotInformation[team][i].needclean && !robotInformation[team][i].imureceive){
                if(processRobot[0].robot[team][i].valid){
                    if(cleancount > 5){
                        robotInformation[team][i].needreport = false;
                        robotInformation[team][i].imucleaned = false;
                        cleancount = 0;
                        continue;
                    }
                    if(IMUDebug) qDebug()<<"imu clean team"<<team<<" robot "<<i;
                    imuClean(team, i);
                    //std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    cleancount++;
                    i--;
                }
                else {                    
                    robotInformation[team][i].needclean = false;
                    //qDebug()<<"fuck robot"<<i<<"cleaning situation: "<<cleaned[team];
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if(MedusaOn) normalTracking(team);
        }
        for(i=0;i<PARAM::ROBOTNUM;i++) {
            robotInformation[team][i].needreport = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        //qDebug()<<"success clean";
    }
    imucleaning[team] = false;
}
void CGlobalData::testforIMU(){
    while(!imuend) {
        imuZero[0][3] = 0;
        qDebug()<<"I am coming";
        std::this_thread::sleep_for(std::chrono::seconds(4));
    }
}
void CGlobalData::normalTracking(int team){
    double diff;
//    antiThread = new std::thread([ = ] {testforIMU();});
//    antiThread->detach();
    while (!imuend) {
        if(IMUDebug) qDebug()<<"normal tracking";
        for(int i=0;i<PARAM::ROBOTNUM;i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            if(!MedusaOn) return;
            if(maintain.validSize()<1 || !processRobot[0].robot[team][i].valid) continue;
            diff = processRobot[0].robot[team][i].angle - robotCommand[team][0].robotSpeed[i].vr;
            if(fabs(Normalize(diff))>0.2) {
                robotInformation[team][i].imucleaned = false;
                if(IMUDebug) qDebug()<<"robot id: "<<i<<"dir diff: "<<diff<<"velovity: "<<robotCommand[team][0].robotSpeed[i].mod<<"rot vel: "<<maintain[-1].robot[team][i].rotateVel;
                if((maintain[-1].robot[team][i].velocity.mod()<1000) && maintain[-1].robot[team][i].rotateVel < 0.4) {
                    if(IMUDebug) qDebug()<<"over check robot id: "<<i;
                    robotInformation[team][i].imureceive = false;
                    robotInformation[team][i].needreport = true;
    //                qDebug()<<"successfully send";
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            else {
                if(IMUDebug) qDebug()<<"wheel tracking checking: "<<i;
                robotInformation[team][i].needreport = true;
                if(maintain[-1].robot[team][i].rotateVel < 0.6)
                robotInformation[team][i].imureceive = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
//                imuClean(team, i);
            }
        }
    }
}

void CGlobalData::imuClean(int team, int i) {
    command.Clear();
    robotInformation[team][i].needreport = true ;
    auto robot_command = command.add_command();
    robot_command->set_robot_id(i);
    robot_command->set_use_dir(false);
    robot_command->set_velocity_r(0);
    robot_command->set_velocity_x(0);
    robot_command->set_velocity_y(0);
    robot_command->set_power(0);
    ZSS::ZActionModule::instance()->sendLegacy(team, command);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void CGlobalData::imureset(int team) {
    for(int i=0;i<PARAM::ROBOTNUM;i++) {
        robotInformation[team][i].needclean = true;
        robotInformation[team][i].imureceive = false;
        robotInformation[team][i].imucleaned = false;
        robotInformation[team][i].imucleaned = false;
    }
}

void CGlobalData::saveImuData() {
    QString key = "";
    for(int i = 0; i < PARAM::ROBOTNUM; i++) {
        for(int j = 0; j < 2; j++) {
           key = QString("Team%1/Robot%2").arg(j).arg(i);
           ipm->changeParam(key, imuZero[j][i]);
        }
    }
}

void CGlobalData::setImuData() {
    QString key = "";
    for(int i = 0; i < PARAM::ROBOTNUM; i++) {
        for(int j = 0; j < 2; j++) {
           key = QString("Team%1/Robot%2").arg(j).arg(i);
           ipm->loadParam(imuZero[j][i], key, 0);
        }
    }
}

void CGlobalData::saoConvertEdge() {
    switch (saoAction) {
    case 0:
        for (int i = 0; i < PARAM::CAMERA; i++) {
            vpm->loadParam(cameraMatrix[i].leftedge.min,    "Camera" + QString::number(i) + "Leftmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].leftedge.max,    "Camera" + QString::number(i) + "Leftmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].rightedge.min,   "Camera" + QString::number(i) + "Rightmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].rightedge.max,   "Camera" + QString::number(i) + "Rightmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].upedge.min,      "Camera" + QString::number(i) + "Upmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].upedge.max,      "Camera" + QString::number(i) + "Upmax",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].downedge.min,    "Camera" + QString::number(i) + "Downmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].downedge.max,    "Camera" + QString::number(i) + "Downmax",	cameraMatrix[i].campos.y());
        }
        break;
    case 1:
        for (int i = 0; i < PARAM::CAMERA; i++) {
            CameraEdge leftedge, rightedge, upedge, downedge;
            vpm->loadParam(leftedge.min,    "Camera" + QString::number(i) + "Leftmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(leftedge.max,    "Camera" + QString::number(i) + "Leftmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(rightedge.min,   "Camera" + QString::number(i) + "Rightmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(rightedge.max,   "Camera" + QString::number(i) + "Rightmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(upedge.min,      "Camera" + QString::number(i) + "Upmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(upedge.max,      "Camera" + QString::number(i) + "Upmax",	cameraMatrix[i].campos.y());
            vpm->loadParam(downedge.min,    "Camera" + QString::number(i) + "Downmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(downedge.max,    "Camera" + QString::number(i) + "Downmax",	cameraMatrix[i].campos.y());
            cameraMatrix[i].leftedge.min	=	downedge.min    + 3000;
            cameraMatrix[i].leftedge.max	=	downedge.max    + 3000;
            cameraMatrix[i].rightedge.min	=	upedge.min      + 3000;
            cameraMatrix[i].rightedge.max	=	upedge.max      + 3000;
            cameraMatrix[i].upedge.min   	=	-leftedge.min        ;
            cameraMatrix[i].upedge.max   	=	-leftedge.max        ;
            cameraMatrix[i].downedge.min	=	-rightedge.min       ;
            cameraMatrix[i].downedge.max	=	-rightedge.max       ;
        }
        break;
    case 2:
        for (int i = 0; i < PARAM::CAMERA; i++) {
            vpm->loadParam(cameraMatrix[i].leftedge.min,    "Camera" + QString::number(i) + "Leftmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].leftedge.max,    "Camera" + QString::number(i) + "Leftmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].rightedge.min,   "Camera" + QString::number(i) + "Rightmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].rightedge.max,   "Camera" + QString::number(i) + "Rightmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].upedge.min,      "Camera" + QString::number(i) + "Upmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].upedge.max,      "Camera" + QString::number(i) + "Upmax",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].downedge.min,    "Camera" + QString::number(i) + "Downmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].downedge.max,    "Camera" + QString::number(i) + "Downmax",	cameraMatrix[i].campos.y());
            cameraMatrix[i].leftedge.min	=	cameraMatrix[i].leftedge.min  * 3 / 2;
            cameraMatrix[i].leftedge.max	=	cameraMatrix[i].leftedge.max  * 3 / 2;
            cameraMatrix[i].rightedge.min	=	cameraMatrix[i].rightedge.min * 3 / 2;
            cameraMatrix[i].rightedge.max	=	cameraMatrix[i].rightedge.max * 3 / 2;
            cameraMatrix[i].upedge.min   	=	cameraMatrix[i].upedge.min    * 3 / 2;
            cameraMatrix[i].upedge.max   	=	cameraMatrix[i].upedge.max    * 3 / 2;
            cameraMatrix[i].downedge.min	=	cameraMatrix[i].downedge.min  * 3 / 2;
            cameraMatrix[i].downedge.max	=	cameraMatrix[i].downedge.max  * 3 / 2;
        }
        break;
    }
}

void CGlobalData::setCameraMatrix(bool real) {
    zpm->loadParam(saoAction, "Alert/SaoAction", 0);
    if (real) {
        for (int i = 0; i < PARAM::CAMERA; i++) {
            double x, y;
            vpm->loadParam(x, "Camera" + QString::number(i) + "CenterX", 0);
            vpm->loadParam(y, "Camera" + QString::number(i) + "CenterY", 0);
            cameraMatrix[i].fillCenter(saoConvert(CGeoPoint(x, y)));
        }
        //saoConvertEdge();
        CameraInit();
    } else {            //for Sim
        cameraMatrix[0].fillCenter(saoConvert(CGeoPoint(PARAM::Field::PITCH_LENGTH/4, PARAM::Field::PITCH_WIDTH/4))); //(-4500,2250);
        cameraMatrix[1].fillCenter(saoConvert(CGeoPoint(PARAM::Field::PITCH_LENGTH/4, -PARAM::Field::PITCH_WIDTH/4))); //(-1500,2250);
        cameraMatrix[2].fillCenter(saoConvert(CGeoPoint(-PARAM::Field::PITCH_LENGTH/4, -PARAM::Field::PITCH_WIDTH/4))); //(1500,2250);
        cameraMatrix[3].fillCenter(saoConvert(CGeoPoint(-PARAM::Field::PITCH_LENGTH/4, PARAM::Field::PITCH_WIDTH/4))); //(4500,2250);
        for (int i = 0; i < PARAM::CAMERA; i++) {
            cameraMatrix[i].leftedge.min = cameraMatrix[i].campos.x() - PARAM::Field::PITCH_LENGTH/4;
            cameraMatrix[i].leftedge.max = cameraMatrix[i].campos.x() - PARAM::Field::PITCH_LENGTH/4 - 100;
            cameraMatrix[i].rightedge.min = cameraMatrix[i].campos.x() + PARAM::Field::PITCH_LENGTH/4;
            cameraMatrix[i].rightedge.max = cameraMatrix[i].campos.x() + PARAM::Field::PITCH_LENGTH/4 + 100;
            cameraMatrix[i].downedge.min = cameraMatrix[i].campos.y() - PARAM::Field::PITCH_WIDTH/4;
            cameraMatrix[i].downedge.max = cameraMatrix[i].campos.y() - PARAM::Field::PITCH_WIDTH/4 - 100;
            cameraMatrix[i].upedge.min = cameraMatrix[i].campos.y() + PARAM::Field::PITCH_WIDTH/4;
            cameraMatrix[i].upedge.max = cameraMatrix[i].campos.y() + PARAM::Field::PITCH_WIDTH/4 + 100;
        }
    }
}

void CGlobalData::CameraInit() {
    for (int i = 0; i < 2; i++) {
        cameraMatrix[i].leftedge.min = cameraMatrix[i].campos.x() - PARAM::Field::PITCH_LENGTH/4;
        cameraMatrix[i].leftedge.max = cameraMatrix[i].campos.x() - PARAM::Field::PITCH_LENGTH/4 - 100;
        cameraMatrix[i].rightedge.min = cameraMatrix[i].campos.x() + PARAM::Field::PITCH_LENGTH/4;
        cameraMatrix[i].rightedge.max = cameraMatrix[i].campos.x() + PARAM::Field::PITCH_LENGTH/4 + 100;
        cameraMatrix[i].downedge.min = cameraMatrix[i].campos.y() - PARAM::Field::PITCH_WIDTH/2;
        cameraMatrix[i].downedge.max = cameraMatrix[i].campos.y() - PARAM::Field::PITCH_WIDTH/2 - 100;
        cameraMatrix[i].upedge.min = cameraMatrix[i].campos.y() + PARAM::Field::PITCH_WIDTH/2;
        cameraMatrix[i].upedge.max = cameraMatrix[i].campos.y() + PARAM::Field::PITCH_WIDTH/2 + 100;
    }
}
