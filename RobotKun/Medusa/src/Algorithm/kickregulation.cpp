#include "kickregulation.h"
#include "staticparams.h"
#include "Global.h"
#include "GDebugEngine.h"
#include "parammanager.h"

namespace  {
double ROLL_FRICTION_RADIO[2] = {1.05, 1.28};
double HIGH_REGULATION_RADIO = 3.75;
double DRIBBLE_POWER_RADIO = 1;
double FLAT_KICK_MAX = 6300.0;
bool IS_SIMULATION = false;
bool KICK_DEBUG = true;
bool IS_RIGHT = false;
bool DEBUG_PRINT = false;
const double DRIBBLE_CHIP_DIR = 50.0 * PARAM::Math::PI / 180.0;
}

CKickRegulation::CKickRegulation()
{
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION,"Alert/IsSimulation",false);
    ZSS::ZParamManager::instance()->loadParam(KICK_DEBUG, "KickRegulation/Debug", true);
    ZSS::ZParamManager::instance()->loadParam(DEBUG_PRINT, "Debug/KickRegulation", false);
    ZSS::ZParamManager::instance()->loadParam(ROLL_FRICTION_RADIO[0], "KickRegulation/lowPowerRadio", 1.05);
    ZSS::ZParamManager::instance()->loadParam(ROLL_FRICTION_RADIO[1], "KickRegulation/HighPowerRadio", 1.28);
    ZSS::ZParamManager::instance()->loadParam(HIGH_REGULATION_RADIO, "KickRegulation/HighRadio4Vx/Vy", 3.75);
    ZSS::ZParamManager::instance()->loadParam(DRIBBLE_POWER_RADIO, "KickRegulation/dribblePowerRadio", 1.0);
    ZSS::ZParamManager::instance()->loadParam(FLAT_KICK_MAX, "KickLimit/FlatKickMax", 6300.0);
    ZSS::ZParamManager::instance()->loadParam(IS_RIGHT, "ZAlert/IsRight", false);
}

bool CKickRegulation::regulate(int player, const CGeoPoint& target, double& needBallVel, double& playerDir, bool isChip) {
    auto& ball = vision->ball();
    auto& kicker = vision->ourPlayer(player);

    if (std::fabs(kicker.Dir() - playerDir) > PARAM::Math::PI / 2 ) {
        return false;
    }

    double originVel = needBallVel;
    double vel2targetDir;
    double tanVel2Target;
    double paralVel2Target;
    double ballRotVel;
//    if (ball.Valid()) {
//        vel2targetDir = Utils::Normalize(ball.RawVel().dir() - playerDir);
//        tanVel2Target = ball.RawVel().mod() * std::sin(vel2targetDir);
//        paralVel2Target = ball.RawVel().mod() * std::cos(vel2targetDir);
//        ballRotVel = 0;
//    } else {
        vel2targetDir = Utils::Normalize(kicker.RawVel().dir() - playerDir);
        tanVel2Target = kicker.RawVel().mod() * std::sin(vel2targetDir);//车相对于目标点的切向速度
        paralVel2Target = kicker.RawVel().mod() * std::cos(vel2targetDir);//车在与目标点连线上的速度
        ballRotVel = kicker.ImuRotateVel() * PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER;//在吸球的情况下球的旋转线速度
//    }


    CVector originDirVector = CVector(needBallVel,0).rotate(playerDir);
    GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + originDirVector, COLOR_WHITE);
    //速度合成
    //1. 切向补偿
    double tanVelCompensate = (tanVel2Target + ballRotVel);// * std::cos(vel2targetDir));
    tanVelCompensate *= std::abs(kicker.ImuRotateVel()) / 1.5;
    //todo!!!! tanvel dir is wrong!!!
    //paralVel2Target -= ballRotVel * std::sin(vel2targetDir);
    if(false){
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, 0), QString("kicker_vel: %1  ballrot_vel: %2").arg(kicker.RawVel().mod()).arg(ballRotVel).toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0, -200), QString("tan_vel: %1  paral_vel: %2").arg(tanVelCompensate).arg(paralVel2Target).toLatin1());
    }
