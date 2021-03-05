#include "chipsolver.h"
#include "globaldata.h"
#include "ballrecords.h"
#include "Eigen/Dense"
#include "debugengine.h"
#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include "Semaphore.h"

extern Semaphore chipsolverEvent;
std::mutex resetMutex;
std::mutex resetFlagMutex;

using namespace std;
using namespace Eigen ;
namespace {
int testmode = 0; //0 时间戳  1测试
int fitmode = 0; // search kickPos or not
static int flyend = 0;
bool time_flag; // system time or cycle time
bool debug_flag; //  draw debug messege
double secpos_coef;
thread* receiveThread = nullptr;
int discardRecordNum; // the first discardRecordNum records are discard
int minRecordNum; // min record num for use, the first discardRecordNum records are discard
}

int Chipsolver::currentCycle = 0;

Chipsolver::Chipsolver(): timeOffset(0.05), tFly(9999), resetFlag(false), dealResultFlag(false) {
     ZSS::ZParamManager::instance()->loadParam(time_flag,"chipsolver/time_flag",0);
     ZSS::ZParamManager::instance()->loadParam(debug_flag,"chipsolver/debug_flag",0);
     ZSS::ZParamManager::instance()->loadParam(secpos_coef,"chipsolver/secpos_coef",0.5);
     ZSS::ZParamManager::instance()->loadParam(discardRecordNum,"chipsolver/discardRecordNum",10);
     ZSS::ZParamManager::instance()->loadParam(minRecordNum,"chipsolver/minRecordNum",5);
     receiveThread = new std::thread([ = ] {run();});
     receiveThread->detach();
}
Chipsolver::~Chipsolver() {

}

void Chipsolver::run(){
    while(true){
        chipsolverEvent.Wait();
        dealresult();
    }
}

double Chipsolver::chipsolveOffset(double timeOff, double g=9810, int searchFlag=0, double x_start=0, double y_start=0) {
    int recordsNum = GlobalData::instance()->ballrecords.validSize();
    int rowNum = recordsNum - discardRecordNum;
    double l1Norm = 0;
    double tOffset = timeOff;

    int colNum = (searchFlag == 1 ? 3 : 5);

    MatrixXf matA(rowNum * 2, colNum);
    VectorXf b(rowNum * 2);
    high_resolution_clock::time_point t0;
    t0 = GlobalData::instance()->ballrecords.getStart().getcapture();

    int j = 0;

    for(int i = GlobalData::instance()->ballrecords.getstartIndex(); i <= GlobalData::instance()->ballrecords.getstartIndex() + recordsNum - 1; i++) {
        if(i >= GlobalData::instance()->ballrecords.getstartIndex() + 1 && i <= GlobalData::instance()->ballrecords.getstartIndex() + discardRecordNum){
            continue;
        }

        double t_system;
        double t_cycle;

        t_system = GlobalData::instance()->ballrecords[i].timeinterval(t0) * 1e-3 + tOffset;

        t_cycle = GlobalData::instance()->ballrecords[i].getidealTime() + tOffset;

        t = time_flag == 0 ? t_system : t_cycle;

        CVector3 campos = GlobalData::instance()->ballrecords[i].getcampos();
        CVector3 posProject = GlobalData::instance()->ballrecords[i].getpos();
        if(searchFlag == 0){
            matA.row(j) << campos.z(), 0, campos.z()*t, 0, (posProject.x() - campos.x())*t;
            b.row(j) << 0.5 * g*t*t*(posProject.x() - campos.x()) + posProject.x()*campos.z();
            j++;
            matA.row(j) << 0, campos.z(), 0, campos.z()*t, (posProject.y() - campos.y())*t;
            b.row(j) << 0.5 * g*t*t*(posProject.y() - campos.y()) + posProject.y()*campos.z();
            j++;
        } else if(searchFlag == 1){
            matA.row(j) << campos.z()*t, 0, (posProject.x() - campos.x())*t;
            b.row(j) << 0.5*g*t*t*(posProject.x()-campos.x()) + campos.z()*posProject.x() - campos.z()*x_start;
            j++;
            matA.row(j) << 0, campos.z()*t, (posProject.y()-campos.y())*t;
            b.row(j) << 0.5*g*t*t*(posProject.y() - campos.y()) + campos.z()*posProject.x() - campos.z()*y_start;
            j++;
        }
    }

    VectorXf v = matA.colPivHouseholderQr().solve(b);
    VectorXf result = matA * v;
    result -= b;
    if(searchFlag == 0){
        kickVel.formXYZ(v(2), v(3), v(4));
        kickPos.formXYZ(v(0), v(1));
    }else if(searchFlag == 1){
        kickVel.formXYZ(v(0), v(1), v(2));
        kickPos.formXYZ(x_start, y_start);
    }

    for(int i = 0; i < rowNum * 2; i++) {
        l1Norm += abs(result(i));
    }
    return l1Norm/j;
}

