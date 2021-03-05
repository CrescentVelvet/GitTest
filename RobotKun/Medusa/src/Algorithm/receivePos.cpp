#include "receivePos.h"
#include "parammanager.h"
#include "ShootRangeList.h"
#include <random>
#include "CMmotion.h"
#include "SkillUtils.h"
#include "GDebugEngine.h"
#include "VisionModule.h"
#include "passposevaluate.h"
namespace {
    bool SAFE_PASS = true;
    // parameters
    const double weight1 = 0.25;//敌方到传球线距离的权值
    const double weight2 = 0.15;//敌方到接球点距离的权值
    const double weight3 = 0;//敌方到射门线距离的权值
    const double weight4 = 300;//射门角度的权值
    const double weight5 = -100;//接球射门角度的权值
    //阀值
    const double thresholdForEnemy2PassLineDist = 300;
    const double thresholdForEnemy2ShootLineDist = 300;
    const double thresholdForEnemy2PointDist = 300;
    const double slack = 0.5;
    const int maxStopBallSpeed = 650;

    static double FRICTION;
    static double powerRatio;

    CGeoPoint runPos[7];
    CGeoPoint passPos;
    int kickPower;

    // TEST FUNCTION
    int testCase = 0;
}

ReceivePos::ReceivePos()
    : _bestReceiver(999)
    , _receiver(999)
{
    bool IS_SIMULATION;
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION,"Alert/IsSimulation",false);
    if (IS_SIMULATION)
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Sim", 400);
    else
        ZSS::ZParamManager::instance()->loadParam(FRICTION,"AlertParam/Friction4Real", 400);

    powerRatio = IS_SIMULATION ? 1.4 : 1.7;
    const double areaLength = 100;
    const double minX = 0;
    const double maxX = 600;
    const double minY = -450;
    const double maxY = 450;
    lineAmount = (maxX - minX) / areaLength;
    columnAmount = (maxY - minY) / areaLength;
    areas.emplace_back(Area());
    double x = maxX;
    double y = minY;
    for (int i = 1; i <= lineAmount; i++) {
        for (int j = 1; j <= columnAmount; j++) {
            areas.emplace_back(Area(CGeoRectangle(x, y, x - areaLength, y + areaLength), CGeoPoint((2 * x - areaLength) / 2, (2 * y + areaLength) / 2), false));
            y += areaLength;
        }
        x -= areaLength;
        y = minY;
    }
}

