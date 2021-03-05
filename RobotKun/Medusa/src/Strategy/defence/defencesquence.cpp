#include "defencesquence.h"
#include "staticparams.h"
#include "Global.h"
namespace {
const int OUR_NO_MARKING_DEFENDER_NUM = 3; //门将及两个贴禁区的后卫
namespace DEFEND_PARAM {
    const float maxDistToBall = 3000;
    const float WtoBallDist = 50;
    const float WtoGoalDist = 400;
    const float WtoGoalDir = 100;
    const float WleaderTurn = 50;
    const float WguardMin = 200;
}
namespace FUTURE_CONSIDER {//采用类似zblock的预测方式，考虑对方stoptime后的可行运动区域
    const float aMax = 4500;
    const float dMax = 4500;
    const float aRotateMax = 4500;
    const float vMax = 3000;
    const float minVel = 1000;
    const float WfutureStop = 0.4;
    const float WfutureTotal = 0.5;
}
bool debug = false;
bool GreaterSort(robotThreat a, robotThreat b) {
    return a.value > b.value;
}

CGeoPoint pointRegulate(CGeoPoint p0) {
    if (Utils::InOurPenaltyArea(p0, PARAM::Vehicle::V2::PLAYER_SIZE * 2))
        p0 = Utils::MakeOutOfTheirPenaltyArea(p0, PARAM::Vehicle::V2::PLAYER_SIZE * 2);
    if(!Utils::IsInField(p0, PARAM::Vehicle::V2::PLAYER_SIZE * 2))
        p0 = Utils::MakeInField(p0, PARAM::Vehicle::V2::PLAYER_SIZE * 2);
    return p0;
}
}
CDefenceSquence::CDefenceSquence()
{
    ZSS::ZParamManager::instance()->loadParam(debug,"Debug/defenceSquence", false);
    for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
        enemyThreat[i].num = i;
    }
    _cycle = 0;
    _lastUpDateCycle = 0;
    _lastChangeLeaderCycle = 0;
}

