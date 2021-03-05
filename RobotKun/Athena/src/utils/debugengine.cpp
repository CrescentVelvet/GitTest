#include "debugengine.h"
#include "staticparams.h"
#include <cstring>
#include "zss_debug.pb.h"
#include "globaldata.h"
namespace{
    ZSS::Protocol::Debug_Msgs guiDebugMsgs;
}
CGDebugEngine::CGDebugEngine(){
}
CGDebugEngine::~CGDebugEngine(){
}

void CGDebugEngine::gui_debug_x(const CGeoPoint& p, int debug_color,int RGB_value){
	CGeoPoint basePos = p;
    static const int x_line_length = 30;
    const CGeoPoint p1 = basePos + CVector( x_line_length, x_line_length);
    const CGeoPoint p2 = basePos + CVector(-x_line_length,-x_line_length);
    gui_debug_line(p1, p2, debug_color,RGB_value);

    const CGeoPoint p3 = basePos + CVector( x_line_length,-x_line_length);
    const CGeoPoint p4 = basePos + CVector(-x_line_length, x_line_length);
    gui_debug_line(p3,p4, debug_color,RGB_value);
}
void CGDebugEngine::gui_debug_points(const std::vector<CGeoPoint> points, int debug_color,int RGB_value){
    debugMutex.lock();
    ZSS::Protocol::Debug_Msg* msg = guiDebugMsgs.add_msgs();
    msg->set_type(ZSS::Protocol::Debug_Msg::Debug_Type::Debug_Msg_Debug_Type_Points);
    msg->set_color(ZSS::Protocol::Debug_Msg_Color(debug_color));
    if(debug_color==COLOR_USE_RGB)  msg->set_rgb_value(RGB_value);
    ZSS::Protocol::Debug_Points* debugPoints = msg->mutable_points();
    for (int i=0; i < points.size(); i++) {
        ZSS::Protocol::Point* point = debugPoints->add_point();
        point->set_x(points[i].x());
        point->set_y(points[i].y());
    }
//    qDebug() << "point size: " << points.size();
    debugMutex.unlock();
}
void CGDebugEngine::gui_debug_line(const CGeoPoint& p1, const CGeoPoint& p2, int debug_color,int RGB_value){
    debugMutex.lock();
    ZSS::Protocol::Debug_Msg* msg = guiDebugMsgs.add_msgs();
    msg->set_type(ZSS::Protocol::Debug_Msg::Debug_Type::Debug_Msg_Debug_Type_LINE);
    msg->set_color(ZSS::Protocol::Debug_Msg_Color(debug_color));
    if(debug_color==COLOR_USE_RGB)  msg->set_rgb_value(RGB_value);
    ZSS::Protocol::Debug_Line* line = msg->mutable_line();
    ZSS::Protocol::Point
    *pos1 = line->mutable_start(),
    *pos2 = line->mutable_end();
    pos1->set_x(p1.x());
    pos1->set_y(p1.y());
    pos2->set_x(p2.x());
    pos2->set_y(p2.y());

	line->set_back(false);
    line->set_forward(false);
    debugMutex.unlock();
}
void CGDebugEngine::gui_debug_arc(const CGeoPoint& p, double r, double start_angle, double span_angle, int debug_color,int RGB_value){
    debugMutex.lock();
    ZSS::Protocol::Debug_Msg* msg = guiDebugMsgs.add_msgs();
    msg->set_type(ZSS::Protocol::Debug_Msg::Debug_Type::Debug_Msg_Debug_Type_ARC);
    msg->set_color(ZSS::Protocol::Debug_Msg_Color(debug_color));
    if(debug_color == COLOR_USE_RGB) {
           msg->set_rgb_value(RGB_value);
       }
    ZSS::Protocol::Debug_Arc* arc = msg->mutable_arc();
	arc->set_start(start_angle);
	arc->set_end(span_angle);
	arc->set_fill(false);
    ZSS::Protocol::Rectangle* rec = arc->mutable_rectangle();
    ZSS::Protocol::Point
		*p1 = rec->mutable_point1(),
		*p2 = rec->mutable_point2();
    CGeoPoint center;
    center.setX(p.x());
    center.setY(p.y());
    p1->set_x(center.x() - r);
    p1->set_y((center.y() - r));
    p2->set_x(center.x() + r);
    p2->set_y((center.y() + r));
    debugMutex.unlock();
}
void CGDebugEngine::gui_debug_triangle(const CGeoPoint& p1, const CGeoPoint& p2, const CGeoPoint& p3, int debug_color,int RGB_value){
    debugMutex.lock();
    ZSS::Protocol::Debug_Msg* msg = guiDebugMsgs.add_msgs();
    msg->set_type(ZSS::Protocol::Debug_Msg::Debug_Type::Debug_Msg_Debug_Type_POLYGON);
    msg->set_color(ZSS::Protocol::Debug_Msg_Color(debug_color));
    if(debug_color == COLOR_USE_RGB) {
           msg->set_rgb_value(RGB_value);
       }
    ZSS::Protocol::Debug_Polygon* tri = msg->mutable_polygon();
    ZSS::Protocol::Point* pos1 = tri->add_vertex();
    ZSS::Protocol::Point* pos2 = tri->add_vertex();
    ZSS::Protocol::Point* pos3 = tri->add_vertex();

    pos1->set_x(p1.x());
    pos1->set_y(p1.y());
    pos2->set_x(p2.x());
    pos2->set_y(p2.y());
    pos3->set_x(p3.x());
    pos3->set_y(p3.y());

    debugMutex.unlock();
}
void CGDebugEngine::gui_debug_robot(const CGeoPoint& p, double robot_dir, int debug_color,int RGB_value){
    debugMutex.lock();
    ZSS::Protocol::Debug_Msg* msg = guiDebugMsgs.add_msgs();
    msg->set_type(ZSS::Protocol::Debug_Msg::Debug_Type::Debug_Msg_Debug_Type_ROBOT);
    msg->set_color(ZSS::Protocol::Debug_Msg_Color(debug_color));
    if(debug_color == COLOR_USE_RGB) {
           msg->set_rgb_value(RGB_value);
       }
    ZSS::Protocol::Debug_Robot* robot = msg->mutable_robot();

	float rdir;

    rdir = robot_dir*180/PARAM::Math::PI; // 转成角度
	robot->set_dir(rdir);

    ZSS::Protocol::Point* robot_pos = robot->mutable_pos();

    robot_pos->set_x(p.x());
    robot_pos->set_y(p.y());

    debugMutex.unlock();
}
void CGDebugEngine::gui_debug_msg(const CGeoPoint& p, const char* msgstr, int debug_color,int RGB_value, const double size, const int weight)
{
    debugMutex.lock();
    ZSS::Protocol::Debug_Msg* msg = guiDebugMsgs.add_msgs();
    msg->set_type(ZSS::Protocol::Debug_Msg::Debug_Type::Debug_Msg_Debug_Type_TEXT);
    msg->set_color(ZSS::Protocol::Debug_Msg_Color(debug_color));
    if(debug_color == COLOR_USE_RGB) {
           msg->set_rgb_value(RGB_value);
       }
    ZSS::Protocol::Debug_Text* text = msg->mutable_text();
    text->set_size(size);
    text->set_weight(weight);
    ZSS::Protocol::Point* center = text->mutable_pos();
    center->set_x(p.x());
    center->set_y(p.y());

    text->set_text(msgstr);
    debugMutex.unlock();
}
void CGDebugEngine::gui_debug_curve(const double num, const double maxLimit, const double minLimit, int debug_color,int RGB_value)
{
    debugMutex.lock();
    ZSS::Protocol::Debug_Msg* msg = guiDebugMsgs.add_msgs();
    msg->set_type(ZSS::Protocol::Debug_Msg::Debug_Type::Debug_Msg_Debug_Type_CURVE);
    msg->set_color(ZSS::Protocol::Debug_Msg_Color(debug_color));
    if(debug_color == COLOR_USE_RGB) {
           msg->set_rgb_value(RGB_value);
       }
    ZSS::Protocol::Debug_Curve_* curve = msg->mutable_curve();
	curve->set_num(num);
	curve->set_minlimit(minLimit);
	curve->set_maxlimit(maxLimit);
    debugMutex.unlock();
}
void CGDebugEngine::send(){
    static QByteArray data;
    debugMutex.lock();
    int size = guiDebugMsgs.ByteSize();
    data.resize(size);
    guiDebugMsgs.SerializeToArray(data.data(),size);
//    std::cout << "size: " << data.size() << ' ' << sent_size << std::endl;
    GlobalData::instance()->athenaDebugMessage = data;
    guiDebugMsgs.clear_msgs();
    debugMutex.unlock();
}
