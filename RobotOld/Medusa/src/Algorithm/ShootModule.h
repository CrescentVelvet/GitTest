#ifndef _SHOOT_MODULE_
#define _SHOOT_MODULE_

#include <VisionModule.h>

#define NEW_COMPENSATE	false

class CVisionModule;
/**
* Shoot.
*分两级，第一级判定player是否能够射门，第二级得到射门的角度与速度
* 使用指南：
*   1、进行能否成功射门判断，利用GenerateShootDir
*   2、在能够射门时，利用getSpeed和getDir读取速度与角度
*
* player 指队员号(只是为了取得robotCap,若小车性能一样,可任取1~6值)
* pos 用于直接计算某点处(而不是球的位置)的射门空挡
*/

class CShootModule{
public:
    CShootModule();
    bool generateShootDir(const int player=-1);
    bool generateBestTarget(const CVisionModule *pVision, CGeoPoint &bestTarget, const CGeoPoint &pos = CGeoPoint(0, 0));
//    bool  validShootPos(const CVisionModule* pVision, CGeoPoint shootPos, double ballVel, CGeoPoint target, double &interTime,
//                       const double responseTime = 0, double ignoreCloseEnemyDist = -9999, bool ignoreTheirGoalie = false,
//                       bool ignoreTheirGuard = false, bool DEBUG_MODE = false);
    bool canShoot(const CVisionModule *pVision, CGeoPoint shootPos);
//---------内部接口-----------
    bool getKickValid ()  {return _kick_valid;}
    bool getCompensate () {return _is_compensated;}
    double getRawKickDir () {return _raw_kick_dir;}
    double getCompensateDir () {return _compensate_value;}
    double getRealKickDir () {return _real_kick_dir;}
//----------svm法计算补偿角度----------
    double calCompensate(double x, double y);
//----------对外接口----------
    double getSpeed(){return _needBallVel;}
    double getDir(){return _real_kick_dir;}

//	void getData(double shoot_vel, double shoot_dir, double outdir);

private:
    void reset();

    bool  _kick_valid;			// 判断能否踢球，即路线是否被完全封堵
    CGeoPoint _kick_target;		// 需要踢到的点
    double _raw_kick_dir;		// 希望球传出的角度，或者射门的最佳角度
//----------补偿相关参数----------
    bool  _is_compensated;		// 小车方向是否需要补偿
    double _compensate_factor;  // 根据速度调整的补偿因子,预留讨论
    double _compensate_value;	// 补偿角度

    double rawdir;//原始待补偿的球速线和车到目标点的夹角
    double shoot_speed;//原始速度
    double shoot_dir;//原始角度
//----------对外接口----------
    double _real_kick_dir;		// 经过角度补偿，小车实际需要摆出的角度
    double _needBallVel;        //经过角度补偿，小车实际需要射出的速度
//----------算法所需参数----------
    double tolerance;
    double stepSize;    //度数步长 弧度制
    double responseTime;
};

typedef NormalSingleton< CShootModule > ShootModule;

#endif // _KICK_DIRECTION_
