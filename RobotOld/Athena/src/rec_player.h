#ifndef REC_PLAYER_H
#define REC_PLAYER_H
#include <QThread>
#include <QFile>
#include "analysis_game.h"

class RecPlayer : public QThread
{
    Q_OBJECT
public:
    RecPlayer();
    bool loadRec(QString& filename, int& maxframe);
    bool start(int position);
    void stop();
    bool good();
    void pause();
    void slowly(int position);
    double calOurHoldingFrame();
    double ourHoldRate();
//    static QFile* replayFile;
    QFile* replayFile;
signals:
    void positionChanged(int frame);
private:
    void run();
    void sendMessage(const QByteArray& packet);
    CVector Polar2Vector(double m,double angle);
    CGeoPoint analy_TheirInterPoint(CGeoPoint enemy, CGeoPoint ball);
    void judgeSide(const QByteArray& packet);
    int cal_nearest(const QByteArray& packet, int team);
    void cal_holdMsg(const QByteArray& packet);
    bool analy_marking(const QByteArray& packet, int team, int num);
    bool analy_guarding(const QByteArray& packet, int team, int num);
    bool analy_goalkeeping(const QByteArray& packet, int team, int num);
    void cal_our_carnum(const QByteArray& packet);
    QList <QByteArray> packets;
    int _Team;
    int _homeSide;
    int _opTeam;
    bool _run;
    int _currentFrame;
    std::string holdMsg;
    double our_car_num[6];
    double their_car_num[6];
    double real_ourframe[3];
    double real_theirframe[3];
};
#endif // REC_PLAYER_H
