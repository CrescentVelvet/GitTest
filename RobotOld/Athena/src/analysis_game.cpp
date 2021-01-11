#include "analysis_game.h"
#include <QIODevice>
#include <QDataStream>
#include "zss_rec.pb.h"
#include "zss_debug.pb.h"

AnalysisGame::AnalysisGame(QObject* parent) : QObject(parent), realFrame(0),
    Team(PARAM::BLUE), homeSide(Left), ourHoldFrame(0.0), theirHoldFrame(0.0), holdMsg(""){
    HoldRate[PARAM::BLUE] = 0.0;
    HoldRate[PARAM::YELLOW] = 0.0;
    for(int i = 0; i < 6; i++){
        our_car_num[i] = 0;
    }
    for(int i = 0; i < 6; i++){
        their_car_num[i] = 0;
    }
    for(int i = 0; i < 3; i++){
        real_ourframe[i] = 0;
    }
    for(int i = 0; i < 3; i++){
        real_theirframe[i] = 0;
    }
}

void AnalysisGame::analy_status(const QByteArray& packet){
    ZSS::Protocol::RecMessage recdata;
    recdata.ParseFromArray(packet.data(), packet.size());
}

CVector AnalysisGame::Polar2Vector(double m,double angle){
    return CVector(m*std::cos(angle),m*std::sin(angle));
}

CGeoPoint AnalysisGame::analy_TheirInterPoint(CGeoPoint enemy, CGeoPoint ball){
    double FRICTION = 80.0;//simmodule = 152
    static const double ballAcc = FRICTION / 2;//球减速度
    constexpr double carAcc = 400;
    constexpr double maxBallSpeed = 650;
    double dist = (enemy - ball).mod();
    double delta = maxBallSpeed * maxBallSpeed + 2 * (carAcc - ballAcc) * dist;
    double t = (std::sqrt(delta) - maxBallSpeed) / (carAcc - ballAcc);
    double enemy2point = 0.5 * carAcc * t * t;
    return enemy + Polar2Vector(enemy2point, (ball - enemy).dir());
}

//判断一辆小车是否在盯人marking
bool AnalysisGame::analy_marking(const QByteArray& packet, int team, int num){
    ZSS::Protocol::RecMessage recdata;
    recdata.ParseFromArray(packet.data(), packet.size());
    //找可信的球位置
    int validNum = 0;
    for(int k = 0; k < recdata.maintainvision().balls().size(); k++){
        if(recdata.maintainvision().balls().ball(k).valid()){
            validNum = k;
            break;
        }
    }
    //确定半场位置
    int side;
    if(team == Team){ side = homeSide; }
    else if(team == opTeam){ side = (homeSide == Right ? Left : Right); }
    else{ qDebug() << "error in analy_marking"; }
//    qDebug() << team << side;
    //找小车传球点位置：位于后场且能拦截传球
    int opteam = 1 - team;
    CGeoPoint me(recdata.maintainvision().maintain(team).robot(num).posx(),
                 recdata.maintainvision().maintain(team).robot(num).posy());
    CGeoPoint ball(recdata.maintainvision().balls().ball(validNum).posx(),
                   recdata.maintainvision().balls().ball(validNum).posy());
    if(recdata.maintainvision().maintain(PARAM::BLUE).has_size()
            && recdata.maintainvision().maintain(PARAM::YELLOW).has_size()){
        for(int j = 0; j < recdata.maintainvision().maintain(opteam).robot().size(); j++){
            CGeoPoint enemy(recdata.maintainvision().maintain(opteam).robot(j).posx(),
                            recdata.maintainvision().maintain(opteam).robot(j).posy());
            CGeoPoint pass_point = analy_TheirInterPoint(enemy, ball);
            double distance_me = me.dist(pass_point);
            double distance_enemy = enemy.dist(pass_point);
            if(side == Right && distance_me < distance_enemy){
                if (enemy.x() > 0) { return true; }
            }
            else if(side == Left && distance_me < distance_enemy){
                if (enemy.x() < 0) { return true; }
            }
        }
    }else{ qDebug() << "error in analy_marking"; }
    return false;
}

