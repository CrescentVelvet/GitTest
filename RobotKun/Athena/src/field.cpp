#include "field.h"
#include "visionmodule.h"
#include "maintain.h"
#include "globaldata.h"
#include "parammanager.h"
#include "globalsettings.h"
#include "simulator.h"
#include "zss_debug.pb.h"
#include "rec_recorder.h"
#include "ballstate.h"
#include <QColor>
#include <QFontDatabase>
#include <QtMath>
#include <QtDebug>
#include <iostream>
#include "geometry.h"
#include <QElapsedTimer>
#include <thread>
namespace {
const static float CAM_HEIGHT = 3800;
const static float MIN_LENGTH = 500;//area length : mm
Qt::KeyboardModifiers mouse_modifiers;
const static QColor CAR_COLOR[2]  = {QColor("#004AB0"), QColor("#E0BE4F")};
const static QColor CAR_SHADOW[2] = {QColor("#66004AB0"), QColor("#66E0C116")};
//const static QColor CAR_SHADOW[2] = {QColor(25, 30, 150), QColor(241, 201, 50)};
const static QColor CAR_DIR[2] = {Qt::white, Qt::white};
const static QColor FONT_COLOR[2] = {QColor("#ffaaaaaa"), QColor("#ffaaaaaa")};
const static QColor DEBUG_COLOR[10] = {
    QColor("#DDFFCC99"),// white
    QColor("#DDE62C29"),// red
    QColor("#DDE69929"),// orange
    QColor("#DDE6E429"),// yellow
    QColor("#DD16E070"),// green
    QColor("#DD078BE6"),// cran
    QColor("#DD2936E6"),// blue
    QColor("#DD7D1EE6"),// purple
    QColor("#DD997900"),// gray
    QColor("#DD5E3D80") // black
};
const static QColor DEBUG_BRUSH_COLOR = QColor(255, 255, 255, 20);
const static QColor COLOR_ORANGE(224, 60, 138);
const static QColor COLOR_ORANGE_SHADOW(224, 60, 138, 50);
const static QColor COLOR_TRANSORANGE(200, 150, 85, 80);
const static QColor COLOR_DARKGREEN(48, 48, 48);
const static QColor COLOR_PINK("#E66045");
const static QColor COLOR_PINK_SHADOW("#66E66045");
const static QColor COLOR_RED("#DDE03849");
const static QColor COLOR_RED_SHADOW("#66E03849");
const static QColor COLOR_LIGHTWHITE(255, 255, 255, 20);
const static QColor COLOR_BLUE(0,137,167);
const static QColor COLOR_LIGHTBLUE(0,137,167, 50);
const static QColor COLOR_GREEN(27,129,62);
const static QColor COLOR_YELLOW(221,210,59);
const static QColor COLOR_LIGHTYELLOW(221,210,59, 50);
const static QColor COLOR_WHITE(255,255,255,50);
const int COLOR_USE_RGB = 10;
const static qreal zoomCoefficient = 0.95;
const static qreal zoomMin = 0.02;
const int ADebugMessage = 2;
const int ballRatio = 2;
int canvasHeight;
int canvasWidth;
int param_width;
int param_height;
int param_canvas_width;
int param_canvas_height;
int param_goalWidth;
int param_goalDepth;
int param_centerCircleRadius;
bool if_ellipse_penalty;
// rectangle penalty
int param_penaltyWidth;
int param_penaltyLength;

auto zpm = ZSS::ZParamManager::instance();
bool isSimulation;
int ballDiameter;
int shadowDiameter;
int carDiameter;
int carFaceWidth;
int numberSize;
int debugPointSize;
qreal zoomRatio = 1;
QPoint zoomStart = QPoint(0, 0);
QRect area;
// for field lines;
QPainterPath painterPath;
QPen pen = QPen(QColor(150, 150, 150), 1);
double x(double _x) {
    return (_x * canvasWidth / param_canvas_width + canvasWidth / 2.0 - zoomStart.x()) / zoomRatio;
}
double y(double _y) {
    return (-_y * canvasHeight / param_canvas_height + canvasHeight / 2.0 - zoomStart.y()) / zoomRatio;
}
QPointF p(const QPointF& _p) {
    return QPointF(x(_p.x()), y(_p.y()));
}
double w(double _w) {
    return _w * canvasWidth / param_canvas_width / zoomRatio;
}
double h(double _h) {
    return -_h * canvasHeight / param_canvas_height / zoomRatio;
}
double a(double _a) {
    return _a * 16;
}
double r(double _r) {
    return _r * 16;
}
double orx(double _x) {
    return (_x - canvasWidth / 2.0) * param_canvas_width / canvasWidth;
}
double ory(double _y) {
    return -(_y - canvasHeight / 2.0) * param_canvas_height / canvasHeight;
}
double orw(double _w) {
    return (_w) * param_canvas_width / canvasWidth;
}
double orh(double _h) {
    return -(_h) * param_canvas_height / canvasHeight;
}
double rx(double x) {
    return ::orx(zoomStart.x() + x * zoomRatio);
}
double ry(double y) {
    return ::ory(zoomStart.y() + y * zoomRatio);
}
QPointF rp(const QPointF& p) {
    return QPointF(rx(p.x()), ry(p.y()));
}
double distance2(double dx, double dy) {
    return dx * dx + dy * dy;
}
template<typename T>
T limitRange(T value, T minValue, T maxValue) {
    return value > maxValue ? maxValue : (value < minValue) ? minValue : value;
}
auto GS = GlobalSettings::instance();

// draw Score
std::thread* score_thread = nullptr;


//multi-car selected
bool selectRobots;
int robotID[PARAM::ROBOTNUM];
int robotTeam;
auto originRobot = GlobalData::instance()->processRobot[0];
}
namespace LeftEvent {
QPoint start;
}
namespace MiddleEvent {
QPoint start;
QPoint zoomStart;
}
qreal Field::getpredict_x() {
    return Maintain::instance()->getpredict_x();
}
qreal Field::getpredict_y() {
    return Maintain::instance()->getpredict_y();
}
Field::Field(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , pixmap(nullptr)
    , image(nullptr)
    , cameraMode(true)
    , _type(-1)
    , socket_score(nullptr) {
    setFillColor(COLOR_DARKGREEN);
    zpm->loadParam(canvasHeight, "canvas/height", 960);
    zpm->loadParam(canvasWidth, "canvas/width", 1280);
    connect(VisionModule::instance(), SIGNAL(needDraw()), this, SLOT(draw()));
    setImplicitWidth(canvasWidth);
    setImplicitHeight(canvasHeight);
    pixmap = new QPixmap(QSize(canvasWidth, canvasHeight));
    image = new QImage(QSize(canvasWidth,canvasHeight),QImage::Format_ARGB32_Premultiplied);
    pixmapPainter.begin(pixmap);
    imagePainter.begin(image);
    score_pixmap = new QPixmap(QSize(canvasWidth, canvasHeight));
    scorePainter.begin(score_pixmap);
    init();
    zpm->loadParam(isSimulation, "Alert/IsSimulation", false);
    zpm->loadParam(ballDiameter, "size/ballDiameter", 100);
    zpm->loadParam(shadowDiameter, "size/shadowDiameter", 30);
    zpm->loadParam(carDiameter, "size/carDiameter", 180);
    zpm->loadParam(carFaceWidth, "size/carFaceWidth", 120);
    zpm->loadParam(numberSize, "size/numberSize", 200);
    zpm->loadParam(debugPointSize, "size/debugPointSize", 3);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    //setAcceptHoverEvents(true);
    connect(GS, SIGNAL(needRepaint()), this, SLOT(repaint()));
    resetAfterMouseEvent();

    //multi-car selected
    std::fill_n(robotID, PARAM::ROBOTNUM, -1);

    // draw Score
    score_thread = new std::thread([ = ] {receiveScore();});
    _t = std::thread([&]{run();});
    _t.detach();

//    sema.Signal();
    //record
    ZRecRecorder::instance()->init();
}

void Field::resetSize(int width,int height){
    //qDebug() << "field resize!!! " << width << height;
    if (width <= 0 || height <= 0) return;
    if (width == area.width() && height == area.height()) return;

    area = QRect(0, 0, width, height);

    pixmapMutex.lock();
    if(pixmapPainter.isActive())
        pixmapPainter.end();
    delete pixmap;
    setSize(width,height);
    pixmap = new QPixmap(QSize(width, height));
    pixmapPainter.begin(pixmap);
    pixmapPainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);
    pixmapMutex.unlock();

    imageMutex.lock();
    if(imagePainter.isActive())
        imagePainter.end();
    delete image;
    image = new QImage(QSize(width, height),QImage::Format_ARGB32_Premultiplied);
    imagePainter.begin(image);
    imagePainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    imageMutex.unlock();

    init();
}

