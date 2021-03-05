#include "passposevaluate.h"
#include "staticparams.h"
#include "Global.h"
#include "ShootRangeList.h"
#include "RefereeBoxIf.h"
#include "parammanager.h"
namespace  {
    bool FREE_KICK_DEBUG = false;

    int FREEKICK_RANGE = 1200;

    // 传球点的参数
    namespace PASS_PARAM {
    const float wDist = 0.15f;             //0.15f;
    const float wPassLineDist = 0;
    const float wTurnAngle = 0.1; //0.1;
    const float wClosestEnemyDist = 0.10f;
    const float wUnderPass = 0;  //0
    const float minPassDist = 1500;
    const float minchipPassDist = 3000;
    const int minSelfPassDist = static_cast<int>(2*PARAM::Vehicle::V2::PLAYER_SIZE);
    }
    // 射门点的参数
    namespace SHOOT_PARAM {
    const float wShootAngle = 0.15f;
    const float wDist = 0.2f;
    const float wRefracAngle = 0.15f;
    const float wAngle2Goal = 0.1f;
    const float wPassLineDist = 0;
    const float wGuardTime = 0.073f;
    }
    // 任意球的参数
    namespace FREE_KICK_PARAM {
    const float wShootAngle = 0.3f;
    const float wClosestEnemyDist = 0.1f;
    const double wDist = 0.2;
    const float wRefracAngle = 0.0f;
    double wAngle2Goal = 0.0;
    const float wGuardTime = 0.1f;
    const float wSector = 0.0f;
    const float wPassLineDist = 0.8f;
    double min2PenaltyDist = 500;
    }


}
CPassPosEvaluate::CPassPosEvaluate(){
    ZSS::ZParamManager::instance()->loadParam(FREEKICK_RANGE, "FreeKick/range", 1000);
    ZSS::ZParamManager::instance()->loadParam(FREE_KICK_DEBUG, "FreeKick/DEBUG", false);
    ZSS::ZParamManager::instance()->loadParam(FREE_KICK_PARAM::wAngle2Goal, "FreeKick/Angle2Goal", 0.0);
    ZSS::ZParamManager::instance()->loadParam(FREE_KICK_PARAM::min2PenaltyDist, "FreeKick/ToPenaltyDist", 500);
}
std::vector<float> CPassPosEvaluate::evaluateFunc(CGeoPoint candidate,CGeoPoint leaderPos,EvaluateMode mode){
    std::vector<float> scores;
    CGeoPoint p0(candidate), p1(PARAM::Field::PITCH_LENGTH / 2, -PARAM::Field::GOAL_WIDTH / 2), p2(PARAM::Field::PITCH_LENGTH / 2, PARAM::Field::GOAL_WIDTH / 2);

    // 計算有效射門角度
    float shootAngle = 0;
    CShootRangeList shootRangeList(vision, 1, p0);
    const CValueRangeList& shootRange = shootRangeList.getShootRange();
    if (shootRange.size() > 0) {
        auto bestRange = shootRange.getMaxRangeWidth();
        if(bestRange) {
            shootAngle = bestRange->getWidth();
        }
    }
    // 計算折射角,折射角越小越适合touch，touch角度要求小于60度
    CVector v1 = leaderPos - p0;
    CVector v2 = p1.midPoint(p2) - p0;
    float refracAngle = fabs(v1.dir() - v2.dir());
    refracAngle = refracAngle > PARAM::Math::PI ? 2*PARAM::Math::PI - refracAngle : refracAngle;
    // 計算離球門的距離
    float defDist = fabs(p0.dist(CGeoPoint(PARAM::Field::PITCH_LENGTH / 2, 0)));
    float Angle2Goal = fabs(fabs(Utils::Normalize(v2.dir())) - PARAM::Math::PI / 4);
    float Angle2Goal4FreeKick = (fabs(Utils::Normalize(v2.dir())) < PARAM::Math::PI / 4.0f && fabs(Utils::Normalize(v2.dir())) > PARAM::Math::PI / 9.0f) ? 1 : 0;
    // 計算敵方到傳球線的距離
    float passLineDist = 9999;
    CGeoSegment BallLine(vision->ball().Pos(), p0);
    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        if(!vision->theirPlayer(i ).Valid())
            continue;
        CGeoPoint targetPos = vision->theirPlayer(i ).Pos();
        float dist = std::min(BallLine.dist2Point(targetPos), 1000.0);
        if(dist < passLineDist)
            passLineDist = dist;
    }
    // 计算接球转身传球需要的角度
    CGeoPoint ballPos = vision->ball().Pos();
    CVector leaderToBall = ballPos - leaderPos;
    CVector leaderToPassPos = p0 - leaderPos;
    float turnAngle = fabs(leaderToBall.dir() - leaderToPassPos.dir());
    turnAngle = turnAngle > PARAM::Math::PI ? 2*PARAM::Math::PI - turnAngle : turnAngle;
    if(ballPos.x() < leaderPos.x()) turnAngle = PARAM::Math::PI;
    // 安全性 计算最近的敌方车的距离
    float closestEnemyDist = 9999;
    for (int i=0; i < PARAM::Field::MAX_PLAYER; i++) {
        if(!vision->theirPlayer(i).Valid())
            continue;
        if(Utils::InTheirPenaltyArea(vision->theirPlayer(i).Pos(), 0))
            continue;
        if(vision->theirPlayer(i).Pos().dist(p0) < closestEnemyDist)
            closestEnemyDist = vision->theirPlayer(i).Pos().dist(p0);
    }
    // 下底传中
    float underPass = (p0.x() < PARAM::Field::PITCH_LENGTH/2 && p0.x() > PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH) ? 1 : 0;
    //预测后卫
    float guardMinTime = 9999;