//判断一辆小车是否在后卫guarding
bool AnalysisGame::analy_guarding(const QByteArray& packet, int team, int num){
    ZSS::Protocol::RecMessage recdata;
    recdata.ParseFromArray(packet.data(), packet.size());
    int opteam = 1 - team;
    int x = recdata.maintainvision().maintain(team).robot(num).posx();
    int y = recdata.maintainvision().maintain(team).robot(num).posy();
    int side;
    if(team == Team){ side = homeSide; }
    else if(team == opTeam){ side = (homeSide == Right ? Left : Right); }
    else{ qDebug() << "error in analy_guarding"; }
    if(recdata.maintainvision().maintain(PARAM::BLUE).has_size() && recdata.maintainvision().maintain(PARAM::YELLOW).has_size()){
        if(side == Right && x > param_width/4){
            return true;
        }
        else if(side == Left && x < -param_width/4){
            return true;
        }
    }else{ qDebug() << "error in analy_guarding"; }
    return false;
}

//判断一辆小车是否在守门goalkeeping
bool AnalysisGame::analy_goalkeeping(const QByteArray& packet, int team, int num){
    ZSS::Protocol::RecMessage recdata;
    recdata.ParseFromArray(packet.data(), packet.size());
    int opteam = 1 - team;
    int x = recdata.maintainvision().maintain(team).robot(num).posx();
    int y = recdata.maintainvision().maintain(team).robot(num).posy();
    int side;
    if(team == Team){ side = homeSide; }
    else if(team == opTeam){ side = (homeSide == Right ? Left : Right); }
    else{ qDebug() << "error in analy_goalkeeping"; }
    if(recdata.maintainvision().maintain(PARAM::BLUE).has_size() && recdata.maintainvision().maintain(PARAM::YELLOW).has_size()){
        if(side == Right && x > param_width/2 - param_penalty_area_l && fabs(y) < param_penalty_radius){
            return true;
        }
        else if(side == Left && x < -param_width/2 + param_penalty_area_l && fabs(y) < param_penalty_radius){
            return true;
        }
    }else{ qDebug() << "error in analy_goalkeeping"; }
    return false;
}

//判断一辆小车是否在截球tackling
//判断一辆小车是否在带球dribbling
//判断一辆小车是否在传球passing
//判断一辆小车是否在接球catching
//判断一辆小车是否在射门shooting

