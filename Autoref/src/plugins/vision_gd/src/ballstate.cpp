#include "ballstate.h"
#include "globaldata.h"
namespace  {
//angle
double Normalize(double angle)
{
    if (fabs(angle) > 10 ){
        std::cout<<angle<<" Normalize Error!!!!!!!!!!!!!!!!!!!!"<<std::endl;
        return 0;
    }
    const double M_2PI = PARAM::Math::PI * 2;
    // 快速粗调整
    angle -= (int)(angle / M_2PI) * M_2PI;

    // 细调整 (-PI,PI]
    while( angle > PARAM::Math::PI ) {
        angle -= M_2PI;
    }

    while( angle <= -PARAM::Math::PI ) {
        angle += M_2PI;
    }
    return angle;
}
//dribble
const double DRIBBLE_DIST = PARAM::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER+PARAM::Field::BALL_SIZE;
const double DRIBBLE_ANGLE = PARAM::Math::PI*20/180;
const int MIN_BELIEVE_CYCLE = 5;
//stop
const double MIN_VEL = 50;
//chip
const double MIN_FLY_TIME = 0.3;
const double MAX_FLY_TIME = 1.4;
const double MAX_VELZ_RATIO = 1.5;
const double MIN_FLY_VEL = 2000;
//collision;
const double MIN_TURN_ANGLE = PARAM::Math::PI*3/180;
const double OBVIOUS_TURN_ANGLE = PARAM::Math::PI*30/180;
const double MAX_COLLISION_DIST = PARAM::Vehicle::V2::PLAYER_SIZE+3*PARAM::Field::BALL_SIZE;
const double TOUCH_DIST = PARAM::Vehicle::V2::PLAYER_SIZE+1.5*PARAM::Field::BALL_SIZE;
//analyse
const int DETECT_DIST = 10; //detect whether the ball in 300mm of vechile
const int LENGTH_THERESHOLD = 5800;
const int WIDTH_THERESHOLD = 4400;

}
//---------------dribble--------------------
DribbleJudger::DribbleJudger(){
    clear();
}
//单帧根据判断是否吸球
bool DribbleJudger::carDribble(const Robot& robot,const Ball& ball){
    CVector robot2ball = ball.pos-robot.pos;
    if(robot.pos.dist(ball.pos)<DRIBBLE_DIST&&fabs(Normalize(robot.angle-robot2ball.dir()))<DRIBBLE_ANGLE) return true;
    else return false;
}
//连续图像做一些计数处理减少状态错误跳转
void DribbleJudger::run( const ReceiveVisionMessage* vision){
    pvision = vision;
    for(int team = 0;team<PARAM::TEAMS;team++){
        RobNum[team] = -1;
        for(int i = 0;i<PARAM::ROBOTNUM;i++){
            if (!pvision->robot[team][i].valid) continue;
            if(carDribble(pvision->robot[team][i],pvision->ball[0])){
                dribbleCount[team][i] = dribbleCount[team][i]+1>20?20:dribbleCount[team][i]+1;
                lostCount[team][i] = 0;
            }
            else {
                lostCount[team][i]++;
            }
            //跳出吸球状态：丢球大于3帧或者距离明显错误
            if(lostCount[team][i]>3||pvision->robot[team][i].pos.dist(pvision->ball[0].pos)>DRIBBLE_DIST) dribbleCount[team][i] = 0;//在dribble拉扯时角度判断可能有偏差，做一个计数
            if(dribbleCount[team][i]>0){
                RobNum[team] = i;
                break;
            }

        }
    }
}

void DribbleJudger::clear(){
    for(int team = 0;team<PARAM::TEAMS;team++){
        RobNum[team] = -1;
        for(int i = 0;i<PARAM::ROBOTNUM;i++){
            dribbleCount[team][i] = 0;
            lostCount[team][i] = 0;
        }
    }
}


//---------------ballpath--------------------
BallPathAnalyzer::BallPathAnalyzer():_lastState(State::_stop),_passState(State::_uncertain_pass),_chipConfidence(0),_flatConfidence(0){
    for(int team = 0;team < PARAM::TEAMS;team++){
        for(int i=0;i<PARAM::ROBOTNUM;i++){
            touchCount[team][i]=0;
        }
    }
}

