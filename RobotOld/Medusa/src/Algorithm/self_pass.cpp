#include <math.h>
#include <string.h>
#include "self_pass.h"
#include "SkillUtils.h"
#include "GDebugEngine.h"
#define PI 3.1415926

CSelfPass::CSelfPass() : _selfpassStatus("SelfPass: the nearest enemy is "),
    _selfpassCannot("I can not self pass."),
    _enemy_num(-1),
    _distanceToEnemy(-99999),
    _angleToEnemy(-99999){
    temp_enemyVel = CGeoPoint(-99999, -99999);
    _farEnemyPoint = CGeoPoint(-99999, -99999);
    _orientatePoint = CGeoPoint(-99999, -99999);
    _moveDirectPoint = CGeoPoint(-99999, -99999);
    temp_farEnemyPoint = CGeoPoint(-99999, -99999);
    temp_orientatePoint = CGeoPoint(-99999, -99999);
    temp_moveDirectPoint = CGeoPoint(-99999, -99999);
}

//Athena显示debug信息
void CSelfPass::draw_selfpassMsg(const CVisionModule *pVision, const int leader){
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-580, 240), (_selfpassStatus + QString("%1").arg(_enemy_num,2,10,QLatin1Char('0'))).toLatin1(), COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-580, 260), (QString("Distance to nearest enemy is : %1").arg(QString::number(_distanceToEnemy))).toLatin1(), COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-580, 280), (QString("Angel to nearest enemy vel is : %1").arg(QString::number(_angleToEnemy))).toLatin1(), COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-580, 300), _selfpassCannot.toLatin1(), COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_arc(temp_enemyVel, 15, 0, 360, COLOR_ORANGE);
    GDebugEngine::Instance()->gui_debug_arc(temp_enemyVel, 20, 0, 360, COLOR_ORANGE);
    GDebugEngine::Instance()->gui_debug_line(me.Pos(), temp_enemyVel, COLOR_ORANGE);
    GDebugEngine::Instance()->gui_debug_arc(farEnemyPoint(), 10, 0, 360, COLOR_YELLOW);
    GDebugEngine::Instance()->gui_debug_arc(temp_farEnemyPoint, 15, 0, 360, COLOR_YELLOW);
    GDebugEngine::Instance()->gui_debug_arc(temp_farEnemyPoint, 20, 0, 360, COLOR_YELLOW);
    GDebugEngine::Instance()->gui_debug_line(me.Pos(), farEnemyPoint(), COLOR_YELLOW);
    GDebugEngine::Instance()->gui_debug_line(me.Pos(), temp_farEnemyPoint, COLOR_YELLOW);
    GDebugEngine::Instance()->gui_debug_arc(orientatePoint(), 10, 0, 360, COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_arc(temp_orientatePoint, 15, 0, 360, COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_arc(temp_orientatePoint, 20, 0, 360, COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_line(me.Pos(), orientatePoint(), COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_line(me.Pos(), temp_orientatePoint, COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_arc(moveDirectPoint(), 10, 0, 360, COLOR_CYAN);
    GDebugEngine::Instance()->gui_debug_arc(temp_moveDirectPoint, 15, 0, 360, COLOR_CYAN);
    GDebugEngine::Instance()->gui_debug_arc(temp_moveDirectPoint, 20, 0, 360, COLOR_CYAN);
    GDebugEngine::Instance()->gui_debug_line(me.Pos(), moveDirectPoint(), COLOR_CYAN);
    GDebugEngine::Instance()->gui_debug_line(me.Pos(), temp_moveDirectPoint, COLOR_CYAN);
}

//判断是否自传
bool CSelfPass::is_selfpass(){
    return true;
}

//图像信息中的车号要加1
void CSelfPass::cal_variousPoint(const CVisionModule *pVision, const int leader){
    cal_farEnemyPoint(pVision, leader);
    cal_orientatePoint(pVision, leader);
    cal_moveDirectPoint(pVision, leader);
    cal_anglenough_EnemyVel(pVision, leader);
    is_anglenough_EnemyVel(pVision, leader, 2.61);
}

//计算距离敌方最远的点_farEnemyPoint
void CSelfPass::cal_farEnemyPoint(const CVisionModule *pVision, const int leader){
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    int enemy;
    double temp_dist_enemy = 99999;
    for(int i = 1; i < Param::Field::MAX_PLAYER; i++){
        if(temp_dist_enemy > pVision->TheirPlayer(i).Pos().dist(me.Pos())){
            enemy = i;
            temp_dist_enemy = pVision->TheirPlayer(enemy).Pos().dist(me.Pos());
        }
    }
    _enemy_num = enemy;
    _distanceToEnemy = temp_dist_enemy;
    double passpoint_x = (me.Pos().x() - pVision->TheirPlayer(enemy).Pos().x()) / me.Pos().dist(pVision->TheirPlayer(enemy).Pos());
    double passpoint_y = (me.Pos().y() - pVision->TheirPlayer(enemy).Pos().y()) / me.Pos().dist(pVision->TheirPlayer(enemy).Pos());
    _farEnemyPoint = CGeoPoint(me.Pos().x() + 20 * passpoint_x, me.Pos().y() + 20 * passpoint_y);
    temp_farEnemyPoint = CGeoPoint(me.Pos().x() + 60 * passpoint_x, me.Pos().y() + 60 * passpoint_y);
}

//计算当前朝向的点_orientatePoint
void CSelfPass::cal_orientatePoint(const CVisionModule *pVision, const int leader){
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    double diretion = me.Dir();
    _orientatePoint = CGeoPoint(me.Pos().x() + 20 * cos(diretion), me.Pos().y() + 20 * sin(diretion));
    temp_orientatePoint = CGeoPoint(me.Pos().x() + 60 * cos(diretion), me.Pos().y() + 60 * sin(diretion));
}

//计算当前运动方向的点_moveDirectPoint
void CSelfPass::cal_moveDirectPoint(const CVisionModule *pVision, const int leader){
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    double diretion;
    if(me.VelX() > 0) {diretion = std::atan(me.VelY() / me.VelX());}
    else {diretion =std::atan(me.VelY() / me.VelX()) - PI;}
    _moveDirectPoint = CGeoPoint(me.Pos().x() + 20 * cos(diretion), me.Pos().y() + 20 * sin(diretion));
    temp_moveDirectPoint = CGeoPoint(me.Pos().x() + 60 * cos(diretion), me.Pos().y() + 60 * sin(diretion));
}

//计算最近敌方车辆与我方leader的相对速度方向
void CSelfPass::cal_anglenough_EnemyVel(const CVisionModule *pVision, const int leader){
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    double delta_vx = pVision->TheirPlayer(_enemy_num).VelX() - me.VelX();
    double delta_vy = pVision->TheirPlayer(_enemy_num).VelY() - me.VelY();
    double delta_v = sqrt(delta_vx * delta_vx + delta_vy * delta_vy);
    temp_enemyVel = CGeoPoint(me.Pos().x() + delta_vx / delta_v * 60, me.Pos().y() + delta_vy / delta_v * 60);
    double x1 = temp_enemyVel.x() - me.Pos().x();
    double y1 = temp_enemyVel.y() - me.Pos().y();
    double x2 = _moveDirectPoint.x() - me.Pos().x();
    double y2 = _moveDirectPoint.y() - me.Pos().y();
    _angleToEnemy = (1.0/400) * (x1 * x2 + y1 * y2) / sqrt(x1 * x1 + y1 * y1) * sqrt(x2 * x2 + y2 * y2);
}

//判断传球点_orientatePoint指向敌方边界上下height
bool CSelfPass::is_angelpenaltyPoint(const CVisionModule *pVision, const int leader, double height){
    if(_orientatePoint.x() < -9999 || _orientatePoint.y() < -9999){return false;}
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    if(_orientatePoint.x() < me.Pos().x()){return false;}
    double tan_me2point = (me.Pos().y() - _orientatePoint.y()) / (me.Pos().x() - _orientatePoint.x());
    double tan_me2penalty1 = (me.Pos().y() - height) / (me.Pos().x() - 600);
    double tan_me2penalty2 = (me.Pos().y() + height) / (me.Pos().x() - 600);
    if(tan_me2point < tan_me2penalty1 && tan_me2point > tan_me2penalty2){return true;}
    else{return false;}
}

//判断_orientatePoint与_farEnemyPoint夹角小于xita
bool CSelfPass::is_anglenough_EnemyFar(const CVisionModule *pVision, const int leader, double xita){
    if(_farEnemyPoint.x() < -9999 || _farEnemyPoint.y() < -9999){return false;}
    if(_orientatePoint.x() < -9999 || _orientatePoint.y() < -9999){return false;}
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    double dis_h = 0.5 * (_farEnemyPoint.dist(_orientatePoint));
    double dis_r = _farEnemyPoint.dist(me.Pos());
    if(sin(xita/2) > dis_h / dis_r){return true;}
    else{return false;}
}

//判断_orientatePoint与_moveEnemyPoint夹角小于xita
bool CSelfPass::is_anglenough_MeMove(const CVisionModule *pVision, const int leader, double xita){
    if(_moveDirectPoint.x() < -9999 || _moveDirectPoint.y() < -9999){return false;}
    if(_orientatePoint.x() < -9999 || _orientatePoint.y() < -9999){return false;}
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    double dis_h = 0.5 * (_moveDirectPoint.dist(_orientatePoint));
    double dis_r = _moveDirectPoint.dist(me.Pos());
    if(sin(xita/2) > dis_h / dis_r){return true;}
    else{return false;}
}

//判断相对速度方向与_moveEnemyPoint夹角大于xita
bool CSelfPass::is_anglenough_EnemyVel(const CVisionModule *pVision, const int leader, double xita){
    if(_moveDirectPoint.x() < -9999 || _moveDirectPoint.y() < -9999){return false;}
    if(_distanceToEnemy < -9999){return false;}
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    double x1 = temp_enemyVel.x() - me.Pos().x();
    double y1 = temp_enemyVel.y() - me.Pos().y();
    double x2 = _moveDirectPoint.x() - me.Pos().x();
    double y2 = _moveDirectPoint.y() - me.Pos().y();
    if(sqrt(x1 * x1 + y1 * y1) < 5){return false;}
    double cos_angle = (1.0/400) * (x1 * x2 + y1 * y2) / sqrt(x1 * x1 + y1 * y1) * sqrt(x2 * x2 + y2 * y2);
    if(cos(xita) > cos_angle){return true;}
    else{return false;}
}

//判断是否靠近边界
bool CSelfPass::is_overBoundary(const CVisionModule *pVision, const int leader){
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    if(me.Pos().x() < -400 && _orientatePoint.x() - me.Pos().x() < 0){return false;}
    if(me.Pos().x() > 400 && _orientatePoint.x() - me.Pos().x() > 0){return false;}
    if(me.Pos().y() < -350 && _orientatePoint.y() - me.Pos().y() < 0){return false;}
    if(me.Pos().y() > 350 && _orientatePoint.y() - me.Pos().y() > 0){return false;}
    return true;
}

//判断能否自传
bool CSelfPass::is_selfpassPoint(const CVisionModule *pVision, const int leader){
    if(is_anglenough_EnemyFar(pVision, leader, 0.523)
            && is_anglenough_MeMove(pVision, leader, 0.261)
            /*&& (is_angelpenaltyPoint(pVision, leader, 450))*/
            && is_anglenough_EnemyVel(pVision, leader, 2.61)
            && is_overBoundary(pVision, leader)
            && _distanceToEnemy < 150){
        _selfpassCannot = "self pass!!!!!!!!!";
        return true;
    }
    else{
        _selfpassCannot = "not---------------";
        return false;
    }
//    return (is_anglenoughPoint(pVision, leader, 0.523) && (is_angelpenaltyPoint(pVision, leader, 400)));
}