//计算进攻性
void AnalysisGame::cal_our_carnum(const QByteArray& packet){
    ZSS::Protocol::RecMessage recdata;
    recdata.ParseFromArray(packet.data(), packet.size());
    ZSS::Protocol::Debug_Msgs debugMsgs;
    //找可信的球位置
    int validNum = 0;
    for(int k = 0; k < recdata.maintainvision().balls().size(); k++){
        if(recdata.maintainvision().balls().ball(k).valid()){
            validNum = k;
            break;
        }
    }
    //找当前状态
    for(int k = 0; k < recdata.debugmsgs_size(); k++){
        debugMsgs = recdata.debugmsgs(k);
//        qDebug() << "recdata.debugmsgs().size() =" << recdata.debugmsgs().size();
        for(int j = 0; j < debugMsgs.msgs_size(); j++){
            if(debugMsgs.msgs(j).type() == ZSS::Protocol::Debug_Msg_Debug_Type_TEXT){
                holdMsg = debugMsgs.msgs(j).text().text();
                if(holdMsg == "Our" || holdMsg == "OurHolding"
                        || holdMsg == "Their" || holdMsg == "TheirHolding"
                        || holdMsg == "Both" || holdMsg == "BothHolding"){
                    break;
                }
            }
        }
        if(holdMsg == "Our" || holdMsg == "OurHolding"
                || holdMsg == "Their" || holdMsg == "TheirHolding"
                || holdMsg == "Both" || holdMsg == "BothHolding"){
            break;
        }
    }
    //计算各种车数
    int team = Team;
    int opteam = opTeam;
    if(recdata.maintainvision().maintain(PARAM::BLUE).has_size()
            && recdata.maintainvision().maintain(PARAM::YELLOW).has_size()){
        for(int m = 0; m < recdata.maintainvision().maintain(team).robot().size(); m++){
            if(holdMsg == "OurHolding"){
                real_ourframe[0]++;
//                qDebug() << m << "get ball in" << CGeoPoint(recdata.maintainvision().maintain(team).robot(m).posx(),
//                                      recdata.maintainvision().maintain(team).robot(m).posy())
//                                 .dist(CGeoPoint(recdata.maintainvision().balls().ball(validNum).posx(),
//                                                 recdata.maintainvision().balls().ball(validNum).posy()));
                if(CGeoPoint(recdata.maintainvision().maintain(team).robot(m).posx(),
                             recdata.maintainvision().maintain(team).robot(m).posy())
                        .dist(CGeoPoint(recdata.maintainvision().balls().ball(validNum).posx(),
                                        recdata.maintainvision().balls().ball(validNum).posy()))
                        < 400){
                    our_car_num[0]++;
                }
                else if(analy_marking(packet, team, m) || analy_guarding(packet, team, m)
                        || analy_goalkeeping(packet, team, m)){
                    our_car_num[1]++;
                }
                else{
                    our_car_num[0]++;
                }
            }
            else if(holdMsg == "TheirHolding"){
                real_ourframe[1]++;
                if(analy_marking(packet, team, m) || analy_guarding(packet, team, m)
                        || analy_goalkeeping(packet, team, m)){
                    our_car_num[3]++;
                }
                else{
                    our_car_num[2]++;
                }
            }
            else if(holdMsg == "Our" || holdMsg == "Their" || holdMsg == "Both" || holdMsg == "BothHolding"){
                real_ourframe[2]++;
                if(analy_marking(packet, team, m) || analy_guarding(packet, team, m)
                        || analy_goalkeeping(packet, team, m)){
                    our_car_num[5]++;
                }
                else{
                    our_car_num[4]++;
                }
            }else{ qDebug() << "has an error in our holdMsg!"; }
        }
        for(int m = 0; m < recdata.maintainvision().maintain(opteam).robot().size(); m++){
            if(holdMsg == "TheirHolding"){
                real_theirframe[0]++;
                if(CGeoPoint(recdata.maintainvision().maintain(opteam).robot(m).posx(),
                             recdata.maintainvision().maintain(opteam).robot(m).posy())
                        .dist(CGeoPoint(recdata.maintainvision().balls().ball(validNum).posx(),
                                        recdata.maintainvision().balls().ball(validNum).posy()))
                        < 400){
                    their_car_num[0]++;
                }
                else if(analy_marking(packet, opteam, m) || analy_guarding(packet, opteam, m)
                        || analy_goalkeeping(packet, opteam, m)){
                    their_car_num[1]++;
                }
                else{
                    their_car_num[0]++;
                }
            }
            else if(holdMsg == "OurHolding"){
                real_theirframe[1]++;
                if(analy_marking(packet, opteam, m) || analy_guarding(packet, opteam, m)
                        || analy_goalkeeping(packet, opteam, m)){
                    their_car_num[3]++;
                }
                else{
                    their_car_num[2]++;
                }
            }
            else if(holdMsg == "Our" || holdMsg == "Their" || holdMsg == "Both" || holdMsg == "BothHolding"){
                real_theirframe[2]++;
                if(analy_marking(packet, opteam, m) || analy_guarding(packet, opteam, m)
                        || analy_goalkeeping(packet, opteam, m)){
                    their_car_num[5]++;
                }
                else{
                    their_car_num[4]++;
                }
            }else{ qDebug() << "has an error in our holdMsg!"; }
        }
    }else{ qDebug() << "has an error in our holdMsg!"; }
}

//计算控球数
void AnalysisGame::Ball_ControlRate(const QByteArray& packet){
    ZSS::Protocol::RecMessage debugText;
    debugText.ParseFromArray(packet.data(), packet.size());
    ZSS::Protocol::Debug_Msgs debugMsgs;
    for(int k = 0; k < debugText.debugmsgs_size(); k++){
        debugMsgs = debugText.debugmsgs(k);
        for(int j = 0; j < debugMsgs.msgs_size(); j++){
            if(debugMsgs.msgs(j).type() == ZSS::Protocol::Debug_Msg_Debug_Type_TEXT){
                holdMsg = debugMsgs.msgs(j).text().text();
                if(holdMsg == "OurHolding" ){
                    ourHoldFrame++;
                    break;
                }
                if(holdMsg == "TheirHolding" ){
                    theirHoldFrame++;
                    break;
                }
            }
        }
    }
}

