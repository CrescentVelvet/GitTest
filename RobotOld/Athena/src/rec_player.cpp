﻿#include "rec_player.h"
#include "field.h"
#include "zss_rec.pb.h"
#include "globaldata.h"
#include "zss_debug.pb.h"
#include "globalsettings.h"
#include <QIODevice>
#include <QDataStream>
#include <QTime>
namespace  {
auto GS = GlobalSettings::instance();
}
RecPlayer::RecPlayer() {
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

bool RecPlayer::loadRec(QString &filename, int &maxframe) {
    replayFile = new QFile(filename);
    packets.clear();
    if (replayFile->open(QIODevice::ReadOnly)) {
        QIODevice* recIO;
        recIO = replayFile;
        QDataStream stream(recIO);
        while (!stream.atEnd()) {
            QByteArray packet;
            stream >> packet;
            packets.append(packet);
        }
        maxframe = packets.size() - 1;
//        replayFile->close();
        delete replayFile;
        replayFile = nullptr;
        return true;
    }
    qDebug() << "open filed";
    return false;
}

bool RecPlayer::start(int position) {
    if (position > packets.size() - 1) {
        return false;
    }

    _currentFrame = position;

    _run = true;
    QThread::start();

    return true;
}

void RecPlayer::slowly(int position) {
    if (position > packets.size() - 1) {
        return;
    }

    _currentFrame = position;

    QThread::start();
}

void RecPlayer::stop() {
    _run = false;
    QThread::wait();
}

bool RecPlayer::good() {
    return packets.size() > 0;
}

void RecPlayer::sendMessage(const QByteArray& packet) {
//    qDebug() << "sendmessage is called" ;
    ZSS::Protocol::RecMessage recMsg;
    recMsg.ParseFromArray(packet.data(), packet.size());
    //ctrlc
    GlobalData::instance()->ctrlCMutex.lock();
    GlobalData::instance()->ctrlC = recMsg.ctrlc();
    GlobalData::instance()->ctrlCMutex.unlock();
    //selectedArea
    GS->maximumX = recMsg.selectedarea().maxx();
    GS->minimumX = recMsg.selectedarea().minx();
    GS->maximumY = recMsg.selectedarea().maxy();
    GS->minimumY = recMsg.selectedarea().miny();
    //maintainVision
    GlobalData::instance()->lastTouch = recMsg.maintainvision().lasttouchteam() == PARAM::BLUE ? recMsg.maintainvision().lasttouch() : recMsg.maintainvision().lasttouch() + PARAM::ROBOTMAXID;
    ReceiveVisionMessage result;
    for(int color = PARAM::BLUE; color <= PARAM::YELLOW; color++) {
//        processMsg = recMsg->mutable_maintainvision()->add_processmsg();
//        processMsg->set_size(robot_vision.robotSize[color]);
        for(int j = 0; j < recMsg.maintainvision().processmsg(color).size(); j++) {
            result.addRobot(color,
                            recMsg.maintainvision().processmsg(color).robot(j).id(),
                            recMsg.maintainvision().processmsg(color).robot(j).posx(),
                            recMsg.maintainvision().processmsg(color).robot(j).posy(),
                            recMsg.maintainvision().processmsg(color).robot(j).angle());
        }
    }
    GlobalData::instance()->processRobot.push(result);
//    qDebug() << "FUCKING MSG" << GlobalData::instance()->processRobot[0].robotSize[0];
//    auto& maintainMsg = GlobalData::instance()->maintain[0];
    ReceiveVisionMessage maintainResult;
    for(int color = PARAM::BLUE; color <= PARAM::YELLOW; color++) {
//        maintain = recMsg->mutable_maintainvision()->add_maintain();
//        maintain->set_size(maintainMsg.robotSize[color]);
        for(int j = 0; j < recMsg.maintainvision().maintain(color).size(); j++) {
            maintainResult.addRobot(color,
                                    recMsg.maintainvision().maintain(color).robot(j).id(),
                                    recMsg.maintainvision().maintain(color).robot(j).posx(),
                                    recMsg.maintainvision().maintain(color).robot(j).posy(),
                                    recMsg.maintainvision().maintain(color).robot(j).angle());
        }
    }
//    recMsg->mutable_maintainvision()->mutable_balls()->set_size(maintainMsg.ballSize);
    for(int j = 0; j < recMsg.maintainvision().balls().size(); j++) {
        maintainResult.addBall(recMsg.maintainvision().balls().ball(j).posx(),
                               recMsg.maintainvision().balls().ball(j).posy());
        maintainResult.ball[j].valid = recMsg.maintainvision().balls().ball(j).valid();
    }
    GlobalData::instance()->maintain.push(maintainResult);

    //debugMsgs
    GlobalData::instance()->debugMutex.lock();
    ZSS::Protocol::Debug_Msgs debugMsgs;
    for (int team = PARAM::BLUE; team <= PARAM::YELLOW; team++) {
        debugMsgs = recMsg.debugmsgs(team);
        int size = debugMsgs.ByteSize();
        if (team == 0) {
            GlobalData::instance()->debugBlueMessages.resize(size);
            debugMsgs.SerializeToArray(GlobalData::instance()->debugBlueMessages.data(), size);
//            std::cout << debugMsgs.msgs_size() << std::endl;
        } else {
            GlobalData::instance()->debugYellowMessages.resize(size);
            debugMsgs.SerializeToArray(GlobalData::instance()->debugYellowMessages.data(), size);
//            std::cout << debugMsgs.msgs_size() << std::endl;
        }
//        qDebug() << "FUCK DEBUG MESSAGE SIZE" <<  debugMsgs.ByteSize() << GlobalData::instance()->debugBlueMessages.size();
    }
    GlobalData::instance()->debugMutex.unlock();
}

void RecPlayer::run() {
    QTime timer;
    timer.start();
    sendMessage(packets.at(_currentFrame));
    emit positionChanged(_currentFrame);
    GlobalSettings::instance()->needRepaint();
    qDebug() << "start~~~analysing~~~running~~~";
    QThread::currentThread()->msleep(12);
//    qDebug() << _currentFrame;
    judgeSide(packets.at(_currentFrame));
    cal_our_carnum(packets.at(_currentFrame));
//    qDebug() << _Team << "---" << _opTeam;
    qDebug() << "our attack in ourball    =" << our_car_num[0] << " | their attack in theirball  =" << their_car_num[0];
    qDebug() << "our defense in ourball   =" << our_car_num[1] << " | their defense in theirball =" << their_car_num[1];
    qDebug() << "our attack in theirball  =" << our_car_num[2] << " | their attack in ourball    =" << their_car_num[2];
    qDebug() << "our defense in theirball =" << our_car_num[3] << " | their defense in ourball   =" << their_car_num[3];
    qDebug() << "our attack in bothball   =" << our_car_num[4] << " | their attack in bothball   =" << their_car_num[4];
    qDebug() << "our defense in bothball  =" << our_car_num[5] << " | their defense in bothball  =" << their_car_num[5];
    qDebug() << _currentFrame;
    for (int i = 0; i < 6; i++) {
        our_car_num[i] = 0;
        their_car_num[i] = 0;
    }
    qDebug() << "end~~~analysing~~~running~~~";
    /*while (_run && ++_currentFrame < packets.size() - 1 && this->isRunning()) {
//        ourHoldRate();
        sendMessage(packets.at(_currentFrame));
        emit positionChanged(_currentFrame);
        GlobalSettings::instance()->needRepaint();
        QThread::currentThread()->msleep(12);
    }*/
}

CVector RecPlayer::Polar2Vector(double m,double angle){
    return CVector(m*std::cos(angle),m*std::sin(angle));
}

CGeoPoint RecPlayer::analy_TheirInterPoint(CGeoPoint enemy, CGeoPoint ball){
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

void RecPlayer::judgeSide(const QByteArray& packet){
    ZSS::Protocol::RecMessage recdata;
    recdata.ParseFromArray(packet.data(), packet.size());
    ZSS::Protocol::Debug_Msgs debugMsgs;
    for(int team = PARAM::BLUE; team <= PARAM::YELLOW; team++){
        debugMsgs = recdata.debugmsgs(team);
        int size = debugMsgs.ByteSize();
        if (team == PARAM::BLUE) {
            GlobalData::instance()->debugBlueMessages.resize(size);
            debugMsgs.SerializeToArray(GlobalData::instance()->debugBlueMessages.data(), size);
            if(debugMsgs.msgs_size() != 0){
                _Team = team;
            }
        } else {
            GlobalData::instance()->debugYellowMessages.resize(size);
            debugMsgs.SerializeToArray(GlobalData::instance()->debugYellowMessages.data(), size);
            if(debugMsgs.msgs_size() != 0){
                _Team = team;
            }
        }
    }
    for(int j = 0; j < recdata.maintainvision().maintain(_Team).robot().size() ; j++){
        if(recdata.maintainvision().maintain(_Team).robot(j).posx() > 4800
                && std::fabs(recdata.maintainvision().maintain(_Team).robot(j).posy()) < 1200){
            _homeSide = Right;
        }
        if(recdata.maintainvision().maintain(_Team).robot(j).posx() < -4800
                && std::fabs(recdata.maintainvision().maintain(_Team).robot(j).posy()) < 1200){
            _homeSide = Left;
        }
    }
    _Team == PARAM::BLUE ? _opTeam = PARAM::YELLOW : _opTeam = PARAM::BLUE;
}

bool RecPlayer::analy_marking(const QByteArray& packet, int team, int num){
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
    if(team == _Team){ side = _homeSide; }
    else if(team == _opTeam){ side = (_homeSide == Right ? Left : Right); }
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

bool RecPlayer::analy_guarding(const QByteArray& packet, int team, int num){
    ZSS::Protocol::RecMessage recdata;
    recdata.ParseFromArray(packet.data(), packet.size());
    int opteam = 1 - team;
    int x = recdata.maintainvision().maintain(team).robot(num).posx();
    int y = recdata.maintainvision().maintain(team).robot(num).posy();
    int side;
    if(team == _Team){ side = _homeSide; }
    else if(team == _opTeam){ side = (_homeSide == Right ? Left : Right); }
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

bool RecPlayer::analy_goalkeeping(const QByteArray& packet, int team, int num){
    ZSS::Protocol::RecMessage recdata;
    recdata.ParseFromArray(packet.data(), packet.size());
    int opteam = 1 - team;
    int x = recdata.maintainvision().maintain(team).robot(num).posx();
    int y = recdata.maintainvision().maintain(team).robot(num).posy();
    int side;
    if(team == _Team){ side = _homeSide; }
    else if(team == _opTeam){ side = (_homeSide == Right ? Left : Right); }
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

void RecPlayer::cal_our_carnum(const QByteArray& packet){
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
    int team = _Team;
    int opteam = _opTeam;
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