double Chipsolver::getbestresult(double g=9810, int searchFlag=0, double x_start=0, double y_start=0) {
    double toff = 0.05;
    double inc = toff / 2;

    //    CVector searchVector = originVel
    if (GlobalData::instance()->ballrecords.validSize() < (minRecordNum + discardRecordNum))
        return -1;

    while(inc > 1e-4) {
        double solveResultNeg = chipsolveOffset(toff - 1e-5, g, searchFlag, x_start, y_start);
        double solveResultPos = chipsolveOffset(toff + 1e-5, g, searchFlag, x_start, y_start);
        if(solveResultNeg > solveResultPos) {
            toff += inc;
        } else {
            toff -= inc;
        }
        inc /= 2;
    }
    //cout<< toff <<endl;
    timeOffset = toff;
    l1Error = chipsolveOffset(toff, g, searchFlag, x_start, y_start);

    if(searchFlag == 1){
        // for every x_start and y_start, we need correspond v vector
        searchKickVel.push_back(kickVel);
    }
    // return l1Error of this kickPos, for evaluate the best kickPos
    return l1Error;
}

bool Chipsolver::dealresult() {
//    qDebug()<< "valid size" << GlobalData::instance()->ballrecords.validSize() ;

    if(GlobalData::instance()->ballrecords.validSize() == 0){
        int teamId = GlobalData::instance()->lastTouch/PARAM::ROBOTNUM;
        int robotId = GlobalData::instance()->lastTouch-teamId*PARAM::ROBOTNUM;
        kickerCenter = GlobalData::instance()->maintain[0].robot[teamId][robotId].pos;
        // the ZHENG vector of robot
        kickerDir = GlobalData::instance()->maintain[0].robot[teamId][robotId].raw_angle;
//        qDebug()<<"teamId"<<teamId<<"robotId"<<robotId;
    }
    if (testmode == 0) setrecord();
    if (t > tFly) {
        flyend++;
        if (flyend == 2) {
            if(debug_flag){
                GDebugEngine::instance()->gui_debug_msg(CGeoPoint(0,200), QString("flytime out").toLatin1(), COLOR_ORANGE);
            }
//            qDebug()<<"reset";
            reset();
            flyend = 0;
        }
    }

    // 避免多线程时ballstate在chipsolver计算时强行reset，强制必须在运行完后通过flag进行reset
    if(resetFlag == true){
        reset();
//        resetFlag = false;
    }

    // resetFlag==true的检测必须在这之前，否则可能会出现其他线程设置resetFlag=true时会被下面这条语句阻塞住，
    // 直到ballrecords到达minRecord时才会进入到reset()，从而导致有效的minRecordNum条数据被清除，最终无法
    // 正确获取及时的数据，也无法绘制出落球点

    //处理二次弹跳
    if(GlobalData::instance()->ballrecords.validSize() <=  (minRecordNum + discardRecordNum)) {
        dealResultFlag = false;
        return false;
    }


    // the centor pos of robot

    CVector originDir(std::cos(kickerDir), std::sin(kickerDir));

    double minLen = PARAM::Vehicle::V2::PLAYER_FRONT_TO_CENTER;
    double maxLen = minLen + minLen/2;
    double minWidth = - (PARAM::Vehicle::V2::PLAYER_MOUTH_WIDTH / 2 - PARAM::Field::BALL_SIZE);
    double maxWidth = PARAM::Vehicle::V2::PLAYER_MOUTH_WIDTH / 2 - PARAM::Field::BALL_SIZE;

    double g = 9810;

    if(fitmode == 1){
        // search kick pos and evaluate the l1 error, both store in seachKickPos vector
        searchkickPos.clear(); // store search pos vector and L1 error
        searchKickVel.clear(); // store v vector correspond to pos vector
        for(double len = minLen; len <= maxLen; len += 5){
            for(double width = minWidth; width <= maxWidth; width += 5){
                // search kick pos
                CGeoPoint tmpKickPos = kickerCenter + originDir*len + originDir.rotate(-PARAM::Math::PI/2.0)*width;
                double tmpL1Error = getbestresult(g, 1, tmpKickPos.x(), tmpKickPos.y());
                searchkickPos.push_back(std::pair<CGeoPoint, double>(tmpKickPos, tmpL1Error)); // save history kick pos and its L1 error
                if(debug_flag)
                    GDebugEngine::instance()->gui_debug_x(tmpKickPos,COLOR_GREEN);
                }
        }

        // pick the best kickPos and kickVel evaluated by L1 Error
        auto minL1ErrorIter = std::min_element(searchkickPos.begin(), searchkickPos.end(), [](std::pair<CGeoPoint, double> a, std::pair<CGeoPoint, double> b) -> bool {return a.second < b.second;});
        _kickPos.formXYZ(minL1ErrorIter->first.x(), minL1ErrorIter->first.y());
        _kickVel = searchKickVel[static_cast<std::vector<CVector3>::size_type>(minL1ErrorIter - searchkickPos.begin())];
    }
    else{
        double tmpL1Error = getbestresult(g, 0);
        _kickPos = kickPos;
        _kickVel = kickVel;
    }

    high_resolution_clock::time_point kickTime;
    kickTime = GlobalData::instance()->ballrecords.getStart().getcapture();

    if (testmode == 0)
        t_system = GlobalData::instance()->ballrecords[0].timeinterval(kickTime) * 1e-3 + timeOffset;
    else
        t_system = GlobalData::instance()->ballrecords[0].getidealTime() + timeOffset;

    t_cycle = getCurrentTime() + timeOffset;

    t = time_flag == 0 ? t_system : t_cycle;

    tFly = _kickVel.z() * 2 / g;

    restTFly = tFly - t;

    // 当前时刻的预测点位置
    posNow = _kickPos.addNew(_kickVel.multiplyNew(t)).addz(-g * 0.5 * t * t);
    // 落地点预测位置
    posEnd = _kickPos.addNew(_kickVel.multiplyNew(tFly)).addz(-g * 0.5 * tFly * tFly);
    posEndQueue.push_back(CGeoPoint(posEnd.x(), posEnd.y()));
    // 落地点预测速度
    velEnd = _kickVel.addz(-g * tFly);
    // 第二落地点
    secondTFly = (secpos_coef * fabs(velEnd.z())) * 2 / g;
    secondPosEnd = CGeoPoint(posEnd.x() + secpos_coef * velEnd.x() * secondTFly, posEnd.y() + secpos_coef * velEnd.y() * secondTFly);
    secondPosEndQueue.push_back(secondPosEnd);
    CGeoPoint p = getposEnd();
//    if(debug_flag){
//        GDebugEngine::instance()->gui_debug_arc(p,60,0,360,COLOR_GREEN); // 挑球预测点
//        GDebugEngine::instance()->gui_debug_arc(secondPosEnd,60,0,360,COLOR_ORANGE); // 第二落地点
//    }


//    cout<<"posEnd"<<"("<< posEnd.x() <<","<<posEnd.y()<<","<<posEnd.z()<<")"<<endl;
    dealResultFlag = true;

    return true;
}

