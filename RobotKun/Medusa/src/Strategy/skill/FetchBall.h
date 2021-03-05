/***************************************
for ball placement
add by Wang in 2019/06/21
***************************************/
#ifndef _FETCHBALL_H_
#define _FETCHBALL_H_
#include <skill/PlayerTask.h>

class CFetchBall : public CStatedTask {
public:
    CFetchBall();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
    virtual void toStream(std::ostream& os) const { os << "Skill: FetchBall\n" << std::endl; }
private:
	int _lastCycle;
    int oppNum;
    bool goBackBall = true;
    bool notDribble = false;
    double MIN_DIST = 100;
    int cnt = 0;
};


#endif // !_FETCHBALL_H_