void Field::paint(QPainter* painter) {
    if(pixmapMutex.try_lock()){
        painter->drawPixmap(area, *pixmap);
        pixmapMutex.unlock();
    }
}
void Field::mousePressEvent(QMouseEvent *e) {
    pressed = e->buttons();
    checkClosestRobot(rx(e->x()), ry(e->y()));
    start = end = rp(e->pos());
    mouse_modifiers = e->modifiers();
    switch(pressed) {
    case Qt::LeftButton:
        leftPressEvent(e);
        break;
    case Qt::RightButton:
        rightPressEvent(e);
        break;
    case Qt::MiddleButton:
        middlePressEvent(e);
        break;
    default:
        break;
    }
    repaint();
}
void Field::mouseMoveEvent(QMouseEvent *e) {
    end = rp(e->pos());
    switch(pressed) {
    case Qt::LeftButton:
        leftMoveEvent(e);
        break;
    case Qt::RightButton:
        rightMoveEvent(e);
        break;
    case Qt::MiddleButton:
        middleMoveEvent(e);
        break;
    default:
        break;
    }
    repaint();
}
void Field::mouseReleaseEvent(QMouseEvent *e) {
    switch(pressed) {
    case Qt::LeftButton:
        leftReleaseEvent(e);
        break;
    case Qt::RightButton:
        rightReleaseEvent(e);
        break;
    case Qt::MiddleButton:
        middleReleaseEvent(e);
        break;
    default:
        break;
    }
    resetAfterMouseEvent();
    repaint();
    //Simulator::instance()->setBall(rx(e->x())/1000.0,ry(e->y())/1000.0);
}
void Field::resetAfterMouseEvent() {
    pressed = 0;
    pressedRobot = false;
    start = end = QPoint(-9999, -9999);
    mouse_modifiers = Qt::NoModifier;
}
void Field::checkClosestRobot(double x, double y) {
    double limit = carDiameter * carDiameter / 4;
    auto& vision = GlobalData::instance()->processRobot[0];
    for(int color = PARAM::BLUE; color <= PARAM::YELLOW; color++) {
        for(int j = 0; j < PARAM::ROBOTNUM; j++) {
            auto& robot = vision.robot[color][j];
            if(distance2(robot.pos.x() - x, robot.pos.y() - y) < limit) {
                if (!selectRobots) {
                    robotID[0] = j;
                    robotTeam = color;
                }
                pressedRobot = true;
                return;
            }
        }
    }
    pressedRobot = false;
}
void Field::leftMoveEvent(QMouseEvent *e) {
    switch(mouse_modifiers) {
    case Qt::NoModifier :
        if(pressedRobot) {
            for (auto& i : robotID) {
                if (i == -1) continue;
                auto& robot = originRobot.robot[robotTeam][i];
                auto dir = robot.angle;
                Simulator::instance()->setRobot((rx(e->x()) - rx(LeftEvent::start.x())) / 1000.0 + robot.pos.x() /1000, ry(e->y())/1000 - ry(LeftEvent::start.y()) / 1000.0 + robot.pos.y() /1000, i, robotTeam == PARAM::YELLOW, dir * 180 / M_PI);
//                if (i == 0) qDebug() << "wzdebug: " << (rx(e->x()) - rx(LeftEvent::start.x())) / 1000.0 + robot.pos.x() /1000 << ry(e->y())/1000 - ry(LeftEvent::start.y()) / 1000.0 + robot.pos.y() /1000
//                                     << rx(LeftEvent::start.y()) / 1000 << robot.pos.y() /1000;
                //auto dir = GlobalData::instance()->processRobot[0].robot[robotTeam][GlobalData::instance()->maintain[0].robotIndex[robotTeam][robotID[i]]].angle;
            }
        } else {
            Simulator::instance()->setBall(rx(e->x()) / 1000.0, ry(e->y()) / 1000.0);
        }
        break;
    case Qt::ControlModifier :
        leftCtrlModifierPressEvent(e);
        break;
    case Qt::AltModifier :
        leftAltModifierPressEvent(e);
        break;
    }
}
void Field::leftPressEvent(QMouseEvent *e) {
    LeftEvent::start.setX(e->x());
    LeftEvent::start.setY(e->y());
    originRobot = GlobalData::instance()->processRobot[0];
    leftMoveEvent(e);
}
void Field::leftReleaseEvent(QMouseEvent *e) {
    switch(mouse_modifiers) {
    case Qt::NoModifier:
        leftMoveEvent(e);
        break;
    case Qt::ControlModifier :
        leftCtrlModifierReleaseEvent(e);
        break;
    case Qt::AltModifier:
        leftAltModifierReleaseEvent(e);
        break;
    }

}
void Field::leftCtrlModifierMoveEvent(QMouseEvent *e) {
    auto x1 = ::rx(e->x());
    auto x2 = ::rx(LeftEvent::start.x());
    auto y1 = ::ry(e->y());
    auto y2 = ::ry(LeftEvent::start.y());
    auto minX = std::min(x1, x2);
    auto maxX = std::max(x1, x2);
    auto minY = std::min(y1, y2);
    auto maxY = std::max(y1, y2);
    if(maxX - minX < MIN_LENGTH * zoomRatio || maxY - minY < MIN_LENGTH * zoomRatio) {
        GlobalSettings::instance()->resetSelectCarArea();
        std::fill_n(robotID, PARAM::ROBOTNUM, -1);
    }
    else {
        robotTeam = PARAM::BLUE;
        GlobalSettings::instance()->setSelectCarArea(minX, maxX, minY, maxY);
    }
}
void Field::leftCtrlModifierPressEvent(QMouseEvent *e) {
    leftCtrlModifierMoveEvent(e);
}
void Field::leftCtrlModifierReleaseEvent(QMouseEvent *e) {
    leftCtrlModifierMoveEvent(e);
    selectRobots = false;
    auto& vision = GlobalData::instance()->processRobot[0];
    for(int j = 0; j < PARAM::ROBOTNUM ; j++) {
        auto& robot = vision.robot[robotTeam][j];
        if(robot.pos.x() < GS->selectCarMaxX && robot.pos.x() > GS->selectCarMinX && robot.pos.y() < GS->selectCarMaxY && robot.pos.y() > GS->selectCarMinY) {
            robotID[j] = j;
            selectRobots = true;
        }
    }
    GlobalSettings::instance()->resetSelectCarArea();
}
void Field::leftAltModifierMoveEvent(QMouseEvent *e) {
    auto x1 = ::rx(e->x());
    auto x2 = ::rx(LeftEvent::start.x());
    auto y1 = ::ry(e->y());
    auto y2 = ::ry(LeftEvent::start.y());
    auto minX = std::min(x1, x2);
    auto maxX = std::max(x1, x2);
    auto minY = std::min(y1, y2);
    auto maxY = std::max(y1, y2);
    if(maxX - minX < MIN_LENGTH * zoomRatio || maxY - minY < MIN_LENGTH * zoomRatio) {
        GlobalSettings::instance()->resetSelectCarArea();
        std::fill_n(robotID, PARAM::ROBOTNUM, -1);
    }
    else {
        robotTeam = PARAM::YELLOW;
        GlobalSettings::instance()->setSelectCarArea(minX, maxX, minY, maxY);
    }
}
void Field::leftAltModifierPressEvent(QMouseEvent *e) {
    leftAltModifierMoveEvent(e);
}
void Field::leftAltModifierReleaseEvent(QMouseEvent *e) {
    leftAltModifierMoveEvent(e);
    selectRobots = false;
    auto& vision = GlobalData::instance()->processRobot[0];
    for(int j = 0; j < PARAM::ROBOTNUM ; j++) {
        auto& robot = vision.robot[robotTeam][j];
        if(robot.pos.x() < GS->selectCarMaxX && robot.pos.x() > GS->selectCarMinX && robot.pos.y() < GS->selectCarMaxY && robot.pos.y() > GS->selectCarMinY) {
            robotID[j] = j;
            selectRobots = true;
        }
    }
    GlobalSettings::instance()->resetSelectCarArea();
}
void Field::rightMoveEvent(QMouseEvent *e) {
    QLineF line(start, end);
    if(pressedRobot) {
        displayData = -line.angle();
        if(displayData < -180) displayData += 360;
        for (auto& i : robotID) {
            if (i == -1) continue;
            auto& robot = GlobalData::instance()->processRobot[0].robot[robotTeam][i];
            Simulator::instance()->setRobot(robot.pos.x() /1000, robot.pos.y() /1000, i, robotTeam == PARAM::YELLOW, displayData);
        }
    } else {
        displayData = ballRatio * line.length() / 1000.0;
    }
}
void Field::rightPressEvent(QMouseEvent *e) {

}
void Field::rightReleaseEvent(QMouseEvent *e) {
    QLineF line(start, end);
    if(!pressedRobot) {
        Simulator::instance()->setBall(start.x() / 1000.0, start.y() / 1000.0, ballRatio * line.dx() / 1000.0, ballRatio * line.dy() / 1000.0);
    }
}
void Field::middleMoveEvent(QMouseEvent *e) {
    switch(mouse_modifiers) {
    case Qt::NoModifier:
        middleNoModifierMoveEvent(e);
        break;
    case Qt::AltModifier:
        middleAltModifierMoveEvent(e);
        break;
    case Qt::ControlModifier:
        middleCtrlModifierMoveEvent(e);
        break;
    default:
        break;
    }
}
void Field::middlePressEvent(QMouseEvent *e) {
    MiddleEvent::start.setX(e->x());
    MiddleEvent::start.setY(e->y());
    switch(mouse_modifiers) {
    case Qt::NoModifier:
        middleNoModifierPressEvent(e);
        break;
    case Qt::AltModifier:
        middleAltModifierPressEvent(e);
        break;
    case Qt::ControlModifier:
        middleCtrlModifierPressEvent(e);
        break;
    default:
        break;
    }
}
void Field::middleReleaseEvent(QMouseEvent *e) {
    switch(mouse_modifiers) {
    case Qt::NoModifier:
        middleNoModifierReleaseEvent(e);
        break;
    case Qt::AltModifier:
        middleAltModifierReleaseEvent(e);
        break;
    case Qt::ControlModifier:
        middleCtrlModifierReleaseEvent(e);
        break;
    default:
        break;
    }
}
void Field::middleNoModifierMoveEvent(QMouseEvent *e) {
    auto t = MiddleEvent::zoomStart + zoomRatio * (MiddleEvent::start - QPoint(e->x(), e->y()));
    zoomStart.setX(limitRange(t.x(), 0, int(area.width() * (1 - zoomRatio))));
    zoomStart.setY(limitRange(t.y(), 0, int(area.height() * (1 - zoomRatio))));
    initPainterPath();
}
void Field::middleNoModifierPressEvent(QMouseEvent *e) {
    MiddleEvent::zoomStart = zoomStart;
}
void Field::middleNoModifierReleaseEvent(QMouseEvent *e) {}
void Field::middleAltModifierMoveEvent(QMouseEvent *e) {
    middleAltModifierPressEvent(e);
}
void Field::middleAltModifierPressEvent(QMouseEvent *e) {
    GlobalSettings::instance()->setBallPlacementPos(::rx(e->x()), ::ry(e->y()));
}
void Field::middleAltModifierReleaseEvent(QMouseEvent *e) {
    middleAltModifierPressEvent(e);
}
void Field::middleCtrlModifierMoveEvent(QMouseEvent *e) {
    auto x1 = ::rx(e->x());
    auto x2 = ::rx(MiddleEvent::start.x());
    auto y1 = ::ry(e->y());
    auto y2 = ::ry(MiddleEvent::start.y());
    auto minX = std::min(x1, x2);
    auto maxX = std::max(x1, x2);
    auto minY = std::min(y1, y2);
    auto maxY = std::max(y1, y2);
    if(maxX - minX < MIN_LENGTH * zoomRatio || maxY - minY < MIN_LENGTH * zoomRatio)
        GlobalSettings::instance()->resetArea();
    else
        GlobalSettings::instance()->setArea(minX, maxX, minY, maxY);
}
void Field::middleCtrlModifierPressEvent(QMouseEvent *e) {
    middleCtrlModifierMoveEvent(e);
}
void Field::middleCtrlModifierReleaseEvent(QMouseEvent *e) {
    middleCtrlModifierMoveEvent(e);
}
#if QT_CONFIG(wheelevent)
void Field::wheelEvent (QWheelEvent *e) {
    qreal oldRatio = zoomRatio;
    zoomRatio = (e->delta() < 0 ? oldRatio/zoomCoefficient : oldRatio*zoomCoefficient);
    zoomRatio = limitRange(zoomRatio, zoomMin, 1.0);
    zoomStart -= e->pos() * (zoomRatio - oldRatio);
    zoomStart.setX(limitRange(zoomStart.x(), 0, int(area.width() * (1 - zoomRatio))));
    zoomStart.setY(limitRange(zoomStart.y(), 0, int(area.height() * (1 - zoomRatio))));
    initPainterPath();
    repaint();
}
#endif
void Field::setSize(int width, int height) {
    canvasWidth = width;
    canvasHeight = height;
    zpm->changeParam("canvas/height", canvasHeight);
    zpm->changeParam("canvas/width", canvasWidth);
}
void Field::init() {
    zpm->loadParam(param_width, "field/width", PARAM::Field::PITCH_LENGTH);
    zpm->loadParam(param_height, "field/height", PARAM::Field::PITCH_WIDTH);
    zpm->loadParam(param_canvas_width, "field/canvasWidth", 13200);
    zpm->loadParam(param_canvas_height, "field/canvasHeight", 9900);
    zpm->loadParam(param_goalWidth, "field/goalWidth", PARAM::Field::GOAL_WIDTH);
    zpm->loadParam(param_goalDepth, "field/goalDepth",  PARAM::Field::GOAL_DEPTH);
    zpm->loadParam(param_penaltyWidth, "field/penaltyWidth", PARAM::Field::PENALTY_AREA_DEPTH);
    zpm->loadParam(param_penaltyLength, "field/penaltyLength", PARAM::Field::PENALTY_AREA_WIDTH);
    zpm->loadParam(param_centerCircleRadius, "field/centerCircleRadius",  PARAM::Field::CENTER_CIRCLE_R);
    ::area = QRect(0, 0, this->property("width").toReal(), this->property("height").toReal());
//    ::size = QSize(this->property("width").toReal()/2.0, this->property("height").toReal()/2.0);
    pressedRobot = false;
    initPainterPath();
    repaint();
}
void Field::repaint() {//change here!!!!!!!
    if(pixmapMutex.try_lock()){
        pixmap->fill(COLOR_DARKGREEN);
        paintInit();
        imageMutex.lock();
        pixmapPainter.drawImage(0,0,*image);
        imageMutex.unlock();
        pixmapMutex.unlock();
        this->update(area);
    }
    sema.Signal();
}
void Field::run(){
    while (true) {
        sema.Wait();
        if(!_draw)
            continue;
        if(imageMutex.try_lock()){
            image->fill(Qt::transparent);
            switch(_type) {
            case 1:
                drawOriginVision(0);
                break;
            case 2:
                imagePainter.setOpacity(0.3);
                score_mutex.lock();
                imagePainter.drawPixmap(0, 0, *score_pixmap);
                score_mutex.unlock();
                imagePainter.setOpacity(1);
                drawMaintainVision(0);
                if (selectRobots) paintSelectedCar();
                drawDebugMessages(PARAM::BLUE); //BLUE
                drawDebugMessages(ADebugMessage);
                break;
            case 3:
                score_mutex.lock();
                score_mutex.unlock();
                drawMaintainVision(0);
                if (selectRobots) paintSelectedCar();
                drawDebugMessages(PARAM::YELLOW); //YELLOW
                drawDebugMessages(ADebugMessage);
                break;
            default:
                paintInit();
            }
            drawBallLine();
            imageMutex.unlock();
        }
    }
}