void BallPathAnalyzer::run(const ReceiveVisionMessage* vision){
    _pVision = vision;
    _lastTouch = -1;
    ballPath.push(vision->ball[0]);
    ////////////////////////////////
    //速度过小时跳出
    ////////////////////////////////
    static int stopCount = 0;
    if(ballPath[0].velocity.mod()<MIN_VEL) stopCount++;
    else stopCount = 0;
    if(stopCount>5) {
        _passState = State::_stop;
        _lastTouch = findTouch();
        reset();
        return;
    }
    ////////////////////////////////
    //跳球、平射置信度判断
    ////////////////////////////////
    bool flag = chipsolver.dealresult();
    if(flag&&ballPath.validSize()>=8){
        double flyTime = chipsolver.getFlytime();
        CVector3 kickVel = chipsolver.getkickVel();
        double ratio = fabs(kickVel.z())/CVector(kickVel.x(),kickVel.y()).mod();
        if(flyTime<MAX_FLY_TIME&&flyTime>MIN_FLY_TIME&&ratio<MAX_VELZ_RATIO&&_passState==State::_uncertain_pass){
            _chipConfidence += 2;
        }
        else if((flyTime<MIN_FLY_TIME||vision->ball[0].velocity.mod()<MIN_FLY_VEL)&&_passState==State::_uncertain_pass){
            _flatConfidence += 2;
        }
        else if(_passState==State::_uncertain_pass) _flatConfidence += 1;
    }
    if(_flatConfidence>10&&_passState==State::_uncertain_pass)  _passState = State::_flat_pass;
    if(_chipConfidence>10&&_passState==State::_uncertain_pass) _passState = State::_chip_pass;

    ////////////////////////////////
    //碰撞检测
    ////////////////////////////////
    //resolve uncertain touch
    if(_passState == State::_flat_pass&&_uncertainTouch>=0) _lastTouch = _uncertainTouch;
    if(_passState == State::_chip_pass&&_uncertainTouch>=0) _uncertainTouch = -1;
    //fly end reset
    if(_passState == State::_chip_pass&&GlobalData::instance()->ballrecords.validSize() == 0) reset();
    int touchNum = findTouch();
    //collision detect
    if(ballPath.validSize()>2){
        CVector ballvec1(ballPath[0].pos.x()-ballPath[-1].pos.x(),ballPath[0].pos.y()-ballPath[-1].pos.y());
        CVector ballvec2(ballPath[-1].pos.x()-ballPath[-2].pos.x(),ballPath[-1].pos.y()-ballPath[-2].pos.y());
        if(/*fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>MIN_TURN_ANGLE&&*/_passState == _flat_pass){
            _lastTouch = touchNum;
        }//small line change
        else
        if(fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>OBVIOUS_TURN_ANGLE&&(_passState == _chip_pass||_passState == _uncertain_pass)){
            _lastTouch = touchNum;
        }//obvious line change
        else if(/*fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>MIN_TURN_ANGLE&&*/_passState == _uncertain_pass){
            _uncertainTouch = touchNum;
        }//uncertain
    }
    if (_lastTouch>=0) reset();
}
//根据距离和折射判断是否有车touch
int BallPathAnalyzer::findTouch(){
    auto& ball = ballPath[0];
    CVector ballvec1;
    CVector ballvec2;
    bool judgePath = (ballPath.size()>=3);
    if(judgePath){
        ballvec1 = CVector(ballPath[0].pos.x()-ballPath[-1].pos.x(),ballPath[0].pos.y()-ballPath[-1].pos.y());
        ballvec2 = CVector(ballPath[-1].pos.x()-ballPath[-2].pos.x(),ballPath[-1].pos.y()-ballPath[-2].pos.y());
    }
    double toRobDist = MAX_COLLISION_DIST;
    int robNum = -1;
    for(int team = 0;team < PARAM::TEAMS;team++){
        for(int i=0;i<PARAM::ROBOTNUM;i++){
            auto& rob = _pVision->robot[team][i];
            //CVector rob2ball = ball.pos - rob.pos;
            CVector ball2robot = rob.pos-ball.pos;
            CGeoPoint collisionPos= ballPath[-1].pos;
            if(ball2robot.mod()<TOUCH_DIST) touchCount[team][i]++;
            else touchCount[team][i] = 0;
            if(touchCount[team][i]>4) {
                robNum = team*PARAM::ROBOTNUM+i;
                break;
            }
            if(judgePath){
                if(fabs(Normalize(ball2robot.dir()-ballvec2.dir()))<PARAM::Math::PI/2&&
                        fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>MIN_TURN_ANGLE&&
                        rob.pos.dist(collisionPos)<toRobDist){
                    robNum = team*PARAM::ROBOTNUM+i;
                    toRobDist = rob.pos.dist(ball.pos);
                }
            }
        }
    }
    return robNum;
}




