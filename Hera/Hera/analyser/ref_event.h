#ifndef REF_EVENT_H
#define REF_EVENT_H

#include "visionmodule.h"
#include "messages_robocup_ssl_geometry.pb.h"
#include "utils.h"
#include "param.h"
#include "vmdata.h"
#include "dataqueue.hpp"
#include "messageformat.h"
#include <map>

const double min_ball_pass_vel = 200;
const double min_vel_collision = 1.5;
const double min_dist = 170;
const double max_vel = 6500;
const double own_dist = 135;
const double own_dir = Param::Math::PI/10;
const double received_dist = 500;
const int max_frame_num = 10;
const double push_dist = 200;
const int dribble_dist = 1000;
const double test_push = 500;

enum State {nor , out_of_field , goal , yellow_collision , blue_collision , overspeed ,
           ball_placement_blue , ball_placement_yellow ,  directkick_blue , directkick_yellow,
              blue_in_penalty , yellow_in_penalty ,penaltykick_blue ,penaltykick_yellow ,
           blue_push , yellow_push};
enum RobotState {Run, blue_kick_ball ,yellow_kick_ball};
enum OwnMessage{other , blue_own ,yellow_own};
enum ReceiveState{no_receive , blue_r , yellow_r };
enum PassMsg{pass_other ,pass_blue , pass_yellow};
enum aimlessMsg{no_aimless ,blue_aimless , yellow_aimless};
enum dribbleMsg{no_dribble,blue_dribble , yellow_dribble};

static State _state = nor;
static RobotState _robotState = Run;
static OwnMessage _ownmsg = other;
static int ownID_blue = -1;
static int ownID_yellow = -1;
static CGeoPoint ownPos_blue = CGeoPoint(9999,9999);
static CGeoPoint ownPos_yellow = CGeoPoint(9999,9999);
static int lastTouch_ID=-1;
static int lastKick_ID_blue = -1;
static int lastKick_ID_yellow = -1;
static ReceiveState _receive_msg =no_receive;
static CGeoPoint lastTouchPos = CGeoPoint(9999,9999);
static aimlessMsg aimlessmsg = no_aimless;
static dribbleMsg dribblemsg = no_dribble;
static CGeoPoint directkick_pos = CGeoPoint(9999,999);
static CGeoPoint placementPos = CGeoPoint(9999,9999);
static int lastcommad = 0;
static std::map<int, CGeoPoint> RobotPair;

static int timer = 0;
static int currentFrame = 0;

class ref_event
{
public:
    ref_event();
    int getState();
    int getRobotState();
    int lastTouchID();

    //get game ref command
    void receiveRefMsg();

    //ball own
    void ballOwnDetect();
    bool ballowns(Robot& bot, int frame);

    //robot id problem
    void robotReset();
    void visionMessage(DataQueue<ReceiveVisionMessage> vm );

    //out_event
    void judgeOutEvent(CGeoPoint pos);

    //collision
    void collisionID();
    bool collisionDetect(CGeoPoint& pos_1 , CGeoPoint& pos_2 , CVector preVel_1 , CVector preVel_2);
    void collsionState(int id_blue , int id_yellow);
    int judgeCollision(CVector vel_blue , CVector vel_yellow);

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

    //
    void run(DataQueue<ReceiveVisionMessage> vm);

private:
    ReceiveState _receive_msg;
    PassMsg passMsg;
    Robot robotBlue[10][PARAM::ROBOTNUM];
    Robot robotYellow[10][PARAM::ROBOTNUM];
    Ball ball[10];
    int gameCommand;
    int gameStage;
    float ballPlaceX,ballPlaceY;

};

#endif // REF_EVENT_H
