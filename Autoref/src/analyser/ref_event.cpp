#include "ref_event.h"
#include "messageformat.h"
#include "geometry.h"
#include "referee.h"
#include "Semaphore.h"
#include <thread>
#include "param.h"
#include "GDebugEngine.h"

Semaphore* getVision[SEMAPHORE_NUM];
Semaphore Kick(0);
Semaphore Touch(0);
Semaphore Own(0);

std::thread* judgeOutT = nullptr;
std::thread* ballOwnT = nullptr;
std::thread* collisionT = nullptr;
std::thread* overSpeedT = nullptr;
std::thread* lastTouchT = nullptr;
std::thread* dribbleT = nullptr;
std::thread* pushT = nullptr;
std::thread* penaltyT = nullptr;
std::thread* kickT = nullptr;
std::thread* passT = nullptr;
std::thread* chipT = nullptr;

ref_event::ref_event() /*: _state(nor),_passMsg(pass_other),_receive_msg(no_receive)*/
{

    for(int i=0 ; i<SEMAPHORE_NUM ; i++){
        getVision[i] = new Semaphore(0);
    }
//
    chipT = new std::thread([=] {ballPathAnalyse();});
    chipT->detach();
//
    judgeOutT = new std::thread([=] {judgeOutEvent(_refVision[0].ball[0].pos);});
    judgeOutT->detach();
//  out of field
    ballOwnT = new std::thread([=] {ballOwnDetect();});
    ballOwnT->detach();
//  ballOwnDetect();
    collisionT = new std::thread([=] {collisionID();});
    collisionT->detach();
//  collisionID();
    overSpeedT = new std::thread([=] {ballOverSpeed();});
    overSpeedT->detach();
//  ballOverSpeed();
    lastTouchT = new std::thread([=] {lastTouchDetect();});
    lastTouchT->detach();
//  lastTouchDetect();
    dribbleT = new std::thread([=] {dribbleDetect();});
    dribbleT->detach();
//  dribbleDetect();
    pushT = new std::thread([=] {pushDetect();});
    pushT->detach();
//  pushDetect();
    penaltyT = new std::thread([=] {penaltyAttackerDetect();});
    penaltyT->detach();
//
    kickT = new std::thread([=] {getBotState();});
    kickT->detach();
//
    passT = new std::thread([=] {passDetect();});
    passT->detach();

}

ref_event::~ref_event(){
}

