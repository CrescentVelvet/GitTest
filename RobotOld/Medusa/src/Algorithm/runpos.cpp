#include "runpos.h"
#include "param.h"
#include "GDebugEngine.h"
#include "ShootRangeList.h"
#include "math.h"
#include "WorldModel.h"
#include "SkillUtils.h"
#include "parammanager.h"

namespace {
bool OTHERPOS_DEBUG = false;
const int ATTACK_AREA_NUM = 4;
const int WRONG_RUNPOS_CHANGE_BUFFER = 20;
CGeoRectangle MIDDLE_LEFT_AREA(0, - Param::Field::PITCH_WIDTH / 2, Param::Field::PITCH_LENGTH / 4, 0);
CGeoRectangle MIDDLE_RIGHT_AREA(0, 0, Param::Field::PITCH_LENGTH / 4, Param::Field::PITCH_WIDTH / 2);
CGeoRectangle FRONT_LEFT_AREA(Param::Field::PITCH_LENGTH / 4, - Param::Field::PITCH_WIDTH / 2, Param::Field::PITCH_LENGTH / 2, 0);
CGeoRectangle FRONT_RIGHT_AREA(Param::Field::PITCH_LENGTH / 4, 0, Param::Field::PITCH_LENGTH / 2, Param::Field::PITCH_WIDTH / 2);
CGeoRectangle ATTACK_AREA[4] = {MIDDLE_LEFT_AREA, MIDDLE_RIGHT_AREA, FRONT_LEFT_AREA, FRONT_RIGHT_AREA};
//bool runPosIsValid[4] = {0,0,0,0};
runPosProperties areaRawPoint[4];
int runPosCycle[2] = {0};
}

RunPos::RunPos() : _pVision(nullptr) {
    // initialize run pos
    ZSS::ZParamManager::instance()->loadParam(OTHERPOS_DEBUG, "Debug/OtherPos", false);
    for (int i = 0; i < 2; ++i) {
        _runPos[i].pos = CGeoPoint((ATTACK_AREA[i]._point[0].x()+ ATTACK_AREA[i]._point[2].x())/2, (ATTACK_AREA[i]._point[0].y()+ ATTACK_AREA[i]._point[2].y())/2);
        _runPos[i].areaNum = i;
    }
}

void RunPos::generateRunPos(const CVisionModule* pVision, const CGeoPoint& avoidPos) {
    // 更新图像信息
    _pVision = pVision;
    _avoidPos = avoidPos;
    generateBroad();

    for (int i = 0; i < ATTACK_AREA_NUM; i++) {
        if (OTHERPOS_DEBUG) {
            GDebugEngine::Instance()->gui_debug_line(ATTACK_AREA[i]._point[0], ATTACK_AREA[i]._point[1], COLOR_WHITE);
            GDebugEngine::Instance()->gui_debug_line(ATTACK_AREA[i]._point[1], ATTACK_AREA[i]._point[2], COLOR_WHITE);
            GDebugEngine::Instance()->gui_debug_line(ATTACK_AREA[i]._point[2], ATTACK_AREA[i]._point[3], COLOR_WHITE);
            GDebugEngine::Instance()->gui_debug_line(ATTACK_AREA[i]._point[3], ATTACK_AREA[i]._point[0], COLOR_WHITE);
        }

        selectAreaBestPoint(i);
        if (ZSkillUtils::instance()->IsInField(_avoidPos)) {
            if (ATTACK_AREA[i].HasPoint(_avoidPos)) {
                areaRawPoint[i].isValid = false;
            }
        }
    }
    judgeRunPosValid();
}

