#ifndef _GET_BALL_V4_H__
#define _GET_BALL_V4_H__
#include <skill/PlayerTask.h>
#include <list>
/**********************************************************
* Skill: GetBallV4
* Description:拿球第四版
***********************************************************/
//#define FILTER_NUM (3)
using namespace std;
struct interInfo {
    double interTimes;
    double interPointXs;
    double interPointYs;
    bool operator < (const interInfo& a) {
        return (interTimes < a.interTimes);
    }
};
class CGetBallV4 :public CStatedTask {
public:
	CGetBallV4();
	virtual void plan(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	int getStaticDir(const CVisionModule* pVision, int staticDir);
    void judgeMode(const CVisionModule* pVision);
	bool canShootBall(const CVisionModule* pVision);
    bool judgeShootMode(const CVisionModule* pVision);
    bool predictedChipInterTime(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, CGeoPoint secondPoint, CVector3 ChipKickvel,double firstChipTime, double RestChiptime,double responseTime);
protected:
	virtual void toStream(std::ostream& os) const { os << "Skill: GetBallV4\n"; }

private:
//    void meanFilter(int robotNum, double &interTime, CGeoPoint &interPoint);
//    bool hysteresisPredict(const CVisionModule* pVision, int robotNum, CGeoPoint& interceptPoint, double& interTime, double buffer=0.2);
	int _lastCycle;
	int getBallMode;
    int lastGetBallMode;
	int cnt = 0;
	bool needdribble = false;
	bool needkick = false;
	bool chip = false;
	bool safeMode = false;
    bool rushMode = false;
	bool isTouch = false;
    bool ballplacement = false;
	CGeoPoint targetPoint;
	CGeoPoint waitPoint;
    CGeoPoint ballLineProjection;
    //double meArriveTime = 9999;
    CGeoPoint interPoint;
    CGeoPoint chipInterPoint;
    double interTime = 9999;
	double last_speed = 0;
    bool needAvoidBall;
    bool canGetBall;
    bool canForwardShoot;
    int fraredOn;
    int fraredOff;
    int staticDir;
//    double interTimes[FILTER_NUM] = {0};
//    double interPointXs[FILTER_NUM] = {0};
//    double interPointYs[FILTER_NUM] = {0};
//    list <interInfo> inters;
    bool lastInterState = false;
};

#endif //_GET_BALL_V4_H__
