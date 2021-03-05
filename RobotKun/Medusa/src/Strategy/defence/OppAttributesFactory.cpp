#include "OppAttributesFactory.h"
#include "AttributeSet.h"
#include "geometry.h"
#include "staticparams.h"
#include "VisionModule.h"
#include "BestPlayer.h"
#include "ShootRangeList.h"
#include "defence/DefenceInfo.h"
#include "PlayInterface.h"
#include "TaskMediator.h"
#include "WorldModel.h"

namespace{
	//定义这里边所需要的静态变量和常量
	CGeoPoint	OUR_GOAL_CENTER;
	CGeoPoint	THEIR_GOAL_CENTER ;
	CGeoLine OUR_GOAL_LINE ;
	CGeoPoint LEFTGUDGE;
	CGeoPoint RIGHTGUDGE;
    double	OPP_PASS_VEL = 2000;				//判断敌人快速传球的球速
};

//attribute define
DEFINE_CLASS_EX_CPP(ATest);//for test 
DEFINE_CLASS_EX_CPP(ATest2);//for test
DEFINE_CLASS_EX_CPP(ACanBeLeader);
DEFINE_CLASS_EX_CPP(ADist2TheirGoal);//return double 到对方球门中心点的距离
DEFINE_CLASS_EX_CPP(ADist2OurGoal);//return double 到我方球门中心点的距离
DEFINE_CLASS_EX_CPP(ADist2Ball);	//return double	 到球的距离
DEFINE_CLASS_EX_CPP(AValid);//return 1 or 0 对方是否存在
DEFINE_CLASS_EX_CPP(ABestPlayer);//return 1 or 0 是否是对方BestPlayer
DEFINE_CLASS_EX_CPP(AShootRange);//return double 射门角度
DEFINE_CLASS_EX_CPP(AShootRangeWithoutMarker);//return double 将我方Marker排除在外时的射门角度
DEFINE_CLASS_EX_CPP(AFillingInDist);//return double 补防距离
DEFINE_CLASS_EX_CPP(AKeeperDirFactor);//return double 对方BestPlayer朝向线与对方BestPlayer和对方连线朝向之差
DEFINE_CLASS_EX_CPP(AMarkerDistFactor);//return double 和我方Marker的距离，无人盯防时该值返回500（暂定）
DEFINE_CLASS_EX_CPP(AMarkerDirFactor);	//retrun double 对方到门朝向和到我方Marker的朝向之差,无人盯防时该值返回PI（暂定）
DEFINE_CLASS_EX_CPP(AShootReflectAngle);//return double 对方射门折射角
DEFINE_CLASS_EX_CPP(AReceiveReflectAngle);//return double or PI传球过程中接球的receiver的折射角
DEFINE_CLASS_EX_CPP(AImmortalFactor);//return double 球到我门中心向量，与球到该队员向量的夹角,静态意义
DEFINE_CLASS_EX_CPP(APassBlock);//return double传球路线阻挡距离，被我方阻挡的距离 ！！注意：大于200cm时即锁定在200
DEFINE_CLASS_EX_CPP(APosX);//return double 位置的X值
DEFINE_CLASS_EX_CPP(APosY);//return double 位置的Y值
DEFINE_CLASS_EX_CPP(AVelX);//return double 速度的X值
DEFINE_CLASS_EX_CPP(AVelY);//return double 速度的Y值
DEFINE_CLASS_EX_CPP(AGoalie);//return 1 or 0 对方是否是goalie
DEFINE_CLASS_EX_CPP(ARecive);//将逐步弃用！！！
DEFINE_CLASS_EX_CPP(ATouchAbility);// return MAX_VALUE or double
DEFINE_CLASS_EX_CPP(AChaseAbility);// return MAX_VALUE or double 