//    auto& ballp = vision->Ball().Pos();
    CGeoSegment shootLine1(p0, p1), shootLine2(p0, p2);
    CGeoPoint p = WorldModel::Instance()->penaltyIntersection(shootLine1),
              q = WorldModel::Instance()->penaltyIntersection(shootLine2);
    WorldModel::Instance()->normalizeCoordinate(p);
    WorldModel::Instance()->normalizeCoordinate(q);
//    GDebugEngine::Instance()->gui_debug_msg(p, "IP",COLOR_RED);
//    GDebugEngine::Instance()->gui_debug_msg(q,"IQ", COLOR_RED);
    for(int i = 1; i < PARAM::Field::MAX_PLAYER; i++) {
        const PlayerVisionT& enemy = vision->theirPlayer(i);
        if (!Utils::InTheirPenaltyArea(enemy.Pos(), 0) && Utils::InTheirPenaltyArea(enemy.Pos(), 50)) {
            float pTime = WorldModel::Instance()->preditTheirGuard(enemy, p);
            float qTime = WorldModel::Instance()->preditTheirGuard(enemy, q);
            if (pTime < guardMinTime) {
                guardMinTime = pTime;
            }
            if (qTime < guardMinTime) {
                guardMinTime = qTime;
            }
        }
    }

    float sectorScore;
    float candiDir = RUNPOS_PARAM::maxSectorDir;
    float candiDist = RUNPOS_PARAM::maxSectorDist;
