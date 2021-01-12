#include "ZMarking.h"
#include <skill/Factory.h>
#include "VisionModule.h"
#include "utils.h"
#include "geometry.h"
#include "defence/DefenceInfo.h"
#include "SkillUtils.h"
#include "TaskMediator.h"

CZMarking::CZMarking(){
	_lastCycle = 0;
}

void CZMarking::plan(const CVisionModule* pVision){
    if ( pVision->getCycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 ){
		setState(BEGINNING);
    }

	const MobileVisionT& ball = pVision->ball();
	int vecNumber = task().executor;   //我方小车
	int enemyNum = task().ball.Sender;  //敌方
    int pri = task().ball.receiver;
    int flag = task().player.flag;
    bool stop_mode = flag & PlayerStatus::AVOID_STOP_BALL_CIRCLE;
    bool half_field_mode = flag & PlayerStatus::AVOID_HALF_FIELD;

	const PlayerVisionT& me = pVision->ourPlayer(vecNumber);
	const PlayerVisionT& enemy = pVision->theirPlayer(enemyNum);
	CGeoPoint enemyPos = enemy.Pos();
	CGeoPoint mePos = me.Pos();	
	double me2ballDir = CVector(ball.Pos() - mePos).dir();
	TaskT MarkingTask(task());
    if(stop_mode) MarkingTask.player.flag |= PlayerStatus::AVOID_STOP_BALL_CIRCLE;

    MarkingTask.player.angle = me2ballDir;
    MarkingTask.player.pos = ZSkillUtils::instance()->getZMarkingPos(pVision, vecNumber, enemyNum, pri, flag);
    if(half_field_mode) {
        MarkingTask.player.specify_path_plan_area = true;
//        if(MarkingTask.player.pos.x() > -15) MarkingTask.player.pos.setX(-15);
    }
    if(enemy.Valid()){
        DefenceInfo::Instance()->setMarkList(pVision,vecNumber,enemyNum);
        DefenceInfo::Instance()->setMarkMode(vecNumber,enemyNum,false);
    }

    if(enemy.Valid()){
        GDebugEngine::Instance()->gui_debug_line(enemyPos,MarkingTask.player.pos,COLOR_GRAY);
    }
    MarkingTask.player.flag = (flag | PlayerStatus::ALLOW_DSS);// use DSS add by Wang in 2019.5.29

    CVector enemy2ball = ball.RawPos() - enemy.Pos();
    CVector me2ball = ball.RawPos() - me.RawPos();

    double theta_1 = Utils::Normalize(enemy.Vel().dir() - enemy2ball.dir());//夹角 = 速度线 - 球线
    double enemyVn = enemy.Vel().mod()*sin(theta_1);
    double enemyVt = enemy.Vel().mod()*cos(theta_1);

    double meVn = enemyVn * me2ball.mod() / enemy2ball.mod();
    double meVt = enemyVt * me2ball.mod() / enemy2ball.mod();

    double theta_2 = atan2(meVn, meVt);
    double meVelDir = Utils::Normalize(theta_2 + me2ball.dir());//速度线 = 夹角 + 球线
    double meVelMod = 300*10;//sqrt(meVn*meVn + meVt*meVt);
    if(enemy.Vel().mod() <  50*10) meVelMod = 0;
    if(Utils::InOurPenaltyArea(MarkingTask.player.pos + Utils::Polar2Vector(meVelMod/3, meVelDir), Param::Vehicle::V2::PLAYER_SIZE) || Utils::InTheirPenaltyArea(MarkingTask.player.pos + Utils::Polar2Vector(meVelMod/3, meVelDir), Param::Vehicle::V2::PLAYER_SIZE)) meVelMod = 0;
//    GDebugEngine::Instance()->gui_debug_line(MarkingTask.player.pos + Utils::Polar2Vector(meVelMod/3, meVelDir), MarkingTask.player.pos, COLOR_ORANGE);
    MarkingTask.player.vel = Utils::Polar2Vector(meVelMod, meVelDir);

    setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(MarkingTask));

	_lastCycle = pVision->getCycle();
	CStatedTask::plan(pVision);
}


CPlayerCommand* CZMarking::execute(const CVisionModule* pVision){
	if( subTask() ) return subTask()->execute(pVision);
    return nullptr;
}