void ref_event::run(){
    receiveRefMsg();

    if(_lastcommad != _gameCommand){
        _timer = 0;
    }
    _lastcommad = _gameCommand;

    switch (_gameCommand) {
    case 16: //ballplacement
    case 17:
        ballPlaceTimeout();
        break;
    case 8: //directkick indirectkick
    case 9:
    case 10:
    case 11:
        directKickOFT();
        break;
    case 6: //penalty kick
    case 7:
        penaltyKickOFT();
        break;
    default:
        break;
    }
//    std::cout << "game command : "<< _gameCommand <<std::endl;
//    std::cout << "timer : " <<_timer<<std::endl;
//    std::cout << "state    : " << _state <<std::endl;
//    std::cout << "ball owns  : " <<_ownmsg <<std::endl;
//    std::cout << "blue own ID : " <<_ownID[PARAM::BLUE]<<std::endl;
//    std::cout << "yellow own ID : " <<_ownID[PARAM::YELLOW]<<std::endl;
//    std::cout << "receive  : " <<_receive_msg<<std::endl;
//    std::cout << "pass msg   :" << _passMsg << std::endl;
//    std::cout << "dribble    : " <<_dribblemsg << std::endl;
//    std::cout << "robot is kick  : " <<_robotState <<std::endl;
//    std::cout << "kick id: " <<_lastKick_ID<<std::endl;
//    std::cout << "last touch ID : " << _lastTouch_ID <<std::endl;
//    std::cout << "chip state : " << _passState <<std::endl;
//    std::cout << "ballPath : " <<ballPath.validSize()<<std::endl;
//    std::cout << "/////////////////////////////" <<std::endl;
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,4000),QString("_state:%1").arg(_state).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,3800),QString("ball owns:%1").arg(_ownmsg).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,3600),QString("blue own ID:%1").arg(_ownID[PARAM::BLUE]).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,3400),QString("yellow own ID:%1").arg(_ownID[PARAM::YELLOW]).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,3200),QString("receive:%1").arg(_receive_msg).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,3000),QString("pass msg:%1").arg(_passMsg).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,2800),QString("dribble:%1").arg(_dribblemsg).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,2600),QString("robot is kick:%1").arg(_robotState).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,2400),QString("kick id:%1").arg(_lastKick_ID).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,2200),QString("last touch ID:%1").arg(_lastTouch_ID).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,2000),QString("chip state:%1").arg(_passState).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,1800),QString("ballPath:%1").arg(ballPath.validSize()).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,1600),QString("_flatConfidence:%1").arg(_flatConfidence).toLatin1(), 1 );
    GDebugEngine::instance()->gui_debug_msg(CGeoPoint(4000,1400),QString("_chipConfidence:%1").arg(_chipConfidence).toLatin1(), 1 );

    for(int i=0;i<ChipSolver::instance()->predict.validSize();i++){
        if(i==0){
            GDebugEngine::instance()->gui_debug_x(ChipSolver::instance()->predict[-i],1);
            GDebugEngine::instance()->gui_debug_arc(ChipSolver::instance()->predict[-i],60,0,360,1);
        }
        if(i<10)  GDebugEngine::instance()->gui_debug_arc(ChipSolver::instance()->predict[-i],60,0,360,1);
        else   GDebugEngine::instance()->gui_debug_arc(ChipSolver::instance()->predict[-i],60,0,360,4);
    }
    GDebugEngine::instance()->send(true);
}
///////////////////////////////////////////////////////////////
void ref_event::receiveVision(DataQueue<ReceiveVisionMessage>& vm){
    robotReset();
    for(int i= -MAX_FRAME+1 ;i<=0 ; i++ ){
        _refVision[-i] = vm[i];
        for(int color = 0; color<PARAM::TEAMS ;color++){
            for(int j = 0; j<vm[i].robotSize[color]; j++){
                _refVision[-i].robot[color][vm[i].robot[color][j].id].valid = true;
                _refVision[-i].robot[color][vm[i].robot[color][j].id].fill(vm[i].robot[color][j]);
            }
        }  // for now
    }
    for(int i=0 ; i<SEMAPHORE_NUM ; i++){
        getVision[i]->Signal();
    }
}

///////////////////////////////////////////////////////////////

void ref_event::ballOwnDetect(){//todo ：ball own 和chip之间可能存在的问题，时序太烦了，等这边时序的框架有了再说吧，不想在这里合进来了 --lzx
    while (true) {
        getVision[0]->Wait();
        bool ifOwn = false;
        for(int i = 0 ; i < PARAM::ROBOTNUM ; i++){
            if(ballowns(_refVision[0].robot[PARAM::BLUE][i],0)){
                        ifOwn = true;
                        _ownmsg = blue_own;
            }

            if(ballowns(_refVision[0].robot[PARAM::BLUE][i],0)){
                        ifOwn = true;
                        _ownmsg = yellow_own;
            }
            if(!ifOwn) {
                _ownmsg = other;
            }
        }
        Own.Signal();
    }
}

bool ref_event::ballowns(const Robot& bot , int frame){
    if(bot.valid){
        CVector robot2ball = _refVision[frame].ball[0].pos - bot.pos;
        if(robot2ball.mod() < OWN_DIST && fabs(robot2ball.dir() - bot.angle) < OWN_DIR){
            return true;
        }
    }
    return false;
}


///////////////////////////////////////////////////////////////

void ref_event::robotReset(){
    for(int i = -MAX_FRAME + 1 ; i <=0 ; i++ ){
        for(int j = 0 ; j < PARAM::ROBOTNUM ; j++){
            _refVision[-i].robot[PARAM::BLUE][j].valid = false;
            _refVision[-i].robot[PARAM::YELLOW][j].valid = false;
        }
    }
}

////////////////////////////////////////////////////////////////////

