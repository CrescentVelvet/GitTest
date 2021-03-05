#ifndef __GLOBAL_DATA__
#define __GLOBAL_DATA__
#include "singleton.hpp"
#include "dataqueue.hpp"
#include "messageformat.h"
#include "ballrecords.h"
#include "zss_cmd.pb.h"
#include <QMutex>
#include <QReadWriteLock>

struct RobotInformation {
    bool infrared;
    bool flat;
    bool chip;
    double battery;
    double capacitance;
    bool needclean;
    bool needreport;
    double imuDir;
    double rawImuDir;
    double imuRotateDir;
    bool imureceive;
    bool imucleaned;
    bool imuavailable;
    RobotInformation(): infrared(false), flat(false), chip(false), battery(0), capacitance(0), needclean(true), needreport(false), imuDir(0), rawImuDir(0), imuRotateDir(0), imureceive(false), imucleaned(false), imuavailable(false){}

//    RobotInformation(): infrared(false), flat(false), chip(false), battery(0), capacitance(0) {}
};
struct RobotSpeed {
    float vx;
    float vy;
    float vr;
    float mod;
    bool use_dir;
    RobotSpeed(): vx(0), vy(0), vr(0), mod(0), use_dir(false) {}
    RobotSpeed(float _x, float _y, float _r, bool use):
        vx(_x), vy(_y), vr(_r), mod(sqrtf(_x*_x +_y*_y)), use_dir(use) {}
};
struct RobotCommands {
    RobotSpeed robotSpeed[PARAM::ROBOTNUM];
    bool valid;
    RobotCommands(): valid(false) {
        for (int i = 0; i < PARAM::ROBOTNUM; i++)
            robotSpeed[i] = RobotSpeed();
    }
};

class CGlobalData {
  public:
    CGlobalData();
    void setCameraMatrix(bool);
    bool cameraUpdate[PARAM::CAMERA];
    bool cameraControl[PARAM::CAMERA];
    bool processControl[3];
    int cameraID[PARAM::CAMERA];//show the mapping of cameras  id
    double robotPossible[2][PARAM::ROBOTNUM];
    RobotInformation robotInformation[PARAM::TEAMS][PARAM::ROBOTNUM];
    QMutex robotInfoMutex;
    DataQueue<RobotCommands> robotCommand[PARAM::TEAMS];
    int commandMissingFrame[PARAM::TEAMS];//team command VALID  --> commandMissingFrame<20
    CameraFix cameraFixMatrix[PARAM::CAMERA];
    SingleCamera cameraMatrix[PARAM::CAMERA];
    DataQueue<ReceiveVisionMessage> camera[PARAM::CAMERA];
    DataQueue<ReceiveVisionMessage> processBall;
    DataQueue<ReceiveVisionMessage> processRobot;
    DataQueue<ReceiveVisionMessage> maintain;
    DataQueue<BallRecord> ballrecords;
    int lastTouch;//Be attention it's id!!!
    CGeoPoint lastTouchPos;
    QByteArray debugBlueMessages;
    QByteArray debugYellowMessages;
    QMutex debugMutex;// debugMessages;
    QReadWriteLock lock;
    bool ctrlC;
    QMutex ctrlCMutex;
    QByteArray athenaDebugMessage;

    // imu
    void imureset(int team);
    void imuClean(int team);
    void imuClean(int team, int id);
    double imuZero[PARAM::TEAMS][PARAM::ROBOTNUM];// for IMU
    int cleancount;
    void setImuData();
    void saveImuData();
    void testforIMU();
    void normalTracking(int team);
    bool imuend;
    bool imucleaning[PARAM::TEAMS];
    bool MedusaOn;
    QReadWriteLock robotinformationlock;
    void CameraInit();


  private:
    CGeoPoint saoConvert(CGeoPoint);
    void  saoConvertEdge();
    int saoAction;
};
typedef Singleton<CGlobalData> GlobalData;
#endif // __GLOBAL_DATA__
