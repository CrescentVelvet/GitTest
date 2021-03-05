#include "BallStatus.h"
#include <RobotSensor.h>
#include "SkillUtils.h"
//#include "KickDirection.h"
#include "Global.h"
#include "utils.h"
#include "NormalPlayUtils.h"
#include "BallSpeedModel.h"
#include "parammanager.h"
#include "staticparams.h"
namespace{
	bool isNearPlayer = false;
	bool isEnemyTouch = false;
    bool isOurTouch = false;

	int standOffCouter=0;
	int ourBallCouter=0;


	int lastTheirBestPlayer=0;
	int lastOurBestPlayer=0;
	const int StateMaxNum=5;
	int stateCouter[StateMaxNum]={0,0,0,0,0};

    bool IS_SIMULATION = false;

    const int KICK_OUT_MIN_VELOCITY = 750;
}

CBallStatus::CBallStatus(void):_chipkickstate(false)
{
	_ballMovingVel = CVector(0,0);
	_isKickedOut = false;
	_ballToucher=0;
	_ballState=None;
	_ballStateCouter=0;

	standOffCouter=0;
	ourBallCouter=0;
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION,"Alert/IsSimulation",false);

	initializeCmdStored();
}

void CBallStatus::UpdateBallStatus(const CVisionModule* pVision)
{
	UpdateBallMoving(pVision);
	CheckKickOutBall(pVision);
	_contactChecker.refereeJudge(pVision);
	_lastBallToucher=_ballToucher;
	_ballToucher=_contactChecker.getContactNum();
	if (_isKickedOut||_isChipKickOut){
		_ballToucher=_kickerNum;
	}

}


void CBallStatus::UpdateBallMoving(const CVisionModule* pVision)
{
	const MobileVisionT& ball = pVision->ball(); // 球	
	isNearPlayer = false;
	for (int i=0; i<PARAM::Field::MAX_PLAYER*2; i++){
		if (pVision->allPlayer(i).Valid() && pVision->allPlayer(i).Pos().dist(ball.Pos())< PARAM::Field::MAX_PLAYER_SIZE/2+5){
			isNearPlayer = true;
			break;
		}
	}
    if (false == isNearPlayer && ball.Vel().mod()>20){
		// 只有当球不在车附近时,才使用预测后的球速来更新; 以免球在车附近时,将球速设为0;
		_ballMovingVel = ball.Vel();
	}
	else{
		// 保留球速，方向使用稳定可靠的方向
		double ballspeed = max(1.0, ball.Vel().mod());
		_ballMovingVel = Utils::Polar2Vector(ballspeed, _ballMovingVel.dir());
		//std::cout<<"BallStatus: Ball Near Player"<<endl;
	}
}

// 从PlayInterface中移出的球状态部分
void CBallStatus::initializeCmdStored()
{
	for (int i=0; i<PARAM::Field::MAX_PLAYER; i++){
		for (int j=0; j<MAX_CMD_STORED; j++){
			_kickCmd[i][j].setKickCmd(i,0,0,0);
		}
	}
}

void CBallStatus::setCommand(CSendCmd kickCmd, int cycle)
{
	_kickCmd[kickCmd.num()][cycle % MAX_CMD_STORED] = kickCmd;
}
void CBallStatus::setCommand(int num, int normalKick, int chipKick, unsigned char dribble, int cycle)
{
	_kickCmd[num][cycle % MAX_CMD_STORED].setKickCmd(num, normalKick, chipKick, dribble);
}

void CBallStatus::clearKickCmd() 
{
	for (int i=0; i<PARAM::Field::MAX_PLAYER; i++){
		for (int j=0; j<MAX_CMD_STORED; j++){
			_kickCmd[i][j].clear();
		}
	}
}

