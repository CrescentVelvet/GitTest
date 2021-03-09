#include "field.h"
#include "field_layers.h"
#include "field_style.h"
#include <functional>
#include "parammanager.h"
#include <QtMath>
namespace {
    auto zpm = ZSS::LParamManager::instance();
    auto p = FP::instance();
    auto t = FT::instance();
    auto s = FS::instance();
    auto x = std::bind(std::mem_fn(&FieldCoordsTransform::x),t,std::placeholders::_1);
    auto y = std::bind(std::mem_fn(&FieldCoordsTransform::y),t,std::placeholders::_1);
    auto w = std::bind(std::mem_fn(&FieldCoordsTransform::w),t,std::placeholders::_1);
    auto h = std::bind(std::mem_fn(&FieldCoordsTransform::h),t,std::placeholders::_1);
    auto a = std::bind(std::mem_fn(&FieldCoordsTransform::a),t,std::placeholders::_1);
    auto r = std::bind(std::mem_fn(&FieldCoordsTransform::r),t,std::placeholders::_1);
    auto& ballDiameter = p->size_ballDiameter;
    auto& shadowDiameter = p->size_shadowDiameter;
    auto& carDiameter = p->size_carDiameter;
    auto& carFaceWidth = p->size_carFaceWidth;
    auto& numberSize = p->size_number;
    auto& debugPointSize = p->size_debugPoint;
}
void FieldLineLayer::initFieldLinePath(){
    fieldLinePath.clear();
    fieldLinePath.addRect(t->x(-p->param_width / 2.0), t->y(-p->param_height / 2.0), t->w(p->param_width), t->h(p->param_height));
    fieldLinePath.addRect(t->x(-p->param_width / 2.0), t->y(-p->param_goalWidth / 2.0), t->w(-p->param_goalDepth), t->h(p->param_goalWidth));
    fieldLinePath.addRect(t->x(p->param_width / 2.0), t->y(-p->param_goalWidth / 2.0), t->w(p->param_goalDepth), t->h(p->param_goalWidth));
    fieldLinePath.moveTo(t->x(0), t->y(p->param_height / 2.0));
    fieldLinePath.lineTo(t->x(0), t->y(-p->param_height / 2.0));
    fieldLinePath.addEllipse(t->x(-p->param_centerCircleRadius), t->y(-p->param_centerCircleRadius), t->w(2 * p->param_centerCircleRadius), t->h(2 * p->param_centerCircleRadius));
    fieldLinePath.addRect(t->x(-p->param_width / 2.0), t->y(-p->param_penaltyLength / 2.0), t->w(p->param_penaltyWidth), t->h(p->param_penaltyLength));
    fieldLinePath.addRect(t->x(p->param_width / 2.0), t->y(-p->param_penaltyLength / 2.0), t->w(-p->param_penaltyWidth), t->h(p->param_penaltyLength));
}
void VisionLayer::drawOriginVision(const Vision_DetectionFrame& frame){
    auto size = frame.origin_balls_size();
    for(int i=0;i<size;i++){
        auto& ball = frame.origin_balls(i);
        paintBall(s->COLOR_BALL_SHADOW,ball.x(),ball.y());
    }
    size = frame.origin_robots_blue_size();
    for(int i=0;i<size;i++){
        auto& robot = frame.origin_robots_blue(i);
        paintRobot(s->COLOR_ROBOT_SHADOW[PARAM::BLUE],robot.robot_id(),robot.x(),robot.y(),robot.orientation());
    }
    size = frame.origin_robots_yellow_size();
    for(int i=0;i<size;i++){
        auto& robot = frame.origin_robots_yellow(i);
        paintRobot(s->COLOR_ROBOT_SHADOW[PARAM::YELLOW],robot.robot_id(),robot.x(),robot.y(),robot.orientation());
    }
}
void VisionLayer::drawFusionVision(const Vision_DetectionFrame& frame){
    auto& ball = frame.balls();
    paintBall(s->COLOR_BALL,ball.x(),ball.y());
    auto size = frame.robots_blue_size();
    for(int i=0;i<size;i++){
        auto& robot = frame.robots_blue(i);
        paintRobot(s->COLOR_ROBOT[PARAM::BLUE],robot.robot_id(),robot.x(),robot.y(),robot.orientation());
    }
    size = frame.robots_yellow_size();
    for(int i=0;i<size;i++){
        auto& robot = frame.robots_yellow(i);
        paintRobot(s->COLOR_ROBOT[PARAM::YELLOW],robot.robot_id(),robot.x(),robot.y(),robot.orientation());
    }
}
void VisionLayer::paintBall(const QColor& color, qreal x, qreal y) {
    _painter.setBrush(QBrush(color));
    _painter.setPen(Qt::NoPen);
    _painter.drawEllipse(QRectF(::x(x - ballDiameter / 2.0), ::y(y - ballDiameter / 2.0), ::w(ballDiameter), ::h(ballDiameter)));
}
void VisionLayer::paintRobot(const QColor& color, quint8 num, qreal x, qreal y, qreal radian, bool drawNum, const QColor& textColor, bool needCircle) {
    static qreal radius = carDiameter / 2.0;
    static qreal chordAngel = qRadiansToDegrees(qAcos(1.0 * carFaceWidth / carDiameter));
    _painter.setBrush(QBrush(color));
    _painter.setPen(QPen(s->COLOR_ROBOT_ORI, ::w(30), Qt::DotLine));
    _painter.drawLine(QPointF(::x(x), ::y(y)), QPointF(::x(x), ::y(y)) + QPointF(::w(200) * qSin(radian + M_PI_2), ::w(200) * qCos(radian + M_PI_2)));
    _painter.setPen(Qt::NoPen);
    _painter.drawChord(QRectF(::x(x - radius), ::y(y - radius), ::w(2 * radius), ::h(2 * radius)), ::a(90.0 - chordAngel + 180 / M_PI * radian), ::r(180.0 + 2 * chordAngel));
    if (needCircle) {
        _painter.setBrush(Qt::NoBrush);
        _painter.setPen(QPen(s->COLOR_ROBOT_ORI, ::w(30)));
        _painter.drawChord(QRectF(::x(x - radius), ::y(y - radius), ::w(2 * radius), ::h(2 * radius)), ::a(90.0 - chordAngel + 180 / M_PI * radian), ::r(180.0 + 2 * chordAngel));
    }
    if (drawNum) {
        _painter.setBrush(Qt::NoBrush);
        _painter.setPen(QPen(textColor, ::w(30)));
        QFont font;
        int fontSize = ::h(-numberSize);
        font.setPixelSize(fontSize);
        _painter.setFont(font);
        _painter.drawText(::x(x - numberSize), ::y(y + carDiameter * 0.4), QString::number(num, 16).toUpper());
    }
}
