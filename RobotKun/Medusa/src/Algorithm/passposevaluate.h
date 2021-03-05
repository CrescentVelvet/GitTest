#ifndef PASSPOSEVALUATE_H
#define PASSPOSEVALUATE_H
#include <vector>
#include "geometry.h"
#include "singleton.hpp"
enum EvaluateMode{
    PASS = 1,
    SHOOT = 2,
    FREE_KICK = 3,
};
class CPassPosEvaluate {
public:
    CPassPosEvaluate();
    std::vector<float> evaluateFunc(CGeoPoint candidate,CGeoPoint leaderPos, EvaluateMode mode);
    std::vector<CGeoPoint> generateFreeKickPoints();
    bool passBack(CGeoPoint candidate,CGeoPoint leaderPos);
    bool pass2lowThreat(CGeoPoint candidate,CGeoPoint leaderPos);
    bool passTooClose(CGeoPoint candidate,CGeoPoint leaderPos,bool selfPass);
    bool chipTooClose(CGeoPoint candidate,CGeoPoint leaderPos);
    bool validPass(CGeoPoint candidate,CGeoPoint leaderPos,bool chip);
    bool validSelfPass(CGeoPoint candidate,CGeoPoint leaderPos);
};
typedef Singleton<CPassPosEvaluate> PassPosEvaluate;
#endif // PASSPOSEVALUATE_H
