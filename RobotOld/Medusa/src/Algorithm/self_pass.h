#ifndef SELF_PASS_H
#define SELF_PASS_H

#include "VisionModule.h"
#include "singleton.h"

namespace SELF {
    const int OurBall = 0;
    const int OurBallHolding = 1;
    const int TheirBall = 2;
    const int TheirBallHolding = 3;
    const int BothBall = 4;
    const int BothBallHolding = 5;
    const int GetBall = 6;
    const int KickBall = 7;
    const int Rush = 8;
}

class CSelfPass{
public:
    CSelfPass();
    void draw_selfpassMsg(const CVisionModule *pVision, const int leader);
    void cal_selfpassPoint(const CVisionModule* pVision, const int leader);
    void cal_selfrunPoint(const CVisionModule* pVision, const int leader);
    bool is_selfpassPoint(const CVisionModule *pVision, const int leader);
    CGeoPoint bestPassPoint() const { return _bestPassPoint; }
    CGeoPoint nowRunPoint() const { return _nowRunPoint; }
    ~CSelfPass(){}
    QString _selfpassStatus;
    int _enemy_num;
private:
    int _ballStatus;
    int _carStatus;
    CGeoPoint _bestPassPoint;
    CGeoPoint _nowRunPoint;
};
typedef NormalSingleton<CSelfPass> SelfPass;
#endif // SELF_PASS_H
