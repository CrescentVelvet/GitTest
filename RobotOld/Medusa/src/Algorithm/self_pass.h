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
    void cal_variousPoint(const CVisionModule* pVision, const int leader);
    void cal_farEnemyPoint(const CVisionModule* pVision, const int leader);
    void cal_orientatePoint(const CVisionModule* pVision, const int leader);
    void cal_moveDirectPoint(const CVisionModule* pVision, const int leader);
    void cal_anglenough_EnemyVel(const CVisionModule* pVision, const int leader);
    bool is_angelpenaltyPoint(const CVisionModule *pVision, const int leader, double height);
    bool is_anglenough_EnemyFar(const CVisionModule *pVision, const int leader, double xita);
    bool is_anglenough_MeMove(const CVisionModule *pVision, const int leader, double xita);
    bool is_anglenough_EnemyVel(const CVisionModule* pVision, const int leader, double xita);
    bool is_selfpassPoint(const CVisionModule *pVision, const int leader);
    bool is_overBoundary(const CVisionModule *pVision, const int leader);
    bool is_selfpass();
    CGeoPoint farEnemyPoint() const { return _farEnemyPoint; }
    CGeoPoint orientatePoint() const { return _orientatePoint; }
    CGeoPoint moveDirectPoint() const { return _moveDirectPoint; }
    ~CSelfPass(){}
private:
    int _ballStatus;
    int _carStatus;
    int _enemy_num;
    double _distanceToEnemy;
    double _angleToEnemy;
    CGeoPoint temp_enemyVel;
    CGeoPoint _farEnemyPoint;
    CGeoPoint _orientatePoint;
    CGeoPoint _moveDirectPoint;
    CGeoPoint temp_farEnemyPoint;
    CGeoPoint temp_orientatePoint;
    CGeoPoint temp_moveDirectPoint;
    QString _selfpassStatus;
    QString _selfpassCannot;
};
typedef NormalSingleton<CSelfPass> SelfPass;
#endif // SELF_PASS_H
