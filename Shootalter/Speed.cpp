#include <cmath>
#include <QDateTime>
#include "Speed.h"
#include <CommandFactory.h>
#include "VisionModule.h"
#include "robot_power.h"
#include "DribbleStatus.h"
#include "ShootModule.h"
#include <KickStatus.h>
#include "parammanager.h"
#include "messidecision.h"

CSpeed::CSpeed()
{

}

void CSpeed::plan(const CVisionModule* pVision)
{

}

CPlayerCommand* CSpeed::execute(const CVisionModule* pVision)
{
    int myNum = task().executor;
    int kick_pre = 1;
    const PlayerVisionT& me = pVision->ourPlayer(myNum);
    const MobileVisionT& ball = pVision->ball();
    double speed_x = task().player.speed_x; // x方向平动速度
    double speed_y = task().player.speed_y; // y方向平动速度
    double myDir = pVision->ourPlayer(myNum).Dir();
    double rotSpeed = task().player.rotate_speed; // 转动速度
    double real_rotSpeed = me.RawRotVel();// 从图像信息读取真正的转动角速度
    double power = task().player.rotdir;// 力度从lua中提取
//    qDebug()<<power;
    if (kick_pre == 1){
        //时间一到直接踢球
        QDateTime KICK(QDateTime::currentDateTimeUtc());
        static qint64 kick_timeold = KICK.toMSecsSinceEpoch();
        static CGeoPoint ball_pos(0,0);
        static int ballPosFlag = 1;
        if (ball.Pos().dist(me.Pos()) > 500 && ballPosFlag == 1){
            ball_pos = ball.Pos();
            ballPosFlag = 0;
            MessiDecision::Instance()->_speedBallPos = CGeoPoint((ball_pos.x()-me.Pos().x())*4+me.Pos().x(),(ball_pos.y()-me.Pos().y())*4+me.Pos().y());
//            qDebug()<<atan(MessiDecision::Instance()->_speedBallPos.y()/MessiDecision::Instance()->_speedBallPos.x())*180/3.14;
//            qDebug()<<atan((ball_pos.y()-me.Pos().y())/(ball_pos.x()-me.Pos().x()))*180/3.14;
//            qDebug()<<ball_pos.x()<<'\t'<<me.Pos().x();
//            qDebug()<<ball_pos.y()<<'\t'<<me.Pos().y();
        }
        if (KICK.toMSecsSinceEpoch() - kick_timeold > 20000){
            kick_timeold = KICK.toMSecsSinceEpoch();
        }
        if (KICK.toMSecsSinceEpoch() - kick_timeold > 5000 && KICK.toMSecsSinceEpoch() - kick_timeold < 20000){
            if (fabs(real_rotSpeed) > 1){KickStatus::Instance()->setKick(myNum, power);}
            kick_timeold = KICK.toMSecsSinceEpoch();
            MessiDecision::Instance()->_speedMePos = me.Pos();
            MessiDecision::Instance()->_speedCarPos = CGeoPoint(me.Pos().x()+cos(me.Dir())*2000,me.Pos().y()+sin(me.Dir())*2000);
//            qDebug()<<me.Dir();
//            qDebug()<<me.RawRotVel();
            ballPosFlag = 1;
        }
//        qDebug()<<KICK.toMSecsSinceEpoch() - kick_timeold;
//        qDebug()<<KICK.toMSecsSinceEpoch();
    }
    else {
        CVector ball2target = task().player.pos - ball.RawPos();// 球到目标点的向量
        double target_Dir = ball2target.dir();
//        double rad_vel = speed_x*cos(myDir) + speed_y*sin(myDir);// 径向平动速度
//        double tan_vel = (speed_x*sin(myDir) - speed_y*cos(myDir)) / 50 * -1 * (sin(myDir)/fabs(sin(myDir)));// 切向平动速度
//        double tan_vel = (speed_y*cos(myDir) - speed_x*sin(myDir)) / 50 * 0.35;// 切向平动速度
//        double tan_vel = (speed_y*cos(myDir) - speed_x*sin(myDir)) / 50 * ball2target.mod() * 0.0001;// 切向平动速度
        double precision = 0.01;// 精度
        double prediction = 138 * atan(real_rotSpeed/power) + 0;// 预测提前量(弧度)(仅考虑转动)
//        double prediction = 138 * atan((real_rotSpeed+tan_vel)/(power+rad_vel)) + 0;// 预测提前量(弧度)(还考虑平动)
//        double prediction = 0;// 无补偿测试
        //对准目标点踢球
        if (fabs(Utils::Normalize(target_Dir - me.RawDir() - prediction)) < precision) {
            KickStatus::Instance()->setKick(myNum, power);
            qDebug()<<target_Dir;
            qDebug()<<me.RawDir();
        }
    }
	CVector globalVel(speed_x, speed_y); // 全局坐标系中的速度矢量
    CVector localVel = globalVel.rotate(-myDir);
    double dribblePower = 3;
//    qDebug()<<tan_vel;
//    qDebug()<<real_rotSpeed;
//    qDebug()<<Utils::Normalize(target_Dir - me.RawDir());
//    qDebug()<<ball2target.mod()*0.0001;
//    std::cout<<Utils::Normalize(target_Dir - me.RawDir())<<'\t'<<prediction<<'\n';
    DribbleStatus::Instance()->setDribbleCommand(myNum, 3);
	return CmdFactory::Instance()->newCommand(CPlayerSpeedV2(myNum, localVel.x(), localVel.y(), rotSpeed, dribblePower));
}
