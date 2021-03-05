#include "CommandInterface.h"
#include "ServerInterface.h"
#include "OptionModule.h"
#include <iostream>
#include "grSim_Packet.pb.h"
#include "zss_cmd.pb.h"
#include "staticparams.h"
#include "game_state.h"
#include "parammanager.h"
#include "staticparams.h"
#include <cmath>
#include <QUdpSocket>
#include "RobotSensor.h"
#include "VisionModule.h"
#include <thread>
CCommandInterface* CCommandInterface::_instance = 0;
namespace {
bool side;
bool IS_SIM = false;
bool USEDIR;
bool COMMANDDEBUG;
int SELF_PORT = 0;
int CHIP_ANGLE = 1;
int TEAM;
const double DEBUG_X = 0;
const double DEBUG_TEXT_LENGTH = 10.0;
const double DEBUG_TITLE = 600;
const double DEBUG_X_STEP = PARAM::Field::PITCH_LENGTH / 32;
const double DEBUG_Y = PARAM::Field::PITCH_WIDTH / 2 - 500;
const double DEBUG_Y_STEP = -PARAM::Field::PITCH_WIDTH / 35;
const double DEBUG_SIZE = 40;
const double DEBUG_WEIGHT = 40;
ZSS::Protocol::Robots_Command robots_command;
std::thread *_thread = nullptr;
double Normalize(double angle)
{
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

CCommandInterface::CCommandInterface(const COptionModule *pOption, QObject *parent)
    : pOption(pOption), QObject(parent),
      receiveSocket(nullptr), command_socket(nullptr) {
    bool isYellow = false;
    ZSS::ZParamManager::instance()->loadParam(SELF_PORT, "Ports/SimSelfPort", 30015);
    ZSS::ZParamManager::instance()->loadParam(CHIP_ANGLE, "Simulator/ChipAngle", 45);
    ZSS::ZParamManager::instance()->loadParam(isYellow, "ZAlert/IsYellow", false);
    ZSS::ZParamManager::instance()->loadParam(IS_SIM, "Alert/IsSimulation", false);
    ZSS::ZParamManager::instance()->loadParam(side, "ZAlert/IsRight", false);
    ZSS::ZParamManager::instance()->loadParam(USEDIR, "IMU/useDir", true);
    ZSS::ZParamManager::instance()->loadParam(COMMANDDEBUG, "Debug/CommandDebug", false);
    TEAM = isYellow ? PARAM::YELLOW : PARAM::BLUE;
    command_socket = new QUdpSocket();
    receiveSocket = new QUdpSocket();
    receiveSocket->bind(QHostAddress::AnyIPv4, ZSS::Athena::CONTROL_BACK_RECEIVE[TEAM], QUdpSocket::ShareAddress);
    _thread = new std::thread([ & ] {receiveInformation();});
    _thread->detach();
}

CCommandInterface::~CCommandInterface(void) {
    delete receiveSocket;
    delete command_socket;
    delete _thread;
    //delete _pServerIf;
}

void CCommandInterface::setSpeed(int num, double dribble, double vx, double vy, double vr, bool use_dir) {
    int number = num;
    if (number < 0 || number > PARAM::Field::MAX_PLAYER - 1) {
        //std::cout << "Robot Number Error in Simulator setSpeed" << number<< std::endl;
        return;
    }
    commands[number].dribble_spin = dribble;
    commands[number].velocity_x = vx;
    commands[number].velocity_y = vy;
    commands[number].velocity_r = vr;
    commands[number].use_dir = use_dir;
//    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, 800), QString("CommandInterface.cpp : CMDvx: %1, CMDvy: %2, CMDvr: %3").arg(vx).arg(vy).arg(vr).toLatin1(), COLOR_GREEN);

}
void CCommandInterface::setKick(int num, double kp, double cp) {
    int number = num;
    if (number < 0 || number > PARAM::Field::MAX_PLAYER - 1) {
        //std::cout << "Robot Number Error in Simulator setKick" << std::endl;
        return;
    }
    commands[number].flat_kick = kp;
    commands[number].chip_kick = cp;
}

void CCommandInterface::sendCommands() {
//    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-400,-200),"COMMAND_VALID : ",COLOR_GRAY);
    for (int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
//        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-216+i*13,-200),VisionModule::Instance()->ourPlayer(i).Valid()?"1":"0",COLOR_GRAY);
        if(!VisionModule::Instance()->ourPlayer(i).Valid()){
            continue;
        }
        auto robot_command = robots_command.add_command();
        robot_command->set_robot_id(i);
        robot_command->set_velocity_x(commands[i].velocity_x);
        robot_command->set_velocity_y(commands[i].velocity_y);
        robot_command->set_velocity_r(commands[i].velocity_r);
        robot_command->set_dribbler_spin(commands[i].dribble_spin);

        if(USEDIR && commands[i].use_dir){
            robot_command->set_use_dir(true);
            if(side){
                robot_command->set_velocity_r(Normalize(commands[i].velocity_r+PARAM::Math::PI));
            }
        }
        else {
            robot_command->set_use_dir(false);
            robot_command->set_velocity_r(commands[i].velocity_r);
        }
        //qDebug()<<"command use_dir: "<<robot_command->use_dir();
        if(commands[i].dribble_spin >=1){
            GDebugEngine::Instance()->gui_debug_arc(VisionModule::Instance()->ourPlayer(i).RawPos(),5,0,360,COLOR_BLACK);
        }
        if(commands[i].chip_kick < 0.001) { //flat kick
            //qDebug()<<"id: "<<i<<commands[i].flat_kick;
            robot_command->set_kick(false);
            robot_command->set_power(commands[i].flat_kick);
        }
        else {
            robot_command->set_kick(true);
            robot_command->set_power(commands[i].chip_kick);
        }
    }
    if(COMMANDDEBUG){
        showCommands();
    }
    int size = ::robots_command.ByteSize();
    QByteArray data(size, 0);
    ::robots_command.SerializeToArray(data.data(), size);
    command_socket->writeDatagram(data.data(), size, QHostAddress(ZSS::LOCAL_ADDRESS), ZSS::Athena::CONTROL_SEND[TEAM]);
    ::robots_command.Clear();
    memset(commands,0,sizeof(RobotCommand)*PARAM::Field::MAX_PLAYER);
}

void CCommandInterface::receiveInformation() {
    ZSS::Protocol::Robot_Status robot_status;
    QByteArray datagram;
    QHostAddress address;
    quint16 udp_port;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        while (receiveSocket->state() == QUdpSocket::BoundState && receiveSocket->hasPendingDatagrams()) {
            datagram.resize(receiveSocket->pendingDatagramSize());
            receiveSocket->readDatagram(datagram.data(), datagram.size(), &address, &udp_port);
            robot_status.ParseFromArray(datagram, datagram.size());
            auto&& id = robot_status.robot_id();
            if(!(id >= 0 && id < PARAM::Field::MAX_PLAYER)) {
                qDebug() << "ERROR received error robot id in command interface." << id;
                continue;
            }
            auto& msg = RobotSensor::Instance()->robotMsg[id];
            msg._mutex.lock();
            msg.infrared = robot_status.infrared();
            msg.chip_kick = robot_status.chip_kick() ? 5 : (msg.chip_kick);
            msg.flat_kick = robot_status.flat_kick() ? 5 : (msg.flat_kick);
            msg._mutex.unlock();
//            qDebug() << address << udp_port << id << msg.infrared
//                     << msg.chip_kick
//                     << msg.flat_kick;
        }
    }
}

CCommandInterface* CCommandInterface::instance(const COptionModule *pOption) {
    if(_instance == 0)
        _instance = new CCommandInterface(pOption);
    return _instance;
}

void CCommandInterface::destruct() {
    if(_instance)
        delete _instance;
}

void CCommandInterface::showCommands(){

    QString ID = "ID";
    QString Velocity_X = "VX";
    QString Velocity_Y = "VY";
    QString Velocity_R = "VR";
    QString Kick_Power = "KP";
    QString Chip_Power = "CP";
    QString Use_Dir = "UD";
    QString isDribbling = "DRI";

    QTextStream ID_stream(&ID);
    QTextStream Velocity_X_stream(&Velocity_X);
    QTextStream Velocity_Y_stream(&Velocity_Y);
    QTextStream Velocity_R_stream(&Velocity_R);
    QTextStream Kick_Power_stream(&Kick_Power);
    QTextStream Chip_Power_stream(&Chip_Power);
    QTextStream Use_Dir_stream(&Use_Dir);
    QTextStream isDribbling_stream(&isDribbling);

    for(int i=0;i<PARAM::Field::MAX_PLAYER;i++){
        ID_stream << qSetFieldWidth(6) << i;
        Velocity_X_stream << qSetFieldWidth(6) << QString::number(commands[i].velocity_x,'f',0);
        Velocity_Y_stream << qSetFieldWidth(6) << QString::number(commands[i].velocity_y,'f',0);
        Velocity_R_stream << qSetFieldWidth(6) << QString::number(commands[i].velocity_r,'f',2);
        Kick_Power_stream << qSetFieldWidth(6) << QString::number(commands[i].flat_kick,'f',0);
        Chip_Power_stream << qSetFieldWidth(6) << QString::number(commands[i].chip_kick,'f',0);
        isDribbling_stream << qSetFieldWidth(6) << QString::number(commands[i].dribble_spin,'f',0);
        Use_Dir_stream << qSetFieldWidth(6) << (commands[i].use_dir ? "1" : "0");
    }
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(DEBUG_X , DEBUG_Y + DEBUG_Y_STEP * -1), ID.toLatin1() , COLOR_YELLOW, DEBUG_WEIGHT, DEBUG_SIZE);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(DEBUG_X , DEBUG_Y + DEBUG_Y_STEP * 0), Velocity_X.toLatin1() , COLOR_YELLOW, DEBUG_WEIGHT, DEBUG_SIZE);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(DEBUG_X , DEBUG_Y + DEBUG_Y_STEP * 1), Velocity_Y.toLatin1() , COLOR_YELLOW, DEBUG_WEIGHT, DEBUG_SIZE);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(DEBUG_X , DEBUG_Y + DEBUG_Y_STEP * 2), Velocity_R.toLatin1() , COLOR_YELLOW, DEBUG_WEIGHT, DEBUG_SIZE);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(DEBUG_X , DEBUG_Y + DEBUG_Y_STEP * 3), Kick_Power.toLatin1() , COLOR_YELLOW, DEBUG_WEIGHT, DEBUG_SIZE);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(DEBUG_X , DEBUG_Y + DEBUG_Y_STEP * 4), Chip_Power.toLatin1() , COLOR_YELLOW, DEBUG_WEIGHT, DEBUG_SIZE);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(DEBUG_X , DEBUG_Y + DEBUG_Y_STEP * 5), Use_Dir.toLatin1() , COLOR_YELLOW, DEBUG_WEIGHT, DEBUG_SIZE);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(DEBUG_X , DEBUG_Y + DEBUG_Y_STEP * 6), isDribbling.toLatin1() , COLOR_YELLOW, DEBUG_WEIGHT, DEBUG_SIZE);
}