void RunPos::generateBroad() {
    const MobileVisionT& ball = _pVision->Ball();
    float bx = ball.X(), by = ball.Y();
    if (ball.X() > 0) {
        MIDDLE_LEFT_AREA = CGeoRectangle((bx > Param::Field::PITCH_LENGTH / 4 ? bx - Param::Field::PITCH_LENGTH / 4 : 0), (by > 0 ? - Param::Field::PITCH_WIDTH / 2 + by / 2 : - Param::Field::PITCH_WIDTH / 2), (bx > Param::Field::PITCH_LENGTH / 4 ? Param::Field::PITCH_LENGTH / 8 + bx / 2 : Param::Field::PITCH_LENGTH / 4), by / 2);
        MIDDLE_RIGHT_AREA = CGeoRectangle((bx > Param::Field::PITCH_LENGTH / 4 ? bx - Param::Field::PITCH_LENGTH / 4 : 0), by / 2, (bx > Param::Field::PITCH_LENGTH / 4 ? Param::Field::PITCH_LENGTH / 8 + bx / 2 : Param::Field::PITCH_LENGTH / 4), (by < 0 ?  Param::Field::PITCH_WIDTH / 2 + by / 2 : Param::Field::PITCH_WIDTH / 2));
//        FRONT_LEFT_AREA = CGeoRectangle((bx > Param::Field::PITCH_LENGTH / 4 ? Param::Field::PITCH_LENGTH / 8 + bx / 2 : Param::Field::PITCH_LENGTH / 4), (by > 0 ? - Param::Field::PITCH_WIDTH / 2 + by / 2 : - Param::Field::PITCH_WIDTH / 2), Param::Field::PITCH_LENGTH / 2, by / 2);
//        FRONT_RIGHT_AREA = CGeoRectangle((bx > Param::Field::PITCH_LENGTH / 4 ? Param::Field::PITCH_LENGTH / 8 + bx / 2 : Param::Field::PITCH_LENGTH / 4), by / 2, Param::Field::PITCH_LENGTH / 2, (by < 0 ? Param::Field::PITCH_WIDTH / 2 + by / 2 : Param::Field::PITCH_WIDTH / 2));
//        FRONT_LEFT_AREA = CGeoRectangle((bx > Param::Field::PITCH_LENGTH / 4 ? Param::Field::PITCH_LENGTH / 8 + bx / 2 : Param::Field::PITCH_LENGTH / 4), - Param::Field::PITCH_WIDTH / 2, Param::Field::PITCH_LENGTH / 2,  by / 2);
//        FRONT_RIGHT_AREA = CGeoRectangle((bx > Param::Field::PITCH_LENGTH / 4 ? Param::Field::PITCH_LENGTH / 8 + bx / 2 : Param::Field::PITCH_LENGTH / 4), by / 2, Param::Field::PITCH_LENGTH / 2, Param::Field::PITCH_WIDTH / 2);
    } else {
        MIDDLE_LEFT_AREA = CGeoRectangle(0, - Param::Field::PITCH_WIDTH / 2, Param::Field::PITCH_LENGTH / 4, 0);
        MIDDLE_RIGHT_AREA = CGeoRectangle(0, 0, Param::Field::PITCH_LENGTH / 4, Param::Field::PITCH_WIDTH / 2);
        FRONT_LEFT_AREA = CGeoRectangle(Param::Field::PITCH_LENGTH / 4, - Param::Field::PITCH_WIDTH / 2, Param::Field::PITCH_LENGTH / 2, 0);
        FRONT_RIGHT_AREA = CGeoRectangle(Param::Field::PITCH_LENGTH / 4, 0, Param::Field::PITCH_LENGTH / 2, Param::Field::PITCH_WIDTH / 2);
    }
    ATTACK_AREA [0] = MIDDLE_LEFT_AREA;
    ATTACK_AREA [1] = MIDDLE_RIGHT_AREA;
    ATTACK_AREA [2] = FRONT_LEFT_AREA;
    ATTACK_AREA [3] = FRONT_RIGHT_AREA;
}