//计算真正比赛总帧数
bool AnalysisGame::cal_RealFrame(const QByteArray& packet){
    ZSS::Protocol::RecMessage debugText;
    debugText.ParseFromArray(packet.data(), packet.size());
    ZSS::Protocol::Debug_Msgs debugMsgs;
    std::string holdMsg;
    bool isHalt = false;
    bool empty = true;
    for(int k = 0; k < debugText.debugmsgs_size(); k++){
        if(isHalt){ break; }
        debugMsgs = debugText.debugmsgs(k);
        for(int j = 0; j < debugMsgs.msgs_size(); j++){
            if(debugMsgs.msgs(j).type() == ZSS::Protocol::Debug_Msg_Debug_Type_TEXT){
                holdMsg = debugMsgs.msgs(j).text().text();
                if(holdMsg != ""){
                    empty = false;
                }
                if(holdMsg == "halt"){
                    isHalt = true;
                    break;
                }
            }
        }
    }
    if(!isHalt && !empty){ return true; }
    else{ return false; }
}

//判断我方队伍的半场
void AnalysisGame::judgeSide(const QByteArray& packet){
    ZSS::Protocol::RecMessage recdata;
    recdata.ParseFromArray(packet.data(), packet.size() );
    ZSS::Protocol::Debug_Msgs debugMsgs;
    for(int team = PARAM::BLUE; team <= PARAM::YELLOW; team++){
        debugMsgs = recdata.debugmsgs(team);
        int size = debugMsgs.ByteSize();
        if (team == PARAM::BLUE) {
            GlobalData::instance()->debugBlueMessages.resize(size);
            debugMsgs.SerializeToArray(GlobalData::instance()->debugBlueMessages.data(), size);
            if(debugMsgs.msgs_size() != 0){
                Team = team;
            }
        } else {
            GlobalData::instance()->debugYellowMessages.resize(size);
            debugMsgs.SerializeToArray(GlobalData::instance()->debugYellowMessages.data(), size);
            if(debugMsgs.msgs_size() != 0){
                Team = team;
            }
        }
    }
    for(int j = 0; j < recdata.maintainvision().maintain(Team).robot().size() ; j++){
        if(recdata.maintainvision().maintain(Team).robot(j).posx() > 4800
                && std::fabs(recdata.maintainvision().maintain(Team).robot(j).posy()) < 1200){
            homeSide = Right;
        }
        if(recdata.maintainvision().maintain(Team).robot(j).posx() < -4800
                && std::fabs(recdata.maintainvision().maintain(Team).robot(j).posy()) < 1200){
            homeSide = Left;
        }
    }
    Team == PARAM::BLUE ? opTeam = PARAM::YELLOW : opTeam = PARAM::BLUE;
}

bool AnalysisGame::loadRec(QString &filename){
    replayFile = new QFile(filename);
    packets.clear();
    if(replayFile->open(QIODevice::ReadOnly)){
        QIODevice* recIO;
        recIO = replayFile;
        QDataStream stream(recIO);
        while(!stream.atEnd()){
            QByteArray packet;
            stream >> packet;
            if(cal_RealFrame(packet)) { packets.append(packet); }
        }
        realFrame = packets.size() - 1;
    }
    replayFile->close();
    delete replayFile;
    replayFile = nullptr;
    return true;
}

void AnalysisGame::loadFile(QString filename){
#ifdef WIN32
    filename.remove(QString("file:///"));
#else
    filename.remove(QString("file://"));
#endif
    realFrame = 0 ;
    Team = PARAM::BLUE ;
    homeSide = Left ;
    holdMsg = "";
    if(loadRec(filename)){ qDebug() << "start to analyse !!!!!"; }
    else{ qDebug() << "open file failed !!!!!"; }
    run();
    qDebug() << "finish to analyse !!!!!";
}

