#include "zdrag.h"
#include <skill/Factory.h>
#include "param.h"
#include "WorldModel.h"
#include "TaskMediator.h"
#include "parammanager.h"

namespace  {
enum {
    GOTO = 1,
    MARKED,
    ANTIDRAG,
    REALDRAG,
    ESCAPE,
    BLOCK,
    FREE,
};
bool DEBUG = true;

const double MIN_SPEED_FOR_ENEMY  = 150;

const double CHECK_MARK_TIME        = 60;
const double CHECK_FREE_DIST        = 30;
const double CHECK_FREE_REDRAG_TIME = 20;
const double MAX_WAIT_TIME          = Param::Vision::FRAME_RATE * 5;

const CGeoSegment PENALTY_BORDER[6] = {CGeoSegment(CGeoPoint(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH, Param::Field::PENALTY_AREA_WIDTH / 2), CGeoPoint(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH, - Param::Field::PENALTY_AREA_WIDTH / 2)),
                                       CGeoSegment(CGeoPoint(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH, Param::Field::PENALTY_AREA_WIDTH / 2), CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::PENALTY_AREA_WIDTH / 2)),
                                       CGeoSegment(CGeoPoint(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH, - Param::Field::PENALTY_AREA_WIDTH / 2), CGeoPoint(Param::Field::PITCH_LENGTH / 2, - Param::Field::PENALTY_AREA_WIDTH / 2)),
                                       CGeoSegment(CGeoPoint(-(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH), Param::Field::PENALTY_AREA_WIDTH / 2), CGeoPoint(-(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH), - Param::Field::PENALTY_AREA_WIDTH / 2)),
                                       CGeoSegment(CGeoPoint(-(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH), Param::Field::PENALTY_AREA_WIDTH / 2), CGeoPoint(-Param::Field::PITCH_LENGTH / 2, Param::Field::PENALTY_AREA_WIDTH / 2)),
                                       CGeoSegment(CGeoPoint(-(Param::Field::PITCH_LENGTH / 2 - Param::Field::PENALTY_AREA_DEPTH), - Param::Field::PENALTY_AREA_WIDTH / 2), CGeoPoint(-Param::Field::PITCH_LENGTH / 2, - Param::Field::PENALTY_AREA_WIDTH / 2))};
}

CZDrag::CZDrag() : _lastCycle(0), _lastState(GOTO), _lastConfirmCycle(0), _state(GOTO)
{
    ZSS::ZParamManager::instance()->loadParam(DEBUG, "Debug/ZDrag", true);
}

