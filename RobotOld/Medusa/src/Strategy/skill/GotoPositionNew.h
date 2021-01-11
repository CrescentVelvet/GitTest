/*************************************************
* 新版车控skill(V2)，旨在用全新规划替代SmartGoto
* Author: Jia Shenhan
* Created Date: 2019/5/10
**************************************************/
#ifndef _GOTO_POSITION_NEW_H_
#define _GOTO_POSITION_NEW_H_
#include <skill/PlayerTask.h>
#include <geometry.h>
#include "ObstacleNew.h"
/************************************************************************/
/*                     CBezierGotoPosition / 跑位                          */
/************************************************************************/

struct PlayerCapabilityT;

class CGotoPositionNew : public CPlayerTask {
  public:
    CGotoPositionNew();
    virtual void plan(const CVisionModule* pVision);
    virtual CPlayerCommand* execute(const CVisionModule* pVision);
    virtual bool isEmpty() const {
        return false;
    }
    const CGeoPoint& reTarget() const {
        return _target;
    }
  protected:
    virtual void toStream(std::ostream& os) const;
    void validateFinalTarget(CGeoPoint& finalTarget, CGeoPoint myPos, double avoidLength, bool isGoalie, bool isBack,ObstaclesNew& obs);
    PlayerCapabilityT setCapability(const CVisionModule* pVision);
    void drawPath(bool drawPathPoints, bool drawPathLine, bool drawViaPoints, int vecNumber);
    bool validateStartPoint(CGeoPoint& startPos, const PlayerVisionT& me, ObstaclesNew* obst, const PlayerCapabilityT& capability, double avoidLength, bool isGoalie, bool isBack);
    vector < CGeoPoint > forBack(const CVisionModule* _pVision, const int vecNum, const CGeoPoint& startPos, const CGeoPoint& targetPos);
  private:
    CGeoPoint _target;
    int playerFlag;
};

#endif
