#ifndef CZBACK_H
#define CZBACK_H
#include "skill/PlayerTask.h"

class CZBack : public CStatedTask
{
public:
    CZBack();
    virtual void plan(const CVisionModule* pVision);
    virtual CPlayerCommand* execute(const CVisionModule* pVision);
    virtual bool isEmpty() const { return false; }
protected:
    virtual void toStream(std::ostream& os) const { os << "ZBack 2019"; }
private:
    int _lastCycle;
};

#endif // CZBACK_H
