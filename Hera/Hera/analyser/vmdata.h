#ifndef VMDATA_H
#define VMDATA_H

#include "dataqueue.hpp"
#include "messageformat.h"
#include "ballrecords.h"

class vmData
{
public:
    vmData();
    DataQueue<ReceiveVisionMessage> camera[PARAM::CAMERA];
    DataQueue<ReceiveVisionMessage> maintain;
    DataQueue<ReceiveVisionMessage> processBall;
    DataQueue<ReceiveVisionMessage> processRobot;
    DataQueue<BallRecord> ballrecords;
};

#endif // VMDATA_H
