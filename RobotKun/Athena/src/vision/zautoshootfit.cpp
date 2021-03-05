#include "zautoshootfit.h"
#include "visionmodule.h"
#include "globaldata.h"
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/Eigenvalues>
#include <thread>
#include "parammanager.h"
#include <QIODevice>
#include <QFile>
#include "zss_kickparam.pb.h"
#include "staticparams.h"
#include <thread>
#include "chipsolver.h"
#include "globaldata.h"
#include "maintain.h"
#include "debugengine.h"
#include <fstream>>

namespace  {
    auto zpm = ZSS::ZParamManager::instance();
    std::thread* receiveThread;


    //udpsend param
    bool send = false;
    bool sendAll = false;
    int team;
    int id;


    //test mode
    double send_power;

    //kick param
    const int max_record_num = 5;
    bool flatparamtest;
    bool chipparamtest;
    bool isWaiting = true;
    bool isGettingVel = false;
    bool isGettingDist = false;
    bool isChanged[PARAM::ROBOTNUM*PARAM::TEAMS] = {false};
    bool calculated[PARAM::ROBOTNUM] = {false};
    double vel[PARAM::ROBOTNUM][max_record_num];
    double dist[PARAM::ROBOTNUM][max_record_num];
    int p[PARAM::ROBOTNUM][max_record_num];
    int cnt[PARAM::Field::MAX_PLAYER]={0};
    double max_vel[3];
    int cycle_cnt = 0;
    int power = -1;
    int kicker_team;
    bool mode;
    int i;
    CGeoPoint chip_robot_pos;
}

CAutoShootFit::CAutoShootFit()
{

    receiveThread = new std::thread([ = ] {receiveKickParam();});
    receiveThread->detach();
    zpm->loadParam(_isrun,"ZAutoFit/isrun",false);
    zpm->loadParam(flatparamtest, "ZAutoFit/flatparamtest", false);
    zpm->loadParam(chipparamtest, "ZAutoFit/chipparamtest", false);
    zpm->loadParam(send, "ZAutoFit/send", false);
    zpm->loadParam(sendAll, "ZAutoFit/sendAll", false);
    zpm->loadParam(team, "ZAutoFit/team", 0);
    zpm->loadParam(id, "ZAutoFit/id", 0);

    readAllKickParam();

    sendSocket.setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
}

CAutoShootFit::~CAutoShootFit() {
    sendSocket.disconnectFromHost();
    receiveSocket.disconnectFromHost();
}

void CAutoShootFit::run() {
//    qDebug() << "??????";
    test();
    if (flatparamtest) {
        GDebugEngine::instance()->gui_debug_msg(CGeoPoint(0,0),QString("FlatPARAMTEST is running").toLatin1());
        flatParamTest();
    }

    if (chipparamtest) {
        GDebugEngine::instance()->gui_debug_msg(CGeoPoint(0,0),QString("ChipPARAMTEST is running").toLatin1());
        chipParamTest();
    }

    if (chipparamtest || flatparamtest) {
        for (int i=0; i<PARAM::ROBOTNUM; i++) {
            if(cnt[i] == max_record_num && calculated[i] == false) {
            calculate(i);
            calculated[i] = true;
            }
        }
    }   

    if (send){
        udpSend();
    }
}

void CAutoShootFit::reset() {


    isWaiting = true;
    isGettingVel = false;
    isGettingDist = false;
    power = -1;
    cycle_cnt = 0;
    kicker_team = -1;
    i = -1;
}

