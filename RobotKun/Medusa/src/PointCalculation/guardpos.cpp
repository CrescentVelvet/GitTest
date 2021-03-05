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
    for(int i=0;i<PARAM::Field::MAX_PLAYER;i++){
        _priority[i] = -1;
        _backPos[i] = CGeoPoint(-9999,-9999);
        _defendNum[i] = -1;
        _backNum[i] = -1;
    }
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

CGeoPoint CGuardPos::backPos(int guardNum, int index, int player, int defendNum)
{
    // 后卫数量限制
    guardNum = std::min(PARAM::Field::MAX_PLAYER, std::max(1, guardNum));
    _guardNum = guardNum;
    index = std::min(guardNum, std::max(1, index));
//    _backNum[index-1] = player;
//    _priority[player] = index;
//    if(index == 1){
//        for(int i = 0; i < _guardNum; i++){
//            if (_backNum[i] < 0) continue;
//            if(!_changeDefend[_backNum[i]]) _defendNum[i] = -1;
//            _changeDefend[_backNum[i]] = -1;
//        }
//    }
//    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1000,1000),QString("%1").arg(defendNum).toLatin1());
    if(defendNum < 0 || !validdefendNum(index-1 ,defendNum))generatePos(guardNum);
    else{
        _backPos[index-1] = generateDefendPos(index-1, defendNum);
//        if(checkGuardPos(_backPos[index-1], player, index-1, defendNum))
//        _defendNum[index -1] = defendNum;
    }
    return _backPos[index-1];
}

CGeoPoint CGuardPos::backPos(int guardNum, int index) {
    return _backPos[index - 1];
}

bool CGuardPos::checkGuardPos(CGeoPoint &guardPos, int realNum, int index, int defendNum) {
    for(int i = 0; i<_guardNum; i++){
        if(_backNum[i] < 0 || i!=index) continue;
        CGeoLine meToTarget(vision->ourPlayer(realNum).Pos(), _backPos[index]);
        if((guardPos.x() - vision->ourPlayer(_backNum[i]).Pos().x())*(vision->ourPlayer(realNum).Pos().x() - vision->ourPlayer(_backNum[i]).Pos().x()) < 0){
            _changeDefend[_backNum[i]] = 1;
            _defendNum[_backNum[i]] = defendNum;
        }
        else if((guardPos.y() - vision->ourPlayer(_backNum[i]).Pos().y())*(vision->ourPlayer(realNum).Pos().y() - vision->ourPlayer(_backNum[i]).Pos().y()) < 0){
            _changeDefend[_backNum[i]] = 1;
        }
    }
}

bool CGuardPos::validdefendNum(int index, int defendNum){
    for(int i=0;i<_guardNum;i++){
        if(i != index && defendNum == _defendNum[i]) return false;
    }
    return true;
}

CGeoPoint CGuardPos::generateDefendPos(int index, int defendNum) {
    CVector defendLine1 = CGeoPoint(-PARAM::Field::PITCH_LENGTH/2, -PARAM::Field::GOAL_WIDTH/2) - vision->theirPlayer(defendNum).Pos();
    CVector defendLine2 = CGeoPoint(-PARAM::Field::PITCH_LENGTH/2, PARAM::Field::GOAL_WIDTH/2) - vision->theirPlayer(defendNum).Pos();
    CVector final = defendLine1.unit() + defendLine2.unit();
    CGeoLine defendLine(vision->theirPlayer(defendNum).Pos(), vision->theirPlayer(defendNum).Pos() + final);
    const PlayerVisionT& defendEnemy = vision->theirPlayer(defendNum);
    CGeoPoint guardPos;
    double y = defendEnemy.Pos().y()>0 ?  PARAM::Field::PENALTY_AREA_WIDTH / 2 + MIN_DIST_TO_PENALTY: -PARAM::Field::PENALTY_AREA_WIDTH / 2 - MIN_DIST_TO_PENALTY;

    CGeoLine penaltyhead(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2 + PARAM::Field::PENALTY_AREA_DEPTH, y), CGeoPoint(-PARAM::Field::PITCH_LENGTH/2 + PARAM::Field::PENALTY_AREA_DEPTH, -y));
    CGeoLine penaltyside(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2 + PARAM::Field::PENALTY_AREA_DEPTH, y), CGeoPoint(-PARAM::Field::PITCH_LENGTH/2, y));
    CGeoPoint touchPoint1 = CGeoLineLineIntersection(defendLine, penaltyhead).IntersectPoint() + CVector(MIN_DIST_TO_PENALTY, 0);
    CGeoPoint touchPoint2 = CGeoLineLineIntersection(defendLine, penaltyside).IntersectPoint();
    guardPos = touchPoint1;
    if(touchPoint1.y()>PARAM::Field::PENALTY_AREA_WIDTH / 2 || touchPoint1.y()< -PARAM::Field::PENALTY_AREA_WIDTH / 2){
        guardPos = touchPoint2;
    }
    bool turnleft = false;
    bool turnright = false;
    for(int i=0; i<_guardNum; i++) {
//        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1000,1000),QString("%1").arg(_guardNum).toLatin1());
        if(i == index) break;
        if(guardPos.dist(_backPos[i])<MIN_DIST_TO_TEAMMATE){
//            GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1000,1000),QString("%1 near %2").arg(index).arg(i).toLatin1());
            if((guardPos.y()<=_backPos[i].y() || turnleft) && !turnright){
                leftNextPos(_backPos[i],guardPos);
                GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1000,1000),QString("%1 left %2").arg(index).arg(i).toLatin1());
                turnleft = true;
            }
            else{
                rightNextPos(_backPos[i],guardPos);
                turnright = true;
                GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1000,800),QString("%1 right %2").arg(index).arg(i).toLatin1());
            }
        }
    }
    GDebugEngine::Instance()->gui_debug_x(guardPos);
    return guardPos;
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
