#include <QAbstractListModel>
#include "viewerinterface.h"

auto xpm = ZSS::ZParamManager::instance();
double VIEW_DISPLAY_FREQUENCE;
void ViewerInterface::changeRobotInfo(int team,int id){
    QDateTime UTC(QDateTime::currentDateTimeUtc());
    qint64 min_sec_new = UTC.toMSecsSinceEpoch();
    //每隔VIEW_DISPLAY_FREQUENCE秒刷新一次UI界面
    if((min_sec_new - min_sec_old[team+id*PARAM::TEAMS]) >= 1000 * VIEW_DISPLAY_FREQUENCE)
    {
//        qDebug() << "update!!!!!!!!!!!!!!!!!!!!!!";
        emit dataChanged(createIndex(team+id*PARAM::TEAMS,0),createIndex(team+id*PARAM::TEAMS,0));
        min_sec_old[team+id*PARAM::TEAMS] = min_sec_new;
    }
}

ViewerInterface::ViewerInterface(QObject *parent){
    /*收到receive信息才改变显示图像change*/
    xpm->loadParam(VIEW_DISPLAY_FREQUENCE, "Debug/VIEW_DISPLAY_FREQUENCE", 0.01);
//    receiveThread = new std::thread([ = ] {view_display();});
//    receiveThread->detach();
    QObject::connect(ZSS::ZActionModule::instance(),SIGNAL(receiveRobotInfo(int,int)),this,SLOT(changeRobotInfo(int,int)));
    QObject::connect(ZSS::ZSimModule::instance(),SIGNAL(receiveSimInfo(int,int)),this,SLOT(changeRobotInfo(int,int)));
}

bool view_display(int team,int id){
//把actionmodule里的inexist放进此处，提高效率
//当inexist变化，或battery变化量大，且time经过一秒以上，才emit dataChanged
    return true;
}

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