void CAutoShootFit::flatParamTest() {
//    qDebug() << "flat";
//    bool isFirstKick = GlobalData::instance()->robotInformation[kicker_team][i].flat && !lastStatus;

//    if (lastStatus != GlobalData::instance()->robotInformation[kicker_team][i].flat) {
//        lastStatus = false;
//    }

//    if (!isWaiting && isFirstKick) {
//        lastStatus = true;
//        isGettingDist = true;
//    }

//   if(GlobalData::instance()->robotInformation[kicker_team][i].flat) qDebug() << "iswaiting" << isWaiting << "isgettingvel" << isGettingVel << "flat flag" << GlobalData::instance()->robotInformation[kicker_team][i].flat;
    if(!isWaiting && GlobalData::instance()->robotInformation[kicker_team][i].flat) {
        isGettingVel = true;
        isWaiting = false;
    }
//    qDebug() << "wait & getvel" <<isWaiting << isGettingVel;
    if (isGettingVel) {
        qDebug() << "isgettingvel" << isWaiting;
        double v = getVel();
        if (v > 0) {
            if (cnt[i] < max_record_num) {
                p[i][cnt[i]] = power;
                vel[i][cnt[i]] = v;
                cnt[i]++;
                recordKickData(i, false, power, send_power, v);
                qDebug() << "data "<< "number: " << i << "cnt:" << cnt[i] << "p:" << power << "vel:" << v;
            }
            reset();
        }
    }
}

void CAutoShootFit::chipParamTest() {

    double lastPreditcDist;
    if (!isWaiting && GlobalData::instance()->robotInformation[kicker_team][i].chip) {
        isGettingDist = true;
        const OriginMessage &robot_vision = GlobalData::instance()->processRobot[0];
        chip_robot_pos = robot_vision.robot[kicker_team][i].pos;
    }

    if (isGettingDist) {
        lastPreditcDist = getDist();
        if (lastPreditcDist > 0 ) {
            if(cnt[i] < max_record_num) {
                p[i][cnt[i]] = power;
                dist[i][cnt[i]] = lastPreditcDist;
                cnt[i]++;
                recordKickData(i, false, power, send_power, lastPreditcDist);
                qDebug() << "data "<< "number: " << i << "cnt:" << cnt[i] << "p:" << power << "dist" << lastPreditcDist;
            }
            reset();
        }
//        qDebug() << "predictdist" << predict_dist;
    }
}

void CAutoShootFit::autoFit() {

    //  a b c
//    truePower
//    trueBallVel


}

void CAutoShootFit::udpSend(){

    if (sendAll) {
        sendAllKickParam();
    } else {
        sendKickParam(team, id);
    }
}

void CAutoShootFit::sendAllKickParam() {
    for (int t=0; t<PARAM::TEAMS; t++) {
        for (int i=0; i<PARAM::ROBOTNUM; i++) {
            sendKickParam(t, i);
        }
    }
}

void CAutoShootFit::sendKickParam(int team, int id) {

      ZSS::Protocol::Robot_Param robot_param;
      auto flat_param = robot_param.mutable_flatparam();
      auto chip_param = robot_param.mutable_chipparam();
      auto robot = robot_param.mutable_robot();
      robot->set_team(team);
      robot->set_id(id);
      KickParam kp = kick_param[id+team*PARAM::ROBOTNUM];
      chip_param->set_ca(kp.ca);
      chip_param->set_cb(kp.cb);
      chip_param->set_cc(kp.cc);
      chip_param->set_cmax(kp.cMax);
      chip_param->set_cmin(kp.cMin);
      flat_param->set_fa(kp.fa);
      flat_param->set_fb(kp.fb);
      flat_param->set_fc(kp.fc);
      flat_param->set_fmax(kp.fMax);
      flat_param->set_fmin(kp.fMin);
      int size = robot_param.ByteSize();
      QByteArray datagram(size, 0);
      robot_param.SerializeToArray(datagram.data(), size);
      sendSocket.writeDatagram(datagram.data(), size, QHostAddress(ZSS::KICKPARAM_ADDRESS), ZSS::Athena::FIT_SYNC);
}

