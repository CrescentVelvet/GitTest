#include <string.h>
#include "self_pass.h"
#include "SkillUtils.h"
#include "GDebugEngine.h"

CSelfPass::CSelfPass() : _selfpassStatus("SelfPass: the nearest enemy is "), _enemy_num(-1){
    _bestPassPoint = CGeoPoint(-99999, -99999);
    _nowRunPoint = CGeoPoint(-99999, -99999);
}

void CSelfPass::draw_selfpassMsg(const CVisionModule *pVision, const int leader){
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(-580, 240), (_selfpassStatus + QString("%1").arg(_enemy_num,2,10,QLatin1Char('0'))).toLatin1(), COLOR_ORANGE);
    GDebugEngine::Instance()->gui_debug_arc(bestPassPoint(), 10, 0, 360, COLOR_YELLOW);
    GDebugEngine::Instance()->gui_debug_arc(bestPassPoint(), 15, 0, 360, COLOR_YELLOW);
    GDebugEngine::Instance()->gui_debug_line(me.Pos(), bestPassPoint(), COLOR_YELLOW);
    GDebugEngine::Instance()->gui_debug_arc(nowRunPoint(), 10, 0, 360, COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_arc(nowRunPoint(), 15, 0, 360, COLOR_GREEN);
    GDebugEngine::Instance()->gui_debug_line(me.Pos(), nowRunPoint(), COLOR_GREEN);
}

//图像信息中的车号要加1
//计算自传的传球点
void CSelfPass::cal_selfpassPoint(const CVisionModule *pVision, const int leader){
    const MobileVisionT& ball = pVision->Ball();
    int enemy;
    double temp_dist_enemy = 99999;
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    for(int i = 1; i < Param::Field::MAX_PLAYER; i++){
        if(temp_dist_enemy > pVision->TheirPlayer(i).Pos().dist(me.Pos())){
            enemy = i;
            temp_dist_enemy = pVision->TheirPlayer(enemy).Pos().dist(me.Pos());
        }
    }
    _enemy_num = enemy;
    double passpoint_x = 50 * (me.Pos().x() - pVision->TheirPlayer(enemy).Pos().x()) / me.Pos().dist(pVision->TheirPlayer(enemy).Pos());
    double passpoint_y = 50 * (me.Pos().y() - pVision->TheirPlayer(enemy).Pos().y()) / me.Pos().dist(pVision->TheirPlayer(enemy).Pos());
    CGeoPoint passpoint_temp(me.Pos().x() + passpoint_x, me.Pos().y() + passpoint_y);
    _bestPassPoint = passpoint_temp;
}

//计算当前运动方向的点
void CSelfPass::cal_selfrunPoint(const CVisionModule *pVision, const int leader){
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    double diretion = me.Dir();
    CGeoPoint runpoint_temp(me.Pos().x() + 50 * cos(diretion), me.Pos().y() + 50 * sin(diretion));
    _nowRunPoint = runpoint_temp;
}

//判断传球点指向敌方禁区
bool CSelfPass::is_selfpassPoint(const CVisionModule *pVision, const int leader){
    int x_point = _bestPassPoint.x();
    int y_point = _bestPassPoint.y();
    if(x_point < -9999 && y_point < -9999){return false;}
    const PlayerVisionT& me = pVision->OurPlayer(leader+1);
    if(x_point < me.Pos().x()){return false;}
    double tan_me2point = (me.Pos().y() - y_point) / (me.Pos().x() - x_point);
    double tan_me2penalty1 = (me.Pos().y() - 240) / (me.Pos().x() - 600);
    double tan_me2penalty2 = (me.Pos().y() + 240) / (me.Pos().x() - 600);
    if(tan_me2point < tan_me2penalty1 && tan_me2point > tan_me2penalty2){return true;}
    else{return false;}
}