void Chipsolver::setResetFlag(bool flag){
    resetFlagMutex.lock();
    resetFlag = flag;
    resetFlagMutex.unlock();
}

void Chipsolver::reset() {
    currentCycle = 0;

    searchkickPos.clear();
    searchKickVel.clear();

    timeOffset = 0.05;
    secondTFly = 0;
    restTFly = 9999;
    tFly = 9999;
    t = 0;
    t_system = 0;
    t_cycle = 0;
    secondPosEnd.fill(99999, 99999);
    posEnd.formXYZ(99999, 99999, 99999);
    kickPos.formXYZ(99999, 99999, 99999);
    _kickPos.formXYZ(99999, 99999, 99999);
    _kickVel.formXYZ(99999, 99999, 99999);
    posEndQueue.clear();
    secondPosEndQueue.clear();
    bestSecondPos.fill(99999, 99999);
    GlobalData::instance()->ballrecords.clear();
    flyend = 0;
    setResetFlag(false);
}

void Chipsolver::setrecord() {
    currentCycle += 1;
    if (GlobalData::instance()->ballrecords.size() != 50)
        GlobalData::instance()->ballrecords = DataQueue<BallRecord>(50);
    //PARAM::CAMERA
    for (int i = 0; i < PARAM::CAMERA; i++) {
        if(GlobalData::instance()->camera[i][0].ballSize == 1) {
            ballrecord.setpos(GlobalData::instance()->camera[i][0].ball[0].pos);
//            cout << ballrecord.getpos().x() << "," << ballrecord.getpos().y() << endl;
//            cout << i<<","<<GlobalData::instance()->camera[i][0].ball[0].pos.x() << "," << GlobalData::instance()->camera[i][0].ball[0].pos.y() << endl;
            ballrecord.setcampos(GlobalData::instance()->cameraMatrix[i].campos, GlobalData::instance()->cameraMatrix[i].height);
//            cout << GlobalData::instance()->cameraMatrix[i].campos << "," << GlobalData::instance()->cameraMatrix[i].height << endl;


            ballrecord.setidealTime(getCurrentTime() + timeOffset);
            ballrecord.settime();
            ballrecord.setcamID(i);
            if(GlobalData::instance()->ballrecords.getcurrentIndex() == (GlobalData::instance()->ballrecords.size() - 1) && GlobalData::instance()->ballrecords.validSize() == GlobalData::instance()->ballrecords.size()) {
                GlobalData::instance()->ballrecords.renew();
            }
            if(debug_flag){
                GDebugEngine::instance()->gui_debug_arc(CGeoPoint(ballrecord.getcampos().x(),ballrecord.getcampos().y()), 30, 0, 360, COLOR_WHITE);
                GDebugEngine::instance()->gui_debug_msg(CGeoPoint(ballrecord.getcampos().x(),ballrecord.getcampos().y()), QString("cam:%1").arg(ballrecord.getcamID()).toLatin1(), COLOR_WHITE);
                GDebugEngine::instance()->gui_debug_arc(CGeoPoint(ballrecord.getpos().x(),ballrecord.getpos().y()), 30, 0, 360, COLOR_PURPLE);
                GDebugEngine::instance()->gui_debug_msg(CGeoPoint(ballrecord.getpos().x(),ballrecord.getpos().y()), QString("pro").toLatin1(), COLOR_PURPLE);
            }
            GlobalData::instance()->ballrecords.push(ballrecord);
        }
    }
    if(debug_flag){
        drawDebugMsg();
    }
}