void CDefenceSquence::update() {
    _cycle = vision->getCycle();
    if(_cycle - _lastUpDateCycle > 10) {
        vector<int>::iterator  defender;//门将及两个贴禁区的后卫
        confirmBestEnemy();//确认对方leader
//    theirLeaderNum = ZSkillUtils::instance()->getTheirBestPlayer();
        //清空上一次序列
        enemyOrder.clear();
        //更新不mark的防守队员
        if(ourPenaltyDefender.size() > 0) {
            for(defender = ourPenaltyDefender.begin(); defender != ourPenaltyDefender.end(); defender++) {
                if(!(Utils::InOurPenaltyArea(vision->ourPlayer(*defender).Pos(), 300))) {;
                    if(defender + 1 == ourPenaltyDefender.end()) {
                        ourPenaltyDefender.erase(std::remove(ourPenaltyDefender.begin(), ourPenaltyDefender.end(), *defender), ourPenaltyDefender.end());
                        break;
                    }
                    ourPenaltyDefender.erase(std::remove(ourPenaltyDefender.begin(), ourPenaltyDefender.end(), *defender), ourPenaltyDefender.end());
                }
            }
        }
        if(ourPenaltyDefender.size() < OUR_NO_MARKING_DEFENDER_NUM) { //原本role改变时予以更新
            for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
                auto temp_defender = vision->ourPlayer(i);
                if(temp_defender.Valid())
                    if(Utils::InOurPenaltyArea(temp_defender.Pos(), 300))
                        if(ourPenaltyDefender.size() < OUR_NO_MARKING_DEFENDER_NUM) {
                            vector<int>::iterator iter = std::find(ourPenaltyDefender.begin(), ourPenaltyDefender.end(), i);
                            if (iter != ourPenaltyDefender.end()) continue;//避免重复导入
                            if(i == theirLeaderNum) continue;//跳过leader防禁区的情况
                            ourPenaltyDefender.push_back(i);
                        }
            }
        }
        std::vector<int> enemyNumVec;
        _attackerAmount = WorldModel::Instance()->getEnemyAmountInArea(-PARAM::Field::PITCH_LENGTH / 2, 0, -PARAM::Field::PITCH_WIDTH / 2, PARAM::Field::PITCH_WIDTH / 2, enemyNumVec);
        for(auto enemy:enemyNumVec) {
            auto temp_attacker = vision->theirPlayer(enemy);
            if(temp_attacker.Valid()) {
                enemyThreat[enemy].preValue = enemyThreat[enemy].value ;
                if(vision->theirPlayer(enemy).Pos().x() > 0) enemyThreat[enemy].value = -1; //不mark后场的车子
                else if(theirLeaderNum == enemy)enemyThreat[enemy].value = 0;
                else enemyThreat[enemy].value = totalEvaluate(enemy, true);
                enemyThreat[enemy].num = enemy;
                enemyOrder.push_back(enemyThreat[enemy]);
            }
        }
        std::sort(enemyOrder.begin(), enemyOrder.end(), GreaterSort);
        _lastUpDateCycle = _cycle;
    }

    if(debug) {
        int RGBvalue[8]= {25467101,252157154,249205173,200200169,131175155,22215683,222211140};
        int i = 0;
        for (vector<robotThreat>::iterator order = enemyOrder.begin(); order != enemyOrder.end(); order++) {
            if(order->value<0) continue;
            i++;
            int num = order->num;
            CGeoPoint p0 = vision->theirPlayer(num).Pos();

            GDebugEngine::Instance()->gui_debug_msg(vision->theirPlayer(order->num).Pos() + CVector(0, 200), QString("order:%1").arg(QString::number(i)).toLatin1(), COLOR_USE_RGB,RGBvalue[i]);
            GDebugEngine::Instance()->gui_debug_msg(vision->theirPlayer(order->num).Pos() + CVector(0, 400), QString("value:%1").arg(QString::number(order->value)).toLatin1(), COLOR_USE_RGB,RGBvalue[i]);
            if(order->value > 0) {
                GDebugEngine::Instance()->gui_debug_msg(p0 + CVector(0, 600), QString("Dist2Ball:%1").arg(QString::number(debugTemp[num].dist2ball)).toLatin1(), COLOR_USE_RGB,RGBvalue[i]);
                GDebugEngine::Instance()->gui_debug_msg(p0 + CVector(0, 800), QString("Dist2Goal:%1").arg(QString::number(debugTemp[num].dist2goal)).toLatin1(), COLOR_USE_RGB,RGBvalue[i]);
                GDebugEngine::Instance()->gui_debug_msg(p0 + CVector(0, 1000), QString("Angle2Goal:%1").arg(QString::number( debugTemp[num].angle2goal)).toLatin1(), COLOR_USE_RGB,RGBvalue[i]);
                GDebugEngine::Instance()->gui_debug_msg(p0 + CVector(0, 1200), QString("turnAngle:%1").arg(QString::number(debugTemp[num].turnAngle)).toLatin1(), COLOR_USE_RGB,RGBvalue[i]);
                GDebugEngine::Instance()->gui_debug_msg(p0 + CVector(0, 1400), QString("guardMinTime:%1").arg(QString::number(debugTemp[num].guardMinTime)).toLatin1(), COLOR_USE_RGB,RGBvalue[i]);
                GDebugEngine::Instance()->gui_debug_msg(p0 + CVector(0, 1600), QString("sectorScore:%1").arg(QString::number( debugTemp[num].sectorScore)).toLatin1(), COLOR_USE_RGB,RGBvalue[i]);
                GDebugEngine::Instance()->gui_debug_msg(p0 + CVector(0, 1800), QString("future:%1").arg(QString::number( debugTemp[num].futuretotal)).toLatin1(), COLOR_USE_RGB,RGBvalue[i]);
                GDebugEngine::Instance()->gui_debug_x(debugTemp[num].stopPoint, COLOR_USE_RGB,RGBvalue[i]);
                GDebugEngine::Instance()->gui_debug_x(debugTemp[num].accPoint, COLOR_USE_RGB,RGBvalue[i]);
                GDebugEngine::Instance()->gui_debug_x(debugTemp[num].leftPoint, COLOR_USE_RGB,RGBvalue[i]);
                GDebugEngine::Instance()->gui_debug_x(debugTemp[num].rightPoint, COLOR_USE_RGB,RGBvalue[i]);
                GDebugEngine::Instance()->gui_debug_x(debugTemp[num].centerPoint, COLOR_USE_RGB,RGBvalue[i]);
//                if(debugTemp[num].MarkCar > 0) GDebugEngine::Instance()->gui_debug_line(vision->ourPlayer(debugTemp[num].MarkCar).Pos(), p0, COLOR_USE_RGB,RGBvalue[i]);
            }
        }
        for(int i = 0 ;i<ourPenaltyDefender.size();i++){
            GDebugEngine::Instance()->gui_debug_msg(vision->ourPlayer(ourPenaltyDefender[i]).Pos() + CVector(0, 200),QString("defencePena").toLatin1(),COLOR_RED);
        }
    }
}
float CDefenceSquence::evaluateThreatBasic(CGeoPoint p0, int num = -1) { //num Debug用，后续再优化一下结构
    CGeoPoint ballPos = vision->ball().Pos();
    CGeoPoint  p1(-PARAM::Field::PITCH_LENGTH / 2, -PARAM::Field::GOAL_WIDTH / 2), p2(-PARAM::Field::PITCH_LENGTH / 2, PARAM::Field::GOAL_WIDTH / 2);

    //leader的处理有待加入
//    //射门角度
//    float shootAngle = 0;

//    CShootRangeList shootRangeList(vision, 1, 0, p0);
//    const CValueRangeList& shootRange = shootRangeList.getShootRange();
//    if (shootRange.size() > 0) {
//        auto bestRange = shootRange.getMaxRangeWidth();
//        if(bestRange) {
//            shootAngle = bestRange->getWidth();
//        }
//    }
//    //计算折射角，折射角小时touch威胁大
    CVector v1 = ballPos - p0;
    CVector v2 = p1.midPoint(p2) - p0;
//    float refracAngle = fabs(v1.dir() - v2.dir());
//    refracAngle = refracAngle > PARAM::Math::PI ? 2 * PARAM::Math::PI - refracAngle : refracAngle;
    //计算到球距离
    float Dist2Ball = fabs(p0.dist(ballPos));
    // 计算到球门
    float Dist2Goal = fabs(p0.dist(CGeoPoint(-PARAM::Field::PITCH_LENGTH / 2, 0)));
    float Angle2Goal = fabs(fabs(Utils::Normalize(v2.dir())) - PARAM::Math::PI * 3 / 4);
    // 对方leader接球转身并传到该点所需的角度
    CVector leaderToBall = ballPos - vision->theirPlayer(theirLeaderNum).Pos();
    CVector leaderToPassPos = p0 - vision->theirPlayer(theirLeaderNum).Pos();
    float turnAngle = fabs(leaderToBall.dir() - leaderToPassPos.dir());
    turnAngle = turnAngle > PARAM::Math::PI ? 2 * PARAM::Math::PI - turnAngle : turnAngle;
    if(ballPos.x() > vision->theirPlayer(theirLeaderNum).Pos().x()) turnAngle = PARAM::Math::PI;
    //我方后卫拦截
    float guardMinTime = 9999;
    CGeoSegment shootLine1(p0, p1), shootLine2(p0, p2);
    CGeoPoint p = WorldModel::Instance()->penaltyIntersection(shootLine1),
              q = WorldModel::Instance()->penaltyIntersection(shootLine2);
    WorldModel::Instance()->normalizeCoordinate(p);
    WorldModel::Instance()->normalizeCoordinate(q);
    int PenaltyDefenderNum = ourPenaltyDefender.size();
    for(int i = 0; i < std::min(OUR_NO_MARKING_DEFENDER_NUM, PenaltyDefenderNum ); i++) {
        const PlayerVisionT& guard = vision->ourPlayer(ourPenaltyDefender[i]);
        if (!Utils::InOurPenaltyArea(guard.Pos(), 0) && Utils::InOurPenaltyArea(guard.Pos(), 50)) {
            float pTime = WorldModel::Instance()->preditOurGuard(guard, p);
            float qTime = WorldModel::Instance()->preditOurGuard(guard, q);
            if (pTime < guardMinTime) {
                guardMinTime = pTime;
            }
            if (qTime < guardMinTime) {
                guardMinTime = qTime;
            }
        }
    }
    //归一化
//    shootAngle = shootAngle > 25 ? 1 : shootAngle / 25;//暂时未计入
//    refracAngle = refracAngle < PARAM::Math::PI / 6 ? PARAM::Math::PI / 6 : refracAngle;//暂时未计入


    Dist2Ball = (Dist2Ball / DEFEND_PARAM::maxDistToBall) > 1 ? 1 : Dist2Ball / DEFEND_PARAM::maxDistToBall;
    Dist2Goal = 1 - Dist2Goal / PARAM::Field::PITCH_LENGTH;
    Angle2Goal = 1 - Angle2Goal / PARAM::Math::PI;
    turnAngle = turnAngle < PARAM::Math::PI / 12 ? PARAM::Math::PI / 12 : turnAngle;
    turnAngle = 1 - turnAngle / PARAM::Math::PI;
    guardMinTime = guardMinTime > 999 ? 0 : ((guardMinTime > 2.0 ? 2.0 : guardMinTime) / 2.0);
    if(debug) {
        debugTemp[num].dist2ball = Dist2Ball * DEFEND_PARAM::WtoBallDist;
        debugTemp[num].dist2goal = Dist2Goal * DEFEND_PARAM::WtoGoalDist;
        debugTemp[num].angle2goal =  Angle2Goal * DEFEND_PARAM::WtoGoalDir;
        debugTemp[num].turnAngle = Angle2Goal * DEFEND_PARAM::WtoGoalDir;
        debugTemp[num].guardMinTime =  guardMinTime * DEFEND_PARAM::WguardMin;

    }
    double totalWeight =  DEFEND_PARAM::WtoBallDist + DEFEND_PARAM::WtoGoalDist +DEFEND_PARAM::WtoGoalDir +DEFEND_PARAM::WleaderTurn +DEFEND_PARAM::WguardMin;
    float basic_value =1/totalWeight*100*(Dist2Ball * DEFEND_PARAM::WtoBallDist + Dist2Goal * DEFEND_PARAM::WtoGoalDist + Angle2Goal * DEFEND_PARAM::WtoGoalDir + turnAngle * DEFEND_PARAM::WleaderTurn + guardMinTime * DEFEND_PARAM::WguardMin);
    //+shootAngle * DEFEND_PARAM::WshootAngle + refracAngle * DEFEND_PARAM::WrefracAngle +

    return  basic_value;
}

