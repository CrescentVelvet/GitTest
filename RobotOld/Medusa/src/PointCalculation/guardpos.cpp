#include "guardpos.h"
#include "GDebugEngine.h"
#include "Global.h"
#include "staticparams.h"
#include "SkillUtils.h"

namespace {
const CGeoPoint GOAL_MIDDLE = CGeoPoint(-PARAM::Field::PITCH_LENGTH/2, 0);
const CGeoPoint GOAL_LEFT = CGeoPoint(-PARAM::Field::PITCH_LENGTH/2, -PARAM::Field::PITCH_WIDTH/2);
const CGeoPoint GOAL_RIGHT = CGeoPoint(-PARAM::Field::PITCH_LENGTH/2, PARAM::Field::PITCH_WIDTH/2);
const double MIN_DIST_TO_PENALTY = PARAM::Vehicle::V2::PLAYER_SIZE*2;
const double MIN_DIST_TO_TEAMMATE = PARAM::Vehicle::V2::PLAYER_SIZE*2 + PARAM::Field::BALL_SIZE*2;
// 后卫的运动范围
CGeoPoint PENALTY_RIGHT_UP(-PARAM::Field::PITCH_LENGTH/2 + PARAM::Field::PENALTY_AREA_DEPTH + MIN_DIST_TO_PENALTY, -(PARAM::Field::PENALTY_AREA_WIDTH/2 + MIN_DIST_TO_PENALTY));
CGeoPoint PENALTY_RIGHT_DOWN(-PARAM::Field::PITCH_LENGTH/2 + PARAM::Field::PENALTY_AREA_DEPTH + MIN_DIST_TO_PENALTY, PARAM::Field::PENALTY_AREA_WIDTH/2 + MIN_DIST_TO_PENALTY);
CGeoPoint PENALTY_LEFT_UP(-PARAM::Field::PITCH_LENGTH/2, -(PARAM::Field::PENALTY_AREA_WIDTH/2 + MIN_DIST_TO_PENALTY));
CGeoPoint PENALTY_LEFT_DOWN(-PARAM::Field::PITCH_LENGTH/2, PARAM::Field::PENALTY_AREA_WIDTH/2 + MIN_DIST_TO_PENALTY);
CGeoRectangle guardMoveRec(PENALTY_LEFT_UP, PENALTY_RIGHT_DOWN);
// 防守目标关键点
CGeoPoint LEFTBACK_CRITICAL_POINT = CGeoPoint(-(PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - PARAM::Vehicle::V2::PLAYER_SIZE), -PARAM::Field::PITCH_WIDTH/2);
CGeoPoint RIGHTBACK_CRITICAL_POINT = CGeoPoint(-(PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH - PARAM::Vehicle::V2::PLAYER_SIZE), PARAM::Field::PITCH_WIDTH/2);
bool DEBUG_GUARD_POS = false;
const double VALID_THRESHOLD = 500;
}

CGuardPos::CGuardPos()
{

}

void CGuardPos::generatePos(int guardNum)
{
    // 后卫数量限制
    guardNum = std::min(PARAM::Field::MAX_PLAYER, std::max(1, guardNum));
    const MobileVisionT& ball = vision->ball();
    // 防守的目标
    int bestenemy = ZSkillUtils::instance()->getTheirBestPlayer();
    CGeoPoint defendTarget = ball.Valid() ? ball.RawPos() : vision->theirPlayer(bestenemy).Pos();
    if ((defendTarget - GOAL_MIDDLE).dir() > (RIGHTBACK_CRITICAL_POINT - GOAL_MIDDLE).dir())
        defendTarget = RIGHTBACK_CRITICAL_POINT;
    else if ((defendTarget - GOAL_MIDDLE).dir() < (LEFTBACK_CRITICAL_POINT - GOAL_MIDDLE).dir())
        defendTarget = LEFTBACK_CRITICAL_POINT;
    GDebugEngine::Instance()->gui_debug_msg(defendTarget, QString("target").toLatin1(), COLOR_CYAN);

    CGeoLine targetToMiddle(defendTarget, GOAL_MIDDLE);
    CGeoLine targetToLeft(defendTarget, GOAL_LEFT);
    CGeoLine targetToRight(defendTarget, GOAL_RIGHT);

    // 计算交点
    CGeoLineRectangleIntersection intersecMiddle(targetToMiddle, guardMoveRec);
    if (intersecMiddle.intersectant()){
        // 中后卫
        bool leftValid=true, rightValid=true;
        if (guardNum%2 == 1){
            _backPos[guardNum/2] = intersecMiddle.point2().dist(GOAL_MIDDLE) < 1e-8 ? intersecMiddle.point1() : intersecMiddle.point2();
            for (int i = 0; i < guardNum/2; i++) {
                leftValid = leftNextPos(_backPos[guardNum/2 - i], _backPos[guardNum/2 - i -1]);
                rightValid = rightNextPos(_backPos[guardNum/2 + i], _backPos[guardNum/2 + i +1]);
            }
        }
        else {
            CGeoPoint intersecPos = intersecMiddle.point2().dist(GOAL_MIDDLE) < 1e-8 ? intersecMiddle.point1() : intersecMiddle.point2();
            leftValid = leftNextPos(intersecPos, _backPos[guardNum/2-1], MIN_DIST_TO_TEAMMATE/2);
            rightValid = rightNextPos(intersecPos, _backPos[guardNum/2], MIN_DIST_TO_TEAMMATE/2);
            for (int i = 0; i < (guardNum-2)/2; i++) {
                leftValid = leftNextPos(_backPos[guardNum/2-1-i], _backPos[guardNum/2-2-i]);
                rightValid = rightNextPos(_backPos[guardNum/2+i], _backPos[guardNum/2+1+i]);
            }
        }
        if (!leftValid){
            for (int i = 1; i < guardNum; i++) {
                rightNextPos(_backPos[i-1], _backPos[i]);
            }
        }
        if (!rightValid){
            for (int i = 1; i < guardNum; i++) {
                leftNextPos(_backPos[guardNum-i], _backPos[guardNum-i-1]);
            }
        }
    }
    else {
        qDebug() << "GUARDPOS: NO INTERSECTION!!!";
    }
    if(DEBUG_GUARD_POS){
        for (int i=0; i < guardNum; i++) {
            GDebugEngine::Instance()->gui_debug_msg(_backPos[i], QString::number(i, 16).toLatin1(), COLOR_CYAN);
        }
    }
}

