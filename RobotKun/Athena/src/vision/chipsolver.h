#ifndef CHIPSOLVER_H
#define CHIPSOLVER_H
#include "globaldata.h"
#include "ballrecords.h"

#include "singleton.hpp"

class Chipsolver {
  public:
    Chipsolver();
    ~Chipsolver();
    void run();
    double chipsolveOffset(double timeOff, double g, int searchFlag, double x_start, double y_start);
    double getbestresult(double g, int searchFlag, double x_start, double y_start);
    bool dealresult();
    void reset();
    void setrecord();
    void setrecord_test();
    CVector3 getkickPos(){
        return _kickPos;
    }
    CVector3 getkickVel(){
        return _kickVel;
    }
    CGeoPoint getposEnd(){
        return CGeoPoint(posEnd.x(),posEnd.y());
    }
    CVector3 getposNow(){
        return posNow;
    }
    double getFlytime(){
        return tFly;
    }
    double getRestFlyTime(){
        return restTFly;
    }
    CGeoPoint getBestPredictPos();
    CGeoPoint getBestSecondPos(){
        getBestPredictPos();
        return bestSecondPos;
    }
    bool getDealResultFlag(){
        return dealResultFlag;
    }

    void setResetFlag(bool flag);

//    MatrixXf calculateW(MatrixXf x, MatrixXf y){
//        return (x * x.transpose()).inverse() * x * y;
//    }


  private:
    bool dealResultFlag;
    bool resetFlag;
    
    std::vector<CGeoPoint> posEndQueue;
    std::vector<CGeoPoint> secondPosEndQueue;

    CGeoPoint bestSecondPos;

    CVector3 kickVel;
    CVector3 kickPos;
    CVector3 _kickVel;
    CVector3 _kickPos;
    CVector3 posNow;
    CVector3 velEnd;
    CVector3 posEnd;
    CGeoPoint secondPosEnd; // 第二落球点
    double l1Error;
    double timeOffset;
    double t;
    double tFly;
    double restTFly;
    double secondTFly;
    CGeoPoint kickerCenter;
    double kickerDir;
    BallRecord ballrecord;

    double t_system;
    double t_cycle;

    void drawDebugMsg();
    static int currentCycle; // store current cycle time
    static double getCurrentTime(){
        return 1.0 * currentCycle / PARAM::Vision::FRAME_RATE;
    }
    std::vector<std::pair<CGeoPoint, double>> searchkickPos;// save history search kick pos
    std::vector<CVector3> searchKickVel;
};
typedef Singleton<Chipsolver> ChipSolver;
#endif // CHIPSOLVER_H
