#ifndef ROBOTINFOVIEWER_H
#define ROBOTINFOVIEWER_H

#include <QQuickPaintedItem>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QMutex>
#include <QPainterPath>
#include <thread>
#include "Semaphore.h"
class RobotInfoViewer : public QQuickPaintedItem{
    Q_OBJECT
    Q_PROPERTY(int type READ type WRITE setType)
    Q_PROPERTY(bool draw READ ifDraw WRITE setDraw)
public:
    Q_INVOKABLE void resetSize(int,int);
    void paint(QPainter* painter) override;
    RobotInfoViewer(QQuickItem *parent = 0);
    virtual ~RobotInfoViewer() {}
    inline int type() { return this->_type; }
    inline void setType(int t) { this->_type = t; }
    inline bool ifDraw() { return this->_draw; }
    inline void setDraw(bool t) { this->_draw = t; }
public slots:
    void draw();
private slots:
    void repaint();
private:
    void init();
    void run();
private:
    std::thread _t;
private:
    bool _draw;
    int _type;
    QPixmap *pixmap;
    QPainter pixmapPainter;
    QRect area;
    QMutex pixmapMutex;

    QImage *image;
    QPainter imagePainter;
    QMutex imageMutex;

    QPen pen;
    Semaphore sema;
private:
    void drawRobotInfo(int);
    void drawValue(double,double,double);
private:
    double x(int,double);
    double y(int,double);
    double w(double);
    double h(double);
    double a(double);
    double r(double);
    double totalWidth;
    double totalHeight;
    double singleWidth;
    double singleHeight;
};

#endif // ROBOTINFOVIEWER_H
