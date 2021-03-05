#include "ballstate.h"
#include "globaldata.h"
#include "debugengine.h"
#include <fstream>>
//#include <chrono>
//using std::chrono::high_resolution_clock;
//using std::chrono::milliseconds;



namespace  {
bool debug_flag; //  draw debug messege
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
double MIN_FLY_TIME = 0.3;
double MAX_FLY_TIME = 1.4;
double MAX_VELZ_RATIO = 1.5;
const double MIN_FLY_VEL = 2000;
//collision;
const double MIN_TURN_ANGLE = PARAM::Math::PI*3/180;
const double OBVIOUS_TURN_ANGLE = PARAM::Math::PI*30/180;
const double MAX_COLLISION_DIST = PARAM::Vehicle::V2::PLAYER_SIZE+3*PARAM::Field::BALL_SIZE;
const double TOUCH_DIST = PARAM::Vehicle::V2::PLAYER_SIZE+1.5*PARAM::Field::BALL_SIZE;
//analyse
const int DETECT_DIST = 150; //detect whether the ball in 300mm of vechile
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
BallPathAnalyzer::BallPathAnalyzer():_tempState(State::_uncertain_pass),_chipConfidence(0),_flatConfidence(0){
    ZSS::ZParamManager::instance()->loadParam(MIN_FLY_TIME,"chipsolver/MIN_FLY_TIME",0.3);
    ZSS::ZParamManager::instance()->loadParam(MAX_FLY_TIME,"chipsolver/MAX_FLY_TIME",1.4);
    ZSS::ZParamManager::instance()->loadParam(MAX_VELZ_RATIO,"chipsolver/MAX_VELZ_RATIO",1.5);
    for(int team = 0;team < PARAM::TEAMS;team++){
        for(int i=0;i<PARAM::ROBOTNUM;i++){
            touchCount[team][i]=0;
        }
    }
}

void BallPathAnalyzer::run(ReceiveVisionMessage* vision){
    _pVision = vision;
    _tempTouch = -1;
    ballPath.push(vision->ball[0]);
    if(debug_flag)
        GDebugEngine::instance()->gui_debug_msg(CGeoPoint(1000,1800), QString("chipcon:%1, flatcon:%2").arg(_chipConfidence).arg(_flatConfidence).toLatin1());
    ////////////////////////////////
    //速度过小时跳出
    ////////////////////////////////
    static int stopCount = 0;
    if(ballPath[0].velocity.mod()<MIN_VEL) stopCount++;
    else stopCount = 0;
    if(stopCount>5) {
        _tempState = State::_stop;
        _tempTouch = findTouch();
//        qDebug()<<"ballstate reset1";
        reset();
        return;
    }
    ////////////////////////////////
    //跳球、平射置信度判断
    ////////////////////////////////

//    auto t_1 = high_resolution_clock::now();
//    auto t1 = getCurrentTime();
//    qDebug()<<"chipsolver time t1: "<<t1;
//    bool flag = chipsolver.dealresult();
//    auto t2 = getCurrentTime();
//    qDebug()<<"chipsolver time t2: "<<t2;
//    auto t_2 = high_resolution_clock::now();
//    qDebug()<<"chipsolver time: "<<t2 - t1;

    bool flag = ChipSolver::instance()->getDealResultFlag();

    if(flag){
        vision->ball[0].predict_pos = ChipSolver::instance()->getposEnd();
        vision->ball[0].best_predict_pos = ChipSolver::instance()->getBestPredictPos();
        vision->ball[0].real_pos.formXYZ(ChipSolver::instance()->getposNow().x(), ChipSolver::instance()->getposNow().y(), ChipSolver::instance()->getposNow().z());
        vision->ball[0].chip_second_pos = ChipSolver::instance()->getBestSecondPos();
        vision->ball[0].chip_fly_time = ChipSolver::instance()->getFlytime();
        vision->ball[0].chip_rest_time = ChipSolver::instance()->getRestFlyTime();
        vision->ball[0].chip_velocity = ChipSolver::instance()->getkickVel();
//        qDebug()<<"x:"<<chipsolver.getBestPredictPos().x()<<" y: "<<chipsolver.getBestPredictPos().y();

        // add last touch pos
        CGeoPoint _lastTouchPos = GlobalData::instance()->lastTouchPos;
        if(debug_flag)
            GDebugEngine::instance()->gui_debug_robot(_lastTouchPos,0,0,COLOR_RED);

        // if the dist of predict kickpos and last touch pos is bigger than 500, not chip, reset path
        CVector3 kickPos = ChipSolver::instance()->getkickPos();
        if(debug_flag)
            GDebugEngine::instance()->gui_debug_msg(CGeoPoint(0,-500),QString("X:%1,Y:%2").arg(kickPos.x()).arg(kickPos.y()).toLatin1());
        if(_lastTouchPos.dist(CGeoPoint(kickPos.x(), kickPos.y())) > 500)
        {
//            if(debug_flag)
//                GDebugEngine::instance()->gui_debug_msg(CGeoPoint(0, -1000), QString("reset").toLatin1(), COLOR_GREEN);
            _flatConfidence = 11;
            ChipSolver::instance()->setResetFlag(true);
        }
    }
    if(flag){
        double flyTime = ChipSolver::instance()->getFlytime();
        CVector3 kickVel = ChipSolver::instance()->getkickVel();
        double ratio = fabs(kickVel.z())/CVector(kickVel.x(),kickVel.y()).mod();
        if(debug_flag){
            std::ofstream ratio_file("./ratio.txt", std::ios::app);
            if(ratio_file.is_open()){
                ratio_file << ratio << std::endl;
                ratio_file.close();
            }
            std::ofstream flytime_file("./flytime.txt", std::ios::app);
            if(flytime_file.is_open()){
                flytime_file << flyTime << std::endl;
                flytime_file.close();
            }
        }
        if(flyTime<MAX_FLY_TIME&&flyTime>MIN_FLY_TIME&&ratio<MAX_VELZ_RATIO&&_tempState==State::_uncertain_pass){
            _chipConfidence += 2;
        }
        else if((flyTime<MIN_FLY_TIME||vision->ball[0].velocity.mod()<MIN_FLY_VEL)&&_tempState==State::_uncertain_pass){
            _flatConfidence += 2;
        }
        else if(_tempState==State::_uncertain_pass) _flatConfidence += 1;

        if(debug_flag){
            GDebugEngine::instance()->gui_debug_msg(CGeoPoint(1000, 1500),QString("flytime:%1, ratio:%2").arg(flyTime).arg(ratio).toLatin1());
        }
    }

    if(_flatConfidence>10&&_tempState==State::_uncertain_pass)  _tempState = State::_flat_pass;
    if(_chipConfidence>10&&_tempState==State::_uncertain_pass) _tempState = State::_chip_pass;

    ////////////////////////////////
    //碰撞检测
    ////////////////////////////////
    //resolve uncertain touch
    if(_tempState == State::_flat_pass&&_uncertainTouch>=0) {
        _tempTouch = _uncertainTouch;
        GlobalData::instance()->lastTouchPos = _uncertainTouchPos;
    }
    if(_tempState == State::_chip_pass&&_uncertainTouch>=0) _uncertainTouch = -1;
    //fly end reset
    if(_tempState == State::_chip_pass&&GlobalData::instance()->ballrecords.validSize() == 0)
    {
//        qDebug()<<"ballstate reset2";
        reset();
    }
    int touchNum = findTouch();
    //collision detect
    if(ballPath.validSize()>2){
        CVector ballvec1(ballPath[0].pos.x()-ballPath[-1].pos.x(),ballPath[0].pos.y()-ballPath[-1].pos.y());
        CVector ballvec2(ballPath[-1].pos.x()-ballPath[-2].pos.x(),ballPath[-1].pos.y()-ballPath[-2].pos.y());
        if(/*fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>MIN_TURN_ANGLE&&*/_tempState == _flat_pass){
            _tempTouch = touchNum;
            GlobalData::instance()->lastTouchPos = CGeoPoint(ballPath[-1].pos.x(), ballPath[-1].pos.y());
        }//small line change
        else
        if(fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>OBVIOUS_TURN_ANGLE&&(_tempState == _chip_pass||_tempState == _uncertain_pass)){
            // 如果球的图像信息消失，会保持原先的位置，此时ballvec等于(0,0)，排除这种情况
            if(!((fabs(ballvec1.x()) < 0.001 && fabs(ballvec1.y()) < 0.001) ||
                 (fabs(ballvec2.x()) < 0.001 && fabs(ballvec2.y()) < 0.001)))
                _tempTouch = touchNum;
        }//obvious line change
        else if(/*fabs(Normalize(ballvec1.dir()-ballvec2.dir()))>MIN_TURN_ANGLE&&*/_tempState == _uncertain_pass){
            _uncertainTouch = touchNum;
            _uncertainTouchPos = CGeoPoint(ballPath[-1].pos.x(), ballPath[-1].pos.y());
        }//uncertain
    }
    if (_tempTouch>=0) {
//        qDebug()<<"ballstate reset3";
        reset();
    }
}
//根据距离和折射判断是否有车touch
// 如果有Touch(球与车紧挨着或球在车上折射)，则返回车号，否则返回-1
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
            // 连续4帧和车挨着
            if(touchCount[team][i]>4) {
                robNum = team*PARAM::ROBOTNUM+i;
                break;
            }
            // 如果ballPath存的球位置大于3帧，就判断是否存在折射
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
    _tempState = State::_uncertain_pass;
    ChipSolver::instance()->setResetFlag(true);
    predictPoses.clear();
    ballPath.clear();
}


