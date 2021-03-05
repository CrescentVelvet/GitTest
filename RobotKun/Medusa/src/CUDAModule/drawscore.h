#ifndef DRAWSCORE_H
#define DRAWSCORE_H
#include "VisionModule.h"
#include "singleton.hpp"
#include "zss_debug.pb.h"
#include <algorithm>
#include <vector>
#include <QUdpSocket>
#include "zss_debug.pb.h"

struct RawScore {
    int x;
    int y;
    double score;
};

class CDrawScore {
  public:
    CDrawScore();
    ~CDrawScore();
    void storePoint(int px, int py, double score);
    void sendPackages();
private:
    std::vector <RawScore> rawScores;
    ZSS::Protocol::Debug_Scores scores;
    QUdpSocket scoreSend;
    bool isYellow;
    bool messiDebug;
    void toProtobuf();
};
typedef Singleton<CDrawScore> DrawScore;

#endif // DRAWSCORE_H
