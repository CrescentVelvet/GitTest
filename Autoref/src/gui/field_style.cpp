#include "field_style.h"
FieldStyle::FieldStyle(){
    COLOR_FIELD = QColor(48,48,48);
    COLOR_ROBOT_ORI = QColor(220, 53, 47);
    COLOR_BALL = QColor(255, 0, 255);
    COLOR_BALL_SHADOW = QColor(255, 0, 255, 60);
    COLOR_ROBOT[PARAM::BLUE] = QColor(25, 30, 150);
    COLOR_ROBOT[PARAM::YELLOW] = QColor(241, 201, 50);
    COLOR_ROBOT_SHADOW[PARAM::BLUE] = QColor(100, 120, 200, 50);
    COLOR_ROBOT_SHADOW[PARAM::YELLOW] = QColor(255, 230, 150, 50);
}
