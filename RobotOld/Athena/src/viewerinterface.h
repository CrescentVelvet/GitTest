#ifndef VIEWERINTERFACE_H
#define VIEWERINTERFACE_H

#include <QAbstractListModel>
#include <QTimer>
#include <QDateTime>
#include "globaldata.h"
#include "actionmodule.h"
#include "simmodule.h"
#include "parammanager.h"

class ViewerInterface : public QAbstractListModel{
    Q_OBJECT    
public slots:
    void changeRobotInfo(int team,int id);
public:
    explicit ViewerInterface(QObject *parent = Q_NULLPTR);
    void exist_display();
    enum Roles {
        robotID = Qt::UserRole + 1,
        robotTeam,
        robotBattery,
        robotCapacitance,
        robotInfrared,
        robotInexist
    };
    Q_ENUM(Roles)
    QHash<int, QByteArray> roleNames() const {
        QHash<int, QByteArray> result = QAbstractListModel::roleNames();
        result.insert(robotID, ("mID"));
        result.insert(robotTeam,("mTeam"));
        result.insert(robotBattery, ("mBattery"));
        result.insert(robotCapacitance, ("mCapacitance"));
        result.insert(robotInfrared, ("mInfrared"));
        result.insert(robotInexist, ("mInexist"));
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
            case robotInexist:
                return displayExist[team][id];
            }
            return 0;
        }
    }
private:
    RobotInformation displayInfomation[PARAM::TEAMS][PARAM::ROBOTNUM];
    bool displayExist[PARAM::TEAMS][PARAM::ROBOTNUM];
};

#endif // VIEWERINTERFACE_H