void BallPathAnalyzer::reset(){
    _chipConfidence = 0;
    _flatConfidence = 0;
    _uncertainTouch = -1;
    _passState = State::_uncertain_pass;
    chipsolver.reset();
    predictPoses.clear();
    ballPath.clear();
}


//---------------main--------------------
BallState::BallState():_lastState(State::_stop)
{

}

void BallState::run(ReceiveVisionMessage* vision){
    _pVision = vision;
    bool pathReset = true;
    _lastTouch = -1;
    if(!dribbleJudge()){
            pathReset = false;
            ballPathAnalyze();
    }
    if(pathReset) ballPathAnalyzer.reset();

    if(_lastTouch>=0) GlobalData::instance()->lastTouch = _lastTouch;
    vision->ball[0].ball_state_machine = _lastState;

}

//对dribble信息的处理（结构有点乱，需要再优化）
bool BallState::dribbleJudge(){
    dribbleJudger.run(_pVision);
    bool dribble[PARAM::TEAMS];
    for(int i = 0;i<PARAM::TEAMS;i++){
        dribble[i] = false;
        if(dribbleJudger.RobNum[i]>=0){
            if(
                    (_lastState==State::_chip_pass||_lastState==State::_uncertain_pass)
                    &&dribbleJudger.dribbleCount[i][dribbleJudger.RobNum[i]]<MIN_BELIEVE_CYCLE
                    )
                continue;
            dribble[i] = true;
        }
    }
    if(dribble[PARAM::BLUE]&&dribble[PARAM::YELLOW]){//struggle
        _lastState = State::_struggle;
        if(dribbleJudger.dribbleCount[PARAM::BLUE][dribbleJudger.RobNum[PARAM::BLUE]]<
                dribbleJudger.dribbleCount[PARAM::YELLOW][dribbleJudger.RobNum[PARAM::YELLOW]])
                _lastTouch = dribbleJudger.RobNum[PARAM::YELLOW];
        else    _lastTouch = dribbleJudger.RobNum[PARAM::BLUE];
        return true;
    }
    else if(dribble[PARAM::BLUE]||dribble[PARAM::YELLOW]){//dribble
        _lastState = State::_dribble;
        _lastTouch = dribble[PARAM::BLUE]?dribbleJudger.RobNum[PARAM::BLUE]:dribbleJudger.RobNum[PARAM::YELLOW]+PARAM::Field::MAX_PLAYER;
        return true;
    }
    return false;
}


void BallState::ballPathAnalyze(){
    ballPathAnalyzer.run(_pVision);
    if(ballPathAnalyzer._lastTouch>=0){
        _lastTouch = ballPathAnalyzer._lastTouch;
    }
    _lastState = ballPathAnalyzer._passState;
}

bool BallState::ballIsOnEdge(CGeoPoint ball) {
    if(ball.x() < -LENGTH_THERESHOLD || ball.x() > LENGTH_THERESHOLD ||
            ball.y() < -WIDTH_THERESHOLD || ball.y() > WIDTH_THERESHOLD) return true;
    return false;
}

bool BallState::ballCloseEnough2Analyze() {
    for (int color = 0; color < PARAM::TEAMS; color ++) {
        for (int i = 0; i < PARAM::ROBOTNUM; i++) {
            auto& vision = GlobalData::instance()->maintain[0];
            if (vision.robot[color][i].valid && vision.ball[0].pos.dist(vision.robot[color][i].pos) < DETECT_DIST) {
                return true;
            }
        }
    }
    return false;
}
//char* BallState::getState(int num){
//    if(num == State::stop) return "stop";
//    else if (num==State::dribble) return "dribble";
//    else if (num==State::struggle) return "struggle";
//    else if (num==State::pass_chip) return "pass_chip";
//    else if (num==State::pass_flat) return "pass_flat";
//    else if (num == State::pass_uncertain) return "pass_uncertain";
//    else return "fuck";
//}
