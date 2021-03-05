#include "drawscore.h"
#include "parammanager.h"
#include "staticparams.h"
#include "GDebugEngine.h"
#include <QString>

namespace  {
    const int COLOR_RANGE = 256;
}

CDrawScore::CDrawScore(){
    ZSS::ZParamManager::instance()->loadParam(isYellow,"ZAlert/IsYellow",false);
    ZSS::ZParamManager::instance()->loadParam(messiDebug, "Debug/Messi", false);
}

CDrawScore::~CDrawScore() {
}

void CDrawScore::storePoint(int px, int py, double score) {
    RawScore rawScore;
    rawScore.x = px;
    rawScore.y = py;
    rawScore.score = score;
    rawScores.push_back(rawScore);
}

void CDrawScore::sendPackages() {
    std::sort(rawScores.begin(), rawScores.end(), [](const RawScore& a, const RawScore& b){return a.score < b.score;});
    // if (messiDebug)
    //     GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(100, 380), QString("MaxScore: %1  MinScore: %2").arg(QString::number(rawScores.back().score)).arg(QString::number(rawScores.begin()->score)).toLatin1(),COLOR_ORANGE);
    //将点均分为若干个颜色,现在情况为把所有点按分值大小分配为256部分，每部分对应一个颜色
    ZSS::Protocol::Debug_Score* score;
    ZSS::Protocol::Point* p;
    int color_size = rawScores.size();
    for (int m = 0; m < COLOR_RANGE; m++) {
        for (int n = m*color_size/COLOR_RANGE; n < (m+1)*color_size/COLOR_RANGE; n++) {
            score = scores.add_scores();
            score->set_color(m);
            p = score->add_p();
            p->set_x(rawScores.at(n).x);
            p->set_y(rawScores.at(n).y);
        }
        //由于UDP发包大小的限制，所以将颜色信息分批次发送
        if((m + 1) % 8 == 0) {
            toProtobuf();
        }
    }
    rawScores.clear();
}

void CDrawScore::toProtobuf() {
    int size = scores.ByteSize();
    QByteArray result(size, 0);
    scores.SerializeToArray(result.data(), size);
    /*qDebug() << "size sent: " <<*/ scoreSend.writeDatagram(result.data(), size, QHostAddress(ZSS::LOCAL_ADDRESS), ZSS::Medusa::DEBUG_SCORE_SEND[isYellow ? 1 : 0]);
    scores.clear_scores();
}
