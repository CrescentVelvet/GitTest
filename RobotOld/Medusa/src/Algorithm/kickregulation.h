#ifndef KICKREGULATION_H
#define KICKREGULATION_H

#include "singleton.hpp"
#include "geometry.h"
class CKickRegulation
{
public:
    CKickRegulation();
    bool regulate(int player, const CGeoPoint& target, double& needBallVel, double& playerDir, bool isChip = false);
};
typedef Singleton<CKickRegulation> KickRegulation;
#endif // KICKREGULATION_H