void RunPos::selectAreaBestPoint(int areaNum) {
    CGeoPoint bestCandidate(9999,9999);
    float bestScore = -9999;
    for (float bx = ATTACK_AREA[areaNum]._point[0].x() + 1; bx < ATTACK_AREA[areaNum]._point[2].x(); bx += 20) {
        for (float by = ATTACK_AREA[areaNum]._point[0].y() + 1; by < ATTACK_AREA[areaNum]._point[2].y(); by += 20) {
            CGeoPoint candidate(bx, by);
            // 排除禁區
            if(Utils::InTheirPenaltyArea(candidate, 18))
                continue;
            //排除过于边界的点
            if(Utils::OutOfField(candidate, 28))
                continue;

            std::vector<float> posScores = evaluateFunc(_pVision,candidate);
            float score = posScores.front();
            if(score > bestScore) {
                bestScore = score;
                bestCandidate = candidate;
            }
        }
    }
    areaRawPoint[areaNum].pos     = bestCandidate;
    areaRawPoint[areaNum].areaNum = areaNum;
    areaRawPoint[areaNum].score   = bestScore;
    areaRawPoint[areaNum].isValid = true;
}

std::vector<float> RunPos::evaluateFunc(const CVisionModule* pVision, const CGeoPoint& candidate) {
    // 1.与对方球门的距离
    CGeoPoint goal = CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0);
    float distToGoal = (candidate - goal).mod() > RUNPOS_PARAM::maxDistToGoal ? RUNPOS_PARAM::maxDistToGoal : (candidate - goal).mod();

    // 2.射门有效角度
    float shootAngle = 0;
    CShootRangeList shootRangeList(pVision, 0, candidate);
    const CValueRangeList& shootRange = shootRangeList.getShootRange();
    if (shootRange.size() > 0) {
        auto bestRange = shootRange.getMaxRangeWidth();
        if (bestRange && bestRange->getWidth() > Param::Field::BALL_SIZE + 5) {	// 要求射门空档足够大
            shootAngle = bestRange->getWidth() > RUNPOS_PARAM::maxShootAngle ? RUNPOS_PARAM::maxShootAngle : bestRange->getWidth();
        }
    }

    // 3.与球的距离
    const MobileVisionT& ball = pVision->Ball();
    float distToBall = (candidate - ball.Pos()).mod() > RUNPOS_PARAM::maxDistToBall ? RUNPOS_PARAM::maxDistToBall : (candidate - ball.Pos()).mod();

    // 4.Angle to Goal
    CVector v2 = goal - candidate;
//            float angle2Goal = fabs(fabs(Utils::Normalize(v2.dir())) - Param::Math::PI / 4) > RUNPOS_PARAM::maxAngle2Goal ? RUNPOS_PARAM::maxAngle2Goal : fabs(fabs(Utils::Normalize(v2.dir())) - Param::Math::PI  / 4);
    float angle2Goal = 0;
    if (fabs(fabs(Utils::Normalize(v2.dir())) - Param::Math::PI / 4) < Param::Math::PI / 12) {
        angle2Goal = Param::Math::PI / 12;
    } else {
        angle2Goal = fabs(fabs(Utils::Normalize(v2.dir())) - Param::Math::PI / 4);
    }

    // 5.Dist to enemy
    float dist2Enemy = RUNPOS_PARAM::maxDist2Enemy;
    std::vector<int> enemyNumVec;
    int num = WorldModel::Instance()->getEnemyAmountInArea(candidate, RUNPOS_PARAM::maxDist2Enemy, enemyNumVec);
    for (int i = 0; i < num; i++) {
//        dist2Enemy += (RUNPOS_PARAM::maxDist2Enemy - candidate.dist(pVision->TheirPlayer(enemyNumVec.at(i)).Pos()));
        if (dist2Enemy > candidate.dist(pVision->TheirPlayer(enemyNumVec.at(i)).Pos())) {
            dist2Enemy = candidate.dist(pVision->TheirPlayer(enemyNumVec.at(i)).Pos());
        }
    }
