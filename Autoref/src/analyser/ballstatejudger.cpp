#include "ballstatejudger.h"
#include "GDebugEngine.h"
#include "globaldata.h"
namespace  {
//angle
double Normalize(double angle)
{
    if (fabs(angle) > 10 ){
        std::cout<<angle<<" Normalize Error!!!!!!!!!!!!!!!!!!!!"<<std::endl;
        return 0;
    }
    const double M_2PI = Param::Math::PI * 2;
    // 快速粗调整
    angle -= (int)(angle / M_2PI) * M_2PI;

    // 细调整 (-PI,PI]
    while( angle > Param::Math::PI ) {
        angle -= M_2PI;
    }

    while( angle <= -Param::Math::PI ) {
        angle += M_2PI;
    }

    return angle;
}
//dribble
const double DRIBBLE_DIST = Param::Vehicle::V2::PLAYER_CENTER_TO_BALL_CENTER+Param::Field::BALL_SIZE;
const double DRIBBLE_ANGLE = Param::Math::PI*20/180;
const int MIN_BELIEVE_CYCLE = 5;
//stop
const double MIN_VEL = 50;
//chip
const double MIN_FLY_TIME = 0.3;
const double MAX_FLY_TIME = 1.4;
const double MAX_VELZ_RATIO = 1.5;
const double MIN_FLY_VEL = 2000;
//collision;
const double MIN_TURN_ANGLE = Param::Math::PI*5/180;
const double MIN_TURN_ANGLE_FLY = Param::Math::PI*30/180;
const double MAX_COLLISION_DIST = Param::Vehicle::V2::PLAYER_SIZE+3*Param::Field::BALL_SIZE;
}


//---------------dribble--------------------
DribbleJudger::DribbleJudger(){
    clear();
}

bool DribbleJudger::carDribble(const Robot& robot,const Ball& ball){
    CVector robot2ball = ball.pos-robot.pos;
    if(robot.pos.dist(ball.pos)<DRIBBLE_DIST&&fabs(Normalize(robot.angle-robot2ball.dir()))<DRIBBLE_ANGLE) return true;
    else return false;
}

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

            if(lostCount[team][i]>3||pvision->robot[team][i].pos.dist(pvision->ball[0].pos)>DRIBBLE_DIST) dribbleCount[team][i] = 0;//在dribble拉扯时角度判断可能有偏差，做一个计数
            if(dribbleCount[team][i]>0){
                RobNum[team] = i;
                break;
            }

        }
    }
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,4000),QString("dribble:blue:%1,yellow:%2").arg(RobNum[0]).arg(RobNum[1]).toLatin1(),COLOR_RED);
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
//---------------stop--------------------
StopJudger::StopJudger():_count(0){

}


void StopJudger::run(const ReceiveVisionMessage* vision){
    if(vision->ball->velocity.mod()<MIN_VEL) _count ++;
    else _count = 0;
    if(_count > 10) _stop = true;
    else _stop = false;

}



//---------------ballpath--------------------
BallPathAnalyzer::BallPathAnalyzer():_lastState(judgestate::stop),_passState(judgestate::pass_uncertain),_chipConfidence(0),_flatConfidence(0){

}

