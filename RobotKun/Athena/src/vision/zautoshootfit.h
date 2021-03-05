#ifndef CAUTOSHOOTFIT_H
#define CAUTOSHOOTFIT_H
#include "staticparams.h"
#include "singleton.hpp"
#include <qudpsocket.h>
#include <mutex>
#include "geometry.h"

class CAutoShootFit
{
public:
    CAutoShootFit();
    ~CAutoShootFit();


    bool getRun() {
        return _isrun;
    }
    void run();
    void test();
    void autoFit();
    void getKickPower(int p, double sp);
    void getKickerMes(int t, int n, bool m/*, quint8 p*/);
    void sendKickParam(int team, int id);
    void receiveKickParam();
    void udpSend();
    struct KickParam {

        int    team;
        int    id;
        double ca;
        double cb;
        double cc;
        double cMax;
        double cMin;
        double fa;
        double fb;
        double fc;
        double fMax;
        double fMin;

        KickParam(): team(-1), id(-1),
                     ca(0), cb(0), cc(0), cMax(0), cMin(0),
                     fa(0), fb(0), fc(0), fMax(0), fMin(0){}
        KickParam(int _team, int _id,
                  double _ca, double _cb, double _cc, double _cMax, double _cMin,
                  double _fa, double _fb, double _fc, double _fMax, double _fMin):
                  team(_team), id(_id),
                  ca(_ca), cb(_cb), cc(_cc), cMax(_cMax), cMin(_cMin),
                  fa(_fa), fb(_fb), fc(_fc), fMax(_fMax), fMin(_fMin){}
    };

    KickParam kick_param[PARAM::ROBOTNUM*PARAM::TEAMS];
private:
    void       recordKickData(int id, bool mode, int power, double sp, double vel_or_dist);
    void       chipParamTest();
    void       flatParamTest();
    bool       _isrun;
    void       readAllKickParam();
    void       sendAllKickParam();
    void       changeParam(KickParam kp);
    KickParam  getParam(int team, int id);
    void       calculate(int i);
    void       reset();
    double     getVel();
    double     getDist();
    QUdpSocket sendSocket;
    QUdpSocket receiveSocket;
    //autofit
    double data[PARAM::ROBOTNUM][127];

};
typedef Singleton <CAutoShootFit> AutoShootFit;
#endif // CAUTOSHOOTFIT_H
