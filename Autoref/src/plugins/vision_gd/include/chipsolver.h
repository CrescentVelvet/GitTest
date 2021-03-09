#ifndef CHIPSOLVER_H
#define CHIPSOLVER_H
#include "globaldata.h"
#include "ballrecords.h"
class Chipsolver {
  public:
    Chipsolver();
    ~Chipsolver();
    double chipsolveOffset(double timeOff);
    void getbestresult();
    bool dealresult();
    void reset();
    void setrecord();
    void setrecord_test();
    DataQueue<CGeoPoint> predict;
    CVector3 getkickPos(){
        return kickPos;
    }
    CVector3 getkickVel(){
        return kickVel;
    }
    CGeoPoint getposEnd(){
        return CGeoPoint(posEnd.x(),posEnd.y());
    }
    double getFlytime(){
        return tFly;
    }
  private:
    CVector3 kickVel;
    CVector3 kickPos;
    CVector3 posNow;
    CVector3 posEnd;
    double l1Error;
    double timeOffset;
    double t;
    double tFly;
    BallRecord ballrecord;

};
#endif // CHIPSOLVER_H