void BallPathAnalyzer::run(const ReceiveVisionMessage* vision){
    _pVision = vision;
    _lastTouch = -1;
    _lastState = GlobalData::instance()->ballstate;
    ballPath.push(vision->ball[0]);
    if(_lastState == judgestate::stop){
        _lastTouch = findTouchStop();
    }
    bool flag = chipsolver.dealresult();
    if(flag&&ballPath.validSize()>=8){
        double flyTime = chipsolver.getFlytime();
        CVector3 kickVel = chipsolver.getkickVel();
        double ratio = fabs(kickVel.z())/CVector(kickVel.x(),kickVel.y()).mod();
        if(flyTime<MAX_FLY_TIME&&flyTime>MIN_FLY_TIME&&ratio<MAX_VELZ_RATIO&&_passState==judgestate::pass_uncertain){
            _chipConfidence += 2;
        }
        else if((flyTime<MIN_FLY_TIME||vision->ball[0].velocity.mod()<MIN_FLY_VEL)&&_passState==judgestate::pass_uncertain){
            _flatConfidence += 2;
        }
        else if(_passState==judgestate::pass_uncertain) _flatConfidence += 1;
    }
    if(_flatConfidence>10&&_passState==judgestate::pass_uncertain)  _passState = judgestate::pass_flat;
    if(_chipConfidence>10&&_passState==judgestate::pass_uncertain) _passState = judgestate::pass_chip;
    //resolve uncertain touch
    if(_passState == judgestate::pass_flat&&_uncertainTouch>=0) _lastTouch = _uncertainTouch;
    if(_passState == judgestate::pass_chip&&_uncertainTouch>=0) _lastTouch = _uncertainTouch;
    //fly end reset
    if(_passState == judgestate::pass_chip&&GlobalData::instance()->ballrecords.validSize() == 0) reset();
    //collision detect
    if(ballPath.validSize()>2){
        CVector ballvec1(ballPath[0].pos.x()-ballPath[-1].pos.x(),ballPath[0].pos.y()-ballPath[-1].pos.y());
        CVector ballvec2(ballPath[-1].pos.x()-ballPath[-2].pos.x(),ballPath[-1].pos.y()-ballPath[-2].pos.y());
        if(fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>MIN_TURN_ANGLE&&_passState == pass_flat){
            _lastTouch = findTouch();
        }//small line change
        else if(fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>MIN_TURN_ANGLE_FLY&&(_passState == pass_chip||_passState == pass_uncertain)){
            _lastTouch = findTouch();
        }//obvious line change
        else if(fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>MIN_TURN_ANGLE&&_passState == pass_uncertain){
            _uncertainTouch = findTouch();
        }//uncertain
    }
    if (_lastTouch>=0) reset();
    for(int i=0;i<chipsolver.predict.validSize();i++){
        if(i==0){
            GDebugEngine::instance()->gui_debug_x(chipsolver.predict[-i],COLOR_RED);
            GDebugEngine::instance()->gui_debug_arc(chipsolver.predict[-i],60,0,360,COLOR_RED);
        }
        if(i<10)  GDebugEngine::instance()->gui_debug_arc(chipsolver.predict[-i],60,0,360,COLOR_BLUE);
        else   GDebugEngine::instance()->gui_debug_arc(chipsolver.predict[-i],60,0,360,COLOR_GREEN);
    }
     GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,3400),QString("chipConfidence:%1").arg(_chipConfidence).toLatin1(),COLOR_RED);
      GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,3200),QString("flatConfidence:%1").arg(_flatConfidence).toLatin1(),COLOR_RED);
     GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,2800),QString("flag:%1,size:%2").arg(flag).arg(GlobalData::instance()->ballrecords.validSize()).toLatin1(),COLOR_RED);
     GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,2600),QString("uncertainTouch:%1").arg(_uncertainTouch).toLatin1(),COLOR_RED);
}

int BallPathAnalyzer::findTouch(){
    auto& ball = ballPath[0];
    auto& oldBall = ballPath[-2];
    CVector ballvec1(ballPath[0].pos.x()-ballPath[-1].pos.x(),ballPath[0].pos.y()-ballPath[-1].pos.y());
    CVector ballvec2(ballPath[-1].pos.x()-ballPath[-2].pos.x(),ballPath[-1].pos.y()-ballPath[-2].pos.y());
    double toRobDist = MAX_COLLISION_DIST;
    int robNum = -1;

     GDebugEngine::instance()->gui_debug_x(ball.pos,COLOR_CYAN);
    for(int team = 0;team < PARAM::TEAMS;team++){
        for(int i=0;i<PARAM::ROBOTNUM;i++){
            auto& rob = _pVision->robot[team][i];
            CVector rob2ball = ball.pos - rob.pos;
            CGeoPoint collisionPos= ballPath[-1].pos;
            if(fabs(Normalize(rob2ball.dir()-ballvec1.dir()))<Param::Math::PI/2&&
                    rob.pos.dist(collisionPos)<toRobDist){
                if( rob.pos.dist(collisionPos)<toRobDist){
                    GDebugEngine::instance()->gui_debug_x(collisionPos,COLOR_WHITE);
                    GDebugEngine::instance()->gui_debug_msg(rob.pos+CVector(200,0),QString("angle:%1").arg(fabs(Normalize(rob2ball.dir()-ball.velocity.dir()))).toLatin1(),COLOR_RED);
                }
                robNum = team*PARAM::ROBOTNUM+i;
                toRobDist = rob.pos.dist(ball.pos);
            }

        }
    }
    return robNum;
}


