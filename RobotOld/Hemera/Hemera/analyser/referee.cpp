#include "referee.h"
#include <QUdpSocket>
#include <thread>
#include <iostream>


Referee::Referee(){
    receiveSocket.bind(QHostAddress::AnyIPv4,REFEREE_PORT, QUdpSocket::ShareAddress);
    receiveSocket.joinMulticastGroup(QHostAddress("224.5.23.1"));
}

Referee::~Referee(){

}

int Referee::getCommand(){
    return refereecommand;
}

int Referee::getStage(){
    return stage;
}

float Referee::getPlacementX(){
    return ballplacementX;
}

float Referee::getPlacementY(){
    return ballplacementY;
}

int Referee::getBlueGoalie(){
    return blue_gk_id;
}

int Referee::getYellowGoalie(){
    return yellow_gk_id;
}

void Referee::receiveRef(void *ptr,int size){
    static SSL_Referee ssl_referee;
//    QByteArray datagram;
//    bool debug = receiveSocket.state() == QUdpSocket::BoundState ;
//    std::cout << debug << std::endl;
//    std::cout << receiveSocket.hasPendingDatagrams() <<std::endl;
//    if(receiveSocket.state() == QUdpSocket::BoundState && receiveSocket.hasPendingDatagrams()){
//        std::cout << "in" << std::endl;
//        datagram.resize(receiveSocket.pendingDatagramSize());
//        receiveSocket.readDatagram(datagram.data(), datagram.size());
        ssl_referee.ParseFromArray( ptr, size );
        refereecommand = ssl_referee.command();
        stage = ssl_referee.stage();
        ballplacementX = ssl_referee.designated_position().x();
        ballplacementY = ssl_referee.designated_position().y();
        yellow_gk_id = ssl_referee.yellow().goalie();
        blue_gk_id = ssl_referee.blue().goalie();
//    }
}
