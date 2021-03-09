#ifndef __FIELD_STYLE_H__
#define __FIELD_STYLE_H__
#include <QColor>
#include "singleton.hpp"
#include "staticparams.h"
class FieldStyle{
public:
    FieldStyle();
    QColor COLOR_FIELD;
    QColor COLOR_ROBOT_ORI;
    QColor COLOR_BALL;
    QColor COLOR_BALL_SHADOW;
    QColor COLOR_ROBOT[PARAM::TEAMS];
    QColor COLOR_ROBOT_SHADOW[PARAM::TEAMS];
};
typedef Singleton<FieldStyle> FS;
#endif // __FIELD_STYLE_H__
