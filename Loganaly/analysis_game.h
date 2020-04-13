#ifndef ANALYSIS_GAME_H
#define ANALYSIS_GAME_H
#include <QQmlEngine>
#include <QDebug>
#include "singleton.hpp"
#include <globaldata.h>
#include <globalsettings.h>
#include <QThread>
#include <QFile>
#include <QObject>
#include <vector>
const double param_width = 12000;
const double param_penalty_radius = 800;
const double param_penalty_area_l = 350;
enum myside{Left , Right};//我方守门方向
enum theirBack{b_high , b_low , b_middle};//敌方进攻时留守数量
enum theirPenaltyState{p_low , p_normal , p_high};
class AnalysisGame : public QObject
{
    Q_OBJECT
public:
    static QObject* instance(QQmlEngine* engine = nullptr, QJSEngine* scriptEngine = nullptr){
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        static AnalysisGame* instance = new AnalysisGame();
        return instance;
    }
    explicit AnalysisGame(QObject* parent = Q_NULLPTR);
    Q_INVOKABLE void loadFile(QString filename);
private:
    CVector Polar2Vector(double m,double angle);
    CGeoPoint analy_TheirInterPoint(CGeoPoint enemy, CGeoPoint ball);
    void Ball_ControlRate(const QByteArray& packet);
    int cal_nearest(const QByteArray& packet, int team);
    void cal_holdMsg(const QByteArray& packet);
    bool loadRec(QString& filename);
    bool cal_RealFrame(const QByteArray& packet);
    void judgeSide(const QByteArray& packet);
    void analy_status(const QByteArray& packet);
    bool analy_marking(const QByteArray& packet, int team, int num);
    bool analy_guarding(const QByteArray& packet, int team, int num);
    bool analy_goalkeeping(const QByteArray& packet, int team, int num);
    void cal_our_carnum(const QByteArray& packet);
    void run();
    std::string holdMsg;
    double ourHoldFrame;
    double theirHoldFrame;
    int realFrame;
    int _Team;
    int _opTeam;
    int homeSide;
    QFile* replayFile;
    QList <QByteArray> packets;
    double HoldRate[PARAM::TEAMS];//控球率
    double our_car_num[6];//进攻时的进攻与防守车数，防守时的进攻与防守车数，争抢时的进攻与防守车数
    double their_car_num[6];
    double real_ourframe[3];//进攻与防守与争抢帧数
    double real_theirframe[3];
};
#endif // ANALYSIS_GAME_H
