#ifndef ANALY_LOG_H
#define ANALY_LOG_H

#include <QObject>
#include <QDebug>
#include "messageformat.h"
#include "globalsettings.h"

namespace ANALY {
    const int LEFT = 0;
    const int RIGHT = 1;
    const int BLUE = 2;
    const int YELLOW = 3;
    const int OurHolding = 4;
    const int TheirHolding = 5;
    const int BothHolding = 6;
    const int Our = 7;
    const int Their = 8;
    const int Both = 9;
    const int Null = 10;
    const int Halt = 11;
    const int Stop = 12;
    const int Ready = 13;
    const int Start = 14;
    const int FIELD_WIDTH = 12000;
    const int FIELD_LENGTH = 9000;
    const int PENALTY_WIDTH = 1200;
    const int PENALTY_LENGTH = 2400;
    const int ROBOT_RADIUS = 100;
    const int HAS_BALL = 400;
}
class AnalyEsthetics : public QObject{
    Q_OBJECT
public:
    AnalyEsthetics();
    void readFrame(int frac, int fraw, QString filename);
    void readReferee(void * ptr, int size);
    void analy_frame(void * ptr, int size);
private:
    bool atPresent(CGeoPoint point);
    CVector Polar2Vector(double m,double angle);
    CGeoPoint analy_InterPoint(CGeoPoint enemy, CGeoPoint ball);
    void judgeSide(CGeoPoint blue_car[], int blue_side);
    bool analy_marking(int team, int num);
    bool analy_guarding(int team, int num);
    bool analy_goalkeeping(int team, int num);
    int cal_nearest(int team);
    void cal_holdMsg();
    void cal_extrovert();
    int current_frame;
    int whole_frame;
    int minX = -ANALY::FIELD_WIDTH/2;
    int maxX = ANALY::FIELD_WIDTH/2;
    int minY = -ANALY::FIELD_LENGTH/2;
    int maxY = ANALY::FIELD_LENGTH/2;
    CGeoPoint e_ball;
    CGeoPoint blue_robot[PARAM::ROBOTNUM];
    CGeoPoint yellow_robot[PARAM::ROBOTNUM];
//    int analy_team;
    char *teamname;
    int analy_status;
    int analy_holdMsg;
    int analy_blueside;
    int analy_yellowside;
    double our_carnum[6];
    double their_carnum[6];
    double real_ourframe[3];
    double real_theirframe[3];
};
#endif // ANALY_LOG_H
