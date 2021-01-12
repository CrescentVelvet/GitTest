#include "rec_recorder.h"
#include "globaldata.h"
#include "globalsettings.h"
#include "field.h"
#include <QDateTime>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>

namespace  {
auto GS = GlobalSettings::instance();
//RecMsg recMsg;
bool isRun = true;

QFile recordFile;
QIODevice* recIO;
QString filename;

//QTime timer;
}
RecRecorder::RecRecorder() {

}
void RecRecorder::init() {
    isRun = true;
//        qDebug() << "I AM RUNNING";
    QDateTime datetime;
//        qDebug() << datetime.currentDateTime().toString("yyyy-MM-dd-HH-mm-ss");
    filename = QString("LOG/Rec").append(datetime.currentDateTime().toString("yyyy-MM-dd-HH-mm-ss")).append(".log");
//    recordFile = new QFile(filename);
    recordFile.setFileName(filename);
    recordFile.open(QIODevice::WriteOnly | QIODevice::Append);
//    recordFile->open(QIODevice::WriteOnly | QIODevice::Append);
}

void RecRecorder::store() {
    if (isRun) {
        recordFile.open(QIODevice::WriteOnly | QIODevice::Append);
        recIO = &recordFile;
        //    Field::repaintLock();
        //    qDebug() << "I AM FUCKING RUNNING";
        //    recMsg = recMsgs.add_recmsgs();
        ZSS::Protocol::RecMessage recMsg;
        ZSS::Protocol::TeamRobotMsg* processMsg;
        ZSS::Protocol::TeamRobotMsg* maintain;
        ZSS::Protocol::Robot4Rec* robot4Rec;
        ZSS::Protocol::Point* ball;
        ZSS::Protocol::Debug_Msgs* debugMsgs;
        //ctrlc
        GlobalData::instance()->ctrlCMutex.lock();
        recMsg.set_ctrlc(GlobalData::instance()->ctrlC);
        GlobalData::instance()->ctrlCMutex.unlock();
        //selectedArea
        recMsg.mutable_selectedarea()->set_maxx(GS->maximumX);
        recMsg.mutable_selectedarea()->set_maxy(GS->maximumY);
        recMsg.mutable_selectedarea()->set_minx(GS->minimumX);
        recMsg.mutable_selectedarea()->set_miny(GS->minimumY);
        //maintainVision
        const OriginMessage &robot_vision = GlobalData::instance()->processRobot[0];
        recMsg.mutable_maintainvision()->set_lasttouch(GlobalData::instance()->lastTouch % PARAM::ROBOTMAXID);
        recMsg.mutable_maintainvision()->set_lasttouchteam(GlobalData::instance()->lastTouch < PARAM::ROBOTMAXID ? PARAM::BLUE : PARAM::YELLOW);
        for(int color = PARAM::BLUE; color <= PARAM::YELLOW; color++) {
            processMsg = recMsg.mutable_maintainvision()->add_processmsg();
            for(int j = 0; j < PARAM::ROBOTNUM; j++) {
                robot4Rec = processMsg->add_robot();
                robot4Rec->set_posx(robot_vision.robot[color][j].pos.x());
                robot4Rec->set_posy(robot_vision.robot[color][j].pos.y());
                robot4Rec->set_angle(robot_vision.robot[color][j].angle);
                robot4Rec->set_valid(robot_vision.robot[color][j].valid);//add by lzx
            }
        }
        auto& maintainMsg = GlobalData::instance()->maintain[0];
        for(int color = PARAM::BLUE; color <= PARAM::YELLOW; color++) {
            maintain = recMsg.mutable_maintainvision()->add_maintain();
           // for(int j = 0; j < maintainMsg.robotSize[color]; j++) {
            for(int j = 0; j < PARAM::ROBOTNUM; j++) {
                robot4Rec = maintain->add_robot();
                robot4Rec->set_posx(maintainMsg.robot[color][j].pos.x());
                robot4Rec->set_posy(maintainMsg.robot[color][j].pos.y());
                robot4Rec->set_angle(maintainMsg.robot[color][j].angle);
                robot4Rec->set_valid(robot_vision.robot[color][j].valid);//add by lzx
            }
        }
        recMsg.mutable_maintainvision()->mutable_balls()->set_size(maintainMsg.ballSize);
        recMsg.mutable_maintainvision()->mutable_balls()->set_valid(maintainMsg.isBallValid);
        for(int j = 0; j < maintainMsg.ballSize; j++) {
            ball = recMsg.mutable_maintainvision()->mutable_balls()->add_ball();
            ball->set_x(maintainMsg.ball[j].pos.x());
            ball->set_y(maintainMsg.ball[j].pos.y());
        }
        //debugMsgs
        GlobalData::instance()->debugMutex.lock();
        for (int team = PARAM::BLUE; team <= PARAM::YELLOW; team++) {
            debugMsgs = recMsg.add_debugmsgs();
            if (team == 0) {
                debugMsgs->ParseFromArray(GlobalData::instance()->debugBlueMessages.data(), GlobalData::instance()->debugBlueMessages.size());
            } else {
                debugMsgs->ParseFromArray(GlobalData::instance()->debugYellowMessages.data(), GlobalData::instance()->debugYellowMessages.size());
            }
            //        qDebug() << "FUCK DEBUG MESSAGE SIZE" <<  debugMsgs->ByteSize();
            //        qDebug() << "FUCK DEBUG MESSAGE SIZE" <<  recMsg.debugmsgs(0).ByteSize();
        }
        GlobalData::instance()->debugMutex.unlock();
        //    Field::repaintUnlock();

        int size = recMsg.ByteSize();
        QByteArray buffer(size, 0);
        recMsg.SerializeToArray(buffer.data(), buffer.size());
        QDataStream stream(recIO);
        stream << buffer;
        //    recordFile->flush();
        //    recordFile->close();
        recordFile.close();
        recMsg.Clear();
        if (recordFile.size() > (qint64)1024 * 1024 * 1024 * 5) {
            stop();
        }
    }
}

void RecRecorder::stop() {
    isRun = false;
    recIO = nullptr;
    QFileInfo afterFile(filename);
    if (afterFile.size() < 1) {
//        QFile emptyFile(filename);
        recordFile.remove();
    }
//    if (recordFile && recordFile->isOpen()) recordFile->close();
//    delete recordFile;
//    recordFile = nullptr;
}
