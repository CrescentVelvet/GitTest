#ifndef RECEIVEPOS_H
#define RECEIVEPOS_H
#include "VisionModule.h"
#include "singleton.hpp"

/*******************************************************/
/*            generate pass pos module                 */
/*            migrated from balladvancedecison         */
/*******************************************************/

class Area{
public:
    Area(CGeoRectangle location=CGeoRectangle(0, 0, 0, 0), CGeoPoint targetPos=CGeoPoint(0,0), bool status=false){
        this->location = location;
        this->targetPos = targetPos;
        this->status = status;
    }
    CGeoRectangle location;
    CGeoPoint targetPos;
    bool status;
};

class ReceivePos
{
public:
    ReceivePos();
    void generatePassPos(const CVisionModule* pVision, const int leader);
    void generateRandomTestPos(const CVisionModule* pVision);
    void testCasePlusPlus();
    CGeoPoint receiveBallPointCompute(const CVisionModule* pVision, int leader,const std::vector<CGeoPoint>& targetPoint);

    int bestReceiver() const { return _bestReceiver; }
    CGeoPoint bestPassPoint() const { return _bestPassPoint; }
private:
    std::vector<Area> areas;
    double lineAmount;
    double columnAmount;
    int _bestReceiver;
    int _receiver;
    CGeoPoint _bestPassPoint;
};
typedef NormalSingleton<ReceivePos> ReceivePosModule;

#endif // RECEIVEPOS_H