int ref_event::getState(){
    return _state;
}

int ref_event::getRobotState(){
    return _robotState;
}

int ref_event::lastTouchID(){
    return _lastTouch_ID;
}

///////////////////////////////////////////////////////////////
bool ref_event::inArea(const CGeoPoint& _p, const CGeoPoint &center, double radius){
    double dis = center.dist(_p);
    if(dis > radius){return true ;}
    else{return false ;}
}

bool ref_event::inArea(const CGeoPoint& _p, double x1, double x2, double y1, double y2){
    if(_p.x() > x1 && _p.x() < x2
            && _p.y() > y1 && _p.y() < y2){
        return true;
    }else{
        return false;
    }
}

///////////////////////////////////////////////////////////////
void ref_event::judgeOutEvent(const CGeoPoint& ball_pos){
    while(true){
        getVision[6]->Wait();
        if(inArea(ball_pos, Param::Field::PITCH_LENGTH/2, Param::Field::PITCH_LENGTH/2+Param::Field::GOAL_DEPTH, -Param::Field::GOAL_WIDTH/2, Param::Field::GOAL_WIDTH/2)
                || inArea(ball_pos,-Param::Field::PITCH_LENGTH/2-Param::Field::GOAL_DEPTH,  -Param::Field::PITCH_LENGTH/2 , -Param::Field::GOAL_WIDTH/2, Param::Field::GOAL_WIDTH/2)){
            _state = goal;
        }else if(!inArea(ball_pos, -Param::Field::PITCH_LENGTH/2, Param::Field::PITCH_LENGTH/2, -Param::Field::PITCH_WIDTH/2, Param::Field::PITCH_WIDTH/2)){
            _state = out_of_field;
        }
    }
}

////////////////////////////////////////////////////////////////

void ref_event::collisionID(){
    while(true){
        getVision[1]->Wait();
        for(int i = 0 ; i < PARAM::ROBOTNUM ; i++){
            for(int j = 0 ; j < PARAM::ROBOTNUM ; j++){
                if(_refVision[0].robot[PARAM::BLUE][i].valid && _refVision[0].robot[PARAM::YELLOW][j].valid
                        && _refVision[0].robot[PARAM::BLUE][i].pos.dist(_refVision[0].robot[PARAM::YELLOW][j].pos) < MIN_DIST){
                    collsionState(i,j);
                }
            }
        }
    }
}

void ref_event::collsionState(int id_blue, int id_yellow){
    if(collisionDetect( _refVision[0].robot[PARAM::BLUE][id_blue].pos, _refVision[0].robot[PARAM::YELLOW][id_yellow].pos ,_refVision[0].robot[PARAM::BLUE][id_blue].velocity  ,_refVision[0].robot[PARAM::YELLOW][id_yellow].velocity )){
        switch(_refVision[0].robot[PARAM::BLUE][id_blue].velocity.mod() > _refVision[0].robot[PARAM::YELLOW][id_blue].velocity.mod() ? PARAM::BLUE :PARAM::YELLOW ){
        case PARAM::BLUE :
            _state = blue_collision;
            break;
        case PARAM::YELLOW :
            _state = yellow_collision;
            break;
        }
    }
}