float CDefenceSquence::totalEvaluate(int num, bool useFuture) {
    auto enemy = vision->theirPlayer(num);
    float total = evaluateThreatBasic(enemy.Pos(), num)/* + evaluateInter(num)*/;
    float totalF1, totalF2, totalF3, totalF4, totalF5;
    totalF1 = totalF2 = totalF3 = totalF4 = totalF5 = 0;
    CVector vel = enemy.Vel();
    CGeoPoint stopPoint = CGeoPoint(9999, 9999);
    CGeoPoint accPoint;
    CGeoPoint leftPoint;
    CGeoPoint rightPoint;
    CGeoPoint centerPoint;
    //针对速度较大的车，考虑其未来可能的大致运动趋势对结果好坏的影响
    if(useFuture && vel.mod() > FUTURE_CONSIDER::minVel) {
        double stopDist = vel.mod2() / (2 * FUTURE_CONSIDER::dMax);
        double stopTime = vel.mod() / FUTURE_CONSIDER::dMax;
        double vM = vel.mod() + FUTURE_CONSIDER::aMax * stopTime;
        double accDist;
        if(vM < FUTURE_CONSIDER::vMax) {
            if(debug) {
                debugTemp[num].stopPoint = CGeoPoint(0, 0);
                debugTemp[num].accPoint = CGeoPoint(0, 0);
                debugTemp[num].leftPoint = CGeoPoint(0, 0);
                debugTemp[num].rightPoint = CGeoPoint(0, 0);
                debugTemp[num].centerPoint = CGeoPoint(0, 0);

            }
            accDist = vel.mod() * stopTime + 0.5 * FUTURE_CONSIDER::aMax * pow(stopTime, 2);
        } else
            accDist = (FUTURE_CONSIDER::vMax * FUTURE_CONSIDER::vMax - vel.mod2()) / (2 * FUTURE_CONSIDER::aMax) + FUTURE_CONSIDER::vMax * (stopTime - (FUTURE_CONSIDER::vMax - vM) / FUTURE_CONSIDER::aMax);

        CVector velDirec = vel / vel.mod();
        CVector direc1(velDirec.y(), -velDirec.x());
        CVector direc2 = -direc1;
        double R = vel.mod2() / FUTURE_CONSIDER::aRotateMax;
        double theta1 = vel.mod() * stopTime / R;
        stopPoint = enemy.Pos() + velDirec * stopDist;
        accPoint = enemy.Pos() + velDirec * accDist;
        leftPoint = enemy.Pos() + direc1 * R * (1 - cos(theta1)) + velDirec * R * sin(theta1);
        rightPoint = enemy.Pos() + direc2 * R * (1 - cos(theta1)) + velDirec * R * sin(theta1);
        centerPoint = CGeoSegment(stopPoint, accPoint).segmentsIntersectPoint(CGeoSegment(rightPoint, leftPoint));
        stopPoint = pointRegulate(stopPoint);
        accPoint = pointRegulate(accPoint);
        leftPoint = pointRegulate(leftPoint);
        rightPoint = pointRegulate(rightPoint);
        centerPoint = pointRegulate(centerPoint);
        if(debug) {
            debugTemp[num].stopPoint = stopPoint;
            debugTemp[num].accPoint = accPoint;
            debugTemp[num].leftPoint = leftPoint;
            debugTemp[num].rightPoint = rightPoint;
            debugTemp[num].centerPoint = centerPoint;

        }
        totalF1 = !Utils::IsInField(stopPoint) ? 0 : evaluateThreatBasic(stopPoint) /*+ evaluateInter(num, true, stopPoint)*/;
        totalF2 = evaluateThreatBasic(accPoint) /*+ evaluateInter( num, true, accPoint)*/;
        totalF3 = evaluateThreatBasic(leftPoint)/* + evaluateInter( num, true, leftPoint)*/;
        totalF4 = evaluateThreatBasic(rightPoint) /*+ evaluateInter( num, true, rightPoint)*/;
        totalF5 = evaluateThreatBasic(centerPoint)/* + evaluateInter( num, true, centerPoint)*/;
        float futureTotal = (FUTURE_CONSIDER::WfutureStop * totalF1 +
                             (totalF2 + totalF3 + totalF4 + totalF5) / 4  * (1 - FUTURE_CONSIDER::WfutureStop)
                            );

        if(debug) {
            debugTemp[num].futuretotal = futureTotal;

        }
        total = total * (1 - FUTURE_CONSIDER::WfutureTotal)
                + futureTotal *  FUTURE_CONSIDER::WfutureTotal;

    }
    return total;
}


