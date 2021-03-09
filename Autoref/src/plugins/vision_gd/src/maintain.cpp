#include "maintain.h"
#include "globaldata.h"
#include "staticparams.h"
#include "visionmodule.h"
#include "dealball.h"
#include "dealrobot.h"
#include <fstream>
#include <iostream>
#include "ballstate.h"
using namespace std;
namespace {
bool whetherTestRobotSpeed = false;

const int MAX_BALL_PER_FRAME = 200;
const int CHIP_DIS = 10;
}
CMaintain::CMaintain(): file("d:\\test.txt"), out(&file) {
    if (::whetherTestRobotSpeed && !file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cout << "Cannot Open" << std::endl;
    }

}
CMaintain::~CMaintain() {
    if (::whetherTestRobotSpeed) file.close();
}


void CMaintain::init() {
    result.init();
    result.addBall(GlobalData::instance()->processBall[0].ball[0]);
    result.isBallValid = GlobalData::instance()->processBall[0].isBallValid;
    for (int color = 0; color < PARAM::TEAMS; color++) {
        for (int i = 0; i < PARAM::ROBOTNUM; i++) {
            result.robot[color][i].fill(GlobalData::instance()->processRobot[0].robot[color][i]);
        }
    }
}


double CMaintain::getpredict_x() {
    return result.ball[0].predict_pos.x();
}

double CMaintain::getpredict_y() {
    return result.ball[0].predict_pos.y();
}

void CMaintain::run() {  //TODO move to visionmodule
    init();
    //必须保证processrobot数组里为真值，否则产生误差累计
    DealRobot::instance()->updateVel(result);
    //Ball Statemachine
    if (ballState::instance()->ballCloseEnough2Analyze() ||
            ballState::instance()->ballIsOnEdge(result.ball[0].pos))
        //离车近
    {

        //判断碰撞
//        CollisionDetect::instance()->analyzeData(result);
        //ball kalmanfilter FOLLOW
        auto & tempMatrix = ballKalmanFilter.follow(result.ball[0].pos);
        DealBall::instance()->updateVel(tempMatrix, result);
    } else {      //离车远
        //ball kalmanfilter
        auto & tempMatrix = ballKalmanFilter.update(result.ball[0].pos);
        DealBall::instance()->updateVel(tempMatrix, result);

        CVector3 realpos;
//        result.ball[0].predict_pos = chipsolver.dealresult(realpos);
        CGeoLine line(result.ball[0].pos, GlobalData::instance()->maintain[-7].ball[0].pos);
        CGeoPoint middlePoint(GlobalData::instance()->maintain[-4].ball[0].pos);
//        if(line.projection(middlePoint).dist(middlePoint) > 1.0 )
//            if (line.projection(middlePoint).dist(middlePoint) > CHIP_DIS &&
//                    result.ball[0].pos.dist(GlobalData::instance()->maintain[0].ball[0].pos) < 20) {
////                std::cout << "FUCK!!! ball dist=" << result.ball[0].pos.dist(GlobalData::instance()->maintain[0].ball[0].pos) << std::endl;
//                result.ball[0].ball_state_machine.ballState = _chip_pass;
////                result.ball[0].fill(realpos.x(), realpos.y(), realpos.z(), CVector(0, 0)); ///TO TEST!!!
//            } else
//                result.ball[0].ball_state_machine.ballState = _flat_pass;
    }
    ballState::instance()->run(&result);
    GlobalData::instance()->maintain.push(result);
}
