#ifndef ZDRAG_H
#define ZDRAG_H
#include <skill/PlayerTask.h>

class CZDrag : public CStatedTask
{
public:
    CZDrag();
    virtual void plan(const CVisionModule* pVision);
    virtual CPlayerCommand *execute(const CVisionModule *pVision);
protected:
    virtual void toStream(std::ostream& os) const {os << "Skill: ZDrag" << std::endl;}

private:
    int _lastCycle;
    int _lastState;
    int _lastConfirmCycle;
    int _state;
    int enemyNum = -999;
    int freeCount = 0;
    int antiCnt = 0;
    int antiVelCount = 0;
    int realCnt = 0;
    int escapeCnt = 0;
    bool confirmFreeBallDist = false;
    double markDist = 9999;
    CVector v1, v2;
    CVector finalVelVec;
    CGeoPoint antiTarget, realTarget;
    double freeBallDist = 9999;
    bool purposeMode = false;
};

#endif // ZDRAG_H