void AnalysisGame::run(){
    //开启逐帧分析
    qDebug() << "analysing~~~running~~~start!";
    int analy_realFrame1 = 0;
    int analy_realFrame2 = 10000;
    std::vector<double>our_aggress_x;
    std::vector<double>our_aggress_y;
    std::vector<double>their_aggress_x;
    std::vector<double>their_aggress_y;
    while(true){
        //每10000帧分析一次
        if(analy_realFrame2 > realFrame) { analy_realFrame2 = realFrame; }
//        qDebug() << analy_realFrame1 << "=" << analy_realFrame2 << "————————————";
        for(int currentFrame = analy_realFrame1; currentFrame < analy_realFrame2; currentFrame++) {
                judgeSide(packets.at(currentFrame));
                cal_our_carnum(packets.at(currentFrame));
//                if (currentFrame%100 == 0) { qDebug() << currentFrame; }
        }
        for(int i = 0; i < 6; i++) {
            our_car_num[i] = our_car_num[i] / real_ourframe[i/2];
            their_car_num[i] = their_car_num[i] / real_theirframe[i/2];
        }
        //保存数据
        our_aggress_x.push_back(our_car_num[0]);
        our_aggress_y.push_back(our_car_num[2]);
        their_aggress_x.push_back(their_car_num[0]);
        their_aggress_y.push_back(their_car_num[2]);
        for(int i = 0; i < 6; i++) {
            our_car_num[i] = 0;
            their_car_num[i] = 0;
        }
        qDebug() << analy_realFrame2;
        if(analy_realFrame2 == realFrame) { break; }
        analy_realFrame1 += 10000;
        analy_realFrame2 += 10000;
    }
    //输出数据
    std::cout << "x1 = [";
    for(int i = 0; i < our_aggress_x.size(); i++){
        std::cout << our_aggress_x[i] << ",";
    }
    std::cout << "]" << std::endl;
    std::cout << "y1 = [";
    for(int i = 0; i < our_aggress_y.size(); i++){
        std::cout << our_aggress_y[i] << ",";
    }
    std::cout << "]" << std::endl;
    std::cout << "x2 = [";
    for(int i = 0; i < their_aggress_x.size(); i++){
        std::cout << their_aggress_x[i] << ",";
    }
    std::cout << "]" << std::endl;
    std::cout << "y2 = [";
    for(int i = 0; i < their_aggress_y.size(); i++){
        std::cout << their_aggress_y[i] << ",";
    }
    std::cout << "]" << std::endl;
    /*for(int currentFrame = 0; currentFrame < realFrame; currentFrame++) {
        judgeSide(packets.at(currentFrame));
        cal_our_carnum(packets.at(currentFrame));
        if (currentFrame%100 == 0) { qDebug() << currentFrame; }
    }
    for(int i = 0; i < 6; i++) {
        our_car_num[i] = our_car_num[i] / real_ourframe[i/2];
        their_car_num[i] = their_car_num[i] / real_theirframe[i/2];
    }
    qDebug() << "我方进攻时候的进攻车数 =" << our_car_num[0] << " | 敌方进攻时候的进攻车数 =" << their_car_num[0];
    qDebug() << "我方进攻时候的防守车数 =" << our_car_num[1] << " | 敌方进攻时候的防守车数 =" << their_car_num[1];
    qDebug() << "我方防守时候的进攻车数 =" << our_car_num[2] << " | 敌方防守时候的进攻车数 =" << their_car_num[2];
    qDebug() << "我方防守时候的防守车数 =" << our_car_num[3] << " | 敌方防守时候的防守车数 =" << their_car_num[3];
    qDebug() << "我方争抢时候的进攻车数 =" << our_car_num[4] << " | 敌方争抢时候的进攻车数 =" << their_car_num[4];
    qDebug() << "我方争抢时候的防守车数 =" << our_car_num[5] << " | 敌方争抢时候的防守车数 =" << their_car_num[5];
    qDebug() << realFrame;*/
    qDebug() << "analysing~~~running~~~end!";
}