CAttributeFactory::CAttributeFactory()
{
	_attrSet = new CAttributeSet();
	configuration();
	OUR_GOAL_CENTER = CGeoPoint(-1*PARAM::Field::PITCH_LENGTH/2,0);
	THEIR_GOAL_CENTER = CGeoPoint(PARAM::Field::PITCH_LENGTH/2,0);
	//modified for Brazil by thj
	LEFTGUDGE = CGeoPoint(-PARAM::Field::PITCH_LENGTH/2, -(PARAM::Field::PITCH_WIDTH/2 - PARAM::Vehicle::V2::PLAYER_SIZE*2));
	RIGHTGUDGE = CGeoPoint(-PARAM::Field::PITCH_LENGTH/2, PARAM::Field::PITCH_WIDTH/2 - PARAM::Vehicle::V2::PLAYER_SIZE*2);
	OUR_GOAL_LINE = CGeoLine(CGeoPoint(-PARAM::Field::PITCH_LENGTH/2.0,-PARAM::Field::PITCH_WIDTH/2.0),CGeoPoint(-PARAM::Field::PITCH_LENGTH/2.0,PARAM::Field::PITCH_WIDTH/2.0));
}

CAttributeFactory::~CAttributeFactory()
{
	delete _attrSet;
}

void CAttributeFactory::configuration()
{
    _attrSet->add(ACanBeLeader());
    _attrSet->add(ADist2TheirGoal());
    _attrSet->add(ADist2OurGoal());
    _attrSet->add(ADist2Ball());
    _attrSet->add(AValid());
    _attrSet->add(ABestPlayer());
    _attrSet->add(AShootRange());
    _attrSet->add(AShootRangeWithoutMarker());
    _attrSet->add(AFillingInDist());
    _attrSet->add(AKeeperDirFactor());
    _attrSet->add(AMarkerDistFactor());
    _attrSet->add(AMarkerDirFactor());
    _attrSet->add(APosX());
    _attrSet->add(APosY());
    _attrSet->add(AVelX());
    _attrSet->add(AVelY());
    _attrSet->add(AGoalie());
    _attrSet->add(ARecive());
    _attrSet->add(AShootReflectAngle());
    _attrSet->add(AReceiveReflectAngle());
    _attrSet->add(APassBlock());
    _attrSet->add(AImmortalFactor());
    _attrSet->add(ATouchAbility());
    _attrSet->add(AChaseAbility());
}

//for test
EVALUATE_ATTRIBUTE(ATest)
{
	double tempValue = 0;//按照习惯，tempValue初始化时将给出最差值
    tempValue = pVision->theirPlayer(num).Pos().x();
    setValue(tempValue/10.0);
}

//for test
EVALUATE_ATTRIBUTE(ATest2)
{
    double tempValue = pVision->theirPlayer(num).Pos().y();
    setValue(tempValue/10.0);
}
// MARKMARKTODOTODO
EVALUATE_ATTRIBUTE(ACanBeLeader)
{
	double tempValue = 0;
	if (num == ZSkillUtils::instance()->getTheirBestPlayer())
	{
		tempValue = 1;
		const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
		if (("TheirIndirectKick" == refMsg || "TheirDirectKick" == refMsg)
            && pVision->ball().X() > 2400
            && pVision->ball().X() > pVision->theirPlayer(num).X())
		{
			for (int i = 0;i < PARAM::Field::MAX_PLAYER; i++)
			{
				if (i != num)
				{
                    double othertoBallDir = (pVision->ball().Pos() - pVision->theirPlayer(i).Pos()).dir();
                    double toLeftGudgeDir = (LEFTGUDGE - pVision->theirPlayer(i).Pos()).dir();
                    double toRightGudgeDir = (RIGHTGUDGE - pVision->theirPlayer(i).Pos()).dir();
                    if (pVision->theirPlayer(i).Pos().dist(pVision->ball().Pos())<500
						&& othertoBallDir >= toRightGudgeDir
						&& othertoBallDir <= toLeftGudgeDir)
					{
						tempValue = 0;
					}
				}
			}
		}
	}
	else if (num != ZSkillUtils::instance()->getTheirBestPlayer())
	{
		int best = ZSkillUtils::instance()->getTheirBestPlayer();
		const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
//        double besttoBallDir = (pVision->ball().Pos() - pVision->theirPlayer(best).Pos()).dir();
		if (("TheirIndirectKick" == refMsg || "TheirDirectKick" == refMsg)
            && pVision->theirPlayer(best).X() < pVision->ball().X())
		{
            double metoBallDir = (pVision->ball().Pos() - pVision->theirPlayer(num).Pos()).dir();
            double toLeftGudgeDir = (LEFTGUDGE - pVision->theirPlayer(num).Pos()).dir();
            double toRightGudgeDir = (RIGHTGUDGE - pVision->theirPlayer(num).Pos()).dir();
            if (pVision->theirPlayer(num).Pos().dist(pVision->ball().Pos()) < 500
				&& metoBallDir >= toRightGudgeDir
				&& metoBallDir <= toLeftGudgeDir)
			{
				tempValue = 1;
			}
		}
	}
	setValue(tempValue);
}

