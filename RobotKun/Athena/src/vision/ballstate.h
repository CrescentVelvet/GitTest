#ifndef BALLSTATE_H
#define BALLSTATE_H
#include "staticparams.h"
#include "messageformat.h"
#include "chipsolver.h"
#include "dataqueue.hpp"
#include "singleton.hpp"
#include <string>
//enum Result {fail, success, undefined};
//enum State {_touched, _shoot, _struggle, _pass, _dribble, _stop, _outoffied, _control, _undefine, _kicked, _chip_pass, _flat_pass};
enum Result {fail, success, undefined};
enum State {_touched, _shoot, _struggle, _pass, _dribble, _stop, _outoffied, _control, _undefine, _kicked, _chip_pass, _flat_pass,_uncertain_pass};

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


class BallPathAnalyzer
{
public:
    BallPathAnalyzer();
    int _tempTouch;
//    CGeoPoint _lastTouchPos;
    int _uncertainTouch;
    CGeoPoint _uncertainTouchPos;
    int _tempState;
    void run(ReceiveVisionMessage* vision);
    void reset();
//    Chipsolver& getChipsolver(){
//        return chipsolver;
//    }
private:
    const ReceiveVisionMessage* _pVision;
//    Chipsolver chipsolver;
    DataQueue<CGeoPoint> predictPoses;
    DataQueue<Ball> ballPath;
    int touchCount[PARAM::TEAMS][PARAM::Field::MAX_PLAYER];
    int findTouch();
    int _chipConfidence;
    int _flatConfidence;
};

class BallState
{
public:
    BallState();
    void run(ReceiveVisionMessage* vision);
    bool ballIsOnEdge(CGeoPoint ball);
    bool ballCloseEnough2Analyze();

//    char* getState(int num);
private:
    ReceiveVisionMessage* _pVision;
    DribbleJudger dribbleJudger;
    BallPathAnalyzer ballPathAnalyzer;
    int _ballState;
    int _tempTouch;
    CGeoPoint _lastTouchPos;
    bool dribbleJudge();
    void ballPathAnalyze();

};
typedef Singleton<BallState> ballState;
#endif // BALLSTATE_H