CGeoPoint Chipsolver::getBestPredictPos(){
    CGeoPoint lastPos(0,0);
    CGeoPoint nowPos(0,0);
    CGeoPoint nextPos(0,0);
    CGeoPoint tmpPos(0,0);
    CGeoPoint lastTmpPos(99999,99999);
    CGeoPoint bestPos(0,0);
    CGeoPoint tmpSecondPos(0,0);
    CGeoPoint lastTmpSecondPos(99999, 99999);
//    CGeoPoint bestSecondPos(0,0);
    bool flag = false;
//    int cnt = 0;
    double dist = 300;
    for(int i = 0; i < int(posEndQueue.size()); i++){
        if(i == 0)
            continue;
        lastPos = posEndQueue[static_cast<std::vector<CGeoPoint>::size_type>(i-1)];
        nowPos = posEndQueue[static_cast<std::vector<CGeoPoint>::size_type>(i)];
        nextPos = posEndQueue[static_cast<std::vector<CGeoPoint>::size_type>(i+1)];
        if(nowPos.dist(lastPos) > dist || nowPos.dist(nextPos) > dist || lastPos.dist(nextPos) > dist)
            continue;
        tmpPos = nowPos.midPoint(lastPos).midPoint(nowPos.midPoint(nextPos));
        tmpSecondPos = secondPosEndQueue[i].midPoint(secondPosEndQueue[i-1]).midPoint(secondPosEndQueue[i].midPoint(secondPosEndQueue[i+1]));
        if(tmpPos.dist(lastTmpPos) <= dist){
            bestPos = tmpPos.midPoint(lastTmpPos);
            bestSecondPos = tmpSecondPos.midPoint(lastTmpSecondPos);
            flag = true;
        }
        lastTmpPos = tmpPos;
        lastTmpSecondPos = tmpSecondPos;
    }
    if(flag == true)
        return bestPos;
    else
        return CGeoPoint(99999, 99999);
}