int BallPathAnalyzer::findTouchStop(){
    auto& ball = ballPath[0];
    double toRobDist = MAX_COLLISION_DIST;
    int robNum = -1;
    CVector ballvec1(ballPath[0].pos.x()-ballPath[-1].pos.x(),ballPath[0].pos.y()-ballPath[-1].pos.y());
    CGeoPoint collisionPos= ballPath[-1].pos;
    GDebugEngine::instance()->gui_debug_line(ball.pos,ball.pos+ball.velocity,COLOR_CYAN);
    for(int team = 0;team < PARAM::TEAMS;team++){
        for(int i=0;i<PARAM::ROBOTNUM;i++){
            auto& rob = _pVision->robot[team][i];
            CVector rob2ball = ball.pos - rob.pos;
            if(fabs(Normalize(rob2ball.dir()-ballvec1.dir()))<Param::Math::PI/2&&
                    rob.pos.dist(collisionPos)<toRobDist){
                robNum = team*PARAM::ROBOTNUM+i;
                toRobDist = rob.pos.dist(ball.pos);
            }
        }
    }
    return robNum;
}

void BallPathAnalyzer::reset(){
    _chipConfidence = 0;
    _flatConfidence = 0;
    _uncertainTouch = -1;
    _passState = judgestate::pass_uncertain;
    chipsolver.reset();
    predictPoses.clear();
    ballPath.clear();
}




//---------------main--------------------
BallStateJudger::BallStateJudger():_lastState(judgestate::stop)
{

}

void BallStateJudger::run(const ReceiveVisionMessage* vision){
    _pVision = vision;
    bool pathReset = true;
    _lastTouch = -1;
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,3000),QString("vel:%1").arg(vision->ball->velocity.mod()).toLatin1(),COLOR_RED);
    if(!dribbleJudge()){
        if(!stopJudge()){
            pathReset = false;
            ballPathAnalyze();
        }
    }
    if(pathReset) ballPathAnalyzer.reset();
    if(_lastTouch>=0) GlobalData::instance()->lastTouch = _lastTouch;
    GlobalData::instance()->ballstate = _lastState;

}

bool BallStateJudger::dribbleJudge(){
    dribbleJudger.run(_pVision);
    bool dribble[PARAM::TEAMS];
    for(int i = 0;i<PARAM::TEAMS;i++){
        dribble[i] = false;
        if(dribbleJudger.RobNum[i]>=0){
            if(
                    (_lastState==judgestate::pass_chip||_lastState==judgestate::pass_uncertain)
                    &&dribbleJudger.dribbleCount[i][dribbleJudger.RobNum[i]]<MIN_BELIEVE_CYCLE
                    )
                continue;
            dribble[i] = true;
        }
    }
    if(dribble[PARAM::BLUE]&&dribble[PARAM::YELLOW]){//struggle
        _lastState = judgestate::struggle;
        if(dribbleJudger.dribbleCount[PARAM::BLUE][dribbleJudger.RobNum[PARAM::BLUE]]<
                dribbleJudger.dribbleCount[PARAM::YELLOW][dribbleJudger.RobNum[PARAM::YELLOW]])
                _lastTouch = dribbleJudger.RobNum[PARAM::YELLOW];
        else    _lastTouch = dribbleJudger.RobNum[PARAM::BLUE];
        return true;
    }
    else if(dribble[PARAM::BLUE]||dribble[PARAM::YELLOW]){//dribble
        _lastState = judgestate::dribble;
        _lastTouch = dribble[PARAM::BLUE]?dribbleJudger.RobNum[PARAM::BLUE]:dribbleJudger.RobNum[PARAM::YELLOW];
        return true;
    }
    return false;
}

bool BallStateJudger::stopJudge(){
    stopJudger.run(_pVision);
    if(stopJudger._stop){
        _lastState = judgestate::stop;
        return true;
    }
    else return false;
}

void BallStateJudger::ballPathAnalyze(){
    ballPathAnalyzer.run(_pVision);
    if(ballPathAnalyzer._lastTouch>=0){
        _lastTouch = ballPathAnalyzer._lastTouch;
    }
    _lastState = ballPathAnalyzer._passState;
}

char* BallStateJudger::getState(int num){
    if(num == judgestate::stop) return "stop";
    else if (num==judgestate::dribble) return "dribble";
    else if (num==judgestate::struggle) return "struggle";
    else if (num==judgestate::pass_chip) return "pass_chip";
    else if (num==judgestate::pass_flat) return "pass_flat";
    else if (num == judgestate::pass_uncertain) return "pass_uncertain";
    else return "fuck";
}
