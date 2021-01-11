#ifndef VIEWERINTERFACE_H
#define VIEWERINTERFACE_H

#include <QAbstractListModel>
#include <QTimer>
#include <QDateTime>
#include "globaldata.h"
#include "actionmodule.h"
#include "simmodule.h"
#include "parammanager.h"
#include "thread"



extern int width[];
class Global : public QObject{
    Q_OBJECT
signals:
    void changeRobotInfo(int);
};

class ViewerInterface : public QAbstractListModel{
    Q_OBJECT
//    QTime receive_current_time;
    qint64 min_sec_old[PARAM::ROBOTNUM*PARAM::TEAMS];
//    int min_sec_new[PARAM::ROBOTNUM*PARAM::TEAMS];
public slots:
    void changeRobotInfo(int team,int id){

        QDateTime UTC(QDateTime::currentDateTimeUtc());
        qint64 min_sec_new = UTC.toMSecsSinceEpoch();
        //每隔0.5秒刷新一次UI界面
        if((min_sec_new - min_sec_old[team+id*PARAM::TEAMS]) >= 500)
        {
            qDebug() << "update!!!!!!!!!!!!!!!!!!!!!!";
            emit dataChanged(createIndex(team+id*PARAM::TEAMS,0),createIndex(team+id*PARAM::TEAMS,0));
            min_sec_old[team+id*PARAM::TEAMS] = min_sec_new;
        }
    }
public:
    explicit ViewerInterface(QObject *parent = Q_NULLPTR){
        /*收到receive信息才改变显示图像change*/
        QObject::connect(ZSS::ZActionModule::instance(),SIGNAL(receiveRobotInfo(int,int)),this,SLOT(changeRobotInfo(int,int)));
        QObject::connect(ZSS::ZSimModule::instance(),SIGNAL(receiveSimInfo(int,int)),this,SLOT(changeRobotInfo(int,int)));
    }
    virtual ~ViewerInterface(){}
    enum Roles {
        robotID = Qt::UserRole + 1,
        robotTeam,
        robotBattery,
        robotCapacitance,
        robotInfrared
    };
    Q_ENUM(Roles)
    QHash<int, QByteArray> roleNames() const {
        QHash<int, QByteArray> result = QAbstractListModel::roleNames();
        result.insert(robotID, ("mID"));
        result.insert(robotTeam,("mTeam"));
        result.insert(robotBattery, ("mBattery"));
        result.insert(robotCapacitance, ("mCapacitance"));
        result.insert(robotInfrared, ("mInfrared"));
        return result;
    }
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override{
        return PARAM::ROBOTNUM*2;
    }
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override{
        return 5;
    }
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
        if(index.isValid()){
            int team = index.row()%PARAM::TEAMS;
            int id = index.row()/PARAM::TEAMS;
            switch(role){
            case robotID:
                return id;
            case robotTeam:
                return team;
            case robotBattery:
                return GlobalData::instance()->robotInformation[team][id].battery;
            case robotCapacitance:
                return GlobalData::instance()->robotInformation[team][id].capacitance;
            case robotInfrared:
                return GlobalData::instance()->robotInformation[team][id].infrared;
            }
            return 0;
        }
    }
};

#endif // VIEWERINTERFACE_H
