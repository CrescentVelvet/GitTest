#include "ref_event.h"
#include "messageformat.h"
#include "geometry.h"
//#include "ssl_referee.pb.h"
#include "referee.h"
#include "Semaphore.h"
#include <thread>

namespace{
    std::thread* fieldT;
    std::thread* ballOnwT;
    std::thread* collisionT;
    std::thread* ballOverSpeedT;
    std::thread* lastTouchT;
    std::thread* dribbleT;
    std::thread* pushT;
}

ref_event::ref_event() : /*_state(nor),*/passMsg(pass_other),_receive_msg(no_receive)
{
}

void ref_event::run(DataQueue<ReceiveVisionMessage> vm){
//    std::cout << "in" <<std::endl;
    robotReset();
    visionMessage(vm);

    receiveRefMsg();

    if(lastcommad != gameCommand){
        timer = 0;
    }
    lastcommad = gameCommand;

    switch (gameCommand) {

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
    default: //norplay
        //fieldT = new std::thread( [=]{judgeOutEvent(vm[0].ball[0].pos);});
        //fieldT->detach();
        /////////////////////////////////////////
        ballOwnDetect();
        //////////////////////////////////////////
        collisionID();
        //////////////////////////////////////////
        ballOverSpeed();
        /////////////////////////////////////////
        lastTouchDetect();
        ////////////////////////////////////////
        getBotState();
        receiveDetet();
        passDetect();
        ///////////////////////////////////////////////
        dribbleDetect();
        ////////////////////////////////////////////////
        penaltyAttackerDetect();
        //////////////////////////////////////////
        pushDetect();
        break;
    }
    std::cout << "robot vel : " << robotBlue[0][2].velocity.mod()<<std::endl;
    std::cout << "ball vel : " << ball[0].velocity.mod() <<std::endl;
    std::cout << "state    : " << _state <<std::endl;
    std::cout << "ball owns  : " <<_ownmsg <<std::endl;
    std::cout << "blue own ID : " <<ownID_blue<<std::endl;
    std::cout << "yellow own ID : " <<ownID_yellow<<std::endl;

    std::cout << "receive  : " <<_receive_msg<<std::endl;
    std::cout << "pass msg   :" << passMsg << std::endl;
    std::cout << "dribble    : " <<dribblemsg << std::endl;

    std::cout << "robot is kick  : " <<_robotState <<std::endl;
    std::cout << "blue kick : " <<lastKick_ID_blue<<std::endl;
    std::cout << "yellow kick : " <<lastKick_ID_yellow<<std::endl;

    std::cout << "last touch ID : " << lastTouch_ID <<std::endl;
    std::cout << "/////////////////////////////" <<std::endl;
}

///////////////////////////////////////////////////////////////

void ref_event::ballOwnDetect(){
    bool own_blue = false;
    bool own_yellow = false;
    for(int i = 0 ; i < PARAM::ROBOTNUM ; i++){
        if(ballowns(robotBlue[0][i],0)){
                    own_blue = true;
                    _ownmsg = blue_own;
        }

        if(ballowns(robotYellow[0][i],0)){
                    own_yellow = true;
                    _ownmsg = yellow_own;
        }
        if(!own_yellow && !own_blue) {
            _ownmsg = other;
        }
    }
}

bool ref_event::ballowns(Robot& bot , int frame){
    if(bot.valid){
        CVector robot2ball = ball[frame].pos - bot.pos;
        if(robot2ball.mod() < own_dist && fabs(robot2ball.dir() - bot.angle) < own_dir){
            return true;
        }
    }
    return false;
}


///////////////////////////////////////////////////////////////

void ref_event::robotReset(){
    for(int i = -max_frame_num + 1 ; i <=0 ; i++ ){
        for(int j = 0 ; j < PARAM::ROBOTNUM ; j++){
            robotBlue[-i][j].valid = false;
            robotYellow[-i][j].valid = false;
        }
    }
}

