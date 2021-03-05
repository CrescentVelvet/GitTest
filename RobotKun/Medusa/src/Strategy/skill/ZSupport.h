/***************************************
@Brief: 协助leader抢球，站在与leader抢球位置相反的地方，当周围有对方协助机器人时改为盯防
@Author: Wang in 2019/06/17
@e-mail: wangyunkai.zju@gmail.com
***************************************/
#ifndef _Z_SUPPORT_H_
#define _Z_SUPPORT_H_
#include <skill/PlayerTask.h>

class CZSupport : public CStatedTask {
public:
    CZSupport();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
    int getTheirSupport(const CVisionModule* pVision, const int oppNum, const double dist);
protected:
    virtual void toStream(std::ostream& os) const { os << "Skill: ZSupport\n" << std::endl; }
private:
	int _lastCycle;
    int oppNum;
};


#endif // !_Z_SUPPORT_H_