//    qDebug() << "FUCK DIST IS" << dist2Enemy;

    // 6. predict their guard
    float guardMinTime = 9999;
    CGeoPoint p1(Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2), p2(Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2);
    CGeoSegment shootLine1(candidate, p1), shootLine2(candidate, p2);
    CGeoPoint p = WorldModel::Instance()->penaltyIntersection(shootLine1),
              q = WorldModel::Instance()->penaltyIntersection(shootLine2);
    WorldModel::Instance()->normalizeCoordinate(p);
    WorldModel::Instance()->normalizeCoordinate(q);
    for(int i = 1; i < Param::Field::MAX_PLAYER; i++) {
        const PlayerVisionT& enemy = pVision->TheirPlayer(i);
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
    //7. sector behind enemy
//    static const float maxDist = 300;
//    static const float maxDir = Param::Math::PI / 180 * 10;
    float sectorScore;
    float candiDir = RUNPOS_PARAM::maxSectorDir;
    float candiDist = RUNPOS_PARAM::maxSectorDist;
//    float candiDir = (candidate - ball.Pos()).dir();
//    float candiDist = candidate.dist(ball.Pos());
    for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
        if(pVision->TheirPlayer(i).Valid()) {
            const PlayerVisionT& enemy = pVision->TheirPlayer(i);
            float baseDir = (enemy.Pos() - ball.Pos()).dir();
            float baseDist = enemy.Pos().dist(ball.Pos());
//            if (fabs(Utils::Normalize(candiDir - baseDir)) < RUNPOS_PARAM::maxSectorDir && candiDist > baseDist) {
//                candiDir = fabs(Utils::Normalize(candiDir - baseDir));
//                candiDist = (candiDist - baseDist) > RUNPOS_PARAM::maxSectorDist ? RUNPOS_PARAM::maxSectorDist : candiDist - baseDist;
//            }
            if (fabs(Utils::Normalize((candidate - ball.Pos()).dir() - baseDir)) < RUNPOS_PARAM::maxSectorDir && candidate.dist(ball.Pos()) > baseDist) {
                candiDir = std::min(fabs(Utils::Normalize((candidate - ball.Pos()).dir() - baseDir)), double(candiDir));
                candiDist = std::min(candidate.dist(ball.Pos()), double(candiDist));
            }
        }
    }

    // 归一化处理
    distToGoal = 1 - distToGoal/RUNPOS_PARAM::maxDistToGoal;
    shootAngle = shootAngle/RUNPOS_PARAM::maxShootAngle;
    distToBall = distToBall/RUNPOS_PARAM::maxDistToBall;
    angle2Goal = 1 - angle2Goal / RUNPOS_PARAM::maxAngle2Goal;
    dist2Enemy = dist2Enemy > RUNPOS_PARAM::maxDist2Enemy ? 1 : dist2Enemy / RUNPOS_PARAM::maxDist2Enemy;
    guardMinTime = guardMinTime > 999 ? 0 : guardMinTime > RUNPOS_PARAM::maxGuardTime ? 1.0 : guardMinTime / RUNPOS_PARAM::maxGuardTime;
    sectorScore = candiDir / RUNPOS_PARAM::maxSectorDir * 0.3f + candiDist / RUNPOS_PARAM::maxSectorDist * 0.7f;

    // 计算得分
    std::vector<float> scores;
    scores.push_back(RUNPOS_PARAM::weight1 * distToGoal +
                     RUNPOS_PARAM::weight2 * shootAngle +
                     RUNPOS_PARAM::weight3 * distToBall +
                     RUNPOS_PARAM::weight4 * angle2Goal +
                     RUNPOS_PARAM::weight5 * dist2Enemy +
                     RUNPOS_PARAM::weight6 * guardMinTime +
                     RUNPOS_PARAM::weight7 * sectorScore);
    scores.push_back(RUNPOS_PARAM::weight1 * distToGoal);
    scores.push_back(RUNPOS_PARAM::weight2 * shootAngle);
    scores.push_back(RUNPOS_PARAM::weight3 * distToBall);
    scores.push_back(RUNPOS_PARAM::weight4 * angle2Goal);
    scores.push_back(RUNPOS_PARAM::weight5 * dist2Enemy);
    scores.push_back(RUNPOS_PARAM::weight6 * guardMinTime);
    scores.push_back(RUNPOS_PARAM::weight7 * sectorScore);
    return scores;
}

