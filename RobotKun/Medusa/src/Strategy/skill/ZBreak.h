/***************************************
ZCirclePass upgrade
add by Wang Yunkai in 2019/05/28
***************************************/
#ifndef _Z_BREAK_H_
#define _Z_BREAK_H_
#include <skill/PlayerTask.h>

class CZBreak : public CStatedTask {
public:
    CZBreak();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
    virtual void toStream(std::ostream& os) const { os << "Skill: ZBreak\n" << std::endl; }
private:
	int _lastCycle;
    bool isDribble = false;
    int grabMode;
    int last_mode;
    int fraredOn;
    int fraredOff;
    CGeoPoint dribblePoint;
    CGeoPoint move_point;
};


#endif // !_Z_BREAK_H_