void CBallStatus::CheckKickOutBall(const CVisionModule* pVision)
{
	_isKickedOut = false;
	_isChipKickOut = false;
	for (int num=0; num < PARAM::Field::MAX_PLAYER; num++){
        //利用通讯信息
//            bool sensorValid = RobotSensor::Instance()->IsInfoValid(num);
//            bool isBallInFoot = RobotSensor::Instance()->IsInfraredOn(num);
        int isKickDeviceOn = RobotSensor::Instance()->IsKickerOn(num);
//            bool isBallControlled = RobotSensor::Instance()->isBallControled(num);
        /*if(sensorValid){
            cout<<pVision->Cycle()<<"  "<<sensorValid<<"  "<<isBallInFoot<<"  "<<isKickDeviceOn<<" "<<isBallControlled<<endl;
        }*/
        // 用于双向通迅出现问题时，且此时红外正常
        if (isKickDeviceOn > 0) {
            _kickerNum = num;
            _isKickedOut = true;
            if (isKickDeviceOn == IS_CHIP_KICK_ON) {
                _isChipKickOut = true;
            }
            break;
        }
	}
    if ((pVision->allPlayer(_ballToucher).RawPos() - pVision->rawBall().Pos()).mod()<98) {
		_ballChipLine = CGeoLine(pVision->allPlayer(_ballToucher).RawPos(), pVision->allPlayer(_ballToucher).Dir());
	}
}


bool CBallStatus::IsBallKickedOut(int num) const
{
    int isKickDeviceOn = RobotSensor::Instance()->IsKickerOn(num);

    // 用于双向通迅出现问题时，且此时红外正常
    if (isKickDeviceOn > 0) {
        return true;
    }

    return false;
}

bool CBallStatus::IsChipKickedOut(int num) const
{
    int isKickDeviceOn = RobotSensor::Instance()->IsKickerOn(num);

    // 用于双向通迅出现问题时，且此时红外正常
    if (isKickDeviceOn == IS_CHIP_KICK_ON) {
        return true;
    }

    return false;
}

void CBallStatus::clearBallStateCouter(){
	memset(stateCouter,0,StateMaxNum*sizeof(int));
	_ballState=None;
}

void subStateJudge(int i,int* stateCouter,bool* enterCond,int* keepThreshold,int& _ballState){
	if (!enterCond[i]){
		stateCouter[i]--;
		if (stateCouter[i]==0){
			_ballState=None;
			memset(stateCouter,0,StateMaxNum*sizeof(int));
		}
	}else{
		stateCouter[i]++;
		stateCouter[i]=min(stateCouter[i],keepThreshold[i]);
	}
}