// draw t_system and t_cycle, predict ball pos_path(with project pos_path), predict best predict pos
void Chipsolver::drawDebugMsg(){
    GDebugEngine::instance()->gui_debug_arc(CGeoPoint(_kickPos.x(), _kickPos.y()),100,0,360, COLOR_CYAN);
    for(int i = GlobalData::instance()->ballrecords.getstartIndex(); i < 0; i++){
        double t = GlobalData::instance()->ballrecords[i].getidealTime();
        CVector3 pos = _kickPos.addNew(_kickVel.multiplyNew(t)).addz(-9810 * 0.5 * t * t);
        GDebugEngine::instance()->gui_debug_arc(CGeoPoint(pos.x(), pos.y()),30,0,360, COLOR_GRAY);
        GDebugEngine::instance()->gui_debug_msg(CGeoPoint(-1500, 100), QString("t_system: %1").arg(t_system).toLatin1(), COLOR_GREEN);
        GDebugEngine::instance()->gui_debug_msg(CGeoPoint(-1500, -100), QString("t_cycle: %1").arg(t_cycle).toLatin1(), COLOR_GREEN);

        double cam_x = ballrecord.getcampos().x();
        double cam_y = ballrecord.getcampos().y();
        double cam_z = ballrecord.getcampos().z();


        double x_p = (cam_z * pos.x() - pos.z() * cam_x) / (cam_z - pos.z());
        double y_p = (cam_z * pos.y() - pos.z() * cam_y) / (cam_z - pos.z());

        GDebugEngine::instance()->gui_debug_arc(CGeoPoint(x_p, y_p),30,0,360, COLOR_GREEN);

        CGeoPoint p = getposEnd();
        GDebugEngine::instance()->gui_debug_arc(p,100,0,360,COLOR_GREEN);
        GDebugEngine::instance()->gui_debug_arc(p,150,0,360,COLOR_GREEN);

        CGeoPoint _predict_pos = getBestPredictPos();
        CGeoPoint fail_pos(99999, 99999);
        if(!(_predict_pos == fail_pos)){
            GDebugEngine::instance()->gui_debug_arc(p,100,0,360,COLOR_RED);
            GDebugEngine::instance()->gui_debug_arc(p,150,0,360,COLOR_RED);
            GDebugEngine::instance()->gui_debug_arc(bestSecondPos,90,0,360,COLOR_ORANGE);
        }

    }
}