void Field::paintInit() {
    drawCtrlC();
    pixmapPainter.strokePath(painterPath, pen);
    drawSelectedArea();
}
void Field::draw() {
    repaint();
}
void Field::drawBallLine() {
    if(pressed == Qt::RightButton) {
        imagePainter.setBrush(QBrush(FONT_COLOR[0]));
        imagePainter.setPen(QPen(FONT_COLOR[0], ::w(20), Qt::DashLine));
        imagePainter.drawLine(p(start), p(end));
        imagePainter.drawText(p(end), QString::fromStdString(std::to_string(displayData)));
    }
}
void Field::initPainterPath() {
    pen.setWidth(::w(30));
    painterPath = QPainterPath();
    painterPath.addRect(::x(-param_width / 2.0), ::y(-param_height / 2.0), ::w(param_width), ::h(param_height));
    painterPath.addRect(::x(-param_width / 2.0), ::y(-param_goalWidth / 2.0), ::w(-param_goalDepth), ::h(param_goalWidth));
    painterPath.addRect(::x(param_width / 2.0), ::y(-param_goalWidth / 2.0), ::w(param_goalDepth), ::h(param_goalWidth));
    painterPath.moveTo(::x(-param_width / 2.0), ::y(0));
    painterPath.lineTo(::x(param_width / 2.0), ::y(0));
    painterPath.moveTo(::x(0), ::y(param_height / 2.0));
    painterPath.lineTo(::x(0), ::y(-param_height / 2.0));
    painterPath.addEllipse(::x(-param_centerCircleRadius), ::y(-param_centerCircleRadius), ::w(2 * param_centerCircleRadius), ::h(2 * param_centerCircleRadius));
    painterPath.addRect(::x(-param_width / 2.0), ::y(-param_penaltyLength / 2.0), ::w(param_penaltyWidth), ::h(param_penaltyLength));
    painterPath.addRect(::x(param_width / 2.0), ::y(-param_penaltyLength / 2.0), ::w(-param_penaltyWidth), ::h(param_penaltyLength));
}
void Field::drawOriginVision(int index) {
    for(int i = 0; i < PARAM::CAMERA; i++) {
        if (GlobalData::instance()->cameraControl[i] == true)
            drawVision(GlobalData::instance()->camera[i][index]);
    }
    for (int i = 0; i < PARAM::CAMERA; i++) {
        if (GlobalData::instance()->cameraControl[i] == true) {
            imagePainter.setBrush( Qt::NoBrush);
            imagePainter.setPen(COLOR_GREEN);
            imagePainter.drawRect(QRect(QPoint(::x(GlobalData::instance()->cameraMatrix[i].leftedge.min), ::y(GlobalData::instance()->cameraMatrix[i].downedge.min)), QPoint(::x(GlobalData::instance()->cameraMatrix[i].rightedge.min), ::y(GlobalData::instance()->cameraMatrix[i].upedge.min))));
            //qDebug()<<GlobalData::instance()->cameraMatrix[i].leftedge.min<<GlobalData::instance()->cameraMatrix[i].downedge.min<<GlobalData::instance()->cameraMatrix[i].rightedge.min<<GlobalData::instance()->cameraMatrix[i].upedge.min;
            imagePainter.setPen(COLOR_RED);
            imagePainter.drawRect(QRect(QPoint(::x(GlobalData::instance()->cameraMatrix[i].leftedge.max), ::y(GlobalData::instance()->cameraMatrix[i].downedge.max)), QPoint(::x(GlobalData::instance()->cameraMatrix[i].rightedge.max), ::y(GlobalData::instance()->cameraMatrix[i].upedge.max))));
            imagePainter.setFont(QFont("Helvetica [Cronyx]", 13, QFont::Bold));
            imagePainter.drawText(::x(GlobalData::instance()->cameraMatrix[i].campos.x() - 100), ::y(GlobalData::instance()->cameraMatrix[i].campos.y()), "Camera" + QString::number(i));
        }
    }
}
void Field::drawMaintainVision(int index) {
    for(int i = -99; i < 0; i ++) {
//        drawVision(GlobalData::instance()->maintain[index + i],true);
        auto& ball = GlobalData::instance()->maintain[index + i].ball[0];
        paintShadow(COLOR_TRANSORANGE, ball.pos.x(), ball.pos.y());
    }
    const OriginMessage &robot_vision = GlobalData::instance()->processRobot[index];
    auto last_touch = GlobalData::instance()->lastTouch % PARAM::ROBOTNUM;
    auto last_touch_team = GlobalData::instance()->lastTouch < PARAM::ROBOTNUM ? PARAM::BLUE : PARAM::YELLOW;
    if (last_touch < PARAM::ROBOTNUM){
        for(int color = PARAM::BLUE; color <= PARAM::YELLOW; color++) {
            for(int j = 0; j < PARAM::ROBOTNUM; j++) {
                auto& robot = robot_vision.robot[color][j];
                if (!robot.valid) continue;
                paintCar(CAR_COLOR[color], j, robot.pos.x(), robot.pos.y(), robot.angle, true, FONT_COLOR[color], j == last_touch && color == last_touch_team);
//            paintCarShadow(robot.pos.x(), robot.pos.y(), robot.angle);
            }
        }
    }

    auto& maintain = GlobalData::instance()->maintain[index];

    for(int color = PARAM::BLUE; color <= PARAM::YELLOW; color++) {
      //  for(int j = 0; j < maintain.robotSize[color]; j++) {
        for(int j = 0; j < PARAM::ROBOTNUM; j++) {
            auto& robot = maintain.robot[color][j];//change by lzx
            if(!robot.valid) continue;
            paintCarShadow(CAR_SHADOW[color],robot.pos.x(), robot.pos.y(), robot.angle);
        }
    }
    for(int j = 0; j < maintain.ballSize; j++) {
        auto& ball = maintain.ball[j];
        paintBall(maintain.isBallValid ? COLOR_ORANGE : COLOR_ORANGE_SHADOW, ball.pos.x(), ball.pos.y(),ballDiameter/2.0);
        paintFocus(maintain.isBallValid ? COLOR_PINK : COLOR_PINK_SHADOW, ball.pos.x(), ball.pos.y(), 500, ballFocusCount++);
    }
    auto& predict_pos = GlobalData::instance()->maintain[0].ball[0].best_predict_pos;
    auto& ball_state = GlobalData::instance()->maintain[0].ball[0].ball_state_machine;
    auto& real_ball_pos = GlobalData::instance()->maintain[0].ball[0].real_pos;
    auto cam_h = CAM_HEIGHT;
    auto ball_h = real_ball_pos.z();
    if(ball_state == State::_chip_pass){
        paintBall(COLOR_WHITE, predict_pos.x(), predict_pos.y(),ballDiameter);
        paintBall(COLOR_WHITE, real_ball_pos.x(), real_ball_pos.y(),cam_h/(cam_h-ball_h)*ballDiameter);
//        paintBall(COLOR_WHITE, real_ball_pos.x(), real_ball_pos.y(),ballDiameter*2);
        imagePainter.setBrush(Qt::NoBrush);
        imagePainter.setPen(QPen(COLOR_ORANGE, ::w(100)));
        QFont font;
        int fontSize = ::h(-numberSize);
        font.setPixelSize(fontSize);
        imagePainter.drawText(::p(QPointF(predict_pos.x(), predict_pos.y())),"P_BPOS");
        imagePainter.drawText(::p(QPointF(real_ball_pos.x(), real_ball_pos.y())),"S_BPOS");
    }
}
void Field::paintCar(const QColor& color, quint8 num, qreal x, qreal y, qreal radian, bool ifDrawNum, const QColor& textColor, bool needCircle) {
    static qreal radius = carDiameter / 2.0;
    static qreal chordAngel = qRadiansToDegrees(qAcos(1.0 * carFaceWidth / carDiameter));
    imagePainter.setBrush(QBrush(color));
    imagePainter.setPen(QPen(COLOR_RED, ::w(30), Qt::DotLine));
    imagePainter.drawLine(QPointF(::x(x), ::y(y)), QPointF(::x(x), ::y(y)) + QPointF(::w(200) * qSin(radian + M_PI_2), ::w(200) * qCos(radian + M_PI_2)));
    imagePainter.setPen(Qt::NoPen);
    imagePainter.drawChord(QRectF(::x(x - radius), ::y(y - radius), ::w(2 * radius), ::h(2 * radius)), ::a(90.0 - chordAngel + 180 / M_PI * radian), ::r(180.0 + 2 * chordAngel));
    if (needCircle) {
        imagePainter.setBrush(Qt::NoBrush);
        imagePainter.setPen(QPen(COLOR_BLUE, ::w(30)));
        imagePainter.drawChord(QRectF(::x(x - radius), ::y(y - radius), ::w(2 * radius), ::h(2 * radius)), ::a(90.0 - chordAngel + 180 / M_PI * radian), ::r(180.0 + 2 * chordAngel));
    }
    if (ifDrawNum) {
        imagePainter.setBrush(Qt::NoBrush);
        imagePainter.setPen(QPen(textColor, ::w(30)));
        QFont font;
        int fontSize = ::h(-numberSize);
        font.setPixelSize(fontSize);
        imagePainter.setFont(font);
        imagePainter.drawText(::x(x - numberSize), ::y(y + carDiameter * 0.4), QString::number(num, 16).toUpper());
    }
}
void Field::paintSelectedCar() {
    static qreal radius = carDiameter / 2.0;
    static qreal chordAngel = qRadiansToDegrees(qAcos(1.0 * carFaceWidth / carDiameter));
    for (auto& i : robotID) {
        if (i == -1) continue;
        auto& robot = GlobalData::instance()->processRobot[0].robot[robotTeam][i];
        imagePainter.setBrush(Qt::NoBrush);
        imagePainter.setPen(QPen(COLOR_GREEN, ::w(50)));
        imagePainter.drawChord(QRectF(::x(robot.pos.x() - radius), ::y(robot.pos.y() - radius), ::w(2 * radius), ::h(2 * radius)), ::a(90.0 - chordAngel + 180 / M_PI * robot.angle), ::r(180.0 + 2 * chordAngel));
    }
}
void Field::paintCarShadow(const QColor& color,qreal x, qreal y, qreal radian) {
    static qreal radius = carDiameter / 2.0;
    static qreal chordAngel = qRadiansToDegrees(qAcos(1.0 * carFaceWidth / carDiameter));
    imagePainter.setBrush(QBrush(color));
    imagePainter.setPen(Qt::NoPen);
//    imagePainter.drawLine(QPointF(::x(x), ::y(y)), QPointF(::x(x), ::y(y)) + QPointF(30 * qSin(radian + M_PI_2), 30 * qCos(radian + M_PI_2)));
    imagePainter.drawChord(QRectF(::x(x - radius), ::y(y - radius), ::w(2 * radius), ::h(2 * radius)), ::a(90.0 - chordAngel + 180 / M_PI * radian), ::r(180.0 + 2 * chordAngel));
}
void Field::paintBall(const QColor& color, qreal x, qreal y, qreal r) {
    imagePainter.setBrush(QBrush(color));
    imagePainter.setPen(Qt::NoPen);
    imagePainter.drawEllipse(QRectF(::x(x - r), ::y(y - r), ::w(r*2), ::h(r*2)));
}
void Field::paintpredict() {
    imagePainter.setBrush(QBrush(Qt::black));
    imagePainter.setPen(Qt::NoPen);
    imagePainter.drawEllipse(::x(getpredict_x() - ballDiameter / 2.0), ::y(getpredict_y() - ballDiameter / 2.0), ::w(ballDiameter), ::h(ballDiameter));
}
void Field::paintShadow(const QColor& color, qreal x, qreal y) {
    imagePainter.setBrush(QBrush(color));
    imagePainter.setPen(Qt::NoPen);
    imagePainter.drawEllipse(QRectF(::x(x - shadowDiameter / 2.0), ::y(y - shadowDiameter / 2.0), ::w(shadowDiameter), ::h(shadowDiameter)));
}
void Field::paintFocus(const QColor& color, qreal x, qreal y, qreal radian, qreal count) {
    static int length = 40;
    imagePainter.setBrush(Qt::NoBrush);
    imagePainter.setPen(QPen(color, ::w(15)));
    imagePainter.drawEllipse(QRectF(::x(x - radian), ::y(y - radian), ::w(2 * radian), ::h(2 * radian)));
    imagePainter.setPen(QPen(color, ::w(30)));
    for(int i = 0; i < 4; i++) {
        imagePainter.drawLine(QPointF(::x(x + (radian + length)*qCos(count / 60.0 + i * M_PI / 2)), ::y(y + (radian + length)*qSin(count / 60.0 + i * M_PI / 2))), QPointF(::x(x + (radian - length)*qCos(count / 60.0 + i * M_PI / 2)), ::y(y + (radian - length)*qSin(count / 60.0 + i * M_PI / 2))));
    }
}
void Field::drawVision(const OriginMessage &vision, bool shadow) {
    for(int color = PARAM::BLUE; color <= PARAM::YELLOW; color++) {
        for(int j = 0; j < PARAM::ROBOTNUM; j++) {
            auto& robot = vision.robot[color][j];
            if(!shadow) {
                paintCar(CAR_COLOR[color], j, robot.pos.x(), robot.pos.y(), robot.angle, true, FONT_COLOR[color]);
            } else {
                paintShadow(CAR_SHADOW[color], robot.pos.x(), robot.pos.y());
            }
        }
    }
    //paintpredict();
    for(int j = 0; j < vision.ballSize; j++) {
        auto& ball = vision.ball[j];
        if(!shadow) {
            paintBall(COLOR_ORANGE, ball.pos.x(), ball.pos.y(),ballDiameter/2.0);
        } else {
            paintShadow(COLOR_TRANSORANGE, ball.pos.x(), ball.pos.y());
        }
    }

}
void Field::drawCtrlC() {
    pixmapPainter.setFont(QFont("Helvetica [Cronyx]", 300, QFont::Bold));
    GlobalData::instance()->ctrlCMutex.lock();
    bool ctrlC = GlobalData::instance()->ctrlC;
    GlobalData::instance()->ctrlCMutex.unlock();
    if(ctrlC) {
        pixmapPainter.drawText(QPointF(0, 300), "CTRL_C");
    }
    pixmapPainter.setFont(QFont("Helvetica [Cronyx]", 13, QFont::Bold));
}
void Field::drawSelectedArea() {
    pixmapPainter.setBrush(QBrush(COLOR_LIGHTWHITE));
    pixmapPainter.setPen(Qt::NoPen);
    pixmapPainter.drawRect(QRectF(::x(GS->minimumX), ::y(GS->minimumY), ::w(GS->maximumX - GS->minimumX), ::h(GS->maximumY - GS->minimumY)));
    pixmapPainter.setPen(QPen(QBrush(robotTeam == PARAM::BLUE ? COLOR_LIGHTBLUE : COLOR_LIGHTYELLOW), 2));
    pixmapPainter.setBrush(QBrush(robotTeam == PARAM::BLUE ? COLOR_LIGHTBLUE : COLOR_LIGHTYELLOW));
    pixmapPainter.drawRect(QRectF(::x(GS->selectCarMinX), ::y(GS->selectCarMinY), ::w(GS->selectCarMaxX - GS->selectCarMinX), ::h(GS->selectCarMaxY - GS->selectCarMinY)));
}
void Field::drawDebugMessages(int team) {
//    int oldFlag = GlobalSettings::instance()->oldLogFlag;
    static qreal chordAngel = qRadiansToDegrees(qAcos(1.0 * carFaceWidth / carDiameter));
    static ZSS::Protocol::Debug_Msgs msgs;
    GlobalData::instance()->debugMutex.lock();
    if (team == 2) {
        msgs.ParseFromArray(GlobalData::instance()->athenaDebugMessage.data(), GlobalData::instance()->athenaDebugMessage.size());
    } else if (team == 0) {
        msgs.ParseFromArray(GlobalData::instance()->debugBlueMessages.data(), GlobalData::instance()->debugBlueMessages.size());
    } else {
        msgs.ParseFromArray(GlobalData::instance()->debugYellowMessages.data(), GlobalData::instance()->debugYellowMessages.size());
    }
    GlobalData::instance()->debugMutex.unlock();
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    imagePainter.setFont(font);
    imagePainter.setBrush(QBrush(DEBUG_BRUSH_COLOR));
    for(int i = 0; i < msgs.msgs_size(); i++) {
        auto& msg = msgs.msgs(i);
        if(msg.color() == COLOR_USE_RGB) {
            int value = msg.rgb_value();
            int rgb[3];
            for(int i = 0; i < 3; i++) {
                rgb[2 - i] = value % 1000;
                value = (value - rgb[2 - i]) / 1000;
                if(rgb[2 - i]>255||rgb[2 - i]<0){//error value
                    rgb[0]=0;rgb[1]=0;rgb[2]=0;break;
                }
            }
            imagePainter.setPen(QPen(QColor(rgb[0], rgb[1], rgb[2]), ::w(10)));
        }
        else {
            imagePainter.setPen(QPen(QColor(DEBUG_COLOR[msg.color()]), ::w(10)));
        }
        double x1, x2, y1, y2;
        double minx,miny,maxx,maxy;
        switch(msg.type()) {
        case ZSS::Protocol::Debug_Msg_Debug_Type_ARC:
            x1 = msg.arc().rectangle().point1().x();
            x2 = msg.arc().rectangle().point2().x();
            y1 = msg.arc().rectangle().point1().y();
            y2 = msg.arc().rectangle().point2().y();
            minx = std::min(x1,x2);
            miny = std::min(y1,y2);
            maxx = std::max(x1,x2);
            maxy = std::max(y1,y2);
            imagePainter.drawArc(QRectF( ::x(double(minx)),
                                          ::y(double(miny)),
                                          ::w((maxx - minx)),
                                          ::h((maxy - miny))),
                                  msg.arc().start() * 16,
                                  msg.arc().end() * 16);

            break;
        case ZSS::Protocol::Debug_Msg_Debug_Type_LINE:
            imagePainter.drawLine(::x(msg.line().start().x()), ::y(msg.line().start().y()), ::x(msg.line().end().x()), ::y(msg.line().end().y()));
            break;
        case ZSS::Protocol::Debug_Msg_Debug_Type_Points: {
            QVector<QLine> lines;
            for(int i = 0; i < msg.points().point_size(); i++) {
                lines.push_back(QLine(::x((msg.points().point(i).x() + debugPointSize)), ::y((msg.points().point(i).y() + debugPointSize)), ::x((msg.points().point(i).x() - debugPointSize)), ::y((msg.points().point(i).y() - debugPointSize))));
                lines.push_back(QLine(::x((msg.points().point(i).x() - debugPointSize)), ::y((msg.points().point(i).y() + debugPointSize)), ::x((msg.points().point(i).x() + debugPointSize)), ::y((msg.points().point(i).y() - debugPointSize))));
            }
            imagePainter.drawLines(lines);
            break;
        }
        case ZSS::Protocol::Debug_Msg_Debug_Type_TEXT:
            if(msg.text().has_size()){
                font.setPointSizeF(::w(msg.text().size()));
            }
            if(msg.text().has_weight()){
                font.setWeight(msg.text().weight());
            }
            imagePainter.setFont(font);
            imagePainter.drawText(QPointF(::x(msg.text().pos().x()), ::y(msg.text().pos().y())), QString::fromStdString(msg.text().text()));
            break;
        case ZSS::Protocol::Debug_Msg_Debug_Type_ROBOT:
            imagePainter.drawChord(QRectF(::x((msg.robot().pos().x()) - 1.2*carDiameter / 2.0), ::y((msg.robot().pos().y()) + 1.2*carDiameter / 2.0), ::w((1.2*carDiameter)), ::h(-(1.2*carDiameter))),::a(90.0 - chordAngel - msg.robot().dir()), ::r(180.0 + 2 * chordAngel));
            break;
        //case Debug_Msg_Debug_Type_CURVE:
        //case Debug_Msg_Debug_Type_POLYGON:
        default:
            qDebug() << "debug message type not support!";
        }
    }
}
float Field::fieldXFromCoordinate(int x) {
    return ::rx(x);
}
float Field::fieldYFromCoordinate(int y) {
    return ::ry(y);
}

