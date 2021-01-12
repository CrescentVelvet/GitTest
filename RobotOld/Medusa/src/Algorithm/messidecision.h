#ifndef MESSIDECISION_H
#define MESSIDECISION_H
#include "VisionModule.h"
#include "singleton.h"
#include "WorldModel.h"
#include "runpos.h"

class CMessiDecision {
  public:
    CMessiDecision();
    void generateAttackDecision(const CVisionModule* pVision);
    bool messiRun() {
        return _messiRun;
    }
    int leaderNum() {
        return _leader;
    }
    int receiverNum() {
        return _receiver;
    }
    int attackerAmount() {
        return _attackerAmount;
    }
    int attackerNum(int i) {
        if (i < 0) return _attacker.front();
        else if (i >= _attacker.size()) return _attacker.back();
        else return _attacker.at(i);
    }
    string nextState() {
        return _state;
    }
    string leaderState() {
        return _leaderState;
    }
    CGeoPoint passPos() {
        return _passPos;
    }
    CGeoPoint flatPassPos() {
        return _flatPassPos;
    }
    CGeoPoint flatShootPos() {
        return _flatShootPos;
    }
    CGeoPoint chipPassPos() {
        return _chipPassPos;
    }
    CGeoPoint chipShootPos() {
        return _chipShootPos;
    }
    CGeoPoint freeKickPos();
    CGeoPoint receiverPos() {
        return _receiverPos;
    }
    CGeoPoint leaderPos() {
        return _leaderPos;
    }
    CGeoPoint leaderWaitPos() {
        return _leaderWaitPos;
    }
    CGeoPoint otherPos(int i) {
        return _otherPos[i - 1];
    }
    CGeoPoint goaliePassPos();
    bool isFlat() {
        return  _isFlat;
    }
    bool needKick() {
        return _canKick;
    }
    bool needChip() {
        return !_isFlat;
    }
    double passVel() {
        return _passVel;
    }
    CGeoPoint freeKickWaitPos();
    bool canDirectKick();
    void reset();
private:
    int _leader;
    int _receiver;
    int _cycle;
    int _lastRecomputeCycle;
    int _lastChangeReceiverCycle;
    int _lastChangeLeaderCycle;
    int _lastUpdateRunPosCycle;
    std::vector<int> _attacker;
    int _attackerAmount;
    int _timeToGetResult;
    int inValidCnt;
    double _passVel;
    bool _messiRun;
    bool _ourBall;
    bool _theirBall;
    bool _both;
    bool _isFlat;
    bool _canKick;
    string _state;
    string _laststate;
    string _leaderState;
    CGeoPoint _passPos;
    CGeoPoint _receiverPos;
    CGeoPoint _leaderPos;
    CGeoPoint _leaderWaitPos;
    CGeoPoint _otherPos[4];
    CGeoPoint _flatPassPos;
    CGeoPoint _flatShootPos;
    CGeoPoint _chipPassPos;
    CGeoPoint _chipShootPos;
    const CVisionModule* _pVision;
    void judgeState();
    void judgeBallState();
    void judgeLeaderState();
    void judgeAttackerAmount();
    void generateLeaderPos();
    bool confirmLeader();
    bool needReceivePos();
    bool needRunPos();
    void generateReceiverAndPos();
    void getPassPos();
    void generateOtherRunPos();
    bool canShootGuard();
//move to worldModule
//    int getEnemyAmountInArea(CGeoPoint center, int radius);
//    int getEnemyAmountInArea(int x1, int x2, int y1, int y2);
};
typedef NormalSingleton<CMessiDecision> MessiDecision;
#endif // MESSIDECISION_H
