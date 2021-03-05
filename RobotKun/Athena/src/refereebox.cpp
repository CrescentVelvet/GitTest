#include <QtDebug>
#include <QElapsedTimer>
#include "refereebox.h"
#include "parammanager.h"
#include "staticparams.h"
#include "globalsettings.h"
#include "globaldata.h"
//void multicastCommand(int state);

RefereeBox::RefereeBox(QObject *parent) :
    QObject(parent),
    _test_mode(false),
    _currentCommand(GameState::HALTED),
    _nextCommand(GameState::HALTED),
    _receiveCmdThread(nullptr),
    receiveSocket(nullptr),
    sendSocket(nullptr){
    ZSS::ZParamManager::instance()->loadParam(_ssl_port,"AlertPorts/SSL_RefereePort",10003);
    ZSS::ZParamManager::instance()->loadParam(_zss_port,"AlertPorts/ZSS_RefereePort",39991);

    sendSocket = new QUdpSocket();
    receiveSocket = new QUdpSocket();

    sendSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
    commandCounter = 1;

    _receiveCmdThread = new std::thread([=] {receiveCommands();});
    _receiveCmdThread->detach();

    receiveSocket->bind(QHostAddress::AnyIPv4,_ssl_port, QUdpSocket::ShareAddress);
    receiveSocket->joinMulticastGroup(QHostAddress(ZSS::REF_ADDRESS));
}
RefereeBox::~RefereeBox(){
    delete _receiveCmdThread;
    _receiveCmdThread = nullptr;

    delete receiveSocket;
    delete sendSocket;

}

void RefereeBox::manualCommands(){
    _ssl_referee.set_packet_timestamp(0);//todo
    _ssl_referee.set_stage(Referee_Stage_NORMAL_FIRST_HALF);//todo
    _ssl_referee.set_stage_time_left(0);//todo

    commandMutex.lock();
    _ssl_referee.set_command(Referee_Command(_currentCommand));
    _ssl_referee.set_command_counter(commandCounter);//todo
    // add test for next_command
    _ssl_referee.set_next_command(Referee_Command(_nextCommand));
    commandMutex.unlock();

    _ssl_referee.set_command_timestamp(0);//todo
    Referee_TeamInfo *yellow = _ssl_referee.mutable_yellow();
    yellow->set_name("ZJUNlict");//todo
    yellow->set_score(0);//todo
    yellow->set_red_cards(0);//todo
    //yellow->set_yellow_card_times(0,0); //todo
    yellow->set_yellow_cards(0);//todo
    yellow->set_timeouts(0.0);//todo
    yellow->set_timeout_time(0.0);//todo
    yellow->set_goalkeeper(0); //todo
    Referee_TeamInfo *blue = _ssl_referee.mutable_blue();
    blue->set_name("ZJUNlict");
    blue->set_score(0);//todo
    blue->set_red_cards(0);//todo
    //blue->set_yellow_card_times(0,0); //todo
    blue->set_yellow_cards(0);//todo
    blue->set_timeouts(0.0);//todo
    blue->set_timeout_time(0.0);//todo
    blue->set_goalkeeper(0); //todo
    Referee_Point *point = _ssl_referee.mutable_designated_position();
    point->set_x(GlobalSettings::instance()->ballPlacementX);
    point->set_y(GlobalSettings::instance()->ballPlacementY);
}

void RefereeBox::receiveCommands() {
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        if (_test_mode) {
            manualCommands();
            sendCommands();
        } else {
            if (receiveSocket->state() == QUdpSocket::BoundState && receiveSocket->hasPendingDatagrams()) {
                QByteArray datagram;
                datagram.resize(receiveSocket->pendingDatagramSize());
                receiveSocket->readDatagram(datagram.data(), datagram.size());
                _ssl_referee.ParseFromArray((void*)datagram.data(), datagram.size());
                sendCommands();
            }
        }
    }
}

void RefereeBox::sendCommands() {
    long size = _ssl_referee.ByteSizeLong();
    QByteArray buffer(size,0);
    _ssl_referee.SerializeToArray(buffer.data(), buffer.size());
    sendSocket->writeDatagram(buffer.data(), buffer.size(),
                                 QHostAddress(ZSS::REF_ADDRESS), _zss_port);
    sendSocket->writeDatagram(buffer.data(), buffer.size(),
                                 QHostAddress(ZSS::REF_ADDRESS), _zss_port+1);
    _ssl_referee.Clear();
}
