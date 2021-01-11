#ifndef VIEWERINTERFACE_H
#define VIEWERINTERFACE_H

#include <QAbstractListModel>
#include <QTimer>
#include <QDateTime>
#include "globaldata.h"
#include "actionmodule.h"
#include "simmodule.h"
#include "parammanager.h"

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
    double robot_battery_old[PARAM::ROBOTNUM*PARAM::TEAMS];
    bool judge_inexist = false;
    bool judge_battery = false;
    bool judge_time = false;
    bool judge_emit = false;
    bool judge_inexist_old[PARAM::ROBOTNUM*PARAM::TEAMS];
public slots:
    void changeRobotInfo(int team,int id);
public:
    explicit ViewerInterface(QObject *parent = Q_NULLPTR);
    virtual ~ViewerInterface(){}
    enum Roles {
        robotID = Qt::UserRole + 1,
        robotTeam,
        robotBattery,
        robotCapacitance,
        robotInfrared,
        robotInexist
    };
    Q_ENUM(Roles)
    void view_display();
    void emit_display();
    QHash<int, QByteArray> roleNames() const;
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
                return GlobalData::instance()->robotInformation[team][id].inexist;
            }
            return 0;
        }
    }
};

#endif // VIEWERINTERFACE_H