//return double		到对方球门中心点的距离
EVALUATE_ATTRIBUTE(ADist2TheirGoal)
{
    double tempValue = pVision->theirPlayer(num).Pos().dist(THEIR_GOAL_CENTER);
    setValue(tempValue/10.0);
}
//return double		到我方球门中心点的距离
EVALUATE_ATTRIBUTE(ADist2OurGoal)
{
    double tempValue = pVision->theirPlayer(num).Pos().dist(OUR_GOAL_CENTER);
    setValue(tempValue/10.0);
}
//return double		到球的距离
EVALUATE_ATTRIBUTE(ADist2Ball)
{
    double tempValue = pVision->theirPlayer(num).Pos().dist(pVision->ball().Pos());
    setValue(tempValue/10.0);
}
//return 1 or 0		对方是否存在
EVALUATE_ATTRIBUTE(AValid)
{
    if (pVision->theirPlayer(num).Valid())
	{
		setValue(1.00);
	} else setValue(0.00);
}
//return 1 or 0		是否是对方BestPlayer
EVALUATE_ATTRIBUTE(ABestPlayer)
{
	if (num == ZSkillUtils::instance()->getTheirBestPlayer())
	{
		setValue(1.00);
	}else setValue(0.00);
}
//return double		射门角度
EVALUATE_ATTRIBUTE(AShootRange)
{
	CShootRangeList shootRangeList(pVision,
													   SR_DEFENCE,
													   0,
                                                       pVision->theirPlayer(num).Pos());
	const CValueRangeList& shootRange = shootRangeList.getShootRange();
	CValueRange* bestRange = NULL;
	double biggestRange = 0;
	if ( shootRange.size() > 0 ){
		bestRange = (CValueRange*)shootRange.getMaxRangeWidth();
		biggestRange = bestRange->getWidth();
	}
	setValue(biggestRange);
}
//return double		将我方Marker排除在外时的射门角度
EVALUATE_ATTRIBUTE(AShootRangeWithoutMarker)
{
	CShootRangeList shootRangeList(pVision,
													   SR_DEFENCE,
													   DefenceInfo::Instance()->getOurMarkDenfender(num),
                                                       pVision->theirPlayer(num).Pos());
	const CValueRangeList& shootRange = shootRangeList.getShootRange();
	CValueRange* bestRange = NULL;
	double biggestRange = 0;
	if ( shootRange.size() > 0 ){
		bestRange = (CValueRange*)shootRange.getMaxRangeWidth();
		biggestRange = bestRange->getWidth();
	}
	setValue(biggestRange);
}
//return double		补防距离
EVALUATE_ATTRIBUTE(AFillingInDist)
{
	const int ourGoalie = TaskMediator::Instance()->goalie();
	const int defender = DefenceInfo::Instance()->getOurMarkDenfender(num);
    const PlayerVisionT& opp = pVision->theirPlayer(num);
    double minDist = 5000;
	double tempDist = 0;
	for (int i = 0;i < PARAM::Field::MAX_PLAYER; i++)
	{
		if (ourGoalie == i || defender == i)
		{
			continue;
		}
        const PlayerVisionT& replacer = pVision->ourPlayer(i);
		double angle_opp2replacer_opp2goal = fabs(Utils::Normalize(CVector(replacer.Pos()-opp.Pos()).dir()-CVector(OUR_GOAL_CENTER-opp.Pos()).dir()));
        if (replacer.X() > opp.X() + 500 || angle_opp2replacer_opp2goal > PARAM::Math::PI * 90 / 180)//X方向被甩来70cm无法补防
		{
			continue;
		} else
		{
			tempDist = replacer.Pos().dist(opp.Pos());
			if (tempDist < minDist)
			{
				minDist = tempDist;
			}
		}
	}
    setValue(minDist/10.0);
}
//return double		对方BestPlayer朝向线与对方BestPlayer和对方连线朝向之差
EVALUATE_ATTRIBUTE(AKeeperDirFactor)
{
	double tempValue = PARAM::Math::PI;
	int keeperNum = ZSkillUtils::instance()->getTheirBestPlayer();
	if (num != keeperNum)
	{
        const PlayerVisionT& theirKeeper = pVision->theirPlayer(keeperNum);
        const PlayerVisionT& opp = pVision->theirPlayer(num);
		tempValue = std::fabs(Utils::Normalize(CVector(opp.Pos() - theirKeeper.Pos()).dir() - theirKeeper.Dir()));
	}
	setValue(tempValue);
}
//return double		和我方Marker的距离，无人盯防时该值返回500（暂定）
EVALUATE_ATTRIBUTE(AMarkerDistFactor)
{
    double tempValue = 5000;
	const int defender = DefenceInfo::Instance()->getOurMarkDenfender(num);
	if (0 != defender)
	{
        tempValue = pVision->theirPlayer(num).Pos().dist(pVision->ourPlayer(defender).Pos());
	} 
    setValue(tempValue/10.0);
}
////retrun double		对方到门朝向和到我方Marker的朝向之差,无人盯防时该值返回PI（暂定）
EVALUATE_ATTRIBUTE(AMarkerDirFactor)
{
	double tempValue = PARAM::Math::PI;
	const int defender = DefenceInfo::Instance()->getOurMarkDenfender(num);
	if (0 != defender)
	{
        const PlayerVisionT& me = pVision->ourPlayer(defender);
        const PlayerVisionT& opp = pVision->theirPlayer(num);
		tempValue = fabs(Utils::Normalize(CVector(me.Pos() - opp.Pos()).dir() - CVector(OUR_GOAL_CENTER - opp.Pos()).dir()));
	} 
	setValue(tempValue);
}