void CAutoShootFit::receiveKickParam() {

    receiveSocket.bind(QHostAddress::AnyIPv4, ZSS::Athena::FIT_SYNC, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    receiveSocket.joinMulticastGroup(QHostAddress(ZSS::KICKPARAM_ADDRESS));
    QByteArray datagram;
    ZSS::Protocol::Robot_Param robot_param;

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
//        qDebug() << "WTF"<< receiveSocket.state() << QUdpSocket::BoundState <<receiveSocket.hasPendingDatagrams();
        if (receiveSocket.state() == QUdpSocket::BoundState && receiveSocket.hasPendingDatagrams()) {
//            qDebug() << "HERE";
            datagram.resize(receiveSocket.pendingDatagramSize());
            receiveSocket.readDatagram(datagram.data(), datagram.size());
            robot_param.ParseFromArray(datagram, datagram.size());
            auto& flat_param = robot_param.flatparam();
            auto& chip_param = robot_param.chipparam();
            auto& robot = robot_param.robot();

            KickParam kp(robot.team(), robot.id(), chip_param.ca(), chip_param.cb(), chip_param.cc(), chip_param.cmax(),chip_param.cmin(),flat_param.fa(), flat_param.fb(), flat_param.fc(), flat_param.fmax(), flat_param.fmin());
            if (kp.team == 1) {
                int id;
//                id = kp.id - 16;
                id = kp.id;
            } else {
                id = kp.id;
            }

            if (!isChanged[id]) {
                qDebug() << id;
                qDebug() << "receive&change" << "team" << robot.team() << "id" << robot.id() << "ca" << chip_param.ca() << "fa" << flat_param.fa();
                changeParam(kp);
                isChanged[id] = true;
            }
        }
    }
}

void CAutoShootFit::test() {

    if(!isWaiting && GlobalData::instance()->robotInformation[kicker_team][i].flat) {
        isGettingVel = true;
        isWaiting = false;
    }
    if (isGettingVel) {
        qDebug() << "isgettingvel" << isWaiting;
        double v = getVel();
        if (v > 0) {
            recordKickData(i, false, power, send_power, v);
//            if (cnt[i] < max_record_num) {
//                p[i][cnt[i]] = power;
//                vel[i][cnt[i]] = v;
//                cnt[i]++;
//                qDebug() << "data "<< "number: " << i << "cnt:" << cnt[i] << "p:" << power << "vel:" << v;
//            }
            reset();
        }
    }

    double lastPreditcDist;
    if (!isWaiting && GlobalData::instance()->robotInformation[kicker_team][i].chip) {
        isGettingDist = true;
        const OriginMessage &robot_vision = GlobalData::instance()->processRobot[0];
        chip_robot_pos = robot_vision.robot[kicker_team][i].pos;
    }

    if (isGettingDist) {
        lastPreditcDist = getDist();
        if (lastPreditcDist > 0 ) {
            recordKickData(i, true, power, send_power,lastPreditcDist);
//            if(cnt[i] < max_record_num) {
//                p[i][cnt[i]] = power;
//                dist[i][cnt[i]] = lastPreditcDist;
//                cnt[i]++;
//                qDebug() << "data "<< "number: " << i << "cnt:" << cnt[i] << "p:" << power << "dist" << lastPreditcDist;
//            }
            reset();
        }
    }
}
void CAutoShootFit::getKickerMes(int t, int n, bool m/*, quint8 p*/) {
//    qDebug() << "getkickermes" << kicker_team << i << mode;
    kicker_team = t;
    i = n;
//    power = p;
    mode = m;
    isWaiting = false;
}

void CAutoShootFit::getKickPower(int p, double sp) {
    //qDebug() << "getpower" << power;
    power = p;
    send_power = sp;
}

double CAutoShootFit::getDist() {

    bool isChip =  GlobalData::instance()->maintain[0].ball[0].ball_state_machine == 10;
    static bool lastStatus = false;
    if (isChip) {
        lastStatus = true;
    }

    if (lastStatus && !isChip) {
        CGeoPoint chip_pos = GlobalData::instance()->maintain[0].ball[0].pos;
        double dist = chip_pos.dist(chip_robot_pos);
        if (abs(chip_pos.x()) < PARAM::Field::PITCH_LENGTH/2 && abs(chip_pos.y()) < PARAM::Field::PITCH_WIDTH/2 ) {
            lastStatus = false;
            GDebugEngine::instance()->gui_debug_msg(CGeoPoint(-1000,0),QString("%1").arg(dist).toLatin1());
            return dist;
        }
    }

    return -1.0;
}

