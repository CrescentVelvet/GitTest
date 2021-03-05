#include <QUdpSocket>
#include "staticparams.h"
#include "RefereeBoxIf.h"
#include <iostream>
#include "playmode.h"
#include "ssl_referee.pb.h"
#include "parammanager.h"
#include "GDebugEngine.h"
#include <thread>
namespace {
    int REFEREE_PORT = 10003;

    struct sCMD_TYPE {
        char cmd;
        unsigned int step;
    };

    struct stGamePacket {
        char cmd;
        unsigned char cmd_counter;
        unsigned char goals_blue;
        unsigned char goals_yellow;
        unsigned short time_remaining;
    };
    int VECTOR = 1;
}

CRefereeBoxInterface::~CRefereeBoxInterface() {
    stop();
}

CRefereeBoxInterface::CRefereeBoxInterface():_playMode(PMNone) {
//    ZSS::ZParamManager::instance()->loadParam(REFEREE_PORT,"AlertPorts/RefereePort",10003);
    ZSS::ZParamManager::instance()->loadParam(REFEREE_PORT,"AlertPorts/ZSS_RefereePort",39991);
    bool isSimulation;
    bool isYellow;
    bool isRight;
    ZSS::ZParamManager::instance()->loadParam(isSimulation,"Alert/IsSimulation",false);
    ZSS::ZParamManager::instance()->loadParam(isYellow,"ZAlert/IsYellow",false);
    ZSS::ZParamManager::instance()->loadParam(isRight,"ZAlert/IsRight",false);
    VECTOR = isRight ? -1 : 1;

    if(isSimulation && isYellow)
        REFEREE_PORT += 1; // use to fight against in simulation mode

    receiveSocket.bind(QHostAddress::AnyIPv4,REFEREE_PORT, QUdpSocket::ShareAddress);
    receiveSocket.joinMulticastGroup(QHostAddress(ZSS::REF_ADDRESS)); // receive Athena ref, need to change ZSS_ADDRESS
}

ZSS_THREAD_FUNCTION void CRefereeBoxInterface::start() {
    _alive = true;
    try {
            auto threadCreator = std::thread([=]{receivingLoop();});
            threadCreator.detach();
    }
    catch(std::exception e) {
        std::cout << "Error: Can't start RefereeBox Interface thread" << std::endl;
    }
}

void CRefereeBoxInterface::stop() {
    _alive = false;
}

void CRefereeBoxInterface::receivingLoop() {
    Referee ssl_referee;
    QByteArray datagram;
    while( _alive ) {
        while (receiveSocket.state() == QUdpSocket::BoundState && receiveSocket.hasPendingDatagrams()) {
            datagram.resize(receiveSocket.pendingDatagramSize());
            receiveSocket.readDatagram(datagram.data(), datagram.size());
            ssl_referee.ParseFromArray((void*)datagram.data(), datagram.size());
//            unsigned long long packet_timestamp = ssl_referee.packet_timestamp();
//            const auto& stage = ssl_referee.stage();
            const auto& command = ssl_referee.command();
            unsigned long command_counter = ssl_referee.command_counter();
//            unsigned long long command_timestamp = ssl_referee.command_timestamp();
            const auto& yellow = ssl_referee.yellow();
            const auto& blue = ssl_referee.blue();
            if(ssl_referee.has_next_command()){
                _next_command = ssl_referee.next_command();
            }else{
                _next_command = 0;
            }
            long long stage_time_left = 0;
            if (ssl_referee.has_stage_time_left())
            {
                stage_time_left = ssl_referee.stage_time_left();
            }
            PlayMode cmd;
            unsigned char cmd_index = 0;
            //command 对应
            switch(command) {
            case 0: cmd = PMHalt; break; // Halt
            case 1: cmd = PMStop; break; // Stop
            case 2: cmd = PMReady; break; // Normal start (Ready)
            case 3: cmd = PMStart; break; // Force start (Start)
            case 4: cmd = PMKickoffYellow; break; // Kickoff Yellow
            case 5: cmd = PMKickoffBlue; break; // Kickoff Blue
            case 6: cmd = PMPenaltyYellow; break; // Penalty Yellow
            case 7: cmd = PMPenaltyBlue; break; // Penalty Blue
            case 8: cmd = PMDirectYellow; break; // Direct Yellow
            case 9: cmd = PMDirectBlue; break; // Direct Blue
            case 10: cmd = PMIndirectYellow; break; // Indirect Yellow
            case 11: cmd = PMIndirectBlue; break; // Indirect Blue
            case 12: cmd = PMTimeoutYellow; break; // Timeout Yellow
            case 13: cmd = PMTimeoutBlue; break; // Timeout Blue
            case 14: cmd = PMGoalYellow; break; // Goal Yellow
            case 15: cmd = PMGoalBlue; break; // Goal Blue
            case 16: cmd = PMBallPlacementYellow; break; // Ball Placement Yellow
            case 17: cmd = PMBallPlacementBlue; break; // Ball Placement Blue
            default:
                std::cout << "refereebox is fucked !!!!! command : " << command << std::endl;
                cmd = PMHalt;break;
            }
            // Get Ball Placement Position
            if(cmd == PMBallPlacementYellow || cmd == PMBallPlacementBlue){
            //change ssl_vision_xy to zjunlict_vision_xy
                refereeMutex.lock();
                _ballPlacementX = VECTOR * ssl_referee.designated_position().x();
                _ballPlacementY = VECTOR * ssl_referee.designated_position().y();
                CGeoPoint point(_ballPlacementX,_ballPlacementY);
                refereeMutex.unlock();
                GDebugEngine::Instance()->gui_debug_x(point,COLOR_WHITE);
                GDebugEngine::Instance()->gui_debug_msg(point,"BP_Point",COLOR_WHITE);
                GDebugEngine::Instance()->gui_debug_arc(point,15,0,360,COLOR_WHITE);
            }
            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100,100),QString("%1 %2").arg(command).arg(playModePair[cmd].ch).toLatin1());

            static unsigned char former_cmd_index = 0;
            cmd_index = command_counter;
            if (cmd_index != former_cmd_index) {
                former_cmd_index = cmd_index;    // 更新上一次指令得标志值
                std::cout << " cmd : " << playModePair[cmd].ch << std::endl;
                if( cmd != PMNone ) {
                    refereeMutex.lock();
                    _playMode = cmd;
                    _blueGoalNum = (int)blue.score();
                    _yellowGoalNum = (int)yellow.score();
                    _blueGoalie = (int)blue.goalkeeper();
                    _yellowGoalie = (int)yellow.goalkeeper();
                    _remainTime =(unsigned short)(stage_time_left / 1000000);
                    refereeMutex.unlock();
                    std::cout << "Protobuf Protocol: RefereeBox Command : " << cmd << " what : " << playModePair[cmd].what << std::endl;
                    std::cout << "Stage_time_left : "<< _remainTime << " Goals for blue : "<<_blueGoalNum
                    << " Goals for yellow : "<< _yellowGoalNum << std::endl;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
