#ifndef GLOBALDATA_H
#define GLOBALDATA_H
#include "messageformat.h"
#include <thread>
#include <QMutex>
#include "singleton.hpp"
#include "dataqueue.hpp"

class Globaldata
{
public:
    Globaldata();
    int refCommand;
    DataQueue<ReceiveVisionMessage> maintain;
};

typedef Singleton<Globaldata> GlobalData;
#endif // GLOBALDATA_H


