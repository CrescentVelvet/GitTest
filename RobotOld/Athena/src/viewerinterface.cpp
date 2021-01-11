#include <QAbstractListModel>
#include <cstring>
#include <thread>
#include "viewerinterface.h"

auto xpm = ZSS::ZParamManager::instance();
double VIEW_DISPLAY_FREQUENCE;
void ViewerInterface::changeRobotInfo(int team,int id){
    //电量判断
//    bool judge_battery = false;
    double robot_battery_new = GlobalData::instance()->robotInformation[team][id].battery;
    if (robot_battery_new - robot_battery_old[team+id*PARAM::TEAMS] >= 0.1){
        judge_battery = true;
        robot_battery_old[team+id*PARAM::TEAMS] = robot_battery_new;
    }

    //时间判断
//    bool judge_time = false;
    QDateTime UTC(QDateTime::currentDateTimeUtc());
    qint64 min_sec_new = UTC.toMSecsSinceEpoch();
    if (min_sec_new - min_sec_old[team+id*PARAM::TEAMS] >= 1000 * VIEW_DISPLAY_FREQUENCE){
        judge_time = true;
        min_sec_old[team+id*PARAM::TEAMS] = min_sec_new;
    }
}

//    qDebug() << " team =" << team << " id =" << id;
//    qDebug() << " judge_time =" << judge_time;
//    qDebug() << " judge_inexist =" << judge_inexist << " judge_battery =" << judge_battery << " judge_emit =" << judge_emit;
//}

    /*//2019.9.30电量显示
    QDateTime UTC(QDateTime::currentDateTimeUtc());
    qint64 min_sec_new = UTC.toMSecsSinceEpoch();
//    每隔VIEW_DISPLAY_FREQUENCE秒刷新一次UI界面
    if ((min_sec_new - min_sec_old[team+id*PARAM::TEAMS]) >= 1000 * VIEW_DISPLAY_FREQUENCE)
    {
//        qDebug() << "update!!!!!!!!!!!!!!!!!!!!!!";
        emit dataChanged(createIndex(team+id*PARAM::TEAMS,0),createIndex(team+id*PARAM::TEAMS,0));
        min_sec_old[team+id*PARAM::TEAMS] = min_sec_new;
//        debug信息
//        qDebug() << " team =" << team << " id =" << id;
//        qDebug() << " judge_inexist =" << judge_inexist;
    }
}*/

ViewerInterface::ViewerInterface(QObject *parent){
    /*收到receive信息才改变显示图像change*/
    xpm->loadParam(VIEW_DISPLAY_FREQUENCE, "Debug/VIEW_DISPLAY_FREQUENCE", 0.01);
    std::thread* receiveThread1 = new std::thread([ = ] {view_display();});
    receiveThread1->detach();
    std::thread* receiveThread2 = new std::thread([ = ] {emit_display();});
    receiveThread2->detach();
    QObject::connect(ZSS::ZActionModule::instance(),SIGNAL(receiveRobotInfo(int,int)),this,SLOT(changeRobotInfo(int,int)));
    QObject::connect(ZSS::ZSimModule::instance(),SIGNAL(receiveSimInfo(int,int)),this,SLOT(changeRobotInfo(int,int)));
}

void ViewerInterface::emit_display(){
    //总判断
    while(true){
//        std::this_thread::sleep_for(std::chrono::microseconds(1000));
//        bool judge_emit = false;
//        judge_emit = (judge_battery || judge_inexist) && judge_time;
        judge_emit = judge_time;
        if (judge_emit){
            for (int team = PARAM::BLUE; team <= PARAM::YELLOW; team++){
                for (int id = 0; id < PARAM::ROBOTNUM; id++){
                    emit dataChanged(createIndex(team+id*PARAM::TEAMS,0),createIndex(team+id*PARAM::TEAMS,0));
                }
            }
            judge_battery = false;
            judge_inexist = false;
            judge_time = false;
        }
    }
}

void ViewerInterface::view_display(){
////把actionmodule里的inexist放进此处，提高效率
////当inexist变化，或battery变化量大，且time经过一秒以上，才emit dataChanged
    bool judge_inexist_new[PARAM::ROBOTNUM*PARAM::TEAMS];
    while(true){
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
//        memset(judge_inexist_new, false, sizeof(judge_inexist_new));
        for (int team = PARAM::BLUE; team <= PARAM::YELLOW; team++){
            for (int id = 0; id < PARAM::ROBOTNUM; id++){
                judge_inexist_new[team+id*PARAM::TEAMS] = false;
                for (int i = 0; i < GlobalData::instance()->processRobot[0].robotSize[team]; i++){
                    auto& robot = GlobalData::instance()->processRobot[0].robot[team][i];
                    if (robot.id == id){
                        judge_inexist_new[team+id*PARAM::TEAMS] = true;
                        break;
                    }
                }
            }
        }
        for (int team = PARAM::BLUE; team <= PARAM::YELLOW; team++){
            for (int id = 0; id < PARAM::ROBOTNUM; id++){
                if (judge_inexist_old[team+id*PARAM::TEAMS] != judge_inexist_new[team+id*PARAM::TEAMS]){
                    judge_inexist = true;
//                    qDebug() << "-------" << id;
                }
                GlobalData::instance()->robotInformation[team][id].inexist = judge_inexist_new[team+id*PARAM::TEAMS];
                if (judge_inexist_new[team+id*PARAM::TEAMS]){
                    GlobalData::instance()->robotInformation[team][id].battery = ZSS::ZActionModule::instance()->temp_battery[id]/1000;
                }
                else{
                    GlobalData::instance()->robotInformation[team][id].battery = 0;
                }
                judge_inexist_old[team+id*PARAM::TEAMS] = judge_inexist_new[team+id*PARAM::TEAMS];
//                qDebug() << " team =" << team << " id =" << id;
//                qDebug() << " judge_inexist =" << judge_inexist_new[team+id*PARAM::TEAMS];
//                qDebug() << "----------------";
            }
        }
        /*for (int team = PARAM::BLUE; team <= PARAM::YELLOW; team++){
            for (int id = 0; id < PARAM::ROBOTNUM; id++){
                emit dataChanged(createIndex(team+id*PARAM::TEAMS,0),createIndex(team+id*PARAM::TEAMS,0));
            }
        }*/
    }
}
    /*//在场判断
    bool judge_inexist = false;
    for (int i = 0; i < GlobalData::instance()->processRobot[0].robotSize[team]; i++){
        auto& robot = GlobalData::instance()->processRobot[0].robot[team][i];
        if (robot.id == id){
            judge_inexist = true;
            break;
        }
    }
    GlobalData::instance()->robotInformation[team][id].inexist = judge_inexist;
    return judge_inexist;
}*/

QHash<int, QByteArray> ViewerInterface::roleNames() const {
    QHash<int, QByteArray> result = QAbstractListModel::roleNames();
    result.insert(robotID, ("mID"));
    result.insert(robotTeam,("mTeam"));
    result.insert(robotBattery, ("mBattery"));
    result.insert(robotCapacitance, ("mCapacitance"));
    result.insert(robotInfrared, ("mInfrared"));
    result.insert(robotInexist, ("mInexist"));
    return result;
}