void ref_event::visionMessage(DataQueue<ReceiveVisionMessage> vm){
    std::cout << vm[0].robotSize[PARAM::BLUE] <<std::endl;
    for(int i = -max_frame_num + 1 ; i <=0 ; i++ ){
        ball[-i] = vm[i].ball[0];
        for(int j = 0 ; j < vm[i].robotSize[PARAM::BLUE] ; j++){
            robotBlue[-i][vm[i].robot[PARAM::BLUE][j].id].valid = true;
            robotBlue[-i][vm[i].robot[PARAM::BLUE][j].id].fill(vm[i].robot[PARAM::BLUE][j]);
        }
        for(int j = 0 ; j < vm[i].robotSize[PARAM::YELLOW] ; j++){
            robotYellow[-i][vm[i].robot[PARAM::YELLOW][j].id].valid = true;
            robotYellow[-i][vm[i].robot[PARAM::YELLOW][j].id].fill(vm[i].robot[PARAM::YELLOW][j]);
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
    return lastTouch_ID;
}


///////////////////////////////////////////////////////////////

void ref_event::judgeOutEvent(CGeoPoint ball_pos){
    if(Utils::instance()->inArea(ball_pos, Param::Field::PITCH_LENGTH/2, Param::Field::PITCH_LENGTH/2+Param::Field::GOAL_DEPTH, -Param::Field::GOAL_WIDTH/2, Param::Field::GOAL_WIDTH/2)
            || Utils::instance()->inArea(ball_pos,-Param::Field::PITCH_LENGTH/2-Param::Field::GOAL_DEPTH,  -Param::Field::PITCH_LENGTH/2 , -Param::Field::GOAL_WIDTH/2, Param::Field::GOAL_WIDTH/2)){
        _state = goal;
    }else if(! Utils::instance()->inArea(ball_pos, -Param::Field::PITCH_LENGTH/2, Param::Field::PITCH_LENGTH/2, -Param::Field::PITCH_WIDTH/2, Param::Field::PITCH_WIDTH/2)){
        _state = out_of_field;
    }
}

////////////////////////////////////////////////////////////////

void ref_event::collisionID(){
    for(int i = 0 ; i < PARAM::ROBOTNUM ; i++){
        for(int j = 0 ; j < PARAM::ROBOTNUM ; j++){
            if(robotBlue[0][i].valid && robotYellow[0][j].valid
                && robotBlue[0][i].pos.dist(robotYellow[0][j].pos) < min_dist){
                collsionState(i,j);
            }
        }
    }
}

void ref_event::collsionState(int id_blue , int id_yellow){
    if(collisionDetect( robotBlue[0][id_blue].pos ,robotYellow[0][id_yellow].pos ,robotBlue[0][id_blue].velocity  ,robotYellow[0][id_yellow].velocity )){
        switch(judgeCollision(robotBlue[0][id_blue].velocity ,robotYellow[0][id_yellow].velocity )){
        case PARAM::BLUE :
            _state = blue_collision;
            break;
        case PARAM::YELLOW :
            _state = yellow_collision;
            break;
        }
    }
}

bool ref_event::collisionDetect(CGeoPoint& pos_1 , CGeoPoint& pos_2 , CVector vel_1 , CVector vel_2 ){
    CVector delta_Vel = vel_1-vel_2;
    double angle_blue = fabs(delta_Vel.dir()-vel_1.dir());
    double angle_yellow = fabs(delta_Vel.dir() - vel_2.dir());
    if(pos_1.dist(pos_2) < min_dist
            && (delta_Vel.mod() * std::cos(angle_blue) > min_vel_collision
                || delta_Vel.mod() * std::cos(angle_yellow) > min_vel_collision)){
        std::cout << delta_Vel.mod() <<std::endl;
        return true;
    }
    return false;
}

int ref_event::judgeCollision(CVector vel_blue , CVector vel_yellow){
    if(vel_blue.mod() > vel_yellow.mod()){
        return PARAM::BLUE;
    }else{
        return PARAM::YELLOW;
    }
}

///////////////////////////////////////////////////////////////////////////

void ref_event::ballOverSpeed(){
    int count = 0 ;
    for(int i = 8 ; i >= 0 ; i--){
        if(ball[i].velocity.mod() > max_vel){
            count++;
        }
    }
    if(count>5){
        _state = overspeed;
    }
}

///////////////////////////////////////////////////////////////////////////

void ref_event::getBotState(){
    for(int i = 0 ; i < PARAM::ROBOTNUM ; i++){
        if(kickDetect(robotBlue[0][i],0)){
                lastKick_ID_blue = i;
                _robotState = blue_kick_ball;
        }
    }

    for(int i = 0 ; i < PARAM::ROBOTNUM ; i++){
        if(kickDetect(robotYellow[0][i],0)){
                lastKick_ID_yellow = i + PARAM::ROBOTNUM;
                _robotState = yellow_kick_ball;
        }
    }
}


bool ref_event::kickDetect(Robot& bot ,int frame){
    if(bot.valid){
        CVector robot2ball = ball[frame].pos - bot.pos;
        Robot robot = bot;
        if( robot.pos.dist(ball[frame].pos) < 500 && robot.pos.dist(ball[frame].pos) > 400
                && fabs((robot2ball.dir() - robot.angle)) < Param::Math::PI / 12
                && ball[frame].velocity.mod() > min_ball_pass_vel
                && fabs(robot.angle - ball[frame].velocity.dir()) < Param::Math::PI / 36)
        {
            return true;
        }
    }
    return false;
}
////////////////////////////////////////////////////////////////////////

void ref_event::lastTouchDetect(){
    for (int i=0;i<PARAM::ROBOTNUM ;i++){
        for (int frame=4;frame>-1;frame--)
        {
            if (robotBlue[frame][i].valid
                    && robotBlue[frame][i].pos.dist(ball[frame].pos)>=85
                    && robotBlue[frame][i].pos.dist(ball[frame].pos)<=125)
              {
                    lastTouchPos= robotBlue[frame][i].pos;
                    lastTouch_ID = i;
            }
        }
    }
    for (int i=0;i<PARAM::ROBOTNUM ;i++){
        for (int frame=4;frame>-1;frame--)
        {
            if (robotYellow[frame][i].valid
                    && robotYellow[frame][i].pos.dist(ball[frame].pos)>=85
                    && robotYellow[frame][i].pos.dist(ball[frame].pos)<=125)
            {
                lastTouchPos =robotYellow[frame][i].pos;
                lastTouch_ID = i+PARAM::ROBOTNUM;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////

void ref_event::passDetect(){
    if(lastKick_ID_blue == lastTouch_ID
            && _robotState == blue_kick_ball
            && _receive_msg == blue_r){
        passMsg = pass_blue;
    }else if(lastKick_ID_yellow == lastTouch_ID
            && _robotState == yellow_kick_ball
            && _receive_msg == yellow_r){
        passMsg = pass_yellow;
    }else {
        passMsg = pass_other;
    }
}

void ref_event::receiveDetet(){
    bool blueReceive = false;
    bool yellowReceive = false;
    for(int i = 0 ; i< PARAM::ROBOTNUM ;i++){
        if(ifRecive(robotBlue[0][i],0)){
                blueReceive = true;
                _receive_msg = blue_r;
            }
    }
    for(int i = 0 ; i< PARAM::ROBOTNUM ;i++){
        if(ifRecive(robotYellow[0][i],0)){
                yellowReceive = true;
                _receive_msg = yellow_r;
            }
    }
    if(!blueReceive && !yellowReceive){
        _receive_msg = no_receive;
    }
}

bool ref_event::ifRecive(Robot& bot , int frame){
    if(bot.valid && bot.pos.dist(ball[frame].pos) < received_dist){
        CVector ball2robot = bot.pos - ball[frame].pos;
        if(fabs(ball[frame].velocity.dir()-ball2robot.dir()) < Param::Math::PI/32){
            return true;
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////
//选边问题
void ref_event::aimlessDetect(){
    if(lastTouch_ID == lastKick_ID_blue && ball[0].pos.x()>Param::Field::PITCH_LENGTH/2
            && lastTouchPos.x() <0){
//        std::cout << "in" <<std::endl;
        aimlessmsg = blue_aimless;
    }else if(lastTouch_ID == lastKick_ID_yellow && ball[0].pos.x()< -Param::Field::PITCH_LENGTH/2
            && lastTouchPos.x() >0){
        aimlessmsg = yellow_aimless;
    }else{
        aimlessmsg = no_aimless;
    }

}

/////////////////////////////////////////////////////////////////////////////////

void ref_event::lastOwnPosDetect(){
    for(int i = 0 ; i < PARAM::ROBOTNUM ; i++){
        if(ballowns(robotBlue[0][i],0) && !ballowns(robotBlue[1][i],1)){
//            std::cout << "true" << std::endl;
            ownPos_blue = robotBlue[0][i].pos;
            ownID_blue = i;
        }
    }
    for(int i = 0 ; i < PARAM::ROBOTNUM ; i++){
        if(ballowns(robotYellow[0][i],0) && !ballowns(robotYellow[1][i],1)){
            ownPos_yellow = robotYellow[0][i].pos;
            ownID_yellow = i;
        }
    }
}

void ref_event::dribbleDetect(){
    lastOwnPosDetect();
//    std::cout << "( " << ownPos_yellow.x() << " , " << ownPos_yellow.y() << " )" <<std::endl;
    if(ballowns(robotBlue[0][ownID_blue],0)){
        if(robotBlue[0][ownID_blue].pos.dist(ownPos_blue) > dribble_dist){
            dribblemsg = blue_dribble;
        }
    }else if(ballowns(robotYellow[0][ownID_yellow],0)){
        if(robotYellow[0][ownID_yellow].pos.dist(ownPos_yellow) > dribble_dist){
            dribblemsg = yellow_dribble;
        }
    }else{
        dribblemsg = no_dribble;
    }
}

////////////////////////////////////////////////////////////////////////////////

void ref_event::receiveRefMsg(){
    gameCommand = RefInfo::instance()->getCommand();
    gameStage = RefInfo::instance()->getStage();

}

/////////////////////////////////////////////////////////////////////////////

void ref_event::directKickOFT(){
    if(gameCommand == 9 || gameCommand == 11){
        timer++;
        if(timer >= 770 && ball[0].pos.dist(placementPos) < 80){
            _state = directkick_blue;
            timer = 0;
        }else if(ball[0].pos.dist(placementPos) > 80){
            timer = 0;
        }
    }
    if(gameCommand == 8 || gameCommand == 10){
        timer++;
        if(timer >= 770 && ball[0].pos.dist(placementPos) < 80){
            _state = directkick_yellow;
            timer = 0;
        }else if(ball[0].pos.dist(placementPos) > 80){
            timer = 0;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////

void ref_event::ballPlaceTimeout(){
    if(gameCommand == 17){
        ballPlaceX = RefInfo::instance()->getPlacementX();
        ballPlaceY = RefInfo::instance()->getPlacementY();
        placementPos.fill(ballPlaceX,ballPlaceY);
        timer++;
        if(timer >= 2310 && ball[0].pos.dist(placementPos) >= 10){
            _state = ball_placement_blue;
            timer = 0;
        }else if(ball[0].pos.dist(placementPos) < 10){
            timer = 0;
        }
    }

    if(gameCommand == 16){
        ballPlaceX = RefInfo::instance()->getPlacementX();
        ballPlaceY = RefInfo::instance()->getPlacementY();
        placementPos.fill(ballPlaceX,ballPlaceY);
        timer++;
        if(timer >= 2310 && ball[0].pos.dist(placementPos) >= 10){
            _state = ball_placement_yellow;
            timer = 0;
        }else if(ball[0].pos.dist(placementPos) < 10){
            timer = 0;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void ref_event::penaltyKickOFT(){

    CGeoPoint penaltyKick = CGeoPoint(Param::Field::PENALTY_MARK_X,0);
    if(gameCommand == 6 && ball[0].pos.dist(penaltyKick) < 50){
        timer++;
        if(timer >= 770 ){
            _state = penaltykick_blue;
            timer = 0;
        }
    }
    if(gameCommand == 7 && ball[0].pos.dist(penaltyKick) < 50){
        timer++;
        if(timer >= 770 ){
            _state = penaltykick_yellow;
            timer = 0;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void ref_event::penaltyAttackerDetect(){
    int blueID = RefInfo::instance()->getBlueGoalie();
    int yellowID = RefInfo::instance()->getYellowGoalie();
    for(int i = 0 ; i<PARAM::ROBOTNUM ; i++){
        if(i != blueID){
            if(fabs(robotBlue[0][i].pos.x()) > Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH
                    && fabs(robotBlue[0][i].pos.y()) < Param::Field::PENALTY_AREA_WIDTH/2 && robotBlue[0][i].valid
                    && fabs(robotBlue[0][i].pos.x()) < Param::Field::PITCH_LENGTH/2){
                _state = blue_in_penalty;
            }
        }
    }
    for(int i = 0 ; i<PARAM::ROBOTNUM ; i++){
        if(i != yellowID){
            if(fabs(robotYellow[0][i].pos.x()) > Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_DEPTH
                    && fabs(robotYellow[0][i].pos.y()) < Param::Field::PENALTY_AREA_WIDTH/2 && robotBlue[0][i].valid
                    && fabs(robotYellow[0][i].pos.x()) < Param::Field::PITCH_LENGTH/2){
                _state = yellow_in_penalty;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////

void ref_event::pushDetect(){
    for(int i = 0 ; i < PARAM::ROBOTNUM ; i++){
        for(int j = 0 ; j < PARAM::ROBOTNUM ; j++){
            if(robotBlue[0][i].valid && robotYellow[0][j].valid
               && robotBlue[0][i].pos.dist(robotYellow[0][j].pos) < push_dist){
                CGeoPoint midPoint = robotBlue[0][i].pos.midPoint(robotYellow[0][j].pos);
                int ID = i*100 + j;
                RobotPair.insert(std::pair<int , CGeoPoint>(ID , midPoint));
            }
        }
    }
    std::map<int , CGeoPoint>::iterator iter;
    for(iter = RobotPair.begin(); iter != RobotPair.end(); iter++){
        int blue_ID = iter->first/100;
        int yellow_ID = iter->first%100;
        if(robotBlue[0][blue_ID].pos.dist(robotYellow[0][yellow_ID].pos) < 200){

            CGeoPoint judgePoint = robotBlue[0][blue_ID].pos.midPoint(robotYellow[0][yellow_ID].pos);
            double distance = judgePoint.dist(iter->second);
            CVector start2end = judgePoint - iter->second;

            if(distance > push_dist && fabs(start2end.dir()-robotBlue[0][blue_ID].angle) < Param::Math::PI / 2){
                _state = blue_push;
                RobotPair.erase(iter);
            }else if(distance > push_dist && fabs(start2end.dir()-robotYellow[0][yellow_ID].angle) < Param::Math::PI / 2){
                _state = yellow_push;
                RobotPair.erase(iter);
            }
        }else {
            RobotPair.erase(iter);
        }
    }
}
///////////////////////////////////////////////////////////////////