EVALUATE_ATTRIBUTE(AShootReflectAngle)
{
	double tempValue = PARAM::Math::PI;
    CGeoPoint ballPos = pVision->ball().Pos();
    CGeoPoint oppPos = pVision->theirPlayer(num).Pos();
	tempValue = fabs(Utils::Normalize(CVector(ballPos - oppPos).dir() - CVector(OUR_GOAL_CENTER - oppPos).dir()));
	setValue(tempValue);
}

EVALUATE_ATTRIBUTE(AReceiveReflectAngle)
{
	double tempValue = PARAM::Math::PI;
    const MobileVisionT& ball = pVision->ball();
	if (ball.Vel().mod() > OPP_PASS_VEL)
	{
		CGeoPoint rBallVelPos = ball.Pos() + Utils::Polar2Vector(100,Utils::Normalize(ball.Vel().dir()+PARAM::Math::PI));
        CGeoPoint oppPos = pVision->theirPlayer(num).Pos();
		CGeoLine ballLine = CGeoLine(ball.Pos(),rBallVelPos);
		CGeoPoint projectP = ballLine.projection(oppPos);
		tempValue = fabs(Utils::Normalize(CVector(rBallVelPos - ball.Pos()).dir() - CVector(OUR_GOAL_CENTER - projectP).dir()));

    if (fabs(ball.Vel().dir()) > PARAM::Math::PI/3*2 && ball.X() > oppPos.x() + PARAM::Vehicle::V2::PLAYER_FRONT_TO_CENTER) {
      double toProjectDist = oppPos.dist(projectP);
      if (toProjectDist < PARAM::Vehicle::V2::PLAYER_SIZE)
        toProjectDist = PARAM::Vehicle::V2::PLAYER_SIZE;
      toProjectDist /= toProjectDist;
      double oppFaceDir = pVision->theirPlayer(num).Dir();
      tempValue = fabs(Utils::Normalize((rBallVelPos - ball.Pos()).dir() - oppFaceDir) * toProjectDist);
    }
	}
	setValue(tempValue);
}

