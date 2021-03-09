#ifndef REFEREE_H
#define REFEREE_H

#include "referee.pb.h"
#include <QUdpSocket>
#include <QMutex>
#include "singleton.hpp"
#include <QMutex>

namespace {
    int REFEREE_PORT = 39991;
}

class Referee
{
public:
    Referee();
    ~Referee();
    void receiveRef(/*void *ptr,int size*/);
    inline int getCommand(){refereeMutex.lock(); int result = _refereeCommand;refereeMutex.unlock();return result;}
    inline int getStage(){refereeMutex.lock(); int result = _stage;refereeMutex.unlock();return result;}
    inline int getBlueGoalie(){refereeMutex.lock(); int result = _blue_gk_id;refereeMutex.unlock();return result;}
    inline int getYellowGoalie(){refereeMutex.lock(); int result = _yellow_gk_id;refereeMutex.unlock();return result;}
    inline float getPlacementX(){refereeMutex.lock(); int result = _ballplacementX;refereeMutex.unlock();return result;}
    inline float getPlacementY(){refereeMutex.lock(); int result = _ballplacementY;refereeMutex.unlock();return result;}
//    void start();
private:
    int _stage;
    int _refereeCommand;
    int _yellow_gk_id;
    int _blue_gk_id;
    float _ballplacementX = 0;
    float _ballplacementY = 0;
    QMutex refereeMutex;
    QUdpSocket receiveSocket;
};
typedef Singleton<Referee>  RefInfo;
#endif // REFEREE_H
