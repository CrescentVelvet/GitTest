#include <QCoreApplication>
#include <iostream>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfoList>
#include <QStringList>
#include <QElapsedTimer>
#include <QThread>
#include "parammanager.h"
#include "visionmodule.h"
//#include "Windows.h"
#include "log_slider.h"
#include "logreader_global.h"
#include "netreceive.h"
#include "netreceive_global.h"
#include "staticparams.h"
#include "netsend.h"
#include "analyser/referee.h"
#include "time.h"
#include "analy_log.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineOption inputOption(QStringList() << "i" << "I", "", "inputoption", "lr");
    QCommandLineOption logFileName("if", "", "logfilename", "");
    QCommandLineOption logFileDir("id", "", "logfiledir", "");
    QCommandLineOption logFileClip("ic");
    QCommandLineOption outputOption(QStringList() << "o" << "O", "", "outputoption", "lw");
    QCommandLineOption outFileName("of", "", "outfilename", "test.zlog");
    QCommandLineOption outFileDir("od", "", "outfiledir", "../bin/");

    QCommandLineOption visionPort("p", "", "visionport", "10020");

    QCommandLineParser parser;
    parser.addOption(inputOption);
    parser.addOption(logFileName);
    parser.addOption(logFileDir);
    parser.addOption(logFileClip);
    parser.addOption(outputOption);
    parser.addOption(outFileName);
    parser.addOption(outFileDir);
    parser.addOption(visionPort);

    parser.process(a);

//    qDebug() << parser.value(inputOption);
//    qDebug() << parser.value(logFileName);
//    qDebug() << parser.value(outputOption);
//    qDebug() << parser.value(outFileName);
//    qDebug() << parser.value(cameraNumber);

//    log读取
    VisionModule vm;
//    log分析
    AnalyEsthetics vv;



//    output输出方式判断
    if (parser.value(outputOption) == "lw") {
//        qDebug() << "The output device is logwriter.";
        qDebug() << QString::fromLocal8Bit("输出方式是logwriter：写入log文件.");
        vm.flag = 0;
    } else if (parser.value(outputOption) == "ns") {
//        qDebug() << "The output device is netsend.";
        qDebug() << QString::fromLocal8Bit("输出方式是netsend：数据传输.");
        vm.flag = 1;
    } else {
//        qDebug() << "Unknown command. You can get more imformation from README.md";
        qDebug() << QString::fromLocal8Bit("指令错误，请看看README.md再来码代码。");
    }

//    input输入方式判断
    if(parser.value(inputOption) == "nr") {
        qDebug() << "Vision port is" << parser.value(visionPort).toInt();
    }
    vm.vision_port = parser.value(visionPort).toInt();