//posX
EVALUATE_ATTRIBUTE(APosX)
{
    double tempValue = pVision->theirPlayer(num).X();
    setValue(tempValue/10.0);
}
//posY
EVALUATE_ATTRIBUTE(APosY)
{
    double tempValue = pVision->theirPlayer(num).Y();
    setValue(tempValue/10.0);
}
//velX
EVALUATE_ATTRIBUTE(AVelX)
{
    double tempValue = pVision->theirPlayer(num).VelX();
    setValue(tempValue/10.0);
}
//velY
EVALUATE_ATTRIBUTE(AVelY)
{
    double tempValue = pVision->theirPlayer(num).VelY();
    setValue(tempValue/10.0);
}

//is goalie,待修改
EVALUATE_ATTRIBUTE(AGoalie)
{
	double tempValue = 0;
    double dist = 10000;
	int goalieNum = 0;
	for (int i = 0;i < PARAM::Field::MAX_PLAYER;++i)
	{
        double myDist = pVision->theirPlayer(i).Pos().dist(THEIR_GOAL_CENTER);
		if (myDist < dist)
		{
			dist = myDist;
			goalieNum = i;
		}
	}
	//modified for Brazil by thj
    if (num == goalieNum && pVision->theirPlayer(num).Pos().x() > PARAM::Field::PITCH_LENGTH/2 - PARAM::Field::PENALTY_AREA_DEPTH)
	{
		tempValue = 1.0;
	}
	setValue(tempValue);
}
//  以球为基点的球速矢量的方向，与球到我的方向的，角度差
EVALUATE_ATTRIBUTE(ARecive)
{
	
	//for chasekick
	//	{
	//		double angleD = fabs(Utils::Normalize(ballSpeedDir - opp.Vel().dir()));
	//		double ballVel2ourGoal = fabs(Utils::Normalize(ball.Vel().dir() - CVector(OUR_GOAL_CENTER - ballPos).dir()));
	//		if (angleD < PARAM::Math::PI / 2.0 && opp.Vel().mod() >= ball.Vel().mod()
	//			&& ballVel2ourGoal < PARAM::Math::PI / 4.0)
	//		{
	//			tempValue = oppPos.dist(ballPos);
	//		}
	//	}
	//
    //setValue(tempValue);
}

EVALUATE_ATTRIBUTE(APassBlock)
{
    double tempValue = 900;
    CGeoPoint oPos = pVision->theirPlayer(num).Pos();
    CGeoPoint ballPos = pVision->ball().Pos();
	for (int i = 0;i < PARAM::Field::MAX_PLAYER;++i)
	{
		int defender = DefenceInfo::Instance()->getOurMarkDenfender(num);
        if (pVision->ourPlayer(i).Valid() && i != defender)
		{
            CGeoPoint myPos = pVision->ourPlayer(i).Pos();
			double tempAngle1 = fabs(Utils::Normalize(CVector(myPos - oPos).dir() - CVector(ballPos - oPos).dir()));
			double tempAngle2 = fabs(Utils::Normalize(CVector(myPos - ballPos).dir() - CVector(oPos - ballPos).dir()));
            if (tempAngle1 < PARAM::Math::PI * 80 / 180 && tempAngle2 < PARAM::Math::PI * 80 / 180	&& myPos.dist(oPos) < 1000)
			{
				CGeoLine tempLine = CGeoLine(ballPos,oPos);
				double dist = tempLine.projection(myPos).dist(myPos);
				if (dist < tempValue)
				{
					tempValue = dist;
				}
			}
		}
	}
    setValue(tempValue/10.0);
}

