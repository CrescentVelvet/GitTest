#ifndef FIELD_H
#define FIELD_H

#include <QQuickPaintedItem>
#include <QPixmap>
#include <QPainter>
#include <mutex>
#include <vector>
#include "zslayer.h"
#include "singleton.hpp"
#include "staticparams.h"
class FieldParam{
public:
    FieldParam();
    ~FieldParam();
    void reload();
public:
    int param_width;
    int param_height;
    int param_canvas_width;
    int param_canvas_height;
    int param_goalWidth;
    int param_goalDepth;
    int param_penaltyWidth;
    int param_penaltyLength;
    int param_centerCircleRadius;
    int param_lineWidth;
    int size_ballDiameter;
    int size_shadowDiameter;
    int size_carDiameter;
    int size_carFaceWidth;
    int size_number;
    int size_debugPoint;
};
class FieldCoordsTransform{
public:
    FieldCoordsTransform();
    ~FieldCoordsTransform();
    void resize(int,int);
    double x(double _x);
    double y(double _y);
    QPointF p(QPointF& _p);
    double w(double _w);
    double h(double _h);
    double a(double _a);
    double r(double _r);

    double orx(double _x);
    double ory(double _y);
    double orw(double _w);
    double orh(double _h);
    double rx(double x);
    double ry(double y);
    QPointF rp(const QPointF& p);
public:
    int canvasWidth;
    int canvasHeight;
    double zoomRatio;
    QPointF zoomStart;
    qreal zoomCoefficient;
    qreal zoomMin;
};
typedef Singleton<FieldParam> FP;
typedef Singleton<FieldCoordsTransform> FT;

class Field : public QQuickPaintedItem,public ZSPlugin{
    Q_OBJECT
public:
    Q_INVOKABLE void resetSize(int,int,bool update = true);
public:
    Field(QQuickItem *parent = 0);
    virtual ~Field();
    void paint(QPainter* painter) override;
    void run() override{}
    void addLayers();
    Q_INVOKABLE void repaint();

    // mouse event handle function
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent * event) override;
#endif
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    // mouse event handle
//    void leftMoveEvent(QMouseEvent *);
//    void leftPressEvent(QMouseEvent *);
//    void leftReleaseEvent(QMouseEvent *);
//    void leftCtrlModifierMoveEvent(QMouseEvent *e);
//    void leftCtrlModifierPressEvent(QMouseEvent *e);
//    void leftCtrlModifierReleaseEvent(QMouseEvent *e);
//    void leftAltModifierMoveEvent(QMouseEvent *e);
//    void leftAltModifierPressEvent(QMouseEvent *e);
//    void leftAltModifierReleaseEvent(QMouseEvent *e);
//    void rightMoveEvent(QMouseEvent *);
//    void rightPressEvent(QMouseEvent *);
//    void rightReleaseEvent(QMouseEvent *);
    void middleMoveEvent(QMouseEvent *);
    void middlePressEvent(QMouseEvent *);
    void middleReleaseEvent(QMouseEvent *);
    void middleNoModifierMoveEvent(QMouseEvent *);
    void middleNoModifierPressEvent(QMouseEvent *);
    void middleNoModifierReleaseEvent(QMouseEvent *);
//    void middleAltModifierMoveEvent(QMouseEvent *);
//    void middleAltModifierPressEvent(QMouseEvent *);
//    void middleAltModifierReleaseEvent(QMouseEvent *);
//    void middleCtrlModifierMoveEvent(QMouseEvent *);
//    void middleCtrlModifierPressEvent(QMouseEvent *);
//    void middleCtrlModifierReleaseEvent(QMouseEvent *);
    void resetAfterMouseEvent();
    void checkClosestRobot(double x, double y);
    void limitZoomStart();

    bool selectRobots;
    int robotID[PARAM::ROBOTNUM];
    int robotTeam;

    int pressed;
    bool pressedRobot;
    QPointF start,end;
    QPointF move_zoomStart;
    QPointF move_start;
    double displayData;
    Qt::KeyboardModifiers mouse_modifiers;
private:
    void init();
    QPixmap* _pixmap;
    QPainter _painter;
    QRect _area;
    std::mutex _pixmap_mutex;
    std::vector<ZSS::Layer*> _layers; // layer list
};
#endif // FIELD_H