//    input输入方式判断
    if (parser.value(inputOption) == "lr") {
//        qDebug() << "The input device is logreader.";
        qDebug() << QString::fromLocal8Bit("输入方式是logreader：读取log文件.");
        LogSlider ls;
//        判断读取log文件的方式
        int cycle1 = 1;
        int cycle2[10] = {1};
        int startIndex[10], duration[10];
//        log剪切文件名
        QStringList m_logfile;
//        log剪切功能
        if (parser.isSet(logFileClip)) {
            m_logfile = ZSS::LParamManager::instance()->allGroups();
            if (m_logfile.size() == 0) {
//                qDebug() << "Can not find \"logclip.ini\" or \"logclip.ini\" is empty.";
                qDebug() << QString::fromLocal8Bit("找不到\"logclip.ini\"文件，或\"logclip.ini\"目录是空的。");
            }
//            cycle1是log文件尺寸
            cycle1 = m_logfile.size();
            for (int i=0; i<cycle1; i++) {
                QStringList allKeys = ZSS::LParamManager::instance()->allKeys(m_logfile[i]);
//                cycle2是？
                cycle2[i] = allKeys.size()/2;
                for (int j=0; j<cycle2[i]; j++) {
                    QString strj = QString::number(j+1);
                    ZSS::LParamManager::instance()->loadParam(startIndex[j], m_logfile[i] + "/Start" + strj, 0);
                    ZSS::LParamManager::instance()->loadParam(duration[j], m_logfile[i] + "/Stop" + strj, 0);
                    duration[j] = duration[j] - startIndex[j];
                    if (duration[j] <= 0){
//                        qDebug() << "Unavailable duration for point" << j << "in file" << m_logfile[i];
                        qDebug() << QString::fromLocal8Bit("点的不可用时间") << j << QString::fromLocal8Bit("在文件中") << m_logfile[i];
                        break;
                    }
                }
            }
            m_logfile.replaceInStrings("...", "/");
        } else if (parser.value(logFileDir) == NULL) {
//            单个log文件读取
            m_logfile.append(parser.value(logFileName));
        } else {
//            批量log文件读取
            QDir inputDir;
            inputDir.setPath(parser.value(logFileDir));
            inputDir.setFilter(QDir::Files);
            QFileInfoList logList = inputDir.entryInfoList();
            for (int k = 0; k<logList.size(); k++)
                m_logfile.append(parser.value(logFileDir).append("/") + logList.at(k).fileName());
            cycle1 = logList.size();
        }

//        开始读取文件
        for(int i=0; i<cycle1; i++) {
            QString m_realLogfile = m_logfile.at(i);
//            将文件路径恢复正常
            m_realLogfile.replace("...", "/");
            if (ls.loadFile(m_realLogfile)) {
//                qDebug() << "Load success! The log file is" << m_logfile.at(i);
                qDebug() << QString::fromLocal8Bit("读取成功！已读取文件：") << m_logfile.at(i);
            } else {
//                qDebug() << "Load failed! Please check your DIR or FILENAME.";
                qDebug() << QString::fromLocal8Bit("读取失败！请检查文件名和目录。");
                break;
            }
//            剪切后j从cycle2[i]开始
//            平时j从1开始
            for (int j=0; j<(parser.isSet(logFileClip) ? cycle2[i] : 1); j++) {
//                输出方式是lw
                if (vm.flag == 0) {
                    QString outFileName = m_logfile.at(i);
                    int a = outFileName.lastIndexOf("/");
                    outFileName.replace(0, a+1, parser.value(outFileDir));
                    if (parser.isSet(logFileClip)) {
                        QString strj = QString::number(j);
                        vm.lw_v.setFileName(outFileName.replace(".log.gz", "_bp_" + strj + ".zlog"));
                        vm.lw_rfb.setFileName(outFileName.replace(".zlog","_rfb.zlog"));
                    } else {
                        vm.lw_v.setFileName(outFileName.replace("log.gz", "zlog"));
                        vm.lw_rfb.setFileName(outFileName.replace(".zlog","_rfb.zlog"));
                    }
                }
//                剪切后当前帧为开始目录[j]，平时当前帧为0
                int m_currentFrame = parser.isSet(logFileClip) ? startIndex[j] : 0;
//                剪切后总帧数为当前帧加持续帧，平时总帧数为包packet的大小
                int size = parser.isSet(logFileClip) ? duration[j] + m_currentFrame : ls.m_player.packets.size();
//                从当前帧到总帧数，循环
                while (++m_currentFrame < size) {
//                    当前帧的包packet信息
                    Frame* packet = ls.m_player.packets.at(m_currentFrame);
//                    包是空的
                    if (packet->type == MESSAGE_BLANK) {
//                    包是未知的
                    } else if (packet->type == MESSAGE_UNKNOWN) {
//                        std::cout << "Error unsupported or corrupt packet found in log file!" << std::endl;
                         qDebug() << QString::fromLocal8Bit("log文件格式错误。");
//                    包是2010与2014版本的
                    } else if (packet->type == MESSAGE_SSL_VISION_2010 || packet->type == MESSAGE_SSL_VISION_2014) {
//                        vm.parse((void *)packet->data.data(), packet->data.size());
// 加入log分析
//                        读取当前帧
                        vv.readFrame(m_currentFrame, size, m_logfile.at(i));
//                        分析每一帧
                        vv.analy_frame((void *)packet->data.data(), packet->data.size());
                    } else if (packet->type == MESSAGE_SSL_REFBOX_2013) {
//                        if (vm.flag == 0) {
//                            QByteArray buffer;
//                            buffer.append(packet->time);
//                            buffer.append(packet->data.data());
//                            vm.lw_rfb.write(buffer);
//                        }
// 加入log分析
                        if (vm.flag == 0) {
//                            读取裁判盒信息
                            vv.readReferee(packet->data.data(), packet->data.size());
                        }
                        RefInfo::instance()->receiveRef((void*)packet->data.data() , packet->data.size());
                    } else {
//                        qDebug() << "Error unsupported message type found in log file!";
                        qDebug() << QString::fromLocal8Bit("log文件中含有不支持的类型。");
                    }
                    if (parser.value(outputOption) == "ns") {
//                        Sleep(1000/60);
                        QThread::msleep(1000/60);
                    }
                    std::cout << m_currentFrame + 1 << "/" << size << "\r";
                }
                std::cout << "\nfinished" << std::endl;
            }
        }
//        qDebug() << "All tasks finished, please find results in" << parser.value(outFileDir);
        qDebug() << QString::fromLocal8Bit("log分析任务完成，结果存储于：") << parser.value(outFileDir);
    } else if (parser.value(inputOption) == "nr") {
//        qDebug() << "The input device is netreceive.";
        qDebug() << QString::fromLocal8Bit("输入方式是netreceive数据传输.");
//        NetReceive nr(parser.value(visionPort).toInt());
        NetReceive nr;
        if (vm.flag == 0) {
//            此处有bug，无法将QString赋值给QString&，难以解决，注释掉
//            vm.lw_v.setFileName(parser.value(outFileName));
//            裁判盒信息
//            vm.lw_rfb.setFileName(parser.value(outFileName).replace(".zlog", "_rfb.zlog"));
            qDebug() << QString::fromLocal8Bit("无法解决的QString问题。");
        }
        //UDP can't work with dll, so force cycle SLOT function.
        while (true) {
            nr.storeData();
            while (!nr.datagrams.isEmpty()) {
                QByteArray datagram = nr.datagrams.dequeue();
                vm.parse((void*)datagram.data(), datagram.size());
                qDebug() << "Some data was gotten from UDP.";
            }
//            Sleep(5);
            QThread::msleep(5);
            qDebug() << "Nothing from UDP.";
        }
    } else {
//        qDebug() << "Unknown command. You can get more imformation from README.md";
        qDebug() << QString::fromLocal8Bit("指令错误，请看看README.md再来码代码。");
    }

//    return a.exec();

    a.exit(0);
}