//    float candiDir = (candidate - ball.Pos()).dir();
//    float candiDist = candidate.dist(ball.Pos());
    for (int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        if(vision->theirPlayer(i).Valid()) {
            const PlayerVisionT& enemy = vision->theirPlayer(i);
            float baseDir = (enemy.Pos() - vision->ball().Pos()).dir();
            float baseDist = enemy.Pos().dist(vision->ball().Pos());
//            if (fabs(Utils::Normalize(candiDir - baseDir)) < RUNPOS_PARAM::maxSectorDir && candiDist > baseDist) {
//                candiDir = fabs(Utils::Normalize(candiDir - baseDir));
//                candiDist = (candiDist - baseDist) > RUNPOS_PARAM::maxSectorDist ? RUNPOS_PARAM::maxSectorDist : candiDist - baseDist;
//            }
            if (fabs(Utils::Normalize((candidate - vision->ball().Pos()).dir() - baseDir)) < RUNPOS_PARAM::maxSectorDir && candidate.dist(vision->ball().Pos()) > baseDist) {
                candiDir = std::min(fabs(Utils::Normalize((candidate - vision->ball().Pos()).dir() - baseDir)), double(candiDir));
                candiDist = std::min(candidate.dist(vision->ball().Pos()), double(candiDist));
            }
        }
    }

    // 归一化
    shootAngle = shootAngle > 25 ? 1 : shootAngle/25;
    refracAngle = refracAngle < PARAM::Math::PI/6 ? PARAM::Math::PI/6 : refracAngle;
    refracAngle = defDist < PARAM::Field::PITCH_LENGTH/2 ? 1 - refracAngle/PARAM::Math::PI : 0;  // 折射角在一定范围内给固定奖励
    defDist = 1 - defDist/PARAM::Field::PITCH_LENGTH;
    Angle2Goal = 1 - Angle2Goal/PARAM::Math::PI;
    passLineDist = passLineDist > 300 ? 1 : passLineDist / 300;
    turnAngle = turnAngle < PARAM::Math::PI/12 ? PARAM::Math::PI/12 : turnAngle;
    turnAngle = 1 - turnAngle/PARAM::Math::PI;
    closestEnemyDist = closestEnemyDist > 150 ? 1 : (closestEnemyDist < 50 ? 0 : (closestEnemyDist - 50)/100);
    guardMinTime = guardMinTime > 999 ? 0 : guardMinTime > 2.0 ? 2.0 : guardMinTime / 2.0;
    sectorScore = candiDir / RUNPOS_PARAM::maxSectorDir * 0.3f + candiDist / RUNPOS_PARAM::maxSectorDist * 0.7f;
    float totalWeight = 0;
    switch (mode) {
    case PASS:
        totalWeight = PASS_PARAM::wDist + PASS_PARAM::wPassLineDist
                + PASS_PARAM::wTurnAngle + PASS_PARAM::wClosestEnemyDist + PASS_PARAM::wUnderPass * underPass;
        scores.push_back(1/totalWeight * 100 * (PASS_PARAM::wDist * defDist
                         + PASS_PARAM::wPassLineDist * passLineDist
                         + PASS_PARAM::wTurnAngle * turnAngle
                         + PASS_PARAM::wClosestEnemyDist * closestEnemyDist
                         + PASS_PARAM::wUnderPass * underPass));
        scores.push_back(1/totalWeight * 100 * PASS_PARAM::wDist * defDist);
        scores.push_back(1/totalWeight * 100 * PASS_PARAM::wPassLineDist * passLineDist);
        scores.push_back(1/totalWeight * 100 * PASS_PARAM::wTurnAngle * turnAngle);
        scores.push_back(1/totalWeight * 100 * PASS_PARAM::wClosestEnemyDist * closestEnemyDist);
        scores.push_back(1/totalWeight * 100 * PASS_PARAM::wUnderPass * underPass);
        break;
    case SHOOT:
        totalWeight = SHOOT_PARAM::wShootAngle + SHOOT_PARAM::wRefracAngle
                + SHOOT_PARAM::wAngle2Goal + SHOOT_PARAM::wDist + SHOOT_PARAM::wPassLineDist + SHOOT_PARAM::wGuardTime;
        scores.push_back(1/totalWeight * 100 * (SHOOT_PARAM::wShootAngle * shootAngle
                         + SHOOT_PARAM::wRefracAngle * refracAngle
                         + SHOOT_PARAM::wDist * defDist
                         + SHOOT_PARAM::wAngle2Goal * Angle2Goal
                         + SHOOT_PARAM::wPassLineDist * passLineDist
                         + SHOOT_PARAM::wGuardTime * guardMinTime));
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wShootAngle * shootAngle);
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wRefracAngle * refracAngle);
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wDist * defDist);
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wAngle2Goal * Angle2Goal);
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wPassLineDist * passLineDist);
        scores.push_back(1/totalWeight * 100 * SHOOT_PARAM::wGuardTime * guardMinTime);
        break;
    case FREE_KICK:
        totalWeight = FREE_KICK_PARAM::wShootAngle + FREE_KICK_PARAM::wClosestEnemyDist + FREE_KICK_PARAM::wRefracAngle
                + FREE_KICK_PARAM::wAngle2Goal + FREE_KICK_PARAM::wDist + FREE_KICK_PARAM::wGuardTime + FREE_KICK_PARAM::wSector + FREE_KICK_PARAM::wPassLineDist;
        scores.push_back(1/totalWeight * 100 * (FREE_KICK_PARAM::wShootAngle * shootAngle
                         + FREE_KICK_PARAM::wClosestEnemyDist * closestEnemyDist
                         + FREE_KICK_PARAM::wRefracAngle * refracAngle
                         + FREE_KICK_PARAM::wDist * defDist
                         + FREE_KICK_PARAM::wAngle2Goal * Angle2Goal4FreeKick
                         + FREE_KICK_PARAM::wGuardTime * guardMinTime
                         + FREE_KICK_PARAM::wSector * sectorScore
                         + FREE_KICK_PARAM::wPassLineDist * passLineDist));
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wShootAngle * shootAngle);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wClosestEnemyDist * closestEnemyDist);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wRefracAngle * refracAngle);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wDist * defDist);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wAngle2Goal * Angle2Goal4FreeKick);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wGuardTime * guardMinTime);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wSector * sectorScore);
        scores.push_back(1/totalWeight * 100 * FREE_KICK_PARAM::wPassLineDist * passLineDist);
        break;
    }
    return scores;
}