//    if (isChip && !IS_SIMULATION) {
//        tanVelCompensate *= 1;
//    }

    //2. 连线方向补偿
    if (IS_SIMULATION) {
        paralVel2Target = 0;
    } else {
        if (isChip) {
            needBallVel = std::sqrt(9800.0 * needBallVel/ (2 * std::tan(DRIBBLE_CHIP_DIR)));//挑球需要把距离换算为x方向速度
            needBallVel = (-paralVel2Target + std::sqrt(paralVel2Target * paralVel2Target + 2 * originVel * 9800.0 / std::tan(DRIBBLE_CHIP_DIR))) / 2;//利用补偿前后vy = vx*tan50不变来计算t，根据路程(originVel)计算补偿后的vx，一元二次方程中，两个根一正一负，取正值
        }
        else {//some trick
            needBallVel -= paralVel2Target;
//            if (needBallVel / std::fabs(tanVelCompensate) < HIGH_REGULATION_RADIO) {
//                needBallVel *= ROLL_FRICTION_RADIO[1];
//            } else {
//                needBallVel *= ROLL_FRICTION_RADIO[0];
//            }
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
        needBallVel = 2 * std::pow(needBallVel, 2) * std::tan(DRIBBLE_CHIP_DIR) / 9800.0;
    }

//    needBallVel = needBallVel > 6000 ? 6000 : needBallVel;
//    //射门时力度特例
//    if (Utils::InTheirPenaltyArea(target, 0)) {
//        needBallVel = std::max(6500.0, needBallVel);
//    }

    if (KICK_DEBUG) {
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + CVector(5000,0).rotate(kicker.Dir()));
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + CVector(5000,0).rotate(kicker.Dir() + 3 * PARAM::Math::PI / 180), COLOR_GRAY);
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + CVector(5000,0).rotate(kicker.Dir() - 3 * PARAM::Math::PI / 180), COLOR_GRAY);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-5000, -3500) * (IS_RIGHT ? -1 : 1), QString("vy: %1  vx: %2  rotV: %3  v: %4").arg(paralVel2Target).arg(tanVel2Target).arg(kicker.RotVel()).arg(vel2targetDir).toLatin1(),COLOR_ORANGE);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-5000, -3300) * (IS_RIGHT ? -1 : 1), QString("oV: %1   V: %2").arg(originVel).arg(needBallVel).toLatin1(),COLOR_ORANGE);
        CVector dirVector = CVector(needBallVel,0).rotate(playerDir);
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + dirVector, COLOR_YELLOW);//调整后角度方向
        CVector tanVel2TargetVector = CVector(tanVelCompensate,0).rotate(originDirVector.dir()-PARAM::Math::PI/2.0);// (target - kicker.Pos()).rotate(-PARAM::Math::PI/2.0) / (target - kicker.Pos()).mod() * tanVelCompensate;
        CVector paralVel2TargetVector = CVector(paralVel2Target,0).rotate(originDirVector.dir());// (target - kicker.Pos())/ (target - kicker.Pos()).mod() * paralVel2Target;
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + tanVel2TargetVector, COLOR_PURPLE);//切向补偿
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + paralVel2TargetVector, COLOR_GREEN);//连线方向补偿
        GDebugEngine::Instance()->gui_debug_line(kicker.Pos(), kicker.Pos() + kicker.RawVel(), COLOR_RED);
    }

    return true;
}

