#include <QtMath>
#include "robotinfoviewer.h"
#include "visionmodule.h"
#include "staticparams.h"
#include "globaldata.h"
#include "maintain.h"
#include "actionmodule.h"
#include "parammanager.h"
#include "globalsettings.h"
namespace  {
    auto GS = GlobalSettings::instance();
}
namespace{
const static QColor COLOR_BACKGROUND(50, 50, 50);
const static QColor COLOR_ROBOT[3] = {QColor(255,255,255),QColor(25, 30, 150, 150), QColor(220, 181, 30, 150)};
const static QColor COLOR_RED(220, 53, 47);
const static QColor COLOR_RED_TRANS(255, 0, 0, 255);
const static QColor COLOR_WHITE_GREY(200,200,200);
const static QColor COLOR_WHITE(255,255,255);
const static QColor COLOR_WHITE_TRANS(255,255,255,200);
const static QColor COLOR_ROBOT_BAN[3] = {QColor(255,255,255),QColor(255,0,150,200), QColor(150,0,255,200)};
}
namespace{
template<typename T>
T limitRange(T value, T minValue, T maxValue) {
    return value > maxValue ? maxValue : (value < minValue) ? minValue : value;
}
namespace{
double Normalize(double angle)
{
    const double M_2PI = PARAM::Math::PI * 2;

    while( angle > PARAM::Math::PI ) {
        angle -= M_2PI;
    }

    while( angle <= -PARAM::Math::PI ) {
        angle += M_2PI;
    }
    return angle;
}
}
std::thread* receiveThread = nullptr;
}
RobotInfoViewer::RobotInfoViewer(QQuickItem *parent)
    : QQuickPaintedItem (parent)
    , pixmap(nullptr)
    , image(nullptr){
    connect(VisionModule::instance(), SIGNAL(needDraw()), this, SLOT(draw()));
    connect(GS, SIGNAL(needRepaint()), this, SLOT(draw()));
    setImplicitWidth(200);
    setImplicitHeight(300);
    pixmap = new QPixmap(QSize(200, 300));
    image = new QImage(QSize(200,300),QImage::Format_ARGB32_Premultiplied);
    _t = std::thread([&]{run();});
    _t.detach();
}
void RobotInfoViewer::paint(QPainter* painter) {
    if(pixmapMutex.try_lock()){
        painter->drawPixmap(area, *pixmap);
        pixmapMutex.unlock();
    }
}
void RobotInfoViewer::draw() {
    repaint();
    if(_draw)
        sema.Signal();
}
void RobotInfoViewer::repaint(){
    if(pixmapMutex.try_lock()){
        pixmap->fill(COLOR_BACKGROUND);
        if(_draw && _type != 1)
            pixmapPainter.drawImage(0,0,*image);
        this->update(area);
        pixmapMutex.unlock();
    }
}
void RobotInfoViewer::resetSize(int width,int height){
    if (width <= 0 || height <= 0) return;
    if (width == area.width() && height == area.height()) return;

    pixmapMutex.lock();
    if(pixmapPainter.isActive()) pixmapPainter.end();
    delete pixmap;
    pixmap = new QPixmap(QSize(width, height));
    pixmapPainter.begin(pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing, true);
    pixmapPainter.setRenderHint(QPainter::TextAntialiasing, true);
    pixmapMutex.unlock();

    imageMutex.lock();
    if(imagePainter.isActive()) imagePainter.end();
    delete image;
    image = new QImage(QSize(width, height),QImage::Format_ARGB32_Premultiplied);
    imagePainter.begin(image);
    imagePainter.setRenderHint(QPainter::Antialiasing, true);
    imagePainter.setRenderHint(QPainter::TextAntialiasing, true);
    imageMutex.unlock();

    area = QRect(0, 0, width, height);

    // should be 1x16
    singleHeight = height/PARAM::Field::MAX_PLAYER;
    singleWidth = width;
    totalHeight = height;
    totalWidth = width;

    init();
}
void RobotInfoViewer::init(){
    repaint();
}
void RobotInfoViewer::run(){
    while(true){
        sema.Wait();
        imageMutex.lock();
        image->fill(COLOR_BACKGROUND);
        for(int i=0;i<PARAM::Field::MAX_PLAYER;i++){
            drawRobotInfo(i);
        }
        imageMutex.unlock();
    }
}
void RobotInfoViewer::drawRobotInfo(int n){
    const static int line_width = 5;
    const static int r_a = 90; // robot mouse angle;
    const static double r_c_x = 0.4; // robot center x / width;
    const static double r_c_y = 0.55; // robot center y / height;
    const static double r_r_s = 0.35; // robot radius size / height;
    const static double d_l = h(r_c_y) - 5; // robot direction line length;

    const static int fontSize = 22;
    const static QFont font("Courier new", fontSize, QFont::Light);

    const double center_x = x(n,r_c_x);
    const double center_y = y(n,r_c_y);
    const QPointF center(center_x,center_y);
    const QPointF bottom_r = center + QPointF(h(r_r_s),h(r_r_s));
    const QPointF bottom_l = center + QPointF(-h(r_r_s),h(r_r_s));
    auto&& p = imagePainter;

    static QColor color;

    double radian;
    double dx;

    bool visiable = GlobalData::instance()->processRobot[0].robot[_type-2][n].valid;
    if(!visiable){
        p.setPen(QPen(QColor(255,255,255,50),3,Qt::DashDotLine));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(x(n,0.1),y(n,0.1),w(0.8),h(0.8),5,5);
        return;
    }

    if(_type == 1) return;
    // infrared
    bool infrared = GlobalData::instance()->robotInformation[_type-2][n].infrared;
    color = infrared ? COLOR_RED : COLOR_ROBOT[_type-1];

    // origin robot
    p.setBrush(QBrush(color));
    p.setPen(Qt::NoPen);
    p.drawChord(x(n,r_c_x)-h(r_r_s),y(n,r_c_y)-h(r_r_s),h(r_r_s*2),h(r_r_s*2),a(r_a/2+90),r(360-r_a));

    // robot number
    p.setFont(font);
    p.setPen(COLOR_WHITE_TRANS);
    p.drawText(center+QPointF(-fontSize,fontSize/2),QString::number(n, 16).toUpper());

    // origin direction
    radian = 0;
    dx = d_l*qCos(M_PI/2+radian)*0.5;
    p.setPen(QPen(COLOR_WHITE,line_width,Qt::SolidLine));
    p.drawLine(center_x - dx,center_y,center_x + dx,center_y-d_l);

    // imu direction diff
    radian = 0;
    if(GlobalData::instance()->robotCommand[_type-2][0].robotSpeed[n].use_dir){
        radian = GlobalData::instance()->processRobot[0].robot[_type-2][n].angle - GlobalData::instance()->robotCommand[_type-2][0].robotSpeed[n].vr;
    }
    dx = d_l*qCos(M_PI/2+radian)*0.5;
    p.setPen(QPen(COLOR_RED_TRANS,3,Qt::SolidLine));
    p.drawLine(center_x - dx,center_y,center_x + dx,center_y-d_l);

    // battery and capacitance
    double battery = GlobalData::instance()->robotInformation[_type-2][n].battery;
    //qDebug()<<battery;
    double capacitance = GlobalData::instance()->robotInformation[_type-2][n].capacitance;
    drawValue(bottom_r.x() + 3,bottom_r.y(),battery);
    drawValue(bottom_r.x() + 16,bottom_r.y(),capacitance);

    // imu calibration
    bool imu_check = false;
    if(abs(radian) < 0.2) imu_check = GlobalData::instance()->robotInformation[_type-2][n].imucleaned;
    if(!imu_check){
        p.setPen(QPen(COLOR_ROBOT_BAN[_type-1],10,Qt::SolidLine));
        p.setBrush(Qt::NoBrush);
        p.drawLine(x(n,0),y(n,0),x(n,1),y(n,1));
        p.drawLine(x(n,1),y(n,0),x(n,0),y(n,1));
    }
}
void RobotInfoViewer::drawValue(double x,double y,double v){
    const static int rect_width = 10;
    const static int rect_height = -30;
    auto&& p = imagePainter;

    static QColor color;

    double cc = limitRange(1.6*v-0.6,0.0,1.0);
    color.setHsl(cc*140,255,v*100+50);

    // rectangle border
    p.setPen(QPen(COLOR_WHITE_GREY,2,Qt::SolidLine));
    p.setBrush(Qt::NoBrush);
    p.drawRect(x,y,rect_width,rect_height);

    // colorful value
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawRect(x,y,rect_width,rect_height*v);
}
double RobotInfoViewer::x(int n,double _x){
    return _x*singleWidth;
}
double RobotInfoViewer::y(int n,double _y){
    return n*singleHeight + _y*singleHeight;
}
double RobotInfoViewer::w(double _w){
    return _w*singleWidth;
}
double RobotInfoViewer::h(double _h){
    return _h*singleHeight;
}
double RobotInfoViewer::a(double _a){
    return _a*16;
}
double RobotInfoViewer::r(double _r){
    return _r*16;
}