void RunPos::judgeRunPosValid() {
    for(int i = 0; i < 2; i++) {
        std::vector <int> ourNumVec;
        _runPos[i].isValid = false;
        for (int j = 0; j < 4; j++) {
            int num = WorldModel::Instance()->getOurAmountInArea(ATTACK_AREA[j]._point[0].x(), ATTACK_AREA[j]._point[2].x(), ATTACK_AREA[j]._point[0].y(), ATTACK_AREA[j]._point[2].y(), ourNumVec);
//            qDebug() << "areaNum:" << j << "ourCarNum:" << num;
            if (ATTACK_AREA[j].HasPoint(_runPos[i].pos) && num <= 1 && !ATTACK_AREA[j].HasPoint(_avoidPos)) {
                _runPos[i].isValid = true;
                _runPos[i].areaNum = j;
                areaRawPoint[j].isValid = false;
                break;
            }
            if (num > 0) {
                areaRawPoint[j].isValid = false;
            }
        }
    }
    if (_runPos[0].areaNum == _runPos[1].areaNum) {
        if (_pVision->Cycle() - runPosCycle[0] > WRONG_RUNPOS_CHANGE_BUFFER) {
            _runPos[0].isValid = false;
            runPosCycle[0] = _pVision->Cycle();
        }
    }
    for (int m = 0; m < 2; m++) {
        if (_runPos[m].isValid) {
            std::vector<float> realRunPosScores;
            realRunPosScores = evaluateFunc(_pVision, _runPos[m].pos);
            if (realRunPosScores.front() + 10 < areaRawPoint[_runPos[m].areaNum].score) {
                _runPos[m] = areaRawPoint[_runPos[m].areaNum];
            }
            continue;
        }
        for (int n = 0; n < 4; n++) {
            if (areaRawPoint[n].isValid) {
                _runPos[m] = areaRawPoint[n];
                break;
            }
        }
    }
//    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, -350), QString("raw:%1 %2 %3 %4").arg(areaRawPoint[0].isValid).arg(areaRawPoint[1].isValid).arg(areaRawPoint[2].isValid).arg(areaRawPoint[3].isValid).toLatin1(), COLOR_RED);
//    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, -330), QString("rawscore:%1 %2 %3 %4").arg(areaRawPoint[0].score).arg(areaRawPoint[1].score).arg(areaRawPoint[2].score).arg(areaRawPoint[3].score).toLatin1(), COLOR_RED);
//    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-500, -310), QString("realscore:%1 %2").arg(_runPos[0].score).arg(_runPos[1].score).toLatin1(), COLOR_RED);
//    GDebugEngine::Instance()->gui_debug_msg(_runPos[0].pos, QString("RUNPOS1").toLatin1(), COLOR_GREEN);
//    GDebugEngine::Instance()->gui_debug_msg(_runPos[1].pos, QString("RUNPOS2").toLatin1(), COLOR_GREEN);
////    GDebugEngine::Instance()->gui_debug_msg(_runPos[2].pos, QString("RUNPOS3").toLatin1(), COLOR_GREEN);
////    GDebugEngine::Instance()->gui_debug_msg(_runPos[3].pos, QString("RUNPOS4").toLatin1(), COLOR_GREEN);

//    GDebugEngine::Instance()->gui_debug_msg(areaRawPoint[0].pos, QString("POS1").toLatin1(), COLOR_PURPLE);
//    GDebugEngine::Instance()->gui_debug_msg(areaRawPoint[1].pos, QString("POS2").toLatin1(), COLOR_PURPLE);
//    GDebugEngine::Instance()->gui_debug_msg(areaRawPoint[2].pos, QString("POS3").toLatin1(), COLOR_PURPLE);
//    GDebugEngine::Instance()->gui_debug_msg(areaRawPoint[3].pos, QString("POS4").toLatin1(), COLOR_PURPLE);
}

//CGeoPoint RunPos::freeKickWaitPos() {
//    if (_avoidPos.x() > 0) {
//        if (_avoidPos.y() > 0)
//            return areaRawPoint[0].pos;
//        else return areaRawPoint[1].pos;
//    } else {
//        return CGeoPoint(0,0);
//    }
//}