CGeoPoint CGuardPos::backPos(int guardNum, int index)
{
    // 后卫数量限制
    guardNum = std::min(PARAM::Field::MAX_PLAYER, std::max(1, guardNum));
    index = std::min(guardNum, std::max(1, index));
    generatePos(guardNum);
    return _backPos[index-1];
}

void CGuardPos::setBackNum(int realNum, int index)
{
    if (index < 1 || index > PARAM::Field::MAX_PLAYER){
        qDebug() << "ERROR in GuardPos: back num out of range";
        return;
    }
    _backNum[index-1] = realNum;
//    qDebug() << "backNum" << realNum << index;
}

int CGuardPos::checkValidNum(int guardNum)
{
    int validCnt = 0;
    int missingCnt = 0;
    for (int i = 0; i < guardNum; ++i) {
//        GDebugEngine::Instance()->gui_debug_msg(vision->OurPlayer(i).Pos(), QString::number(i, 16).toLatin1(), COLOR_RED);
        if (validBackPos(_backPos[i], _backNum[i]))
            validCnt++;
        else {
            _missingBack[missingCnt] = i + 1;
            missingCnt++;
        }
    }
    return validCnt;
}

bool CGuardPos::validBackPos(CGeoPoint backPos, int realNum)
{
    return vision->ourPlayer(realNum).Pos().dist(backPos) <= VALID_THRESHOLD;
}

int CGuardPos::missingBackIndex(int i)
{
    if (i < 1 || i > PARAM::Field::MAX_PLAYER){
        qDebug() << "ERROR in GuardPos: missing back out of range";
        i = std::min(PARAM::Field::MAX_PLAYER, std::max(1, i));
    }
    return _missingBack[i-1];
}

bool CGuardPos::leftNextPos(CGeoPoint basePos, CGeoPoint& nextPos, double dist)
{
    if (dist < 0) dist = MIN_DIST_TO_TEAMMATE;
    if (basePos.y() >= PENALTY_RIGHT_UP.y() + dist && fabs(basePos.x() - PENALTY_RIGHT_DOWN.x()) < 1e-8){
        nextPos = basePos + CVector(0, -dist);
    }
    else if (basePos.y() < PENALTY_RIGHT_UP.y() + dist) {
        if (fabs(basePos.x() - PENALTY_RIGHT_UP.x()) < 1e-8) {
            nextPos = PENALTY_RIGHT_UP + CVector(-sqrt(pow(dist, 2) - pow(basePos.y() - PENALTY_RIGHT_UP.y(), 2)), 0);
        }
        else if (fabs(basePos.x() - PENALTY_LEFT_UP.x()) >= dist) {
            nextPos = basePos + CVector(-dist, 0);
        }
        else {
            nextPos = PENALTY_LEFT_UP;
            return false;
        }
    }
    else {
        if (fabs(basePos.x() - PENALTY_RIGHT_DOWN.x()) <= dist) {
            nextPos = PENALTY_RIGHT_DOWN + CVector(0, -sqrt(pow(dist, 2) - pow(basePos.x() - PENALTY_RIGHT_DOWN.x(), 2)));
        }
        else {
            nextPos = basePos + CVector(dist, 0);
        }
    }
    return true;
}

bool CGuardPos::rightNextPos(CGeoPoint basePos, CGeoPoint &nextPos, double dist)
{
    if (dist < 0) dist = MIN_DIST_TO_TEAMMATE;
    if (basePos.y() <= PENALTY_RIGHT_DOWN.y() - dist && fabs(basePos.x() - PENALTY_RIGHT_UP.x()) < 1e-8){
        nextPos = basePos + CVector(0, dist);
    }
    else if (basePos.y() > PENALTY_RIGHT_DOWN.y() - dist) {
        if (fabs(basePos.x() - PENALTY_RIGHT_DOWN.x()) < 1e-8) {
            nextPos = PENALTY_RIGHT_DOWN + CVector(-sqrt(pow(dist, 2) - pow(basePos.y() - PENALTY_RIGHT_DOWN.y(), 2)), 0);
        }
        else if (fabs(basePos.x() - PENALTY_LEFT_DOWN.x()) >= dist) {
            nextPos = basePos + CVector(-dist, 0);
        }
        else {
            nextPos = PENALTY_LEFT_DOWN;
            return false;
        }
    }
    else {
        if (fabs(basePos.x() - PENALTY_RIGHT_UP.x()) <= dist) {
            nextPos = PENALTY_RIGHT_UP + CVector(0, sqrt(pow(dist, 2) - pow(basePos.x() - PENALTY_RIGHT_UP.x(), 2)));
        }
        else {
            nextPos = basePos + CVector(dist, 0);
        }
    }
    return true;
}