void CDefenceSquence::confirmBestEnemy() {
    if (_cycle - _lastChangeLeaderCycle > 10) {
        //采用messi做法， 根据截球点判断
        int bestInterPlayer = theirLeaderNum;
        double bestInterTime = 9999;
        double bestInterDist = 9999;
        if(theirLeaderNum > 0) {
            bestInterTime = ZSkillUtils::instance()->getTheirInterTime(theirLeaderNum) - 0.3; //避免leader狂跳
            bestInterDist = vision->theirPlayer(theirLeaderNum).Pos().dist(vision->ball().Pos()) - 30;
        }
        for (int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
            if(vision->theirPlayer(i).Valid() && !Utils::InTheirPenaltyArea(vision->theirPlayer(i).Pos(), 6 * PARAM::Vehicle::V2::PLAYER_SIZE)) {
                double interTime = ZSkillUtils::instance()->getTheirInterTime(i);
                double newInterDist = vision->theirPlayer(i).Pos().dist(vision->ball().Pos());
                if(interTime < bestInterTime && newInterDist < bestInterDist) {
                    bestInterPlayer = i;
                    bestInterTime = interTime;
                    bestInterDist = newInterDist;
                }
            }
        }
        if(theirLeaderNum != bestInterPlayer ) _lastChangeLeaderCycle = _cycle;
        theirLeaderNum = bestInterPlayer;
    }
}




//    if (ourmark > 0) {//在考虑射门时，去除我防最近的盯人车
//        vector<int>::iterator defender = find(ourPenaltyDefender.begin(),ourPenaltyDefender.end(),outmark);
//        if (defender!=ourPenaltyDefender.end()&&
//        (num!=theirLeaderNum||(num==theirLeaderNum and ourmark!= MessiDecision::Instance()->leaderNum())
//                )
//            )//对于对方的leader，不删除我方leader
//        {
//            deleteCar =1;
//            CShootRangeList shootRangeList(vision,1,outmark,enemyPos);
//            const CValueRangeList& shootRange = shootRangeList.getShootRange();
//            if (shootRange.size() > 0) {
//                auto bestRange = shootRange.getMaxRangeWidth();
//                if(bestRange) {
//                    shootAngle = bestRange->getWidth();
//                }
//            }
//        }
//    }
//    if(!deleteCar){
//        CShootRangeList shootRangeList(vision,1,0,enemyPos);
//        const CValueRangeList& shootRange = shootRangeList.getShootRange();
//        if (shootRange.size() > 0) {
//            auto bestRange = shootRange.getMaxRangeWidth();
//            if(bestRange) {
//                shootAngle = bestRange->getWidth();
//            }
//        }
//    }

