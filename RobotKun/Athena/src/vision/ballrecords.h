#ifndef BALLRECORDS_H
#define BALLRECORDS_H
#include "messageformat.h"
#include "geometry.h"
#include "staticparams.h"
#include "parammanager.h"
#include <iostream>
#include <chrono>
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

class  BallRecord {
  public:
    BallRecord() {}
    high_resolution_clock::time_point getcapture() const {
        return capturetime;
    }
    CVector3 getpos()const {
        return pos;
    }
    CVector3 getcampos()const {
        return campos;
    }
    int getcamID()const {
        return camID;
    }
    double getidealTime() const{
        return idealTime;
    }
    long long timeinterval(high_resolution_clock::time_point capturetime0)const {
        milliseconds timeInterval = std::chrono::duration_cast<milliseconds>(capturetime - capturetime0);
        return timeInterval.count();
    }
    void setpos(double x, double y, double z = 0) {
        pos.formXYZ(x, y, z);
    }
    void setpos(CGeoPoint pos2, double z = 0) {
        //std::cout << "get setpos" << pos2 << z << std::endl;
        pos.formXYZ(pos2, z);
    }
    void setcampos(double x, double y, double z = 300) {
        campos.formXYZ(x, y, z);
    }
    void setcampos(CGeoPoint pos2, double z = 300) {
        //std::cout << "get setcampos" << pos2 << z << std::endl;
        campos.formXYZ(pos2, z);
    }
    void settime() {
        capturetime = high_resolution_clock::now();
    }
    void setcamID(int ID) {
        camID = ID;
    }
    void setidealTime(double t){
        idealTime=t;
    }
  private:
    CVector3 pos;
    CVector3 campos;
    high_resolution_clock::time_point capturetime;
    double idealTime;
    int camID;
    //for test


};
#endif // BALLRECORDS_H