double CAutoShootFit::getVel() {

    double maxv = -1.0;
    double v = GlobalData::instance()->maintain[0].ball->velocity.mod();

    if (v > max_vel[2] and v < max_vel[1]) {
        max_vel[2] = v;
    }
    else if (v > max_vel[1] and v < max_vel[0]) {
        max_vel[2] = max_vel[1];
        max_vel[1] = v;
    }
    else if (v > max_vel[0]) {
        max_vel[2] = max_vel[1];
        max_vel[1] = max_vel[0];
        max_vel[0] = v;
    }
    qDebug() << "vel:" << v << "cycle" << cycle_cnt;
    cycle_cnt ++;

    if(cycle_cnt == 30) {
        maxv= (max_vel[0] + max_vel[1] + max_vel[2]) / 3;
        if(maxv>0){

        }
        else {
            qDebug() << "error when getvel";
        }
        for (int i=0; i<3; i++) {
            max_vel[i] = -1;
        }
    }

    return maxv;
}

void CAutoShootFit::calculate( int i ) {

    qDebug() << "calculate" << "id" << i;

    int N_order_fit = 2;
    bool mode = chipparamtest;
    Eigen::MatrixXd m(cnt[i], N_order_fit+1);
    for ( int index = 0; index < cnt[i]; ++index) {
        for (int n = N_order_fit, dex = 0; n >= 1; --n, ++dex) {
            m(index, dex) = pow(mode ? dist[i][index] : vel[i][index], n);
        }
        m(index, N_order_fit) = 1;
    }
    qDebug() << "vel or dist";
    for (int j=0; j<max_record_num; j++) {
        qDebug() <<m(j,0) << m(j,1) << m(j,2);
    }

    Eigen::MatrixXd mp(cnt[i], 1);
    for ( int index = 0; index < cnt[i]; ++index) {
        mp(index, 0) = p[i][index];
    }

    qDebug() << "p:";
    for (int j=0; j<max_record_num; j++) {
        qDebug() << mp(j,0);
    }

    Eigen::MatrixXd mparam(N_order_fit,1);

    mparam = (m.transpose() * m).inverse() * m.transpose() * mp;
    for (int j=0; j<3; j++) {
        qDebug() << "mparam:" << mparam(j,0);
    }

//    if (kicker_team==1) i += 16;
    QString robot_id = QString::number(i);
    QString key = "";
    double radio = mode ? 1: 100000;
    key = QString("Robot%1/%2_A").arg(i).arg(mode ? "CHIP" : "FLAT");
    ZSS::KParamManager::instance()->changeParam(key, mparam(0,0)*radio);
    key = QString("Robot%1/%2_B").arg(i).arg(mode ? "CHIP" : "FLAT");
    ZSS::KParamManager::instance()->changeParam(key, mparam(1,0));
    key = QString("Robot%1/%2_C").arg(i).arg(mode ? "CHIP" : "FLAT");
    ZSS::KParamManager::instance()->changeParam(key, mparam(2,0));

    key = QString("Robot%1/%2_MAX").arg(i).arg(mode ? "CHIP" : "FLAT");
    int maxpower = int(mparam(0,0) * mparam(0,0)*6500 + mparam(1,0) *6500 + mparam(2,0));
    if (maxpower > 127) maxpower = 125;
    qDebug() << maxpower;
    ZSS::KParamManager::instance()->changeParam(key, maxpower);
}

