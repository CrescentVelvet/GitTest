#ifndef DEALROBOT_H
#define DEALROBOT_H
#include "kalmanfilter.h"
#include "kalmanfilterdir.h"

#include <singleton.hpp>
#include "messageformat.h"
/**
 * @brief main class of process robot
 */
class CDealRobot {
  public:
    CDealRobot();
    void run();
    void mergeRobot();
    void selectRobot();
    void updateVel(ReceiveVisionMessage& result);
//    bool updateCommand(ZSS::Protocol::Robot_Command command);

  private:
    Robot robotSeqence[PARAM::TEAMS][PARAM::ROBOTMAXID][PARAM::CAMERA];
    ReceiveVisionMessage result;
    Robot sortTemp[PARAM::TEAMS][PARAM::ROBOTMAXID];
    Robot lastRobot[PARAM::TEAMS][PARAM::ROBOTMAXID];
    Robot currentRobot[PARAM::TEAMS][PARAM::ROBOTMAXID];
    KalmanFilter _kalmanFilter[PARAM::TEAMS][PARAM::ROBOTMAXID];
    KalmanFilter _dirFilter[PARAM::TEAMS][PARAM::ROBOTMAXID];
    int validNum[PARAM::TEAMS];
    int minBelieveFrame, ourMaxLostFrame, theirMaxLostFrame, fieldWidth, fieldHeight;

    void init();
    double calculateWeight(int camID, CGeoPoint roboPos);
    void setPossibility();
    bool isOnField(CGeoPoint);

};
typedef Singleton <CDealRobot> DealRobot;

#endif // DEALROBOT_H
