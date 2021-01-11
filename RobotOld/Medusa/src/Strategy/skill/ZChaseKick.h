#ifndef __ZCHASE_KICK_H__
#define __ZCHASE_KICK_H__
#include <skill/PlayerTask.h>

class CZChaseKick :public CStatedTask {
public:
	CZChaseKick();
	virtual void plan(const CVisionModule* pVision); 
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: ZChaseKick\n"; }

private:
	CPlayerCommand* _directCommand;	//直接发送命令
	int _lastCycle;
	int _stateCounter;
	int _goKickCouter;
	double _compensateDir;
};

#endif //__ZCHASE_KICK_H__
