#include "kickregulation.h"
#include "staticparams.h"
#include "Global.h"
#include "GDebugEngine.h"
#include "parammanager.h"

namespace  {
double ROLL_FRICTION_RADIO[2] = {1.05, 1.28};
double HIGH_REGULATION_RADIO = 3.75;
double DRIBBLE_POWER_RADIO = 1;
bool IS_SIMULATION = false;
bool KICK_DEBUG = true;
bool IS_RIGHT = false;

const double DRIBBLE_CHIP_DIR = 50 * Param::Math::PI / 180;
}

CKickRegulation::CKickRegulation()
{
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION,"Alert/IsSimulation",false);
    ZSS::ZParamManager::instance()->loadParam(KICK_DEBUG, "KickRegulation/Debug", true);
    ZSS::ZParamManager::instance()->loadParam(ROLL_FRICTION_RADIO[0], "KickRegulation/lowPowerRadio", 1.05);
    ZSS::ZParamManager::instance()->loadParam(ROLL_FRICTION_RADIO[1], "KickRegulation/HighPowerRadio", 1.28);
    ZSS::ZParamManager::instance()->loadParam(HIGH_REGULATION_RADIO, "KickRegulation/HighRadio4Vx/Vy", 3.75);
    ZSS::ZParamManager::instance()->loadParam(DRIBBLE_POWER_RADIO, "KickRegulation/dribblePowerRadio", 1.0);
    ZSS::ZParamManager::instance()->loadParam(IS_RIGHT, "ZAlert/IsRight", false);
}

bool CKickRegulation::regulate(int player, const CGeoPoint& target, double& needBallVel, double& playerDir, bool isChip) {
    CVisionModule* pVision = vision;
    const MobileVisionT& ball = pVision->Ball();
    const PlayerVisionT& kicker = pVision->OurPlayer(player);

    if (fabs(kicker.Dir() - playerDir) > Param::Math::PI / 2 ) {
        return false;
    }

    double originVel = needBallVel;
    double vel2targetDir;
    double tanVel2Target;
    double paralVel2Target;
    double ballRotVel;
    if (ball.Valid()) {
        vel2targetDir = Utils::Normalize(ball.Vel().dir() - playerDir);
        tanVel2Target = ball.Vel().mod() * std::sin(vel2targetDir);
        paralVel2Target = ball.Vel().mod() * std::cos(vel2targetDir);
        ballRotVel = 0;
    } else {
        vel2targetDir = Utils::Normalize(kicker.Vel().dir() - playerDir);
        tanVel2Target = kicker.Vel().mod() * std::sin(vel2targetDir);//车相对于目标点的切向速度
        paralVel2Target = kicker.Vel().mod() * std::cos(vel2targetDir);//车在与目标点连线上的速度
        ballRotVel = kicker.RotVel() * Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER;//在吸球的情况下球的旋转线速度
    }


    CVector originDirVector = CVector(needBallVel,0).rotate(playerDir);
    GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + originDirVector, COLOR_WHITE);
    //速度合成
    //1. 切向补偿
    double tanVelCompensate = 9999.0;
    tanVelCompensate = (tanVel2Target + ballRotVel);
    if (isChip && !IS_SIMULATION) {
        tanVelCompensate *= 1;
    }

    //2. 连线方向补偿
    if (IS_SIMULATION) {
        paralVel2Target = 0;
    } else {
        if (isChip) {
            needBallVel = std::sqrt(980 * needBallVel/ (2 * std::tan(50 * Param::Math::PI / 180)));//挑球需要把距离换算为x方向速度
            needBallVel = (-paralVel2Target + std::sqrt(paralVel2Target * paralVel2Target + 2 * originVel * 980 / std::tan(DRIBBLE_CHIP_DIR))) / 2;//利用补偿前后vy = vx*tan50不变来计算t，根据路程(originVel)计算补偿后的vx，一元二次方程中，两个根一正一负，取正值
        }
        else {
            needBallVel -= paralVel2Target;
            if (needBallVel / std::fabs(tanVelCompensate) < HIGH_REGULATION_RADIO) {
                needBallVel *= ROLL_FRICTION_RADIO[1];
            } else {
                needBallVel *= ROLL_FRICTION_RADIO[0];
            }
        }
    }

    //3. 角度合成及调整
    double compensateDir = std::atan2(tanVelCompensate, needBallVel);
    playerDir -= compensateDir;

    //4. 力度合成及调整
    needBallVel = std::sqrt(std::pow(needBallVel, 2) + std::pow(tanVelCompensate, 2));
    if (!(IS_SIMULATION || isChip)) {
        needBallVel *= DRIBBLE_POWER_RADIO;
    }
    if (isChip) {
        needBallVel = 2 * std::pow(needBallVel, 2) * std::tan(50 * Param::Math::PI / 180) / 980;
    }

    needBallVel = needBallVel > 600 ? 600 : needBallVel;
    //射门时力度特例
    if (Utils::InTheirPenaltyArea(target, 0)) {
        needBallVel = std::max(650.0, needBallVel);
    }

    if (KICK_DEBUG) {
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + CVector(500,0).rotate(kicker.Dir()));
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + CVector(500,0).rotate(kicker.Dir() + 3 * Param::Math::PI / 180), COLOR_GRAY);
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + CVector(500,0).rotate(kicker.Dir() - 3 * Param::Math::PI / 180), COLOR_GRAY);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, -350) * (IS_RIGHT ? -1 : 1), QString("vy: %1  vx: %2  rotV: %3  v: %4").arg(paralVel2Target).arg(tanVel2Target).arg(kicker.RotVel()).arg(vel2targetDir).toLatin1(),COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, -330) * (IS_RIGHT ? -1 : 1), QString("oV: %1   V: %2").arg(originVel).arg(needBallVel).toLatin1(),COLOR_ORANGE);
        CVector dirVector = CVector(needBallVel,0).rotate(playerDir);
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + dirVector, COLOR_YELLOW);//调整后角度方向
        CVector tanVel2TargetVector = CVector(tanVelCompensate,0).rotate(originDirVector.dir()-Param::Math::PI/2.0);// (target - kicker.Pos()).rotate(-Param::Math::PI/2.0) / (target - kicker.Pos()).mod() * tanVelCompensate;
        CVector paralVel2TargetVector = CVector(paralVel2Target,0).rotate(originDirVector.dir());// (target - kicker.Pos())/ (target - kicker.Pos()).mod() * paralVel2Target;
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + tanVel2TargetVector, COLOR_PURPLE);//切向补偿
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + paralVel2TargetVector, COLOR_GREEN);//连线方向补偿
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + kicker.Vel(), COLOR_RED);
    }

    return true;
}
