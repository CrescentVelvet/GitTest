#ifndef REFEREE_H
#define REFEREE_H

#include "referee.pb.h"
#include <QUdpSocket>
#include <QMutex>
#include "singleton.hpp"

static int stage;
static int refereecommand;
static int yellow_gk_id;
static int blue_gk_id;

namespace {
    int REFEREE_PORT = 10003;
}

class Referee
{
public:
    Referee();
    ~Referee();
    void receiveRef(void *ptr,int size);
    int getCommand();
    int getStage();
    float getPlacementY();
    float getPlacementX();
    int getYellowGoalie();
    int getBlueGoalie();
//    void start();
private:
    QUdpSocket receiveSocket;
    float ballplacementX = 0;
    float ballplacementY = 0;
};
typedef Singleton<Referee>  RefInfo;
#endif // REFEREE_H
