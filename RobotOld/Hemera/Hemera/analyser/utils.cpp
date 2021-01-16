#include "utils.h"

CUtils::CUtils()
{

}

bool CUtils::inArea(CGeoPoint _p, const CGeoPoint &center, double radius){
    double dis = center.dist(_p);
    if(dis > radius){return true ;}
    else{return false ;}
}

bool CUtils::inArea(CGeoPoint _p, double x1, double x2, double y1, double y2){
    if(_p.x() > x1 && _p.x() < x2
            && _p.y() > y1 && _p.y() < y2){
        return true;
    }else{
        return false;
    }
}
