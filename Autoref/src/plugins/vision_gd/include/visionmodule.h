#ifndef __VISIONMODULE_H__
#define __VISIONMODULE_H__
#include "zsplugin.hpp"
#include "singleton.hpp"
#include "messages_robocup_ssl_wrapper.pb.h"
#include "vision_detection.pb.h"
#include "staticparams.h"
#include "messageformat.h"
class CVisionModule : public ZSPlugin {
public:
    CVisionModule();
    ~CVisionModule();
    void run() override;
    void parseCameraMode(int);
    void udpSocketConnect(bool);
    void udpSocketDisconnect();
    void parse(const SSL_WrapperPacket&);
    void setIfEdgeTest(bool);
    bool showIfEdgeTest();
    quint16 getFPS();
    bool dealWithData();
    void setInterfaceIndex(const int);
    void storeData();
private:
    CGeoPoint saoConvert(CGeoPoint);
    double saoConvert(double);
    void edgeTest();
    void send();
    void checkCommand();
    quint64 counter;
    int saoAction;
    int vision_port;
    bool collectNewVision();
    bool IF_EDGE_TEST;
    Vision_DetectionFrame detectionFrame;
};
typedef Singleton <CVisionModule> VisionModule;
#endif // __VISIONMODULE_H__
