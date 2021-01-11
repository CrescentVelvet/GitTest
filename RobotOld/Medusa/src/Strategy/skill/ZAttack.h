/***************************************
写出来是啥就是啥
add by Wang Yunkai in 2019/04/26
***************************************/
#ifndef _Z_ATTACK_H_
#define _Z_ATTACK_H_
#include <skill/PlayerTask.h>

class CZAttack : public CStatedTask {
public:
    CZAttack();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
protected:
    virtual void toStream(std::ostream& os) const { os << "Skill: ZAttack\n" << std::endl; }
private:
	int _lastCycle;
    int fraredOn;
};


#endif // !_Z_ATTACK_H_
