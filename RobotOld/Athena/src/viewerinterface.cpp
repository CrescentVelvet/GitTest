#include <QAbstractListModel>
#include <thread>
#include "viewerinterface.h"

namespace {
auto xpm = ZSS::ZParamManager::instance();
const double MIN_BATTERY_CHANGE = 0.05;
const double MIN_CAPACITANCE_CHANGE = 0.05;
}

ViewerInterface::ViewerInterface(QObject *parent){
    std::thread* receiveThread = new std::thread([ = ] {exist_display();});
    receiveThread->detach();
    QObject::connect(ZSS::ZActionModule::instance(),SIGNAL(receiveRobotInfo(int,int)),this,SLOT(changeRobotInfo(int,int)));
    QObject::connect(ZSS::ZSimModule::instance(),SIGNAL(receiveSimInfo(int,int)),this,SLOT(changeRobotInfo(int,int)));
}

void ViewerInterface::changeRobotInfo(int team,int id){
    // infrared
    if (GlobalData::instance()->robotInformation[team][id].infrared != displayInfomation[team][id].infrared){
        displayInfomation[team][id].infrared = GlobalData::instance()->robotInformation[team][id].infrared;
        emit dataChanged(createIndex(team+id*PARAM::TEAMS,0),createIndex(team+id*PARAM::TEAMS,0),{Roles::robotInfrared});
    }
    // capacitance
    if (fabs(GlobalData::instance()->robotInformation[team][id].capacitance - displayInfomation[team][id].capacitance) > MIN_CAPACITANCE_CHANGE){
        displayInfomation[team][id].capacitance = GlobalData::instance()->robotInformation[team][id].capacitance;
        emit dataChanged(createIndex(team+id*PARAM::TEAMS,0),createIndex(team+id*PARAM::TEAMS,0),{Roles::robotCapacitance});
    }
    // battery
    if (fabs(GlobalData::instance()->robotInformation[team][id].battery - displayInfomation[team][id].battery) >= MIN_BATTERY_CHANGE){
        displayInfomation[team][id].battery = GlobalData::instance()->robotInformation[team][id].battery;
        emit dataChanged(createIndex(team+id*PARAM::TEAMS,0),createIndex(team+id*PARAM::TEAMS,0),{Roles::robotBattery});
    }
}

void ViewerInterface::exist_display(){
    bool exist_new[PARAM::TEAMS][PARAM::ROBOTMAXID];
    while(true){
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
        // exist
        for (int team = PARAM::BLUE; team <= PARAM::YELLOW; team++){
            for (int id = 0; id < PARAM::ROBOTNUM; id++){
                exist_new[team][id] = false;
                for (int i = 0; i < GlobalData::instance()->processRobot[0].robotSize[team]; i++){
                    auto& robot = GlobalData::instance()->processRobot[0].robot[team][i];
                    if (robot.id == id){
                        exist_new[team][id] = true;
                        break;
                    }
                }
                if (exist_new[team][id] != displayExist[team][id]){
                    displayExist[team][id] = exist_new[team][id];
                    emit dataChanged(createIndex(team+id*PARAM::TEAMS,0),createIndex(team+id*PARAM::TEAMS,0),{Roles::robotInexist});
                }
            }
        }
    }
}
