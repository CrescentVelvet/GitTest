#include "Speed.h"
#include <CommandFactory.h>
#include "VisionModule.h"
#include "robot_power.h"
#include "DribbleStatus.h"
#include "ShootModule.h"
#include <KickStatus.h>
#include <cmath>

CSpeed::CSpeed()
{

}

void CSpeed::plan(const CVisionModule* pVision)
{

}

CPlayerCommand* CSpeed::execute(const CVisionModule* pVision)
{
    int myNum = task().executor;
    double power = task().player.rotdir;
//    qDebug()<<power;
    const PlayerVisionT& me = pVision->ourPlayer(myNum);
    const MobileVisionT& ball = pVision->ball();
    double myDir = pVision->ourPlayer(myNum).Dir();
	double speed_x = task().player.speed_x; // x方向平动速度
	double speed_y = task().player.speed_y; // y方向平动速度
    double rotSpeed = task().player.rotate_speed; // 转动速度
    double real_rotSpeed = me.RawRotVel();
    double dribblePower = 3;
    CVector ball2target = task().player.pos - ball.RawPos();
    double target_Dir = ball2target.dir();
    double precision = 0.01;//传球精度
    double prediction = 76.5*atan(rotSpeed/power) + 0.1;//预测提前量(弧度)
//    prediction = 0;
	CVector globalVel(speed_x, speed_y); // 全局坐标系中的速度矢量
    CVector localVel = globalVel.rotate(-myDir);
//    qDebug()<<Utils::Normalize(me.RawDir());
//    qDebug()<<Utils::Normalize(target_Dir);
//    qDebug()<<fabs(Utils::Normalize(me.RawDir() - target_Dir));
//    qDebug()<<fabs(Utils::Normalize(target_Dir - me.RawDir() - prediction));
//    qDebug()<<rotSpeed;
//    qDebug()<<power;
//    qDebug()<<fabs(Utils::Normalize(target_Dir - me.RawDir() - prediction));
//    qDebug()<<real_rotSpeed;
    if (fabs(Utils::Normalize(target_Dir - me.RawDir() - prediction)) < precision) {
        KickStatus::Instance()->setKick(myNum, power);
//        qDebug()<<"fuck"<<fabs(Utils::Normalize(me.RawDir() - target_Dir));
    }
    DribbleStatus::Instance()->setDribbleCommand(myNum, 3);
	return CmdFactory::Instance()->newCommand(CPlayerSpeedV2(myNum, localVel.x(), localVel.y(), rotSpeed, dribblePower));
}