bool ref_event::collisionDetect(const CGeoPoint& pos_1, const CGeoPoint& pos_2, CVector& vel_1, CVector& vel_2 ){
    CVector delta_Vel = vel_1-vel_2;
    double angle_blue = fabs(delta_Vel.dir()-vel_1.dir());
    double angle_yellow = fabs(delta_Vel.dir() - vel_2.dir());
    if(pos_1.dist(pos_2) < MIN_DIST
            && (delta_Vel.mod() * std::cos(angle_blue) > MIN_VEL_COLLISION
                || delta_Vel.mod() * std::cos(angle_yellow) > MIN_VEL_COLLISION)){
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////

void ref_event::ballOverSpeed(){
    while(true){
        getVision[2]->Wait();

        int count = 0 ;
        for(int i = BALL_SPEED_JUDGE_FRAME ; i >= 0 ; i--){
            if(_refVision[i].ball[0].velocity.mod() > MAX_VEL){
                count++;
            }
        }
        if(count>BALL_SPEED_CONFIRM_FRAME){
            _state = overspeed;
        }
    }
}

///////////////////////////////////////////////////////////////////////////

void ref_event::getBotState(){
    while(true){
        getVision[8]->Wait();
//        qDebug()<< Q_FUNC_INFO;
        for(int i = 0 ; i < PARAM::ROBOTNUM ; i++){
            if(kickDetect(_refVision[0].robot[PARAM::BLUE][i],0)
                    && _lastTouch_ID < PARAM::ROBOTNUM){
                _lastKick_ID = i;
                _robotState = blue_kick_ball;
            }
            if(kickDetect(_refVision[0].robot[PARAM::YELLOW][i],0)
                    && _lastTouch_ID >= PARAM::ROBOTNUM ){
                _lastKick_ID = i + PARAM::ROBOTNUM;
                _robotState = yellow_kick_ball;
            }
        }
        receiveDetet();
        Kick.Signal();
    }
}


bool ref_event::kickDetect(Robot& robot,int frame){
    if(robot.valid){
        CVector robot2ball = _refVision[frame].ball[0].pos - robot.pos;
        if( robot.pos.dist(_refVision[frame].ball[0].pos) < KICK_CIRCLE_UPLIMIT && robot.pos.dist(_refVision[frame].ball[0].pos) > KICK_CIRCLE_LOWLIMIT
                && fabs((robot2ball.dir() - robot.angle)) < Param::Math::PI / 12
                && _refVision[frame].ball[0].velocity.mod() > MIN_BALL_KICK_VEL
                && fabs(robot.angle - _refVision[frame].ball[0].velocity.dir()) < Param::Math::PI / 36)
        {
            return true;
        }
    }
    return false;
}
////////////////////////////////////////////////////////////////////////

void ref_event::lastTouchDetect(){
    while(true){
        getVision[3]->Wait();
        int touch_ID = -1;
        CGeoPoint touch_POS(9999,9999);
        for (int i=0; i<PARAM::ROBOTNUM; i++){
            for (int frame=4;frame>-1;frame--)
            {
                if (_refVision[frame].robot[PARAM::BLUE][i].valid
                        && _refVision[frame].robot[PARAM::BLUE][i].pos.dist(_refVision[frame].ball[0].pos)>=LASTTOUCH_LOWLIMIT
                        && _refVision[frame].robot[PARAM::BLUE][i].pos.dist(_refVision[frame].ball[0].pos)<=LASTTOUCH_UPLIMIT)
                {
                    touch_POS= _refVision[frame].robot[PARAM::BLUE][i].pos;
                    touch_ID = i;
                }
                if (_refVision[frame].robot[PARAM::YELLOW][i].valid
                        && _refVision[frame].robot[PARAM::YELLOW][i].pos.dist(_refVision[frame].ball[0].pos)>=LASTTOUCH_LOWLIMIT
                        && _refVision[frame].robot[PARAM::YELLOW][i].pos.dist(_refVision[frame].ball[0].pos)<=LASTTOUCH_UPLIMIT)
                {
                    touch_POS =_refVision[frame].robot[PARAM::YELLOW][i].pos;
                    touch_ID = i+PARAM::ROBOTNUM;
                }
            }
        }
        if(touch_ID>=0){
            _uncertainTouch_ID = touch_ID;
            _uncertainTouchPos = touch_POS;
        }
        Touch.Signal();
    }
}

//////////////////////////////////////////////////////////////////////////////////

void ref_event::passDetect(){
    while(true){
        Kick.Wait();
        if(_lastKick_ID == _lastTouch_ID
                && _robotState == blue_kick_ball
                && _receive_msg == blue_r){
            _passMsg = pass_blue;
        }else if(_lastKick_ID == _lastTouch_ID
                 && _robotState == yellow_kick_ball
                 && _receive_msg == yellow_r){
            _passMsg = pass_yellow;
        }else {
            _passMsg = pass_other;
        }
    }
}

void ref_event::receiveDetet(){
    bool ifReceive = false;
    for(int i = 0 ; i< PARAM::ROBOTNUM ;i++){
        if(ifRecive(_refVision[0].robot[PARAM::BLUE][i],0)){
                ifReceive = true;
                _receive_msg = blue_r;
        }
        if(ifRecive(_refVision[0].robot[PARAM::YELLOW][i],0)){
                ifReceive = true;
                _receive_msg = yellow_r;
        }
    }
//    if(ifReceive){
//        _receive_msg = no_receive;
//    }
}

bool ref_event::ifRecive(Robot& bot , int frame){
    CVector ball2robot = bot.pos - _refVision[frame].ball[0].pos;
    if(bot.valid && bot.pos.dist(_refVision[frame].ball[0].pos) < RECEIVED_DIST){
        if(fabs(_refVision[frame].ball[0].velocity.dir()-ball2robot.dir()) < Param::Math::PI/6
                && _refVision[frame].ball[0].velocity.mod() > 10){
            return true;
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////
//选边问题
//void ref_event::aimlessDetect(){
//    if(_lastTouch_ID == _lastKick_ID_blue && ball[0].pos.x()>Param::Field::PITCH_LENGTH/2
//            && _lastTouchPos.x() <0){
////        std::cout << "in" <<std::endl;
//        _aimlessmsg = blue_aimless;
//    }else if(_lastTouch_ID == _lastKick_ID_yellow && ball[0].pos.x()< -Param::Field::PITCH_LENGTH/2
//            && _lastTouchPos.x() >0){
//        _aimlessmsg = yellow_aimless;
//    }else{
//        _aimlessmsg = no_aimless;
//    }

//}

/////////////////////////////////////////////////////////////////////////////////

void ref_event::lastOwnPosDetect(){
    for(int i=0 ; i<PARAM::TEAMS ; i++){
        for(int j=0 ; j<PARAM::ROBOTNUM ; j++){
            if(ballowns(_refVision[0].robot[i][j],0) && !ballowns(_refVision[1].robot[i][j],1)){
                _ownPos[i] = _refVision[0].robot[i][j].pos;
                _ownID[i]= j;
            }
        }
    }
}

void ref_event::dribbleDetect(){
    while(true){
        getVision[4]->Wait();
        lastOwnPosDetect();
        if(ballowns(_refVision[0].robot[PARAM::BLUE][_ownID[PARAM::BLUE]],0)){
            if(_refVision[0].robot[PARAM::BLUE][_ownID[PARAM::BLUE]].pos.dist(_ownPos[PARAM::BLUE]) > DOUBLE_DIST){
                _dribblemsg = blue_dribble;
            }
        }else if(ballowns(_refVision[0].robot[PARAM::YELLOW][_ownID[PARAM::YELLOW]],0)){
            if(_refVision[0].robot[PARAM::YELLOW][_ownID[PARAM::YELLOW]].pos.dist(_ownPos[PARAM::YELLOW]) > DOUBLE_DIST){
                _dribblemsg = yellow_dribble;
            }
        }else{
            _dribblemsg = no_dribble;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ref_event::receiveRefMsg(){
    _gameCommand = RefInfo::instance()->getCommand();
    _gameStage = RefInfo::instance()->getStage();
}

/////////////////////////////////////////////////////////////////////////////

void ref_event::directKickOFT(){
        _timer++;
        if(_timer >= DIRECTKCIK_TIME && _refVision[0].ball[0].pos.dist(_placementPos) < DIRECTKCIK_DIST){
            switch (_gameCommand) {
            case 9:
            case 11:
                _state = directkick_blue;
                break;
            case 8:
            case 10:
                _state = directkick_yellow;
                break;
            }
            _timer = 0;
        }else if(_refVision[0].ball[0].pos.dist(_placementPos) > DIRECTKCIK_DIST){
            _timer = 0;
        }
}

//////////////////////////////////////////////////////////////////////////////////

void ref_event::ballPlaceTimeout(){
    _placementPos.fill(RefInfo::instance()->getPlacementX(),RefInfo::instance()->getPlacementY());
        _timer++;
        if(_timer >= PALACEMENT_TIME && _refVision[0].ball[0].pos.dist(_placementPos) >= PALACEMENT_DIST){
            switch(_gameCommand){
            case 17:
                _state = ball_placement_blue;
                break;
            case 16:
                _state = ball_placement_yellow;
                break;
            }
            _timer = 0;
        }else if(_refVision[0].ball[0].pos.dist(_placementPos) < PALACEMENT_DIST){
            _timer = 0;
        }
}

/////////////////////////////////////////////////////////////////////////////

void ref_event::penaltyKickOFT(){
    CGeoPoint penaltyKick = CGeoPoint(Param::Field::PENALTY_MARK_X,0);
    if(_refVision[0].ball[0].pos.dist(penaltyKick) < PENALTY_KICK_DIST){
        _timer++;
        if(_timer >= PENALTY_KICK_TIME ){
            switch (_gameCommand) {
            case 6:
                _state = penaltykick_blue;
                break;
            case 7:
                _state = penaltykick_yellow;
                break;
            }
            _timer = 0;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void ref_event::penaltyAttackerDetect(){
    while(true){
        getVision[7]->Wait();
        int blueID = RefInfo::instance()->getBlueGoalie();
        int yellowID = RefInfo::instance()->getYellowGoalie();
        for(int i = 0 ; i<PARAM::ROBOTNUM ; i++){
            if(i != blueID){
                if(fabs(_refVision[0].robot[PARAM::BLUE][i].pos.x()) > Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH
                        && fabs(_refVision[0].robot[PARAM::BLUE][i].pos.y()) < Param::Field::PENALTY_AREA_WIDTH/2 && _refVision[0].robot[PARAM::BLUE][i].valid
                        && fabs(_refVision[0].robot[PARAM::BLUE][i].pos.x()) < Param::Field::PITCH_LENGTH/2){
                    _state = blue_in_penalty;
                }
            }
            if(i != yellowID){
                if(fabs(_refVision[0].robot[PARAM::YELLOW][i].pos.x()) > Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH
                        && fabs(_refVision[0].robot[PARAM::YELLOW][i].pos.y()) < Param::Field::PENALTY_AREA_WIDTH/2 && _refVision[0].robot[PARAM::YELLOW][i].valid
                        && fabs(_refVision[0].robot[PARAM::YELLOW][i].pos.x()) < Param::Field::PITCH_LENGTH/2){
                    _state = yellow_in_penalty;
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////

void ref_event::pushDetect(){
    while(true){
        getVision[5]->Wait();
        for(int i = 0 ; i < PARAM::ROBOTNUM ; i++){
            for(int j = 0 ; j < PARAM::ROBOTNUM ; j++){
                if(_refVision[0].robot[PARAM::BLUE][i].valid && _refVision[0].robot[PARAM::YELLOW][j].valid
                        && _refVision[0].robot[PARAM::BLUE][i].pos.dist(_refVision[0].robot[PARAM::YELLOW][i].pos) < PUSH_PAIR_DIST){
                    CGeoPoint midPoint = _refVision[0].robot[PARAM::BLUE][i].pos.midPoint(_refVision[0].robot[PARAM::YELLOW][i].pos);
                    int ID = i*100 + j;
                    _RobotPair.insert(std::pair<int , CGeoPoint>(ID , midPoint));
                }
            }
        }
        std::map<int , CGeoPoint>::iterator iter;
        for(iter = _RobotPair.begin(); iter != _RobotPair.end(); iter++){
            int blue_ID = iter->first/100;
            int yellow_ID = iter->first%100;
            if(_refVision[0].robot[PARAM::BLUE][blue_ID].pos.dist(_refVision[0].robot[PARAM::YELLOW][yellow_ID].pos) < PUSH_PAIR_DIST){

                CGeoPoint judgePoint = _refVision[0].robot[PARAM::BLUE][blue_ID].pos.midPoint(_refVision[0].robot[PARAM::YELLOW][yellow_ID].pos);
                double distance = judgePoint.dist(iter->second);
                CVector start2end = judgePoint - iter->second;

                if(distance > PUSH_DIST && fabs(start2end.dir()-_refVision[0].robot[PARAM::BLUE][blue_ID].angle) < Param::Math::PI / 2){
                    _state = blue_push;
                    _RobotPair.erase(iter++);
                }else if(distance > PUSH_DIST && fabs(start2end.dir()-_refVision[0].robot[PARAM::YELLOW][yellow_ID].angle) < Param::Math::PI / 2){
                    _state = yellow_push;
                    _RobotPair.erase(iter++);
                }
            }else {
                _RobotPair.erase(iter++);
            }
        }
    }
}
///////////////////////////////////////////////////////////////////
void ref_event::ballPathReset(){
    _chipConfidence = 0;
    _flatConfidence = 0;
    _passState = pass_uncertain;
    _uncertainTouch_ID = -1;
    _uncertainTouchPos = CGeoPoint(9999,9999);
    ballPath.clear();
    ChipSolver::instance()->reset();
}
void ref_event::ballPathAnalyse(){
    while(true){
//        qDebug() << Q_FUNC_INFO;
        Touch.Wait();
        Own.Wait();
        if(_ownmsg!=other) {
            if(_passState == pass_flat||_dribbleCount >= CONFIRM_DRIBBLE_CYCLE){
                           _lastTouch_ID = _ownID[PARAM::BLUE]>=0?_ownID[PARAM::BLUE]:_ownID[PARAM::YELLOW];
                           _lastTouchPos = _ownID[PARAM::BLUE]>=0?_ownPos[PARAM::BLUE]:_ownPos[PARAM::YELLOW];
                           ballPathReset();
                       }
            _dribbleCount++;
        }
        else{
            _dribbleCount = 0 ;
            ballPath.push(_refVision[0].ball[0].pos);
            if( ChipSolver::instance()->dealresult()&&ballPath.validSize()>=8){
//                std::cout<<"chipsolver!"<<std::endl;
                double flyTime = ChipSolver::instance()->getFlytime();
                CVector3 kickVel = ChipSolver::instance()->getkickVel();
                double ratio = fabs(kickVel.z())/CVector(kickVel.x(),kickVel.y()).mod();
                if(flyTime<MAX_FLY_TIME&&flyTime>MIN_FLY_TIME&&ratio<MAX_VELZ_RATIO&&_passState==pass_uncertain){
                    _chipConfidence += 2;
                }
                else if((flyTime<MIN_FLY_TIME||_refVision[0].ball[0].velocity.mod()<MIN_FLY_VEL)&&_passState==pass_uncertain){
                    _flatConfidence += 2;
                }
                else if(_passState==pass_uncertain) _flatConfidence += 1;
            }
            if(_flatConfidence>10&&_passState==pass_uncertain)  _passState = pass_flat;
            if(_chipConfidence>10&&_passState==pass_uncertain) _passState = pass_chip;

            if(_uncertainTouch_ID>=0){
            bool touch = false;
            CVector ballvec1(ballPath[0].x()-ballPath[-2].x(),ballPath[0].y()-ballPath[-2].y());
            CVector ballvec2(ballPath[-2].x()-ballPath[-4].x(),ballPath[-2].y()-ballPath[-4].y());
            if(/*fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>MIN_TURN_ANGLE&&*/_passState == pass_flat){
                _lastTouchPos = _uncertainTouchPos;
                _lastTouch_ID = _uncertainTouch_ID;

                touch = true;
            }//small line change
            else if(fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>OBVIOUS_TURN_ANGLE&&(_passState == pass_chip||_passState == pass_uncertain)){
                _lastTouchPos = _uncertainTouchPos;
                _lastTouch_ID = _uncertainTouch_ID;
                touch = true;
            }//obvious line change
            else if(fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>MIN_TURN_ANGLE&&_passState == pass_uncertain){
            }//uncertain
            else if(_passState == pass_chip){//fly over;
                _uncertainTouch_ID = -1;
                _uncertainTouchPos = CGeoPoint(9999,9999);
                if(ChipSolver::instance()->getRecordSize()==0) ballPathReset();
            }
            if(touch) ballPathReset();
            }
        }
    }
}