//---------------main--------------------
BallState::BallState():_ballState(State::_stop){
    ZSS::ZParamManager::instance()->loadParam(debug_flag,"chipsolver/debug_flag",0);
}

void BallState::run(ReceiveVisionMessage* vision){
//    auto t_1 = high_resolution_clock::now();
//    auto t1 = getCurrentTime();
//    qDebug()<<"ballstate time t1: "<<t1;
    _pVision = vision;
    bool pathReset = true;
    // _lastTouch is the temp lasttouch in this cycle
    _tempTouch = -1;
    if(!dribbleJudge()){
            pathReset = false;
            ballPathAnalyze();
    }

    if(pathReset)
    {
//        qDebug()<<"ballstate reset4";
        ballPathAnalyzer.reset();
    }

    if(_tempTouch>=0)
        GlobalData::instance()->lastTouch = _tempTouch;

    if((GlobalData::instance()->maintain[0].ball[0].ball_state_machine == State::_dribble) && !(_ballState == State::_dribble))
    {
        // attention! if the robot dribble but not kick, it will not work
        // last touch only store the pos when robot dribble the ball
        // it need change to last state is dribble and now state is not dribble
        GlobalData::instance()->lastTouchPos = _pVision->robot[GlobalData::instance()->lastTouch/PARAM::ROBOTNUM][GlobalData::instance()->lastTouch-GlobalData::instance()->lastTouch/PARAM::ROBOTNUM*PARAM::ROBOTNUM].pos;
    }
    vision->ball[0].ball_state_machine = _ballState;

    if(debug_flag){
        QString stateName[] = {"touched", "shoot", "struggle", "pass", "dribble", "stop", "outoffied", "control", "undefine", "kicked", "chip_pass", "flat_pass","uncertain_pass"};
        GDebugEngine::instance()->gui_debug_msg(CGeoPoint(1000,2000), QString("state:%1, chip:%2, validsize:%3").arg(stateName[_ballState]).arg(_ballState==10).arg(GlobalData::instance()->ballrecords.validSize()).toLatin1(), COLOR_ORANGE);
        if(_ballState == 10)
            GDebugEngine::instance()->gui_debug_arc(CGeoPoint(0,0),400, 0, 360,COLOR_RED);
    }

//    auto t2 = getCurrentTime();
//    qDebug()<<"chipsolver time t2: "<<t2;
}

