#include "debugger.h"
#include "globaldata.h"
#include <thread>
namespace {
std::thread* blueDebuggerThread = nullptr;
std::thread* yellowDebuggerThread = nullptr;
//std::thread* analyserDebuggerThread = nullptr;
}
Debugger::Debugger(QObject *parent) : QObject(parent){
//    QObject::connect(&receiverBlue,SIGNAL(readyRead()),this,SLOT(receiveBlue()),Qt::DirectConnection);
    if(receiverBlue.bind(QHostAddress::AnyIPv4,ZSS::Athena::DEBUG_MSG_RECEIVE[0], QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)){
        blueDebuggerThread = new std::thread([=] {receiveBlue();});
        blueDebuggerThread->detach();
    }
//    QObject::connect(&receiverYellow,SIGNAL(readyRead()),this,SLOT(receiveYellow()),Qt::DirectConnection);
    if(receiverYellow.bind(QHostAddress::AnyIPv4,ZSS::Athena::DEBUG_MSG_RECEIVE[1], QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)){
        yellowDebuggerThread = new std::thread([=] {receiveYellow();});
        yellowDebuggerThread->detach();
    }

//    if(receiverAnalyser.bind(QHostAddress::AnyIPv4,ZSS::Athena::DEBUG_MSG_RECEIVE[2], QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)){
//        analyserDebuggerThread = new std::thread([=] {receiveAnalyser();});
//        analyserDebuggerThread->detach();
//    }
}

void Debugger::receiveBlue(){
    auto& datagram = GlobalData::instance()->debugBlueMessages;
    while(true){
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        while (receiverBlue.state() == QUdpSocket::BoundState && receiverBlue.hasPendingDatagrams()) {
            GlobalData::instance()->debugMutex.lock();
            datagram.resize(receiverBlue.pendingDatagramSize());
            receiverBlue.readDatagram(datagram.data(),datagram.size());
            GlobalData::instance()->debugMutex.unlock();
        }
    }
}

void Debugger::receiveYellow(){
    auto& datagram = GlobalData::instance()->debugYellowMessages;
    while(true){
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        while (receiverYellow.state() == QUdpSocket::BoundState && receiverYellow.hasPendingDatagrams()) {
            GlobalData::instance()->debugMutex.lock();
            datagram.resize(receiverYellow.pendingDatagramSize());
            receiverYellow.readDatagram(datagram.data(),datagram.size());
            GlobalData::instance()->debugMutex.unlock();
        }
    }
}

//void Debugger::receiveAnalyser(){
//    auto& datagram = GlobalData::instance()->debugAnalyserMessages;
//    while(true){
//        std::this_thread::sleep_for(std::chrono::microseconds(500));
//        while (receiverAnalyser.state() == QUdpSocket::BoundState && receiverAnalyser.hasPendingDatagrams()) {
//            GlobalData::instance()->debugMutex.lock();
//            datagram.resize(receiverAnalyser.pendingDatagramSize());
//            receiverYellow.readDatagram(datagram.data(),datagram.size());
//            GlobalData::instance()->debugMutex.unlock();
//        }
//    }
//}