string CBallStatus::checkBallState(const CVisionModule* pVision,int meNum){
	const MobileVisionT& ball=pVision->ball();
	const CGeoPoint rawBallPos=pVision->rawBall().Pos();
	int ourAdvancerNum=meNum;
	if (! Utils::PlayerNumValid(ourAdvancerNum)) {
		ourAdvancerNum = ZSkillUtils::instance()->getOurBestPlayer();
		if (! Utils::PlayerNumValid(ourAdvancerNum)) {
			ourAdvancerNum = 1;
		}
	}
	int theirAdvancerNum=ZSkillUtils::instance()->getTheirBestPlayer();
	if (! Utils::PlayerNumValid(theirAdvancerNum)){
		theirAdvancerNum=NormalPlayUtils::getTheirMostClosetoPos(pVision,pVision->ourPlayer(meNum).Pos());
	}


	//cout<<ourAdvancerNum<<" "<<theirAdvancerNum<<endl;
	
	if (!ball.Valid()){
		ourAdvancerNum=lastOurBestPlayer;
		theirAdvancerNum=lastTheirBestPlayer;
	}
	
	PlayerVisionT me=pVision->ourPlayer(ourAdvancerNum);
	PlayerVisionT he=pVision->theirPlayer(theirAdvancerNum);
//	double meSpeed=pVision->ourRawPlayerSpeed(ourAdvancerNum).mod();
    double heSpeed=pVision->getTheirRawPlayerSpeed(theirAdvancerNum).mod();
	double balltoMeDist=rawBallPos.dist(me.Pos());
	double balltoHeDist=rawBallPos.dist(he.Pos());
	double metoHeDist=me.Pos().dist(he.Pos());
//	double metoHeDir=(he.Pos()-me.Pos()).dir();

	const double ballSpeed=ball.Vel().mod();
	const double ballVelDir = ball.Vel().dir();
	const CVector self2ball = rawBallPos - me.Pos();	
	const CVector he2ball=rawBallPos - he.Pos();
	const CVector ball2self = me.Pos() -rawBallPos;
//	const CVector ball2he =he.Pos() - rawBallPos;
	const double dist2ball=self2ball.mod();
	const double distHe2ball=he2ball.mod();

	const double dAngleMeBall2BallVelDir = fabs(Utils::Normalize(ball2self.dir() - ballVelDir));	//球车向量与球速线夹角
    const CGeoSegment ballMovingSeg = CGeoSegment(rawBallPos+Utils::Polar2Vector(100,Utils::Normalize(ballVelDir)),rawBallPos+Utils::Polar2Vector(8000,Utils::Normalize(ballVelDir)));
	const CGeoPoint projMe = ballMovingSeg.projection(me.Pos());					//小车在球移动线上的投影点
	double projDist = projMe.dist(me.Pos());

    const CGeoSegment balltoMeSeg = CGeoSegment(rawBallPos+Utils::Polar2Vector(200,Utils::Normalize(ballVelDir)),rawBallPos+Utils::Polar2Vector(dist2ball+300,Utils::Normalize(ballVelDir)));
	const CGeoPoint projHe = ballMovingSeg.projection(he.Pos());
	double projDistHe =projHe.dist(he.Pos());

	double antiHeDir=Utils::Normalize(he.Dir()+PARAM::Math::PI);
//	const double shootDir=KickDirection::Instance()->getRealKickDir();
	CGeoPoint predictBallPos=BallSpeedModel::Instance()->posForTime(30,pVision);

	const double metoBallAngle=(rawBallPos-me.Pos()).dir();
	const double antiMetoBallAngle=(me.Pos()-rawBallPos).dir();
    const CGeoSegment metoBallSeg=CGeoSegment(me.Pos()+Utils::Polar2Vector(100,antiMetoBallAngle),me.Pos()+Utils::Polar2Vector(1000,metoBallAngle));
	const CGeoPoint projHeInMetoBallSeg=metoBallSeg.projection(he.Pos());
	const CGeoPoint meHead=me.Pos()+Utils::Polar2Vector(PARAM::Vehicle::V2::PLAYER_FRONT_TO_CENTER,me.Dir());
	//isOurBall
	double diffAngleMeToBall2Me=fabs(Utils::Normalize(me.Dir()-metoBallAngle));
	double allowInfrontAngleBuffer = (dist2ball/(PARAM::Vehicle::V2::PLAYER_SIZE))*PARAM::Vehicle::V2::KICK_ANGLE < PARAM::Math::PI/5.0?
		(dist2ball/(PARAM::Vehicle::V2::PLAYER_SIZE))*PARAM::Vehicle::V2::KICK_ANGLE:PARAM::Math::PI/5.0;
	double allowInfrontAngleBufferHe = (distHe2ball/(PARAM::Vehicle::V2::PLAYER_SIZE))*PARAM::Vehicle::V2::KICK_ANGLE < PARAM::Math::PI/5.0?
		(distHe2ball/(PARAM::Vehicle::V2::PLAYER_SIZE))*PARAM::Vehicle::V2::KICK_ANGLE:PARAM::Math::PI/5.0;

	bool isBallInFront = fabs(Utils::Normalize(self2ball.dir()-me.Dir())) < allowInfrontAngleBuffer
		&& dist2ball < (2.5*PARAM::Vehicle::V2::PLAYER_SIZE + PARAM::Field::BALL_SIZE);
	bool isBallInHeFront =fabs(Utils::Normalize(he2ball.dir()-he.Dir())) < allowInfrontAngleBufferHe
		&& distHe2ball < (dist2ball+PARAM::Vehicle::V2::PLAYER_SIZE + PARAM::Field::BALL_SIZE);

    bool isBallJustInFront=ball.Vel().mod()<100 && fabs(Utils::Normalize(self2ball.dir()-me.Dir()))<PARAM::Math::PI/15
        && meHead.dist(rawBallPos)<80 && fabs(me.Dir())<PARAM::Math::PI/3;

//	bool isHeInMeFront = fabs(Utils::Normalize(metoHeDir-me.Dir())) < PARAM::Math::PI/3
//		&& metoHeDist <dist2ball+4*PARAM::Vehicle::V2::PLAYER_SIZE ;

    bool isBallMovtoMe=ballSpeed<4000&&ballSpeed>1200&&(dAngleMeBall2BallVelDir<PARAM::Math::PI/4||projDist<500)
        &&ballMovingSeg.IsPointOnLineOnSegment(projMe)&&projMe.dist(rawBallPos)>300
        &&NormalPlayUtils::noEnemyInPassLine(pVision,rawBallPos,me.Pos(),200);

//	bool isBallMovtoHe=balltoMeSeg.IsPointOnLineOnSegment(projHe)&&projDistHe<projDist+35&&Utils::Normalize(antiHeDir-ball.Vel().dir())<PARAM::Math::PI/3;


	//standOff
    bool isHeFrontToMe=fabs(Utils::Normalize(me.Dir()-antiHeDir))<PARAM::Math::PI/3&&me.Pos().dist(he.Pos())<400;
    bool isMeHeStandOff=isHeFrontToMe&&me.Pos().dist(he.Pos())<350;
    bool isStandOff=(ball.Valid()&&ballSpeed<500&&isBallInFront&&self2ball.mod()<150||!ball.Valid())&&isMeHeStandOff
        ||(me.Pos().dist(he.Pos())<305&&ballSpeed<300&&(metoBallSeg.IsPointOnLineOnSegment(projHeInMetoBallSeg)||diffAngleMeToBall2Me>PARAM::Math::PI/2));
    int judgeRange=300;
    if (dist2ball>1500){
        judgeRange=500;
    }else if (dist2ball>800){
        judgeRange=400;
    }else if (dist2ball>300){
        judgeRange=300;
	}else{
        judgeRange=250;
	}
	

	bool canGiveUpAdvance=getBallToucher()>PARAM::Field::MAX_PLAYER
        &&ball.Vel().mod()>800&&!Utils::IsInField(predictBallPos,-100)
		&&fabs(ball.Vel().dir())>PARAM::Math::PI*80/180
        &&he.Pos().dist(rawBallPos)>me.Pos().dist(rawBallPos)+300
        &&!NormalPlayUtils::isEnemyFrontToBall(pVision,300)
		&&!NormalPlayUtils::ballMoveToOurDefendArea(pVision)
		//&&me.Pos().dist(rawBallPos)/200>0.5
		;

	
	double heSpeedInBallVel=heSpeed*cos(Utils::Normalize(he.Vel().dir()-ball.Vel().dir()))*1.5;
    bool notHeGetBallBefore=projDistHe>projDist+350||Utils::Normalize(he2ball.dir()-he.Dir())>PARAM::Math::PI/2;
	if (balltoMeSeg.IsPointOnLineOnSegment(projHe)){
		notHeGetBallBefore=notHeGetBallBefore||projDist/ballSpeed<projDistHe/(ballSpeed+heSpeedInBallVel);
	}else if(!ballMovingSeg.IsPointOnLineOnSegment(projHe)){
		notHeGetBallBefore=notHeGetBallBefore||projDist/ballSpeed<projDistHe/(ballSpeed-heSpeedInBallVel);
	}else{
		notHeGetBallBefore=true;
	}

	int closedHeNum=NormalPlayUtils::getTheirMostClosetoPos(pVision,ball.Pos());
	CGeoPoint theirClosedCar=pVision->theirPlayer(closedHeNum).Pos();
	bool isOurBall=ball.Valid()&&
		(isBallMovtoMe && notHeGetBallBefore
			||isBallJustInFront&&!isHeFrontToMe
			||(!canGiveUpAdvance&&!isBallInHeFront
            &&(balltoHeDist-balltoMeDist>judgeRange || balltoHeDist>balltoMeDist+150 && isBallInFront)
            &&ball.Pos().dist(theirClosedCar)>balltoMeDist+150
			)
		);
	//cout<<isBallMovtoMe<<" "<<isBallJustInFront<<" "<<isHeFrontToMe<<" "<<isBallInHeFront<<endl;
	//waitTouch
    bool canWaitAdvance=ballSpeed>3500&&fabs(ballVelDir)<PARAM::Math::PI/3;
	canWaitAdvance=false;
	//giveUpAdvance


	//cout<<"canGive"<<" "<<canGiveUpAdvance<<endl;


	//memset(stateCouter,0,StateMaxNum*sizeof(int));
	int jumpThreshold[StateMaxNum];
	jumpThreshold[None]=0;
	jumpThreshold[OurBall]=5;
	jumpThreshold[StandOff]=15;
	jumpThreshold[WaitAdvance]=5;
	jumpThreshold[GiveUpAdvance]=5;

	int keepThreshold[StateMaxNum];
	keepThreshold[None]=0;
	keepThreshold[OurBall]=8;
	keepThreshold[StandOff]=45;
	keepThreshold[WaitAdvance]=5;
	keepThreshold[GiveUpAdvance]=10;

	bool enterCond[StateMaxNum];
	enterCond[None]=false;
	enterCond[OurBall]=isOurBall;
	enterCond[StandOff]=isStandOff&&!isOurBall;
	enterCond[WaitAdvance]=canWaitAdvance;
	enterCond[GiveUpAdvance]=canGiveUpAdvance;
	switch (_ballState)
	{
	case None:
		//if (IsBallKickedOut()&&fabs(Utils::Normalize(me.Dir()-shootDir))<PARAM::Math::PI/10){
		//	memset(stateCouter,0,StateMaxNum*sizeof(int));
		//	stateCouter[WaitAdvance]=keepThreshold[WaitAdvance];
		//	_ballState=WaitAdvance;
		//	break;
		//}
		for (int i=1;i<StateMaxNum;i++){
			if (enterCond[i]){
				stateCouter[i]++;
				if (stateCouter[i]>=jumpThreshold[i]){
					memset(stateCouter,0,StateMaxNum*sizeof(int));
					stateCouter[i]=keepThreshold[i];
					_ballState=i;
					break;
				}
			}else{
				stateCouter[i]--;
				stateCouter[i]=max(stateCouter[i],0);
			}
		}
		break;
	case OurBall:
		//if (IsBallKickedOut()&&fabs(Utils::Normalize(me.Dir()-shootDir))<PARAM::Math::PI/10){
		//	memset(stateCouter,0,StateMaxNum*sizeof(int));
		//	stateCouter[WaitAdvance]=keepThreshold[WaitAdvance];
		//	_ballState=WaitAdvance;
		//	break;
		//}
		subStateJudge(OurBall,stateCouter,enterCond,keepThreshold,_ballState);
		break;
	case StandOff:
		if (world->IsBestPlayerChanged()/*||IsBallKickedOut()*/){
			_ballState=None;
			memset(stateCouter,0,StateMaxNum*sizeof(int));
			break;
		}
		subStateJudge(StandOff,stateCouter,enterCond,keepThreshold,_ballState);
		break;
	case WaitAdvance:
		if (ballToucherChanged()){
			_ballState=None;
			memset(stateCouter,0,StateMaxNum*sizeof(int));
			break;
		}
		subStateJudge(WaitAdvance,stateCouter,enterCond,keepThreshold,_ballState);
		break;
	case GiveUpAdvance:
		subStateJudge(GiveUpAdvance,stateCouter,enterCond,keepThreshold,_ballState);
		break;
	}
	if (ball.Valid()){
		lastTheirBestPlayer=ZSkillUtils::instance()->getTheirBestPlayer();
		lastOurBestPlayer=ZSkillUtils::instance()->getOurBestPlayer();
	}
	switch(_ballState){
	case OurBall:
		return "OurBall";
	case None:
		return "None";
	case StandOff:
		return "StandOff";
	case WaitAdvance:
		return "WaitAdvance";
	case GiveUpAdvance:
		return "GiveUpAdvance";
	}
}
