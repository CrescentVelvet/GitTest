#ifndef DEFENCESQUENCE_H
#define DEFENCESQUENCE_H
#include "geometry.h"
#include <vector>
#include "staticparams.h"
#include "singleton.h"
struct robotThreat {
    int num;
    float value  = 0;
    float preValue = 0;
};
struct debugMessage {
    int MarkCar = -1;
    float futuretotal;
    float dist2ball;
    float dist2goal;
    float angle2goal;
    float turnAngle;
    float guardMinTime;
    float sectorScore;
    CGeoPoint stopPoint;
    CGeoPoint accPoint;
    CGeoPoint leftPoint;
    CGeoPoint rightPoint;
    CGeoPoint centerPoint;

};
class CDefenceSquence
{
public:
    CDefenceSquence();
    void update();
    int getAttackNum(int order){
        if(enemyOrder.size()==0) return -1;
        if (order < 0) return enemyOrder.front().num;
        else if (order >= enemyOrder.size()) return enemyOrder.back().num;
        else return enemyOrder.at(order).num;
    }

    int attackerAmount() {
        return _attackerAmount;
    }
    std::vector<robotThreat> enemyOrder;
private:
    int _cycle;
    int _lastUpDateCycle;
    int _lastChangeLeaderCycle;
    int _attackerAmount;
    float evaluateThreatBasic(CGeoPoint p0, int num);
    float totalEvaluate(int num, bool useFuture);
    void confirmBestEnemy();
    robotThreat enemyThreat[PARAM::Field::MAX_PLAYER];
    int theirLeaderNum;
    std::vector<int> ourPenaltyDefender;
    debugMessage debugTemp[PARAM::Field::MAX_PLAYER];
};
    typedef NormalSingleton< CDefenceSquence > DefenceSquence;
#endif // DEFENCESQUENCE_H