double CKickRegulation::regulateCheck(int player, double needBallVel, double needDir, bool isChip, double tolerance) {
    auto& kicker = vision->ourPlayer(player);

    if (std::fabs(kicker.Dir() - needDir) > PARAM::Math::PI / 2) {
        return 0.0;
    }

    double vel2targetDir, needTanVel, needParalVel;
    double vel2faceDir, tanVel, paralVel, ballRotVel;
    double vel2targetDir_img, needTanVel_img, needParalVel_img, paralVel_img;
    if(isChip) needBallVel = std::sqrt(9800.0 * needBallVel/ (2 * std::tan(DRIBBLE_CHIP_DIR)));
    if(!IS_SIMULATION){
        vel2targetDir_img = Utils::Normalize(needDir - kicker.Dir());
        vel2targetDir = Utils::Normalize(needDir - kicker.ImuDir());
    }

    else {
        vel2targetDir = vel2targetDir_img = Utils::Normalize(needDir - kicker.Dir());
    }

    needTanVel = needBallVel * std::sin(vel2targetDir);
    needParalVel = needBallVel * std::cos(vel2targetDir);

    needTanVel_img = needBallVel * std::sin(vel2targetDir_img);
    needParalVel_img = needBallVel * std::cos(vel2targetDir_img);

    float vel2faceDir_img;
    float tanVel_img;
    if(!IS_SIMULATION){
        vel2faceDir = Utils::Normalize(kicker.RawVel().dir() - kicker.ImuDir());
        vel2faceDir_img = Utils::Normalize(kicker.RawVel().dir() - kicker.Dir());
    }
    else vel2faceDir = vel2faceDir_img = Utils::Normalize(kicker.RawVel().dir() - kicker.Dir());
    paralVel = kicker.RawVel().mod() * std::cos(vel2faceDir);
    paralVel_img = kicker.RawVel().mod() * std::cos(vel2faceDir_img);
    if(!IS_SIMULATION) ballRotVel = kicker.ImuRotateVel() * PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER;
    else ballRotVel = kicker.RotVel() * PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER;
//    qDebug()<<"vw:%lf"<<kicker.ImuRotateVel();
    if(!IS_SIMULATION) ballRotVel *= std::abs(kicker.ImuRotateVel()) / 1.3;
    tanVel = kicker.RawVel().mod() * std::sin(vel2faceDir) + ballRotVel;
    tanVel_img = kicker.RawVel().mod() * std::sin(vel2faceDir_img) + ballRotVel;
    if(tanVel * tanVel > needBallVel * needBallVel) return 0.0;

    double error_imu = std::abs(Utils::Normalize(std::asin(needTanVel / needBallVel) - std::asin(tanVel / needBallVel)));
    double error_img = std::abs(Utils::Normalize(std::asin(needTanVel_img / needBallVel) - std::asin(tanVel_img / needBallVel)));
    if(DEBUG_PRINT){
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,200), QString("needDir: %1  kickerDir: %2 velDir: %3").arg(needDir).arg(kicker.Dir()).arg(kicker.RawVel().dir()).toLatin1(),COLOR_RED);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,0), QString("needvp: %1  needvt: %2 2targetdir: %3").arg(needParalVel).arg(needTanVel).arg(vel2targetDir).toLatin1(),COLOR_RED);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,-200), QString("realvp: %1   realvt: %2 2veldir:%3").arg(paralVel).arg(tanVel).arg(vel2faceDir).toLatin1(),COLOR_RED);
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,-400), QString("ballrot: %1 error_imu: %2 error_img: %3").arg(ballRotVel).arg(error_imu).arg(error_img).toLatin1(),COLOR_RED);
    }
    double tolerance_img = 2 * tolerance;
    if(error_imu > tolerance)// || error_img > tolerance_img)
        return 0.0;
    if(isChip){
        if(IS_SIMULATION) return (2 * std::pow(needBallVel, 2) * std::tan(DRIBBLE_CHIP_DIR) / 9800.0);
        needBallVel = (-paralVel + std::sqrt(paralVel * paralVel + 2 * needParalVel * 9800.0 / std::tan(DRIBBLE_CHIP_DIR))) / 2;
        return (2 * std::pow(needBallVel, 2) * std::tan(DRIBBLE_CHIP_DIR) / 9800.0);
    }
    if(IS_SIMULATION) return std::sqrt(needBallVel * needBallVel - tanVel * tanVel);
    if(needBallVel < 1100) return std::sqrt(needBallVel * needBallVel - tanVel * tanVel);
    return std::sqrt(needBallVel * needBallVel - tanVel * tanVel) - paralVel;//needParalVel - paralVel;
}
