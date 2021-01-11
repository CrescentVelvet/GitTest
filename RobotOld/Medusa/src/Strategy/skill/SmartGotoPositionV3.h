#ifndef SMARTGOTOPOSITIONV3_H
#define SMARTGOTOPOSITIONV3_H

#include <skill/PlayerTask.h>
#include <WorldDefine.h>
#include <VisionModule.h>
#include "ObstacleNew.h"

struct PlayerCapabilityT;
class CSmartGotoPositionV3 : public CPlayerTask{
public:
    CSmartGotoPositionV3();
    virtual void plan(const CVisionModule* pVision);
protected:
    virtual void toStream(std::ostream& os) const;

private:
    bool current_trajectory_safe;
    void validateFinalTarget(CGeoPoint& finalTarget, const CVisionModule* pVision, bool shrinkTheirPenalty, double avoidLength, bool isGoalie);
    void validateStartPoint(CGeoPoint& startPoint, double avoidLength, bool isGoalie, ObstaclesNew& obs);

    CGeoPoint finalPointBallPlacement(const CGeoPoint& startPoint, const CGeoPoint& target, const CGeoPoint& segPoint1, const CGeoPoint& segPoint2, const double avoidLength, ObstaclesNew& obst, bool& canDirect);
    CVector validateFinalVel(const bool isGoalie, const CGeoPoint& startPos, const CGeoPoint &targetPos, const CVector &targetVel);
    PlayerCapabilityT setCapability(const CVisionModule* pVision);

    CGeoPoint dealPlanFail(CGeoPoint startPoint, CGeoPoint nextPoint, double avoidLength, bool shrinkTheirPenalty);
    int playerFlag;
    PlayerCapabilityT _capability;
    PlayerPoseT nextStep;
    int _lastCycle;
};

#endif // SMARTGOTOPOSITIONV3_H
