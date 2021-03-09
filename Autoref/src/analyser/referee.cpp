#include "referee.h"
#include <QUdpSocket>
#include <thread>
#include <iostream>
#include <thread>
namespace {
    std::thread* t;
}

Referee::Referee(){
    receiveSocket.bind(QHostAddress::AnyIPv4,REFEREE_PORT, QUdpSocket::ShareAddress);
    receiveSocket.joinMulticastGroup(QHostAddress("224.5.23.1"));
    t = new std::thread([=] {receiveRef();});
    t->detach();
}

Referee::~Referee(){

}

void Referee::receiveRef(/*void *ptr,int size*/){
    static SSL_Referee ssl_referee;
    QByteArray datagram;
    bool debug = receiveSocket.state() == QUdpSocket::BoundState ;
    std::cout <<"boundstate : " << debug << std::endl;
    std::cout <<"pending : " << receiveSocket.hasPendingDatagrams() <<std::endl;
    while(true){
        while(receiveSocket.state() == QUdpSocket::BoundState && receiveSocket.hasPendingDatagrams()){
            datagram.resize(receiveSocket.pendingDatagramSize());
            receiveSocket.readDatagram(datagram.data(), datagram.size());
            //        ssl_referee.ParseFromArray( ptr, size );
            ssl_referee.ParseFromArray((void*)datagram.data(), datagram.size());
            refereeMutex.lock();
            _refereeCommand = ssl_referee.command();
            _stage = ssl_referee.stage();
            _ballplacementX = ssl_referee.designated_position().x();
            _ballplacementY = ssl_referee.designated_position().y();
            _yellow_gk_id = ssl_referee.yellow().goalie();
            _blue_gk_id = ssl_referee.blue().goalie();
            refereeMutex.unlock();
//            std::cout << "ref info : " << _refereeCommand <<std::endl;
        }
    }
//    std::cout << "ref info : " << refereecommand <<std::endl;
}