void CAutoShootFit::changeParam(KickParam kp) {  //clear


    if (kp.team == 1) {
        int id;
        id = kp.id - 16;
    } else {
        id = kp.id;
    }
//    qDebug() << "change" << "team" << kp.team << "id" << kp.id << "ca" << kp.ca << "fa" << kp.fa;
    QString key = "";
    key = QString("Robot%1/CHIP_A").arg(id);
    ZSS::KParamManager::instance()->changeParam(key, kp.ca);
    key = QString("Robot%1/CHIP_B").arg(id);
    ZSS::KParamManager::instance()->changeParam(key, kp.cb);
    key = QString("Robot%1/CHIP_C").arg(id);
    ZSS::KParamManager::instance()->changeParam(key, kp.cc);
    key = QString("Robot%1/CHIP_MAX").arg(id);
    ZSS::KParamManager::instance()->changeParam(key, kp.cMax);
    key = QString("Robot%1/CHIP_MIN").arg(id);
    ZSS::KParamManager::instance()->changeParam(key, kp.cMin);
    key = QString("Robot%1/FLAT_A").arg(id);
    ZSS::KParamManager::instance()->changeParam(key, kp.fa);
    key = QString("Robot%1/FLAT_B").arg(id);
    ZSS::KParamManager::instance()->changeParam(key, kp.fb);
    key = QString("Robot%1/FLAT_C").arg(id);
    ZSS::KParamManager::instance()->changeParam(key, kp.fc);
    key = QString("Robot%1/FLAT_MAX").arg(id);
    ZSS::KParamManager::instance()->changeParam(key, kp.fMax);
    key = QString("Robot%1/FLAT_MIN").arg(id);
    ZSS::KParamManager::instance()->changeParam(key, kp.fMin);
}

CAutoShootFit::KickParam CAutoShootFit::getParam(int team, int id) {   //clear

    CAutoShootFit::KickParam kp;
    kp.team = team;
    kp.id = id;
//    if (team==1) {
//        id = id+16;
//    }
    QString key = "";
    key = QString("Robot%1/CHIP_A").arg(id);
    ZSS::KParamManager::instance()->loadParam(kp.ca, key, 0);
    key = QString("Robot%1/CHIP_B").arg(id);
    ZSS::KParamManager::instance()->loadParam(kp.cb, key, 0);
    key = QString("Robot%1/CHIP_C").arg(id);
    ZSS::KParamManager::instance()->loadParam(kp.cc, key, 0);
    key = QString("Robot%1/CHIP_MAX").arg(id);
    ZSS::KParamManager::instance()->loadParam(kp.cMax, key, 0);
    key = QString("Robot%1/CHIP_MIN").arg(id);
    ZSS::KParamManager::instance()->loadParam(kp.cMin, key, 0);
    key = QString("Robot%1/FLAT_A").arg(id);
    ZSS::KParamManager::instance()->loadParam(kp.fa, key, 0);
    key = QString("Robot%1/FLAT_B").arg(id);
    ZSS::KParamManager::instance()->loadParam(kp.fb, key, 0);
    key = QString("Robot%1/FLAT_C").arg(id);
    ZSS::KParamManager::instance()->loadParam(kp.fc, key, 0);
    key = QString("Robot%1/FLAT_MAX").arg(id);
    ZSS::KParamManager::instance()->loadParam(kp.fMax, key, 0);
    key = QString("Robot%1/FLAT_MIN").arg(id);
    ZSS::KParamManager::instance()->loadParam(kp.fMin, key, 0);
    return kp;
}

void CAutoShootFit::readAllKickParam(){  //clear
    for (int t=0; t<PARAM::TEAMS; t++) {
        for (int i=0; i<PARAM::ROBOTNUM; i++) {
            kick_param[i+t*PARAM::ROBOTNUM] = getParam(t, i);
//            qDebug() << kick_param[i+t*PARAM::ROBOTNUM].fb << i+t*PARAM::ROBOTNUM;
        }
    }
}

void CAutoShootFit::recordKickData(int id, bool mode, int power, double sp,double vel_or_dist) {
    std::ofstream ratio_file("./data/KickData.txt", std::ios::app);
    if(ratio_file.is_open()){
        int m = mode ? 1 : 0;
        ratio_file << id << " " << m << " " << power << " " << sp << " " << vel_or_dist << std::endl;
        ratio_file.close();
    }
}
