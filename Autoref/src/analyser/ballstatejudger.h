#ifndef BALLSTATEJUDGER_H
#define BALLSTATEJUDGER_H
#include "param.h"
#include "staticparams.h"
#include "messageformat.h"
#include "chipsolver.h"
#include "dataqueue.hpp"
#include "singleton.hpp"
#include <string>
namespace  {
enum judgestate{
    stop = 0,
    dribble,
    struggle,
    pass_uncertain,
    pass_chip,
    pass_flat
};
}
class DribbleJudger
{
public:
    DribbleJudger();
    int RobNum[PARAM::TEAMS];
    int dribbleCount[PARAM::ROBOTNUM][PARAM::ROBOTNUM];
    int lostCount[PARAM::ROBOTNUM][PARAM::ROBOTNUM];
    void run(const ReceiveVisionMessage* vision);
private:
    const ReceiveVisionMessage* pvision;
    bool carDribble(const Robot& robot,const Ball& ball);
    void clear();
};

class StopJudger
{
public:
    StopJudger();
    bool _stop;
    void run(const ReceiveVisionMessage* vision);
private:
    int _count;

};

class BallPathAnalyzer
{
public:
    BallPathAnalyzer();
    int _lastState;
    int _lastTouch;
    int _uncertainTouch;
    int _passState;
    void run(const ReceiveVisionMessage* vision);
    void reset();
private:
    const ReceiveVisionMessage* _pVision;
    Chipsolver chipsolver;
    DataQueue<CGeoPoint> predictPoses;
    DataQueue<Ball> ballPath;
    int findTouch();
    int findTouchStop();
    int _chipConfidence;
    int _flatConfidence;
};

class BallStateJudger
{
public:
    BallStateJudger();
    void run( const ReceiveVisionMessage* vision);
    char* getState(int num);
private:
    const ReceiveVisionMessage* _pVision;
    DribbleJudger dribbleJudger;
    StopJudger stopJudger;
    BallPathAnalyzer ballPathAnalyzer;
    int _lastState;
    int _lastTouch;
    bool dribbleJudge();
    bool stopJudge();
    void ballPathAnalyze();

};
typedef Singleton<BallStateJudger> ballStateJudger;
#endif // BALLSTATEJUDGER_H
