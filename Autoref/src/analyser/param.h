#ifndef PARAM_H
#define PARAM_H

namespace Param{
    namespace Field{
        const int POS_SIDE_LEFT = 10;
        const int POS_SIDE_RIGHT = -10;
        const int MAX_PLAYER = 160;
        const int MAX_PLAYER_NUM = 160;
        const int MAX_BALL_SPEED = 6300;
        /* Ball */
        const double BALL_SIZE	= 21.5; // 半径
        /* Player */
        const double MAX_PLAYER_SIZE = 18;
        /* Field */
        extern double PITCH_LENGTH; // 场地长
        extern double PITCH_WIDTH; // 场地宽
        extern double PITCH_MARGIN; // 场地的边界宽度
        extern double CENTER_CIRCLE_R; // 中圈半径
        extern bool   IF_USE_ELLIPSE;  // whether use ellipse penalty
        extern double PENALTY_AREA_WIDTH; // rectangle 禁区宽度
        extern double PENALTY_AREA_DEPTH; // rectangle 禁区深度
        extern double PENALTY_AREA_R; // ellipse 两个圆弧
        extern double PENALTY_AREA_L; // ellipse 连接两个圆弧的线段
        extern double PENALTY_L;	 //代替PENALTY_AREA_L
        extern double PENALTY_MARK_X; // 点球点的X坐标
        extern double OUTER_PENALTY_AREA_WIDTH; // 外围禁区宽度(界外开球时不能站在该线内)
        //extern double FIELD_WALL_DIST; // 场地护栏到边界的距离
        extern double GOAL_WIDTH; // 球门宽度
        extern double GOAL_DEPTH; // 球门深度
        extern double GOAL_POST_AVOID_LENGTH;
        extern double GOAL_POST_THICKNESS;
        extern double FREE_KICK_AVOID_BALL_DIST; // 开任意球的时候,对方必须离球这么远
        extern double RATIO;				//what's this?
    }
    namespace Physics {
        const double G = 9.8;
    }
    namespace Math{
        const double PI = 3.14159265358979323846;
    }
    namespace Vehicle{
        namespace V2{
            extern const double PLAYER_SIZE;
            extern const double PLAYER_FRONT_TO_CENTER ;
            extern const double PLAYER_CENTER_TO_BALL_CENTER ;
            extern const double KICK_ANGLE; // 可以击球的最大相对身体角度
            extern const double DRIBBLE_SIZE; // 带球时离球的距离
            extern const double DRIBBLE_ANGLE; // 可以带球的最大相对身体角度
            extern const double HEAD_ANGLE; // 前面的开口角度
            extern const double TOUCH_SHIFT_DIST; //做Touch时后退的距离
        }
    }
    namespace Rule{
        extern const int Version; // 规则的版本
        extern const double MaxDribbleDist; // 最大带球距离
    }
}
#endif // PARAM_H
