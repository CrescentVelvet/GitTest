#ifndef REF_EVENT_H
#define REF_EVENT_H

#include "visionmodule.h"
#include "messages_robocup_ssl_geometry.pb.h"
#include "dataqueue.hpp"
#include "messageformat.h"
#include <map>
#include "singleton.hpp"
#include "param.h"
#include "chipsolver.h"

namespace {
const double MIN_BALL_KICK_VEL = 200;//大写
const double MIN_VEL_COLLISION = 1.5;
const double MIN_DIST = 170;
const double MAX_VEL = 650;
const double OWN_DIST = 135;
const double OWN_DIR = Param::Math::PI/10;
const double RECEIVED_DIST = 500;
const int MAX_FRAME = 10;
const double PUSH_PAIR_DIST = 200;
const int DOUBLE_DIST = 1000;
const int KICK_CIRCLE_UPLIMIT = 500;
const int KICK_CIRCLE_LOWLIMIT = 400;
const int LASTTOUCH_LOWLIMIT = 85;
const int LASTTOUCH_UPLIMIT = 125;
const int DIRECTKCIK_TIME = 770;
const int DIRECTKCIK_DIST = 80;
const int PALACEMENT_TIME = 2310;
const int PALACEMENT_DIST = 10;
const int PENALTY_KICK_TIME = 770;
const int PENALTY_KICK_DIST = 50;
const int PUSH_DIST = 500;
const int SEMAPHORE_NUM = 9;
const int BALL_SPEED_JUDGE_FRAME=8;
const int BALL_SPEED_CONFIRM_FRAME=5;
//chip
const double MIN_FLY_TIME = 0.3;
const double MAX_FLY_TIME = 1.4;
const double MAX_VELZ_RATIO = 1.5;
const double MIN_FLY_VEL = 2000;
//collision;
const double MIN_TURN_ANGLE = Param::Math::PI*5/180;
const double OBVIOUS_TURN_ANGLE = Param::Math::PI*30/180;
const double MAX_COLLISION_DIST = Param::Vehicle::V2::PLAYER_SIZE+3*Param::Field::BALL_SIZE;

const int CONFIRM_DRIBBLE_CYCLE = 4;
double Normalize(double angle)
{
    if (fabs(angle) > 10 ){
        std::cout<<angle<<" Normalize Error!!!!!!!!!!!!!!!!!!!!"<<std::endl;
        return 0;
    }
    const double M_2PI = Param::Math::PI * 2;
    // 快速粗调整
    angle -= (int)(angle / M_2PI) * M_2PI;

    // 细调整 (-PI,PI]
    while( angle > Param::Math::PI ) {
        angle -= M_2PI;
    }

    while( angle <= -Param::Math::PI ) {
        angle += M_2PI;
    }

    return angle;
}
}

enum State {nor , out_of_field , goal , yellow_collision , blue_collision , overspeed ,
           ball_placement_blue , ball_placement_yellow ,  directkick_blue , directkick_yellow,
              blue_in_penalty , yellow_in_penalty ,penaltykick_blue ,penaltykick_yellow ,
           blue_push , yellow_push};
enum RobotState {Run, blue_kick_ball ,yellow_kick_ball};
enum OwnMessage{other , blue_own ,yellow_own};
enum ReceiveState{no_receive , blue_r , yellow_r };
enum PassMsg{pass_other ,pass_blue , pass_yellow};
enum dribbleMsg{no_dribble,blue_dribble , yellow_dribble};
enum passState{pass_uncertain,pass_flat,pass_chip};
class ref_event
{
public:
    ref_event();
    ~ref_event();
    int getState();
    int getRobotState();
    int lastTouchID();
    //
    void receiveVision(DataQueue<ReceiveVisionMessage>& vm);
    
    //point in one area
    bool inArea(const CGeoPoint& _p, double x1, double x2, double y1, double y2);
    bool inArea(const CGeoPoint& _p, const CGeoPoint &center , double radius);

    //get game ref command
    void receiveRefMsg();

    //ball own
    void ballOwnDetect();
    bool ballowns(const Robot& bot, int frame);

    //robot id problem
    void robotReset();
//    void visionMessage(DataQueue<ReceiveVisionMessage> vm );

    //out_event
    void judgeOutEvent(const CGeoPoint& pos);

    //collision
    void collisionID();
    bool collisionDetect(const CGeoPoint& pos_1, const CGeoPoint& pos_2, CVector& preVel_1, CVector& preVel_2);
    void collsionState(int id_blue , int id_yellow);
    int judgeCollision(CVector& vel_blue , CVector& vel_yellow);

    //overspeed
    void ballOverSpeed();

    //kick
    void getBotState();
    bool kickDetect(Robot& bot , int frame);
    bool ifRecive(Robot& bot , int frame);

    //pass
    void receiveDetet();
    void passDetect();

    //push
    void pushDetect();

    //last touch
    void lastTouchDetect();

    //aimless kick
    void lastOwnPosDetect();
    void aimlessDetect();

    //dribble
    void dribbleDetect();

    //out of time
    void directKickOFT();

    //attacker in penalty
    void penaltyAttackerDetect();

    //ball placement timeout
    void ballPlaceTimeout();

    //double touch
    void doubleTouchDetect();

    //penalty kick timeout
    void penaltyKickOFT();

    //chip judge(mix with touch judge)
    void ballPathAnalyse();
    void ballPathReset();
    //
    void run();


private:
    ReceiveVisionMessage _refVision[MAX_FRAME];
    PassMsg _passMsg = pass_other;
    State _state = nor;
    RobotState _robotState = Run;
    OwnMessage _ownmsg = other;
    int _ownID[PARAM::TEAMS];
//    int _ownID_blue = -1;
//    int _ownID_yellow = -1;
    CGeoPoint _ownPos[PARAM::TEAMS];
//    CGeoPoint _ownPos_blue = CGeoPoint(9999,9999);
//    CGeoPoint _ownPos_yellow = CGeoPoint(9999,9999);
    int _lastTouch_ID=-1;
    int _lastKick_ID = -1;
//    int lastKick_ID_yellow = -1;
    ReceiveState _receive_msg =no_receive;
    CGeoPoint _lastTouchPos = CGeoPoint(9999,9999);
    dribbleMsg _dribblemsg = no_dribble;
    CGeoPoint _placementPos = CGeoPoint(9999,9999);
    int _gameCommand = 0;
    int _gameStage = 0;
    int _lastcommad = 0;
    int _uncertainTouch_ID = -1;
    CGeoPoint _uncertainTouchPos = CGeoPoint(9999,9999);
    std::map<int, CGeoPoint> _RobotPair;

    int _timer = 0;

    //
    DataQueue<CGeoPoint> ballPath;
    int _chipConfidence;
    int _flatConfidence;
    passState _passState;
    int _dribbleCount;
};
typedef Singleton<ref_event> refJudge;
#endif // REF_EVENT_H
