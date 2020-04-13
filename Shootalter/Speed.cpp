#include "Speed.h"
#include <CommandFactory.h>
#include "VisionModule.h"
#include "robot_power.h"
#include "DribbleStatus.h"
#include "ShootModule.h"
#include <KickStatus.h>
#include <cmath>
#include "parammanager.h"

CSpeed::CSpeed()
{

}

void CSpeed::plan(const CVisionModule* pVision)
{

}

CPlayerCommand* CSpeed::execute(const CVisionModule* pVision)
{
    int myNum = task().executor;
    const PlayerVisionT& me = pVision->ourPlayer(myNum);
    const MobileVisionT& ball = pVision->ball();
//    double power_bias;
//    ZSS::ZParamManager::instance()->loadParam(power_bias,"KickLimit/FlatKickMax",1);
//    double power = task().player.rotdir;
    double power = me.Pos().dist(task().player.pos) * 1 + 1000;
    if(power>7500){power=7500;}
    if(power<2000){power=2000;}
//    qDebug()<<power;
    double myDir = pVision->ourPlayer(myNum).Dir();
    double dribblePower = 3;
    CVector ball2target = task().player.pos - ball.RawPos();// 球到目标点的向量
    double target_Dir = ball2target.dir();
	double speed_x = task().player.speed_x; // x方向平动速度
	double speed_y = task().player.speed_y; // y方向平动速度
    double rad_vel = speed_x*cos(myDir) + speed_y*sin(myDir);// 径向平动速度
//    double tan_vel = (speed_x*sin(myDir) - speed_y*cos(myDir)) / 50 * -1 * (sin(myDir)/fabs(sin(myDir)));// 切向平动速度
//    double tan_vel = (speed_y*cos(myDir) - speed_x*sin(myDir)) / 50 * 0.35;// 切向平动速度
    double tan_vel = (speed_y*cos(myDir) - speed_x*sin(myDir)) / 50 * ball2target.mod() * 0.0001;// 切向平动速度
    double rotSpeed = task().player.rotate_speed; // 转动速度
    double real_rotSpeed = me.RawRotVel();// 从图像信息读取真正的转动角速度
    double precision = 0.01;// 精度
//    double prediction = 140 * atan(real_rotSpeed/power) + 0;// 预测提前量(弧度)(仅考虑转动)
//    double prediction = 138 * atan((real_rotSpeed+tan_vel)/(power+rad_vel)) + 0;// 预测提前量(弧度)(还考虑平动)
    double prediction = 0;// 无补偿测试
	CVector globalVel(speed_x, speed_y); // 全局坐标系中的速度矢量
    CVector localVel = globalVel.rotate(-myDir);
//    qDebug()<<tan_vel;
//    qDebug()<<real_rotSpeed;
//    qDebug()<<Utils::Normalize(target_Dir - me.RawDir());
//    qDebug()<<ball2target.mod()*0.0001;
//    std::cout<<Utils::Normalize(target_Dir - me.RawDir())<<'\t'<<prediction<<'\n';
    if (fabs(Utils::Normalize(target_Dir - me.RawDir() - prediction)) < precision) {
        KickStatus::Instance()->setKick(myNum, power);
//        qDebug()<<"fuck"<<fabs(Utils::Normalize(me.RawDir() - target_Dir));
    }
    DribbleStatus::Instance()->setDribbleCommand(myNum, 3);
	return CmdFactory::Instance()->newCommand(CPlayerSpeedV2(myNum, localVel.x(), localVel.y(), rotSpeed, dribblePower));
}