//对dribble信息的处理（结构有点乱，需要再优化）
// 判断并更新当前帧的laststate和lasttouch
bool BallState::dribbleJudge(){
    dribbleJudger.run(_pVision);
    bool dribble[PARAM::TEAMS];
    for(int i = 0;i<PARAM::TEAMS;i++){
        dribble[i] = false;
        if(dribbleJudger.RobNum[i]>=0){
            if(
                    // 如果判定为dribble但球是从车边上飞过(dribble的count小于3帧)，则令dribble为false
                    (GlobalData::instance()->maintain[0].ball[0].ball_state_machine==State::_chip_pass||GlobalData::instance()->maintain[0].ball[0].ball_state_machine==State::_uncertain_pass)
                    &&dribbleJudger.dribbleCount[i][dribbleJudger.RobNum[i]]<MIN_BELIEVE_CYCLE
                    )
                continue;
            dribble[i] = true;
        }
    }
    // 如果两辆车都在吸球
    if(dribble[PARAM::BLUE]&&dribble[PARAM::YELLOW]){//struggle
        _ballState = State::_struggle;
        if(dribbleJudger.dribbleCount[PARAM::BLUE][dribbleJudger.RobNum[PARAM::BLUE]]<
                dribbleJudger.dribbleCount[PARAM::YELLOW][dribbleJudger.RobNum[PARAM::YELLOW]])
                _tempTouch = dribbleJudger.RobNum[PARAM::YELLOW]+PARAM::Field::MAX_PLAYER; // 黄车号+所有的蓝车数
        else    _tempTouch = dribbleJudger.RobNum[PARAM::BLUE];
        return true;
    }
    // 如果其中一辆车在吸球
    else if(dribble[PARAM::BLUE]||dribble[PARAM::YELLOW]){//dribble
        _ballState = State::_dribble;
        _tempTouch = dribble[PARAM::BLUE]?dribbleJudger.RobNum[PARAM::BLUE]:dribbleJudger.RobNum[PARAM::YELLOW]+PARAM::Field::MAX_PLAYER;
        return true;
    }
    // 如果没有车在吸球
    return false;
}


void BallState::ballPathAnalyze(){
    ballPathAnalyzer.run(_pVision);
    if(ballPathAnalyzer._tempTouch>=0){
        _tempTouch = ballPathAnalyzer._tempTouch;
    }
    _ballState = ballPathAnalyzer._tempState;
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
