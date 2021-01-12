#ifndef COLLISIONDETECT_H
#define COLLISIONDETECT_H
#include "singleton.hpp"
#include "globaldata.h"
#include "geometry.h"

class CCollisionDetect {
  public:
    CCollisionDetect();
    //virtual ~CCollisionDetect();
    void visionAlart();
    void analyzeData(ReceiveVisionMessage & result);
    bool ballCloseEnough2Analyze();
    bool ballIsOnEdge(CGeoPoint);
  private:
    int _RecordFlag = 0;
    int _RecordIndex = -1;
    int PointN;
    //int LastTouch=0;//0 for struggle; 1 for our ;2 for their; 3 for wall;

};
typedef Singleton<CCollisionDetect> CollisionDetect;

#endif // COLLISIONDETECT_H