void CZDrag::plan(const CVisionModule* pVision) {
    if (pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1) {
        setState(BEGINNING);
        enemyNum = -999;
        markDist = 9999;
        v1.setVector(0,0);
        v2 = v1;
        finalVelVec = v2;
        antiTarget.fill(0,0);
        realTarget.fill(0,0);
        freeBallDist = 300;
        freeCount = 0;
        antiCnt = 0;
        antiVelCount = 0;
        realCnt = 0;
        escapeCnt = 0;
        confirmFreeBallDist = false;
        _state = GOTO;
        purposeMode = false;
    }

//    if (pVision->Cycle() - _lastConfirmCycle > MIN_CHANGE_MARK_TIME) {
//        enemyNum = -999;
//    }

    int vecNumber = task().executor;
    CGeoPoint target = task().player.pos;
    CGeoPoint purposeTarget = CGeoPoint(task().player.kickpower, task().player.chipkickpower);
    const PlayerVisionT& me = pVision->OurPlayer(vecNumber);
    const double me2BallDir = Utils::Normalize((pVision->Ball().RawPos() - me.RawPos()).dir());
    const CGeoPoint interPoint = ZSkillUtils::instance()->predictedTheirInterPoint(me.RawPos(), pVision->Ball().RawPos());
    const double me2InterPoint = me.RawPos().dist(interPoint);

    if (ZSkillUtils::instance()->IsInField(purposeTarget) && purposeTarget.dist(target) > 100) {
        purposeMode = true;
    } else {
        purposeMode = false;
        purposeTarget =  pVision->Ball().RawPos() + CVector(-20,0).rotate(me2BallDir);//加了避球的flag后，点不能直接发在球上
    }
    if (_state == GOTO || _state == FREE/*|| enemyNum < 0*/) {
//        qDebug() << "JUDGE MARKED IN CYCLE " << pVision->Cycle() << "MARK DIST IS" << markDist;
        if (target.dist(me.RawPos()) < Param::Vehicle::V2::PLAYER_SIZE * 2) {
            int enemyMarkCar = -999;
            std::vector<int> enemyNumVec;
            int num = WorldModel::Instance()->getEnemyAmountInArea(interPoint, me2InterPoint, enemyNumVec, 20);
            if (num > 0) {
                for (int i = 0; i < num; i++) {
                    auto& enemy = pVision->TheirPlayer(enemyNumVec[i]);
                    if (enemy.Valid()) { // && !Utils::InTheirPenaltyArea(enemy.Pos(), 30) //不要取消后卫
                        double tmpDist = enemy.Pos().dist(me.RawPos());
                        if(tmpDist < markDist) {
                            markDist = tmpDist;
                            enemyMarkCar = enemyNumVec[i];
                            markDist -= 20;
                        }
                    }
                }
            }
            enemyNum = enemyMarkCar;
//            qDebug() << "enemyNUM is " << enemyNum;
            if (enemyNum > 0) {
                _state = MARKED;
//                qDebug() << "enemyNUM is " << enemyNum << pVision->Cycle();
            } else {
                _state = FREE;
                enemyNum = -999;
                freeBallDist = 300;
//                qDebug() << "I AM FREE BECAUSE NO ENEMY MARKING ME";
            }
        } else {
            _state = GOTO;
        }
    }

    CGeoPoint ball = pVision->Ball().RawPos();
    if (purposeMode) {
        ball = purposeTarget;
    }
    double judgeNum = -1;
    double me2BallAngle = Utils::Normalize((ball - me.RawPos()).dir());
    double me2BallDist = me.RawPos().dist(ball);
    double enemy2BallDist = 9999;
    double enemy2meDist = 9999;
    double enemy2InterPointDist = 9999;
    if (enemyNum > 0) {
        judgeNum = ((ball.x() - me.X()) * (pVision->TheirPlayer(enemyNum).Y() - me.Y())
                        - (pVision->TheirPlayer(enemyNum).X() - me.X()) * (ball.y() - me.Y())) * (purposeMode ? -1 : 1);//判断公式，正值为顺时针，负为逆时针 // purposeMode下需要反拉
        enemy2BallDist = pVision->TheirPlayer(enemyNum).Pos().dist(ball);
        enemy2meDist = pVision->TheirPlayer(enemyNum).Pos().dist(me.RawPos());
        enemy2InterPointDist = pVision->TheirPlayer(enemyNum).Pos().dist(interPoint);
    }

    if (_state == MARKED) {
        if (me.Vel().mod() < 10) {
            v1.setVector(3 * (enemy2meDist > 50 ? enemy2meDist : 50), 0);
            v1 = v1.rotate(me2BallAngle + (judgeNum > 0 ? 1 : -1) * 90 * Param::Math::PI / 180);
            antiTarget = me.RawPos() + v1;
            if (purposeMode && pVision->TheirPlayer(enemyNum).Pos().dist(purposeTarget) - 20 > me.RawPos().dist(purposeTarget)) {
                _state = ESCAPE;
            } else {
                for (int i = 0; i < 6; i++) {
                    if (CGeoSegment(me.RawPos(), antiTarget).IsSegmentsIntersect(PENALTY_BORDER[i])) {
                        _state = BLOCK;
                        break;
                    }
                    _state = ANTIDRAG;
                }
            }
        }
    }
    if (_state == ANTIDRAG) {
        antiCnt = pVision->TheirPlayer(enemyNum).Vel().mod() < 30 ? antiCnt + 1 : 0;
//        qDebug() << "THEIR VEL IS " <<pVision->TheirPlayer(enemyNum).Vel().mod() << "THEIR NUM IS " << enemyNum;
        if (antiCnt > CHECK_MARK_TIME) {
            antiCnt = 0;
            _state = ESCAPE;
//            qDebug() << "I AM FREE BECAUSE ENEMY IS TOO SLOW";
        }
        if (pVision->TheirPlayer(enemyNum).Vel().mod() > MIN_SPEED_FOR_ENEMY) {
            _state = REALDRAG;
            v2.setVector(3 * (enemy2meDist > 100 ? enemy2meDist : 100), 0);
            v2 = v2.rotate(v1.dir() + Param::Math::PI);
            realTarget = me.RawPos() + v2;
        }
        antiVelCount = me.RawPos().dist(antiTarget) < 50 ? antiVelCount + 1 : 0;
        if (antiVelCount > CHECK_FREE_REDRAG_TIME) {
            _state = MARKED;
            antiVelCount = 0;
        }
    }
    if (_state == REALDRAG) {
//        //test for non-zero speed
//        double finalSpeed = 300;
//        double finalSpeedDir = 0;
//        if (purposeMode) {
//            //nothing
//        } else {
//            auto& enemy = pVision->TheirPlayer(enemyNum);
//            double enemy2BallSpeedDir = Utils::Normalize(-(ball - enemy.Pos()).dir() - enemy.Vel().dir());
//            double enemy2BallSpeed = enemy.Vel().mod() * std::cos(enemy2BallSpeedDir);
//            double enemyRotSpeedAroundBall = enemy2BallSpeed * std::tan(enemy2BallSpeedDir) * (-judgeNum / std::fabs(judgeNum));
//            finalSpeedDir = std::atan2(enemyRotSpeedAroundBall, enemy2BallSpeed * 2);
//            if (enemy2BallSpeed < 0 && enemyRotSpeedAroundBall < 0) {
//                finalSpeedDir = 45 * Param::Math::PI / 180;
//            } else if(enemy2BallDist < 0) {
//                finalSpeedDir = 0;
//            } else if(enemyRotSpeedAroundBall < 0) {
//                finalSpeedDir = 90 * Param::Math::PI / 180 * (-judgeNum / std::fabs(judgeNum));
//            }
//            finalSpeedDir = (ball - me.RawPos()).dir() + finalSpeedDir;
//            finalVelVec = Utils::Polar2Vector(finalSpeed, finalSpeedDir);
//            v2 = Utils::Polar2Vector(3 * (enemy2meDist > 100 ? enemy2meDist : 100), finalSpeedDir);
//            realTarget = me.RawPos() + v2;
//            if (DEBUG) {
//                GDebugEngine::Instance()->gui_debug_line(enemy.Pos(), enemy.Pos() + CVector(enemyRotSpeedAroundBall, 0).rotate(Utils::Normalize((ball - enemy.Pos()).dir()) + Param::Math::PI / 2));
//                GDebugEngine::Instance()->gui_debug_line(enemy.Pos(), enemy.Pos() + Utils::Polar2Vector(enemy2BallSpeed, enemy2BallSpeedDir), COLOR_PURPLE);
//            }
//            if (enemy.Vel().mod() < 20) _state = ANTIDRAG;
//        }

        if (purposeMode) {
            CGeoLine purposeLine(me.RawPos(), purposeTarget);
            CGeoSegment purposeSegment(me.RawPos(), purposeTarget);
            CGeoPoint purposeProjection = purposeLine.projection(pVision->TheirPlayer(enemyNum).Pos());
//            qDebug() << "condition1 : " <<purposeSegment.IsPointOnLineOnSegment(purposeProjection)
//                     << "condition2 : " <<(me.RawPos().dist(purposeProjection) < pVision->TheirPlayer(enemyNum).Pos().dist(purposeProjection))
//                     << "condition3 : " <<( pVision->TheirPlayer(enemyNum).Pos().dist(purposeProjection) > 70);
            if (purposeSegment.IsPointOnLineOnSegment(purposeProjection) && me.RawPos().dist(purposeProjection) < pVision->TheirPlayer(enemyNum).Pos().dist(purposeProjection) && pVision->TheirPlayer(enemyNum).Pos().dist(purposeProjection) > 70) {
                _state = ESCAPE;
            }
        }
        else if (enemy2InterPointDist > (me2InterPoint > 50 ? me2InterPoint  : (me2InterPoint + CHECK_FREE_DIST))) {
            _state = BLOCK;
        }
        realCnt = me.RawPos().dist(realTarget) < 100 ? realCnt + 1 : 0;
        if (realCnt > CHECK_FREE_REDRAG_TIME) {
            _state = MARKED;
            realCnt = 0;
        }
    }
    if (_state == ESCAPE) {
//        qDebug() << "I ESCAPE BUT NOTHING HAPPERN WHEN YOU SEE THIS CONTINUS TWICE" << freeBallDist;
//        GDebugEngine::Instance()->gui_debug_arc(ball, freeBallDist ,0 , 360.0, COLOR_PURPLE);
        if (purposeMode) {
            escapeCnt = (me.RawPos().dist(purposeTarget) < Param::Vehicle::V2::PLAYER_SIZE * 2) ? escapeCnt + 1 : 0;
            if (escapeCnt > MAX_WAIT_TIME) {
                _state = GOTO;
                markDist = 9999;
                escapeCnt = 0;
            }
        } else {
            if (!confirmFreeBallDist) {
                freeBallDist = me2BallDist * 0.6;
                confirmFreeBallDist = true;
            }
            if (me2BallDist < (freeBallDist < 200 ? 200 : freeBallDist)) {
    //            qDebug() << "I WILL ESCAPE TO GOTO" << "me2BallDist" << me2BallDist << "freeBallDist" << freeBallDist;
                _state = GOTO;
                markDist = 9999;
                confirmFreeBallDist = false;
            }
            if (enemy2BallDist - me2BallDist > 40 && !(enemy2BallDist > 8888) ) {
    //                qDebug() << "enemyDist" << enemy2BallDist << "meDist" << me2BallDist;
                _state = FREE;
    //            qDebug() << "I AM FREE" ;
                confirmFreeBallDist = false;
            }
        }
    }
    if (_state == BLOCK) {
//        qDebug() << "I AM BLOCKING!" << pVision->Cycle();
        if (me2BallDist < 200) {
//            qDebug() << "I WILL FREE TO GOTO" << pVision->Cycle();
            _state = GOTO;
            markDist = 9999;
        }
        if (pVision->TheirPlayer(enemyNum).Vel().mod() < 5) {
            _state = FREE;
        }
//        qDebug() << "state1" << (enemy2BallDist - me2BallDist < 20) << "state2" << (me2BallDist < ((2.0 * freeBallDist / 3.0) > 100 ? 100 : (2.0 * freeBallDist / 3.0)));
    }
    if (_state == FREE) {
        if (enemy2BallDist - me2BallDist < 20 || me2BallDist < (freeBallDist< 200 ? 200 :freeBallDist)) {
//            qDebug() << "I WILL FREE TO GOTO" << "me2BallDist" << me2BallDist << "freeBallDist" << freeBallDist;
            _state = GOTO;
            markDist = 9999;
        }
//        qDebug() << "state1" << (enemy2BallDist - me2BallDist < 20) << "state2" << (me2BallDist < ((2.0 * freeBallDist / 3.0) > 100 ? 100 : (2.0 * freeBallDist / 3.0)));
    }

    freeCount = me.RawPos().dist(task().player.pos) < Param::Vehicle::V2::PLAYER_SIZE * 2 ? ++freeCount : 0;
//    qDebug() << "freeCount is" << freeCount << (me.RawPos().dist(task().player.pos) < Param::Vehicle::V2::PLAYER_SIZE * 2) << vecNumber;
    if (freeCount > MAX_WAIT_TIME) {
//        qDebug() << "FUCK BUG BECAUSE ME";
        _state = ESCAPE;
        freeCount = 0;
    }

    //防止车出场
    if (Utils::OutOfField(antiTarget, Param::Vehicle::V2::PLAYER_SIZE * 2))
        antiTarget = Utils::MakeInField(antiTarget, Param::Vehicle::V2::PLAYER_SIZE * 2);
    if (Utils::OutOfField(realTarget, Param::Vehicle::V2::PLAYER_SIZE * 2))
        realTarget = Utils::MakeInField(realTarget, Param::Vehicle::V2::PLAYER_SIZE * 2);
    //防止车进入禁区
    if (Utils::InTheirPenaltyArea(antiTarget, Param::Vehicle::V2::PLAYER_SIZE * 2))
        antiTarget = Utils::MakeOutOfTheirPenaltyArea(antiTarget,Param::Vehicle::V2::PLAYER_SIZE * 2);
    if (Utils::InOurPenaltyArea(antiTarget, Param::Vehicle::V2::PLAYER_SIZE * 2))
        antiTarget = Utils::MakeOutOfOurPenaltyArea(antiTarget,Param::Vehicle::V2::PLAYER_SIZE * 2);
    if (Utils::InTheirPenaltyArea(realTarget, Param::Vehicle::V2::PLAYER_SIZE * 2))
        realTarget = Utils::MakeOutOfTheirPenaltyArea(realTarget,Param::Vehicle::V2::PLAYER_SIZE * 2);
    if (Utils::InOurPenaltyArea(realTarget, Param::Vehicle::V2::PLAYER_SIZE * 2))
        realTarget = Utils::MakeOutOfOurPenaltyArea(realTarget,Param::Vehicle::V2::PLAYER_SIZE * 2);

    if((_state == MARKED || _state == ANTIDRAG || _state == REALDRAG) && DEBUG) {
        GDebugEngine::Instance()->gui_debug_msg(pVision->TheirPlayer(enemyNum).Pos(), QString("W").toLatin1(), COLOR_PURPLE);
        GDebugEngine::Instance()->gui_debug_line(pVision->TheirPlayer(enemyNum).Pos(), me.RawPos(), COLOR_PURPLE);
    }
    if (DEBUG) {
        GDebugEngine::Instance()->gui_debug_x(target, COLOR_PURPLE);
        GDebugEngine::Instance()->gui_debug_arc(interPoint, me2InterPoint + 20, 0.0, 360.0);
        GDebugEngine::Instance()->gui_debug_msg(me.RawPos() + CVector(0, 20), QString("STATE: %1").arg(QString::number(_state)).toLatin1(), COLOR_PURPLE);
    }
//    std::cout << "ZDRAG STATE NOW IS " << _state << " " << pVision->Cycle()<< std::endl;

    TaskT dragTask(task());
    dragTask.player.flag |= PlayerStatus::ALLOW_DSS | PlayerStatus::DODGE_BALL;

    switch (_state) {
    case GOTO:
        dragTask.player.pos = target;
        dragTask.player.angle = me2BallDir;
        setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(dragTask));
        break;

    case MARKED:
        setSubTask(TaskFactoryV2::Instance()->StopRobot(dragTask));
        break;

    case ANTIDRAG:
        GDebugEngine::Instance()->gui_debug_msg(antiTarget, QString("ANTI").toLatin1(), COLOR_PURPLE);
        dragTask.player.pos = antiTarget;
        dragTask.player.angle = me2BallDir;
        setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(dragTask));
        break;

    case REALDRAG:
        dragTask.player.pos = realTarget;
        dragTask.player.angle = me2BallDir;
