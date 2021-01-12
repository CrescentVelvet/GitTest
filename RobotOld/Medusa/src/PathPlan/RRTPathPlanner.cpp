#include "RRTPathPlanner.h"
#include <cmu/obstacle.h>
#include <cmu/path_planner.h>
#include <cmu/constants.h>
#include <param.h>
#include <utils.h>
#include <GDebugEngine.h>
#include "cmu/vector.h"
#include <math.h>
#include "PlayInterface.h"
#include <TaskMediator.h>

namespace{
    const double TEAMMATE_AVOID_DIST = Param::Vehicle::V2::PLAYER_SIZE /*+ 4.0f*/; // 2014/03/13 修改，为了减少stop的时候卡住的概率 yys
    const double OPP_AVOID_DIST = Param::Vehicle::V2::PLAYER_SIZE /*+ 5.5f*/;
    const double BALL_AVOID_DIST = Param::Field::BALL_SIZE /*+ 5.0f*/;
	//const double VELMIN = 1.0f; //最小速度
	path_planner pathPlanner[Param::Field::MAX_PLAYER * 2];  //ERRT算法实例
}

CRRTPathPlanner::CRRTPathPlanner(const CVisionModule* pVision, const TaskT& task, obstacles& obs, const CGeoPoint& myPos, bool searchInCircle, CGeoPoint circleCenter, double circleRadius) {
	int player = task.executor;
	vector2f playerVel(pVision->ourPlayer(player).VelX(), pVision->ourPlayer(player).VelY());
    // set initial state
    state initial;
    state goal;
    vector<state> result;
    if(Utils::OutOfField(myPos, -40))
        initial.pos = vector2f(pVision->ourPlayer(player).X(), pVision->ourPlayer(player).Y());
    else
        initial.pos = vector2f(myPos.x(), myPos.y());
    goal.pos = vector2f(task.player.pos.x(), task.player.pos.y());

    if(!searchInCircle) {
        pathPlanner[player - 1].init(400, 150, 0.05, 0.55, Param::Field::MAX_PLAYER_SIZE / 2, initial);
        result= pathPlanner[player - 1].plan(&obs, 1, initial, playerVel, goal);
    }
    else{
        double searchStep = 9.0;

        pathPlanner[player - 1 + Param::Field::MAX_PLAYER].init(400, 0, 0.05, 0.0, searchStep, initial, true, vector2f(circleCenter.x(), circleCenter.y()), circleRadius);

        result= pathPlanner[player - 1 + Param::Field::MAX_PLAYER].plan(&obs, 1, initial, playerVel, goal);

    }

    _path.clear();
    for (int i = 1; i < result.size(); i++) {
        _path.push_back(CGeoPoint(result[i].pos.x, result[i].pos.y)); // ?? ?…￥è·ˉ?????1
    }
}