EVALUATE_ATTRIBUTE(AImmortalFactor)//用于防范远距离immortal
{
    static CGeoPoint lastBallPos = pVision->ball().Pos();
    static int lastCycle = pVision->getCycle();
	double tempValue = 0;
	const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
	if ("TheirIndirectKick" == refMsg || "TheirDirectKick" == refMsg)
	{
        if (pVision->getCycle() > lastCycle)
		{
            if (lastBallPos.dist(pVision->ball().Pos()) > 30)
			{
                lastBallPos = pVision->ball().Pos();
			}
            lastCycle = pVision->getCycle();
		}
		//进行计算
		if (lastBallPos.x() > PARAM::Field::PITCH_LENGTH / 6)
		{
            CGeoPoint oppPos = pVision->theirPlayer(num).Pos();
            CGeoPoint ballPos = pVision->ball().Pos();
			CVector ball2opp = oppPos - ballPos;
			double angleDiff = fabs(Utils::Normalize(ball2opp.dir() - CVector(OUR_GOAL_CENTER - ballPos).dir()));
			CGeoLine line_ball2goal = CGeoLine(ballPos,OUR_GOAL_CENTER);
			double opp2lineDist = line_ball2goal.projection(oppPos).dist(oppPos);
            if (oppPos.dist(ballPos) < 1800  && angleDiff < PARAM::Math::PI * 0.5)
			{
                tempValue = 10000.0 / ( opp2lineDist/10.0 + 0.1);
				tempValue = tempValue > 400 ? 400 : tempValue;
			}			
		}
	}
	//cout<< " AImmortalFactor    :" << tempValue << endl;
	setValue(tempValue);
}

EVALUATE_ATTRIBUTE(ATouchAbility)
{
    double tempValue = 600;//该属性的值越小，则属性性能越好
    const MobileVisionT& ball = pVision->ball();
    double ballVelMod = pVision->ball().Vel().mod();
	if (ballVelMod > OPP_PASS_VEL)
	{
		CGeoPoint ballPos = ball.Pos();
		double ballSpeedDir = ball.Vel().dir();
        const PlayerVisionT& opp = pVision->theirPlayer(num);
		CGeoPoint oppPos = opp.Pos() + Utils::Polar2Vector(opp.Vel().mod()*0.7,opp.Vel().dir());
		CVector ball2opp = oppPos - ballPos;
		double angleDiff = fabs(Utils::Normalize(ball2opp.dir() - ballSpeedDir));
		if (angleDiff < PARAM::Math::PI * 0.6 && oppPos.x() < PARAM::Field::PITCH_LENGTH / 4)//0.6为可以touch的角度极限
		{
			CGeoLine ballSpeedLine = CGeoLine(ballPos,ballSpeedDir);
			CGeoPoint proPos = ballSpeedLine.projection(oppPos);
            tempValue = proPos.dist(oppPos)/ 10.0 / (proPos.dist(ballPos) / (ballVelMod + 100) + 0.1) + oppPos.dist(ballPos)/10.0 * 0.35;
		}
	}
	setValue(tempValue);
}