//// draw score
void Field::receiveScore() {
    socket_score = new QUdpSocket();
    socket_score->bind(QHostAddress(ZSS::LOCAL_ADDRESS), 20003);
    score_mutex.lock();
    score_pixmap->fill(COLOR_DARKGREEN);
    score_mutex.unlock();
    scorePainter.setPen(Qt::NoPen);
//    double c = 0;
//    double step = 0.0000001;
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));

//        QElapsedTimer timer;
//        timer.start();
//        for(int i=-6600;i<6600;i+=RECT_SIZE){
//            for(int j=-5000;j<5000;j+=RECT_SIZE){
//                c += step;
//                if(c >= 1 || c <= 0) step = - step;
//                c = limitRange(c,0.0,1.0);
//                auto r = limitRange(4*c-2,0.0,1.0)*255;
//                auto g = limitRange(c<0.5?2*c:4-4*c,0.0,1.0)*255;
//                auto b = limitRange(-2*c+1,0.0,1.0)*255;
//                scorePainter.setBrush(QColor(r,g,b));
//                score_mutex.lock();
//                scorePainter.drawRect(QRectF(::x(i),::y(j),::w(RECT_SIZE),::h(RECT_SIZE)));
//                score_mutex.unlock();
//            }
//        }
//        qDebug() << "mark fuck : " << timer.nsecsElapsed()/1000000.0 << "millisecond";


        parseScores(socket_score);
    }
}
void Field::parseScores(QUdpSocket* const socket) {
    static QByteArray datagram;
    static ZSS::Protocol::Debug_Scores scores;
    while (socket->state() == QUdpSocket::BoundState && socket->hasPendingDatagrams()) {
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(), datagram.size());
        scores.ParseFromArray(datagram.data(), datagram.size());
        auto size = scores.scores_size();
        for(int i = 0; i < size; i++) {
            auto score = scores.scores(i);
            auto color = score.color();
            if(color < 0 || color >= COLOR_LEVEL) {
                std::cerr << "DEBUG_SCORE : not correct color : " << color << std::endl;
                continue;
            }
            auto size = score.p_size();
            auto c = limitRange((double)(color) / COLOR_LEVEL, 0.0, 1.0);
            auto r = limitRange(4 * c - 2, 0.0, 1.0) * 255;
            auto g = limitRange(c < 0.5 ? 2 * c : 4 - 4 * c, 0.0, 1.0) * 255;
            auto b = limitRange(-2 * c + 1, 0.0, 1.0) * 255;
            scorePainter.setBrush(QColor(r, g, b));
            for(int k = 0; k < size; k++) {
                auto p = score.p(k);
                score_mutex.lock();
                scorePainter.drawRect(QRectF(::x(p.x()), ::y(p.y()), ::w(RECT_SIZE), ::h(-RECT_SIZE)));
                score_mutex.unlock();
            }
        }
    }
}
