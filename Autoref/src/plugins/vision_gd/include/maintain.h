#ifndef MAINTAIN_H
#define MAINTAIN_H
#include "singleton.hpp"
#include "vision_detection.pb.h"
#include "kalmanfilter.h"
#include <messageformat.h>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
/**
 * @brief Maintain and combine the result of dealrobot and dealball
 */
class CMaintain {
  public:
    CMaintain();
    ~CMaintain();
    void run();
    void init();
    void ChipPrediction();
    void StateMachine();
    double getpredict_x();
    double getpredict_y();

  private:
    QFile file;
    QTextStream out;
    ReceiveVisionMessage result;
    KalmanFilter ballKalmanFilter;
};
typedef Singleton<CMaintain> Maintain;

#endif // MAINTAIN_H
