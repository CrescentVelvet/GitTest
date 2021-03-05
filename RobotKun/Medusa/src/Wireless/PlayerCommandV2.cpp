﻿#include "PlayerCommandV2.h"
#include <utils.h>
#include <sstream>
//#include "CommandSender.h"
#include <CommandInterface.h>
#include <robot_power.h>
#include <RobotCapability.h>
#include "PlayInterface.h"
#include "TaskMediator.h"

namespace{
const bool SPEED_MODE = true;
inline int merge_command_type(int type, unsigned char dribble)
{
    if( dribble > DRIBBLE_DISABLED){
        type |= 0x08;
    }
    return type;
}
const double RSPEED_DRIBBLEHIGH = 1;	// 当旋转速度大于此值使用

const double ACTION_ERROR = 1.0; // 执行误差
}

/************************************************************************/
/*                      CPlayerSpeed                                    */
/************************************************************************/
//CPlayerSpeedV2::CPlayerSpeedV2(int number, double xspeed, double yspeed, double rspeed, unsigned char dribble, int flag)
//	: CPlayerCommand(number,dribble)
//	,_rspeed(rspeed)
CPlayerSpeedV2::CPlayerSpeedV2(int number, double xspeed, double yspeed, double rspeed, bool useDir, unsigned char dribble, int flag)
    : CPlayerCommand(number,dribble)
    ,_rspeed(rspeed),_use_dir(useDir)
{
    const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(1, 1);
    CVector velocity(xspeed, yspeed);

    // 把输出指令限制在小车的运动能力之内 added by qxz 08.06.07
    if (flag == 0) {
        if (number == TaskMediator::Instance()->goalie()) {
            if (velocity.mod() > robotCap->maxSpeedGoalie(0))
                velocity = Utils::Polar2Vector(robotCap->maxSpeedGoalie(0), velocity.dir());
        } else if (TaskMediator::Instance()->leftBack() != 0 && number == TaskMediator::Instance()->leftBack()
                   || TaskMediator::Instance()->rightBack() != 0 && number == TaskMediator::Instance()->rightBack()
                   || TaskMediator::Instance()->singleBack() != 0 && number == TaskMediator::Instance()->singleBack()
                   || TaskMediator::Instance()->sideBack() != 0 && number == TaskMediator::Instance()->sideBack()) {
            if (velocity.mod() > robotCap->maxSpeedBack(0))
                velocity = Utils::Polar2Vector(robotCap->maxSpeedBack(0), velocity.dir());
        } else {
            if (velocity.mod() > robotCap->maxSpeed(0))
                velocity = Utils::Polar2Vector(robotCap->maxSpeed(0), velocity.dir());
        }
    }
    else {
        if (velocity.mod() > robotCap->maxRushTranslationSpeed())
            velocity = Utils::Polar2Vector(robotCap->maxRushTranslationSpeed(), velocity.dir());
    }

    _xspeed = velocity.x();
    _yspeed = velocity.y();
}

void CPlayerSpeedV2::execute(int realNum){
//    CCommandInterface::instance()->setSpeed(realNum, dribble(), xspeed(), yspeed(), rspeed());
    CCommandInterface::instance()->setSpeed(realNum, dribble(), xspeed(), yspeed(), rspeed(), use_dir());
}
void CPlayerSpeedV2::toStream(std::ostream& os) const
{
    os << xspeed() << "\t" << yspeed() << "\t" << rspeed();
}
CVector CPlayerSpeedV2::getAffectedVel() const
{
    return CVector(xspeed(), yspeed());
}
double CPlayerSpeedV2::getAffectedRotateSpeed() const
{
    return rspeed();
}

bool CPlayerSpeedV2::getAffectedUseDir() const
{
    return use_dir();
}

/************************************************************************/
/*                          Kick                                        */
/************************************************************************/
void CPlayerKickV2::execute(int realNum){
    CCommandInterface::instance()->setKick(realNum, _normal, _chip);
}
void CPlayerKickV2::toStream(std::ostream& os) const
{
    os << number();
    if( _normal ){
        if( _pass ){
            os << " pass " << _pass;
        }else{
            os << " kick " << _normal;
        }
    }else{
        os << " chip kick " << _chip;
    }
}

/************************************************************************/
/*                         Gyro(陀螺仪)                                 */
/************************************************************************/
void CPlayerGyroV2::execute()
{
    cout << "Unknown execute in CPlayerGyroV2" << endl;
}
void CPlayerGyroV2::toStream(std::ostream& os) const
{
    os << "rotNum "<< number()<<"radius "<<_radius<<"angle "<<_angle;
}