//注意，图像信息中的车号是从1开始的，所以接受到的车号要加1
void ReceivePos::generatePassPos(const CVisionModule *pVision, const int leader)
{
    const MobileVisionT& ball = pVision->ball();
    const int minPassDist = 150;
    std::vector <CGeoPoint> tempPoint;
    std::vector <CGeoPoint> receivePoint;
    double maxPlayerScore = -99999;
    int pointCount;
    _receiver = 999;
    for (int i = 2; i <= PARAM::Field::MAX_PLAYER; i++) {
        if (i == leader + 1)
            continue;
        bool canSafePass = true;
        pointCount = 0;
        const PlayerVisionT& me = pVision->ourPlayer(i);
        if (me.Valid()){
            for (auto area : areas) {
                bool bestToPoint = true;
                double myTime = predictedTime(me, area.targetPos);
                double myDist = me.Pos().dist(area.targetPos);
                for(int k = 0; k < PARAM::Field::MAX_PLAYER; k++){
                    if (k == leader + 1)
                        continue;
                    const PlayerVisionT& teamMate = pVision->ourPlayer(k);
                    double teamMateDist = teamMate.Pos().dist(area.targetPos);
                    if (teamMateDist < myDist){
                        bestToPoint = false;
                        break;
                    }
                }
                if (bestToPoint){
                    double minAnemyTime = 9999;// = ZSkillUtils::instance()->getTheirBestInterTime();
                    for (int k = 0; k < PARAM::Field::MAX_PLAYER; k++) {
                        const PlayerVisionT& anemy = pVision->theirPlayer(k);
                        if (anemy.Valid()) {
                            double anemyTime = predictedTime(anemy, area.targetPos);
                            if (anemyTime < minAnemyTime)
                                minAnemyTime = anemyTime;
                        }
                    }
                    double ball2Target = ball.Pos().dist(area.targetPos);
                    double ballPassTime = myTime + slack;
                    double acc = (-FRICTION) / 2;
                    double ballSpeed = (ball2Target - acc * ballPassTime * ballPassTime / 2) / ballPassTime * powerRatio;
                    bool canNotPass = false;
                    if (ballSpeed > maxStopBallSpeed){
                        ballSpeed = 650;
                        if (ballSpeed * ballSpeed - FRICTION * ball2Target > 0) {
                            ballPassTime = 2 * (-ballSpeed + sqrt(ballSpeed * ballSpeed - FRICTION * ball2Target)) / (-FRICTION);
                        }
                        else {
                            ballPassTime = 9999;
                        }
                        ballPassTime += slack;
                    }
                    if (ballPassTime * ballPassTime > -2 * acc * ball2Target){
                       canNotPass = true;
                    }
                    canSafePass = ZSkillUtils::instance()->isSafeShoot(pVision,ballSpeed,area.targetPos);
                    if (SAFE_PASS && !canSafePass){
                       canNotPass = true;
                    }
                    if ((ballPassTime < minAnemyTime * 0.9) && !Utils::InTheirPenaltyArea(area.targetPos, 10) && Utils::IsInField(area.targetPos) && ball.Pos().dist(area.targetPos) > minPassDist && !canNotPass) {
                        area.status = true;
                        tempPoint.push_back(area.targetPos);
                        pointCount++;
                    }
                    else {
                        area.status = false;
                    }
                }
            }
            if (tempPoint.size()) {
                CGeoPoint theirGoal = CGeoPoint(600, 0);
                double meToTheirGoal = me.Pos().dist(theirGoal);
                double ballPassDist = me.Pos().dist(ball.Pos());
                double playerScore = pointCount * 10 - meToTheirGoal * 2 / 3 - ballPassDist * 2 / 4.5;
                if (playerScore > maxPlayerScore) {
                    if (ball.Pos().dist(pVision->ourPlayer(i).Pos()) > minPassDist) {
                        receivePoint = tempPoint;
                        _receiver = i;
                        maxPlayerScore = playerScore;
                    }
                }
            }
        }
        tempPoint.clear();
    }
    int receiver = 1;
    if (_receiver != 999) {
        receiver = _receiver;
        runPos[receiver - 1] = receiveBallPointCompute(pVision, leader, receivePoint);
        passPos = runPos[receiver - 1];
        GDebugEngine::Instance()->gui_debug_msg(passPos, "D", COLOR_BLUE);
        double myTime = predictedTime(pVision->ourPlayer(_receiver), passPos);
        double ball2Target = ball.Pos().dist(passPos);
        double ballPassTime = myTime + slack;
        double acc = (-FRICTION) / 2;
        double ballspeed = (ball2Target - acc * ballPassTime * ballPassTime / 2) / ballPassTime;
        kickPower = (ballspeed * powerRatio > maxStopBallSpeed) ? 650 : static_cast<int>(ballspeed * powerRatio);
        _bestReceiver = _receiver - 1;
        _bestPassPoint = passPos;
    }
    else {
        CGeoPoint theirGoal = CGeoPoint(PARAM::Field::PITCH_LENGTH,0);
        passPos = theirGoal;
        kickPower = 6500;
        double minLeader2Other = 9999999;
        for (int i = 2; i<=PARAM::Field::MAX_PLAYER; i++) {
            if (i == leader + 1)
                continue;
            double leader2Other = pVision->ourPlayer(i).Pos().dist2(pVision->ourPlayer(leader ).Pos());
            if (leader2Other < minLeader2Other) {
                minLeader2Other = leader2Other;
                _bestReceiver = i - 1;
            }
        }
        _bestPassPoint = passPos;
        // std::cout << "SHOOTING FOR CANNOT COMPUTE RECEVIER!" << std::endl; //wzdebug
    }
}

CGeoPoint ReceivePos::receiveBallPointCompute(const CVisionModule *pVision, int leader, const std::vector<CGeoPoint> &targetPoint)
{
    if (targetPoint.size() == 0) return CGeoPoint(PARAM::Field::PITCH_LENGTH / 2, 0);
    CGeoPoint bestPoint;
    float maxPointScore = -0xffff;
    for (auto target : targetPoint) {
        std::vector<float> scores = PassPosEvaluate::instance()->evaluateFunc(target,pVision->ourPlayer(leader).Pos(),EvaluateMode::PASS);
        float score = scores.front();
        if (score > maxPointScore) {
            maxPointScore = score;
            bestPoint = target;
        }
    }
    return bestPoint;
}

void ReceivePos::generateRandomTestPos(const CVisionModule *pVision)
{
//    int randNum = rand() % 4;
    switch (testCase) {
    case 0:
        _bestPassPoint = CGeoPoint(200, 200);
        break;
    case 1:
        _bestPassPoint = CGeoPoint(-200, 200);
        break;
    case 2:
        _bestPassPoint = CGeoPoint(-200, -200);
        break;
    case 3:
        _bestPassPoint = CGeoPoint(200, -200);
        break;
    }
    GDebugEngine::Instance()->gui_debug_msg(_bestPassPoint, "D", COLOR_BLUE);
    double bestDist = 9999;
    int bestNum = 999;
    for (int i=0; i<PARAM::Field::MAX_PLAYER; i++) {
        const PlayerVisionT& player = pVision->ourPlayer(i);
        if (player.Valid()){
            double dist = (_bestPassPoint - player.Pos()).mod();
            if (dist < bestDist){
                bestDist = dist;
                bestNum = i;
            }
        }
    }
    if ( bestNum != 999 )
        _bestReceiver = bestNum - 1;
    else {
        _bestReceiver = 999;
    }

}

void ReceivePos::testCasePlusPlus()
{
    testCase++;
    testCase = testCase%4;
}
