#include "field.h"
#include "field_layers.h"
#include "field_style.h"
#include "parammanager.h"
#include <QElapsedTimer>
#include <QtDebug>
#include <cmath>
namespace{
    auto zpm = ZSS::LParamManager::instance();
    auto p = FP::instance();
    auto t = FT::instance();
    auto s = FS::instance();

    // function
    template<typename T>
    T limitRange(T value, T minValue, T maxValue) {
        return value > maxValue ? maxValue : (value < minValue) ? minValue : value;
    }
}
FieldParam::FieldParam(){
    reload();
}
FieldParam::~FieldParam(){}
void FieldParam::reload(){
    zpm->loadParam(param_width, "field/width", 9000);
    zpm->loadParam(param_height, "field/height", 6000);
    zpm->loadParam(param_canvas_width, "field/canvasWidth", 9900);
    zpm->loadParam(param_canvas_height, "field/canvasHeight", 6600);
    zpm->loadParam(param_goalWidth, "field/goalWidth", 1000);
    zpm->loadParam(param_goalDepth, "field/goalDepth",  200);
    zpm->loadParam(param_penaltyWidth, "field/penaltyWidth", 1000);
    zpm->loadParam(param_penaltyLength, "field/penaltyLength", 2000);
    zpm->loadParam(param_centerCircleRadius, "field/centerCircleRadius",  500);
    zpm->loadParam(param_lineWidth,"field/lineWidth",15);
    zpm->loadParam(size_ballDiameter, "size/ballDiameter", 100);
    zpm->loadParam(size_shadowDiameter, "size/shadowDiameter", 30);
    zpm->loadParam(size_carDiameter, "size/carDiameter", 180);
    zpm->loadParam(size_carFaceWidth, "size/carFaceWidth", 120);
    zpm->loadParam(size_number, "size/numberSize", 200);
    zpm->loadParam(size_debugPoint, "size/debugPointSize", 5);
}
FieldCoordsTransform::FieldCoordsTransform():zoomRatio(1),zoomCoefficient(0.95),zoomMin(0.15){}
FieldCoordsTransform::~FieldCoordsTransform(){}
void FieldCoordsTransform::resize(int w,int h){
    double ratio = double(::p->param_canvas_width)/::p->param_canvas_height;
    canvasWidth = std::min(w,int(h*ratio));
    canvasHeight = std::min(int(w/ratio),h);
    zoomStart.setX((canvasWidth-w)/2);
    zoomStart.setY((canvasHeight-h)/2);
}
double FieldCoordsTransform::x(double _x) {
    return (_x * canvasWidth / ::p->param_canvas_width + canvasWidth / 2.0 - zoomStart.x()) / zoomRatio;
}
double FieldCoordsTransform::y(double _y) {
    return (-_y * canvasHeight / ::p->param_canvas_height + canvasHeight / 2.0 - zoomStart.y()) / zoomRatio;
}
QPointF FieldCoordsTransform::p(QPointF& _p) {
    return QPointF(x(_p.x()), y(_p.y()));
}
double FieldCoordsTransform::w(double _w) {
    return _w * canvasWidth / ::p->param_canvas_width / zoomRatio;
}
double FieldCoordsTransform::h(double _h) {
    return -_h * canvasHeight / ::p->param_canvas_height / zoomRatio;
}
double FieldCoordsTransform::a(double _a) {
    return _a * 16;
}
double FieldCoordsTransform::r(double _r) {
    return _r * 16;
}
double FieldCoordsTransform::orx(double _x) {
    return (_x - canvasWidth / 2.0) * ::p->param_canvas_width / canvasWidth;
}
double FieldCoordsTransform::ory(double _y) {
    return -(_y - canvasHeight / 2.0) * ::p->param_canvas_height / canvasHeight;
}
double FieldCoordsTransform::orw(double _w) {
    return (_w) * ::p->param_canvas_width / canvasWidth;
}
double FieldCoordsTransform::orh(double _h) {
    return -(_h) * ::p->param_canvas_height / canvasHeight;
}
double FieldCoordsTransform::rx(double x) {
    return orx(zoomStart.x() + x * zoomRatio);
}
double FieldCoordsTransform::ry(double y) {
    return ory(zoomStart.y() + y * zoomRatio);
}
QPointF FieldCoordsTransform::rp(const QPointF& p) {
    return QPointF(rx(p.x()), ry(p.y()));
}
Field::Field(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , ZSPlugin("FieldBase")
    ,_pixmap(nullptr){
    declare_publish("draw_signal");
    addLayers();
    for(auto _l : _layers){
        this->link(_l,"draw_signal");
    }

    int width,height;
    zpm->loadParam(width,"auto/fieldWidth",500);
    zpm->loadParam(height,"auto/fieldHeight",350);
    resetSize(width,height,false);
    for(auto _l : _layers){
        _l->start_detach();
    }
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
}
Field::~Field(){
    for(auto _l : _layers)
        _l->set_exit();
    for(auto _l : _layers)
        delete _l;
}
void Field::paint(QPainter* painter){
    if(_pixmap_mutex.try_lock()){
        painter->drawPixmap(_area, *_pixmap);
        _pixmap_mutex.unlock();
    }
}
void Field::init(){
    _painter.setPen(QPen(QBrush(QColor(255,255,255)),20));
    _painter.setRenderHint(QPainter::Antialiasing, true);
    _painter.setRenderHint(QPainter::TextAntialiasing, true);
    for(auto _l : _layers){
        _l->lock();
        _l->init();
        _l->unlock();
    }
}
void Field::repaint(){
    if(_pixmap_mutex.try_lock())
    {
        _pixmap->fill(s->COLOR_FIELD);//COLORTODO
        for(auto _l : _layers){
            _l->lock();
            _painter.drawImage(0,0,*(_l->get()));
            _l->unlock();
        }
        _pixmap_mutex.unlock();
        this->update(_area);
    }
}
// Q_INVOKABLE Function
void Field::resetSize(int width,int height,bool update){
    if (width <= 0 || height <= 0) return;
    if (width == _area.width() && height == _area.height()) return;
    t->resize(width,height);
    for(auto _l : _layers){
        _l->resize(width,height);
    }
    publish("draw_signal");
    if(update){
        zpm->changeParam("auto/fieldWidth",width);
        zpm->changeParam("auto/fieldHeight",height);
    }
    _pixmap_mutex.lock();
    {
        qDebug() << "\"fieldBase\" resize function : " << width << height;
        if(_painter.isActive())
            _painter.end();
        delete _pixmap;
        _pixmap = new QPixmap(QSize(width,height));
        _pixmap->fill(s->COLOR_FIELD);
        _painter.begin(_pixmap);
        _area = QRect(0, 0, width, height);
        init();
    }
    _pixmap_mutex.unlock();
}

#if QT_CONFIG(wheelevent)
void Field::wheelEvent (QWheelEvent *e) {
    qreal oldRatio = t->zoomRatio;
    t->zoomRatio = (e->delta() < 0 ? oldRatio/t->zoomCoefficient : oldRatio*t->zoomCoefficient);
    t->zoomRatio = limitRange(t->zoomRatio, t->zoomMin, 1.0);
    t->zoomStart -= e->pos() * (t->zoomRatio - oldRatio);
    limitZoomStart();
    init();
    repaint();
}
#endif

void Field::mousePressEvent(QMouseEvent *e) {
    pressed = e->buttons();
    checkClosestRobot(t->rx(e->x()), t->ry(e->y()));
    start = end = t->rp(e->pos());
    mouse_modifiers = e->modifiers();
    switch(pressed) {
    case Qt::LeftButton:
//        leftPressEvent(e);
        break;
    case Qt::RightButton:
//        rightPressEvent(e);
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
    end = t->rp(e->pos());
    switch(pressed) {
    case Qt::LeftButton:
//        leftMoveEvent(e);
        break;
    case Qt::RightButton:
//        rightMoveEvent(e);
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
//        leftReleaseEvent(e);
        break;
    case Qt::RightButton:
//        rightReleaseEvent(e);
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
void Field::middleMoveEvent(QMouseEvent *e) {
    switch(mouse_modifiers) {
    case Qt::NoModifier:
        middleNoModifierMoveEvent(e);
        break;
    case Qt::AltModifier:
//        middleAltModifierMoveEvent(e);
        break;
    case Qt::ControlModifier:
//        middleCtrlModifierMoveEvent(e);
        break;
    default:
        break;
    }
}
void Field::middlePressEvent(QMouseEvent *e) {
    move_start.setX(e->x());
    move_start.setY(e->y());
    switch(mouse_modifiers) {
    case Qt::NoModifier:
        middleNoModifierPressEvent(e);
        break;
    case Qt::AltModifier:
//        middleAltModifierPressEvent(e);
        break;
    case Qt::ControlModifier:
//        middleCtrlModifierPressEvent(e);
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
//        middleAltModifierReleaseEvent(e);
        break;
    case Qt::ControlModifier:
//        middleCtrlModifierReleaseEvent(e);
        break;
    default:
        break;
    }
}
void Field::middleNoModifierMoveEvent(QMouseEvent *e) {
    auto new_start = move_zoomStart + ::t->zoomRatio * (move_start - QPoint(e->x(), e->y()));
    t->zoomStart = new_start;
    limitZoomStart();
    init();
}
void Field::middleNoModifierPressEvent(QMouseEvent *e) {
    move_zoomStart = ::t->zoomStart;
}
void Field::middleNoModifierReleaseEvent(QMouseEvent *e) {}
void Field::checkClosestRobot(double x, double y) {
//    double limit = ::p->size_carDiameter * ::p->size_carDiameter / 4;
//    auto& vision = GlobalData::instance()->processRobot[0];
//    for(int color = PARAM::BLUE; color <= PARAM::YELLOW; color++) {
//        for(int j = 0; j < PARAM::ROBOTNUM; j++) {
//            auto& robot = vision.robot[color][j];
//            if(distance2(robot.pos.x() - x, robot.pos.y() - y) < limit) {
//                if (!selectRobots) {
//                    robotID[0] = j;
//                    robotTeam = color;
//                }
//                pressedRobot = true;
//                return;
//            }
//        }
//    }
    pressedRobot = false;
}
void Field::limitZoomStart(){
    auto cw = t->canvasWidth;
    auto ch = t->canvasHeight;
    auto aw = _area.width();
    auto ah = _area.height();
    auto dw = (cw-aw)/2;
    auto dh = (ch-ah)/2;
    t->zoomStart.setX(limitRange(t->zoomStart.x(), dw*t->zoomRatio, dw*t->zoomRatio+(cw) * (1 - t->zoomRatio)));
    t->zoomStart.setY(limitRange(t->zoomStart.y(), dh*t->zoomRatio, dh*t->zoomRatio+(ch) * (1 - t->zoomRatio)));

}
