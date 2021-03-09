#ifndef __MESSAGEFORMAT_H__
#define __MESSAGEFORMAT_H__
#include "staticparams.h"
#include <iostream>
#include "geometry.h"
/************************************************************************/
/*                 Basic Vision Classes                                 */
/************************************************************************/
//namespace {
/*
    struct Pos2d{
        double x;
        double y;
        Pos2d():x(-32767),y(-32767){}
        Pos2d(double _x,double _y){x=_x;y=_y;}
        bool fill(double x,double y){
            this->x=x;
            this->y=y;
            return true;
        }
        double dist(Pos2d p){
            return sqrt((x-p.x)*(x-p.x)+(y-p.y)*(y-p.y));
        }
    };
    */

//enum ballState {received, touched, kicked, struggle, chip_pass, flat_pass};

struct Ball {
    double height;
    int cameraID;
    CGeoPoint pos;
    CGeoPoint predict_pos;
    int ball_state_machine;
    CVector velocity;
    Ball(): height(0), cameraID(-1) {}
    bool fill(double x = 0, double y = 0, double height = 0, int id = -1) {
        this->pos.setX(x);
        this->pos.setY(y);
        this->height = height;
        this->cameraID = id;
        return true;
    }
    bool fill(const Ball& ball) {
        return fill(ball.pos.x(), ball.pos.y(), ball.height, ball.cameraID);
    }
};
struct Robot {
    CGeoPoint pos;
    CGeoPoint predict_pos;
    double angle;
    double raw_angle;
    CVector velocity;
    CVector accelerate;
    CVector raw_vel;
    double rotateVel;
    double rawRotateVel;
    bool valid = false;
    Robot() {init();}
    Robot(double x, double y, double angle, bool valid) {
        init();
        pos.setX(x);
        pos.setY(y);
        this->angle = angle;
        this->raw_angle = angle;
        this->valid = valid;
    }
    bool init() {
        pos.setX(99999);
        pos.setY(99999);
        this->angle = 0;
        this->raw_angle = 0;
        this->velocity = CVector(0, 0);
        this->raw_vel = CVector(0, 0);
        this->accelerate = CVector(0, 0);
        this->rotateVel = 0.0;
        this->rawRotateVel = 0.0;
        this->valid = false;
        return true;
    }
    bool fill(double x, double y, double angle, bool valid = false) {
        pos.setX(x);
        pos.setY(y);
        this->angle = angle;
        this->raw_angle = angle;
        this->valid = valid;
        return true;
    }
    bool fill(const Robot& robot) {
        return fill(robot.pos.x(), robot.pos.y(), robot.angle, robot.valid);
    }
};
struct CameraEdge {
    double min, max;
};
struct SingleCamera {
    unsigned short id;
    CGeoPoint campos;
    double height;
    CameraEdge leftedge, rightedge, upedge, downedge;
    SingleCamera(): height(3800) {} //set 3800mm as an example（in our lab）
    void fillCenter(double x, double y) {
        campos.fill(x, y);
        leftedge.min = rightedge.min = x;
        upedge.min = downedge.min = y;
    }
    void fillCenter(CGeoPoint p) {
        campos = p;
        leftedge.min = rightedge.min = p.x();
        upedge.min = downedge.min = p.y();
    }
};
struct CameraFix {
    double theta, t1, t2;
    CameraFix(): theta(0), t1(0), t2(0) {}
};
//}


//* Robot store the data, Index and Size exist for the index search for robots;
//* Index : -1 means not exist, or it means the index in Robot Array;
//* VisionMessage use for the final processed vision data.
class OriginMessage {
  public:
    unsigned short ballSize;
    Robot robot[2][PARAM::ROBOTNUM];
    Ball ball[PARAM::BALLNUM];
    bool isBallValid = false;
    OriginMessage(): ballSize(0) {}
    void init() {
        ballSize = 0;
        isBallValid = false;
        ball[0].pos.fill(99999,99999);
        for (int color = 0; color < PARAM::TEAMS; color++) {
            for(int i = 0; i < PARAM::ROBOTNUM; i++) {
                robot[color][i].init();
            }
        }
    }
    bool addBall(double x, double y, double height = 0, int id = -1) {
        return ballSize >= PARAM::BALLNUM ? false : ball[ballSize++].fill(x, y, height, id);
    }
    bool addBall(CGeoPoint point, double height = 0, int id = -1) {
        return ballSize >= PARAM::BALLNUM ? false : ball[ballSize++].fill(point.x(), point.y(), height, id);
    }
    bool addBall(const Ball& b) {
        return ballSize >= PARAM::BALLNUM ? false : ball[ballSize++].fill(b);
    }
    bool setRobot(int color, int id, double x, double y, double angel, bool valid) {
        if (id >= PARAM::ROBOTNUM || id < 0) {
            std::cout << "Error! Invalid robotID in setRobot!" << std::endl;
            return false;
        }
        return robot[color][id].fill(x, y, angel, valid);
    }
    bool setRobot(int color, int id, const CGeoPoint& point, double angel) {
        if (id >= PARAM::ROBOTNUM || id < 0) {
            std::cout << "Error! Invalid robotID in setRobot!" << std::endl;
            return false;
        }
        return robot[color][id].fill(point.x(), point.y(), angel, true);
    }
    bool setRobot(int color, int id, const Robot& temp) {
        if (id >= PARAM::ROBOTNUM || id < 0) {
            std::cout << "Error! Invalid robotID in setRobot!" << std::endl;
            return false;
        }
        return robot[color][id].fill(temp);
    }
};

class ReceiveVisionMessage: public OriginMessage {
  public:
    unsigned short camID;
    unsigned long long frame;
    double captureTime; // ![1] TODO -> gjy

    ReceiveVisionMessage(): camID(-1), frame(-1), captureTime(-1) {}
    void clear() {
        captureTime = frame = camID = -1;
        init();
    }
};

#endif // __MESSAGEFORMAT_H__
