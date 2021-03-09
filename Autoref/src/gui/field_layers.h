#ifndef __FIELD_LAYERS_H__
#define __FIELD_LAYERS_H__
#include "zslayer.h"
#include <QPainterPath>
#include "vision_detection.pb.h"
class FieldLineLayer : public ZSS::Layer{
public:
    FieldLineLayer();
    ~FieldLineLayer();
    void init() override;
    void run() override;
private:
    void initFieldLinePath();
    QPainterPath fieldLinePath;
};
class VisionLayer: public ZSS::Layer{
public:
    VisionLayer();
    ~VisionLayer();
    void init() override;
    void run() override;
private:
    void drawOriginVision(const Vision_DetectionFrame&);
    void drawFusionVision(const Vision_DetectionFrame&);
private:
    void paintRobot(const QColor& color,quint8 num,qreal x,qreal y,qreal radian
                      ,bool ifDrawNum = true,const QColor& textColor = Qt::white,bool needCircle = false);
    void paintBall(const QColor& color,qreal x,qreal y);

};

#endif // __FIELD_LAYERS_H__