std::vector<CGeoPoint> CPassPosEvaluate::generateFreeKickPoints(){
    // 跟球门的距离
    static const int MIN_DIST_TO_GOAL = 1100;
    static const int MAX_DIST_TO_GOAL = MIN_DIST_TO_GOAL + FREEKICK_RANGE;
    // 跟球门的夹角
    static const int MAX_ANGLE_TO_GOAL = 60;
    // 距离、角度的最小单位
    static const int DIST_UNIT = 100;
    static const int ANGLE_UNIT = 3;
    // 挑球距离
    static const int MAX_CHIP_DIST = 6000;
    // 根据不同开球位置设置不同的起始点
    CGeoPoint GOAL;
    double freekickPointBuf = 0;
    CGeoPoint kickStartPos;
    if (VisionModule::Instance()->getCurrentRefereeMsg() == "OurBallPlacement") {
        kickStartPos.fill(RefereeBoxInterface::Instance()->getBallPlacementPosX(), RefereeBoxInterface::Instance()->getBallPlacementPosY());
    } else {
        kickStartPos = vision->ball().Pos();
    }
    GOAL.fill((PARAM::Field::PITCH_LENGTH / 2 - kickStartPos.x()) < MAX_CHIP_DIST ? PARAM::Field::PITCH_LENGTH / 2 : kickStartPos.x() + MAX_CHIP_DIST, 0);
    freekickPointBuf = (PARAM::Field::PITCH_WIDTH / 2 - std::fabs(kickStartPos.y())) / 3;
    // 生成候选点
    std::vector<CGeoPoint> candidates;
    for (int dist=MIN_DIST_TO_GOAL; dist<=MAX_DIST_TO_GOAL; dist+=DIST_UNIT) {
        for (int angle=-MAX_ANGLE_TO_GOAL; angle<=MAX_ANGLE_TO_GOAL; angle+=ANGLE_UNIT) {
            CGeoPoint candidate = GOAL + Utils::Polar2Vector(dist, (angle + 180)*PARAM::Math::PI/180);
            candidate = candidate + Utils::Polar2Vector(freekickPointBuf, PARAM::Math::PI / 2 * ((candidate.y() > 0) ? 1 : -1));
            if (Utils::InTheirPenaltyArea(candidate, FREE_KICK_PARAM::min2PenaltyDist))
                continue;
            if (candidate.dist(vision->ball().Pos()) < PARAM::Field::PITCH_LENGTH / 4) //防止离球过近
                continue;
            if (std::fabs(kickStartPos.y()) > PARAM::Field::PENALTY_AREA_WIDTH / 2 && candidate.y() * kickStartPos.y() > 0)
                continue;
            if (std::fabs(kickStartPos.y()) < PARAM::Field::PENALTY_AREA_WIDTH / 2 && std::fabs(candidate.y()) < PARAM::Field::PENALTY_AREA_WIDTH / 2)
                continue;
            candidates.push_back(candidate);
        }
    }
    if(FREE_KICK_DEBUG) GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,0), QString("BUFDIST: %1").arg(freekickPointBuf).toLatin1(), COLOR_ORANGE);
    return candidates;
}

bool CPassPosEvaluate::passBack(CGeoPoint candidate,CGeoPoint leaderPos){
    return ((leaderPos.x() < PARAM::Field::PITCH_LENGTH/6 && candidate.x() < leaderPos.x())
            || (leaderPos.x() >= PARAM::Field::PITCH_LENGTH/6 && candidate.x() < PARAM::Field::PITCH_LENGTH/6));
}

bool CPassPosEvaluate::pass2lowThreat(CGeoPoint candidate,CGeoPoint leaderPos){
    return (leaderPos.x() > PARAM::Field::PITCH_LENGTH/4 && fabs(candidate.y()) > PARAM::Field::PITCH_WIDTH/3);
}

bool CPassPosEvaluate::passTooClose(CGeoPoint candidate, CGeoPoint leaderPos,bool selfPass){
    return((leaderPos.dist(candidate) <PASS_PARAM::minPassDist && !selfPass) || (leaderPos.dist(candidate) < PASS_PARAM::minSelfPassDist));
}
bool CPassPosEvaluate::chipTooClose(CGeoPoint candidate, CGeoPoint leaderPos){
     return((leaderPos.dist(candidate) <PASS_PARAM::minchipPassDist));
}
bool CPassPosEvaluate::validPass(CGeoPoint candidate, CGeoPoint leaderPos,bool flat){
    //flat pass
    if(flat&&passTooClose(candidate,leaderPos,false)) return false;
    //chip pass
    if(!flat&&chipTooClose(candidate,leaderPos)) return false;
    if(!flat&&!Utils::IsInField(candidate, 200)) return false;
    //common
    if(passBack(candidate,leaderPos)) return false;
    //if(pass2lowThreat(candidate,leaderPos)) return false;
    return true;
}
bool CPassPosEvaluate::validSelfPass(CGeoPoint candidate, CGeoPoint leaderPos){
    if(candidate.dist(leaderPos)> PARAM::Field::PITCH_LENGTH/6) return false;
    if(passTooClose(candidate,leaderPos,true)) return false;
    if(!Utils::IsInField(candidate, 500)) return false;
    if(leaderPos.x() < PARAM::Field::PITCH_LENGTH/6 && candidate.x() < leaderPos.x()) return false;
    return true;
}