//        dragTask.player.vel = finalVelVec;
        setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(dragTask));
        break;

    case ESCAPE:
        dragTask.player.pos = purposeTarget;
        dragTask.player.angle = me2BallDir;
        setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(dragTask));
        break;

    case BLOCK:
        dragTask.player.angle = me2BallDir;
        dragTask.player.pos = ZSkillUtils::instance()->getMarkingPoint(pVision->TheirPlayer(enemyNum).Pos(), pVision->TheirPlayer(enemyNum).Vel(), 450, 450, 450, 300, pVision->Ball().RawPos());
        setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(dragTask));
        break;

    case FREE:
        markDist = 9999;
        dragTask.player.angle = me2BallDir;
//        dragTask.player.max_acceleration = MAX_ACC - 200;
        setSubTask(TaskFactoryV2::Instance()->StopRobot(dragTask));
        break;

    default:
        dragTask.player.pos = target;
        dragTask.player.angle = me2BallDir;
        setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(dragTask));
        break;
    }

    _lastCycle = pVision->Cycle();
    _lastState = _state;
    return CStatedTask::plan(pVision);
}

CPlayerCommand* CZDrag::execute(const CVisionModule *pVision) {
    if (subTask()) {
        return subTask()->execute(pVision);
    }
    return nullptr;
}