EVALUATE_ATTRIBUTE(AChaseAbility)
{
	//自己处理球速数据
    static CGeoPoint logBallPos = pVision->ball().RawPos();
    static int logCycle = pVision->getCycle();
    if (pVision->getCycle() > logCycle)
	{
        logCycle = pVision->getCycle();
        logBallPos = pVision->ball().RawPos();
	}
	//特别针对immortal的chaseKicker
	double tempValue = 0;
    const MobileVisionT& ball = pVision->ball();
    const PlayerVisionT& opp = pVision->theirPlayer(num);
    CVector ballVel = pVision->ball().Pos() - logBallPos;
	double angle_opp2ball_opp2goal = fabs(Utils::Normalize(CVector(ball.Pos() - opp.Pos()).dir()-CVector(OUR_GOAL_CENTER - opp.Pos()).dir()));

    if (ballVel.mod() > OPP_PASS_VEL && ballVel.mod() < 6000)//球速在传球速度和最小射门速度之间
	{
		CGeoLine ballVelLine = CGeoLine(ball.Pos(),ball.Vel().dir());
		CGeoLineLineIntersection interPoint = CGeoLineLineIntersection(ballVelLine,OUR_GOAL_LINE); 
		bool ballVelToOurGoal = false;
		if (interPoint.Intersectant())
		{
			if (interPoint.IntersectPoint().dist(OUR_GOAL_CENTER) < PARAM::Field::PITCH_WIDTH/2.0)//待修改！！
			{
				ballVelToOurGoal = true;
			}
		}
		if (/*angle_ballVel_ball2goal < PARAM::Math::PI * 20 / 180 ||*/ ballVelToOurGoal)//球是朝球门连线走过去的,也可以考虑用球速线与我底线的交点 与 goal点的距离判断
		{
			double angle_oppDir_ball2goal = fabs(Utils::Normalize(opp.Dir()-CVector(OUR_GOAL_CENTER - ball.Pos()).dir()));
			double angle_oppVel_opp2goal = fabs(Utils::Normalize(opp.Vel().dir()-CVector(OUR_GOAL_CENTER - opp.Pos()).dir()));
			if (angle_opp2ball_opp2goal < PARAM::Math::PI * 60 / 180)
			{
				double angle_oppVel_opp2ball = fabs(Utils::Normalize(opp.Vel().dir()-CVector(ball.Pos() - opp.Pos()).dir()));
				//球门、球、对手均处在immortalKick的位置
				if (opp.Vel().mod() >= 15 
					&& angle_oppDir_ball2goal < PARAM::Math::PI * 30 / 180
					&& (angle_oppVel_opp2ball < PARAM::Math::PI * 50 / 180 || angle_oppVel_opp2goal < PARAM::Math::PI * 60 / 180)
					&& opp.Pos().dist(ball.Pos()) < 150)
				{
					//ok
                    tempValue = 800 - opp.Pos().dist(ball.Pos())/10.0;
				}
			}
			else if (angle_opp2ball_opp2goal < PARAM::Math::PI * 150 / 180)//球正传过球身的一刻
			{				
				if (ball.Pos().dist(opp.Pos()) < 35 && angle_oppDir_ball2goal < PARAM::Math::PI * 35 / 180)
				{
					//ok
                    tempValue = 750 - opp.Pos().dist(ball.Pos())/10.0;
				}
			}
			else
			{//相当于刚传球出来的情况,克服掉刚传出来的一瞬间receiver跳票
				//double angle_ballVel_ball2opp = fabs(Utils::Normalize(ball.Vel().dir()-CVector(opp.Pos() - ball.Pos()).dir()));
                if (ball.Pos().dist(opp.Pos()) < 1500 /*&& angle_ballVel_ball2opp < PARAM::Math::PI * 60 / 180*/
					&& angle_oppDir_ball2goal < PARAM::Math::PI * 30 / 180)
				{
					//ok
                    tempValue = 700 - opp.Pos().dist(ball.Pos())/10.0;
				}
			}
		}
	}else if(ballVel.mod() <= OPP_PASS_VEL){
		//日本队的回旋immortalKick: TODO
	}
	//tempValue = 1000 - opp.dist.ball
	setValue(tempValue);
}
