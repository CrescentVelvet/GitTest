#ifndef _ZMARKING_H_
#define _ZMARKING_H_
#include <skill/PlayerTask.h>

/************************************************************************
@Brief: 非零速盯人
Add by Wang Yunkai in 2018.05.18
Modify by Wang Yunkai in 2019.6.24
************************************************************************/
class CZMarking : public CStatedTask{
public:
	CZMarking();
	virtual void plan(const CVisionModule* pVision);
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
protected:
	virtual void toStream(std::ostream& os) const { os << "Marking Defense"; }

private:
	int _lastCycle;
};
#endif //_ZMARKING_H_
