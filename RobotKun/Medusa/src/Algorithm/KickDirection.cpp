#include "KickDirection.h"
#include <ShootRangeList.h>
#include "BallStatus.h"
#include "GDebugEngine.h"
#include "Compensate.h"
#include <fstream>
#include <sstream>
#include "Global.h"
#include "parammanager.h"
namespace {
    CGeoPoint Shoot2Goal;
    const double MinShootWidth = 1*PARAM::Field::BALL_SIZE; //someone modified it to 0 ! comment by wyk 2019.5.14
    const bool VERBOSE_MODE = false;
    double compensatevalue[100][50];
//    const CGeoPoint MAX_GOAL_POST(PARAM::Field::PITCH_LENGTH/2,PARAM::Field::GOAL_WIDTH/2);
//    const CGeoPoint MIN_GOAL_POST(PARAM::Field::PITCH_LENGTH/2,-PARAM::Field::GOAL_WIDTH/2);
}


CKickDirection::CKickDirection( )
{
    reset();
    Shoot2Goal = CGeoPoint(PARAM::Field::PITCH_LENGTH/2.0,0.0);
    const string path = PARAM::File::PlayBookPath;
    string fullname = path + COMPENSATE_FILE_NAME;
    ifstream infile(fullname.c_str());
    if (!infile) {
        cerr << "error opening file data"<< endl;
        exit(-1);
    }
    string line;
    int rowcount;
    int columncount;
    getline(infile,line);
    istringstream lineStream(line);
    lineStream>>rowcount>>columncount;
    for (int i =0;i<rowcount;i++){
        getline(infile,line);
        istringstream lineStream(line);
        for(int j=0;j<columncount;j++){
            lineStream>>compensatevalue[i][j];
        }
    }
};

void CKickDirection::reset()
{
    _kick_valid = false;
    _is_compensated = false;

    _raw_kick_dir = 0.0;
    _compensate_value = 0.0;
    _real_kick_dir = 0.0;

    _kick_target = Shoot2Goal;
    _compensate_factor = 0.0;
}

double CKickDirection::GenerateShootDir(const int player, const CGeoPoint pos)
{
    ///> 进行重置
    reset();
    CVisionModule* pVision = vision;
    const MobileVisionT & ball = pVision->ball();
    const PlayerVisionT & kicker = pVision->ourPlayer(player);
    const CGeoPoint & kickerPos = kicker.Valid() ? kicker.RawPos() : kicker.Pos();
//    static bool kickflag=false;
    double maxRange = -9999;
    ///> 计算踢球的方向,并生成踢球目标
    CShootRangeList shootRangeList(pVision, player, pos);
    const CValueRange* bestRange = nullptr;
    const CValueRangeList& shootRange = shootRangeList.getShootRange();
    if (shootRange.size() > 0) {
        bestRange = shootRange.getMaxRangeWidth();
        if (bestRange && bestRange->getWidth() > MinShootWidth) {	// 要求射门空档足够大
            maxRange = bestRange->getWidth();
            /*if(bestRange->getSize() > canShootMinAngle)*/ _kick_valid = true;
            _raw_kick_dir = bestRange->getMiddle();
//            const double maxdir = (MAX_GOAL_POST - kickerPos).dir();
//            const double mindir = (MIN_GOAL_POST - kickerPos).dir();
//            if(fabs(maxdir - bestRange->getMax())<0.01){
//            }
//            GDebugEngine::Instance()->gui_debug_line(kickerPos,kickerPos+Utils::Polar2Vector(500,bestRange->getMax()),COLOR_RED);
//            GDebugEngine::Instance()->gui_debug_line(kickerPos,kickerPos+Utils::Polar2Vector(500,bestRange->getMin()),COLOR_YELLOW);
//            GDebugEngine::Instance()->gui_debug_line(kickerPos,kickerPos+Utils::Polar2Vector(500,_raw_kick_dir),COLOR_RED);
            CGeoLine shootLine = CGeoLine(kickerPos,kickerPos+Utils::Polar2Vector(500,_raw_kick_dir));
            CGeoLine bottomLine = CGeoLine(CGeoPoint(PARAM::Field::PITCH_LENGTH/2.0,-PARAM::Field::GOAL_WIDTH/2),CGeoPoint(PARAM::Field::PITCH_LENGTH/2.0,PARAM::Field::GOAL_WIDTH/2));
            CGeoLineLineIntersection intersect = CGeoLineLineIntersection(shootLine,bottomLine);
            if (intersect.Intersectant()) {
                _kick_target = intersect.IntersectPoint();
            }
        }
    } else{
        _kick_target = Shoot2Goal;
    }


    ///> 踢球方向补偿
    // CVector self2ball = ball.Pos() - kickerPos;
    // double ballVelDir = ball.Vel().dir();
    // double ballVelReverse = Utils::Normalize(ballVelDir+PARAM::Math::PI);	// 球速反向

    if (VERBOSE_MODE) {
        GDebugEngine::Instance()->gui_debug_line(kickerPos,kickerPos+Utils::Polar2Vector(1000,_real_kick_dir),COLOR_CYAN);
        GDebugEngine::Instance()->gui_debug_line(kickerPos,kickerPos+Utils::Polar2Vector(1000,_raw_kick_dir),COLOR_WHITE);
        GDebugEngine::Instance()->gui_debug_line(kickerPos,kickerPos+Utils::Polar2Vector(1000,kicker.Dir()),COLOR_BLACK);
    }

    // bug fix [5/16/2011 cliffyin]
    if (! _kick_valid) {
        _raw_kick_dir  = (Shoot2Goal- kickerPos).dir();
        _real_kick_dir = (Shoot2Goal- kickerPos).dir();
        _kick_target = Shoot2Goal;
    }

    //查表方式补偿
//    static int i = 0;
    double ballspeed = ball.Vel().mod();

    double tempdir = (Utils::Normalize(Utils::Normalize(pVision->ball().Vel().dir()+PARAM::Math::PI)-(_kick_target - pVision->ourPlayer(player).Pos()).dir()))*180/PARAM::Math::PI;
    int ratio = 0;
    if (tempdir>0){
        ratio = 1;
    }else{
        ratio = -1;
    }
    rawdir=abs((Utils::Normalize(Utils::Normalize(pVision->ball().Vel().dir()+PARAM::Math::PI)-(_kick_target - pVision->ourPlayer(player).Pos()).dir()))*180/PARAM::Math::PI);
    // cout << rawdir << endl;
    if (rawdir > 70 && rawdir < 110){
        rawdir = 80;
        //cout<<"kickdirection rawdir changed to 80"<<endl;
    }
    // 改成入射角？？？？？

    shoot_speed = ballspeed;
    shoot_dir = rawdir;

    _compensate_value = Compensate::Instance()->checkCompensate(ballspeed,rawdir);

    // cout << "KickDirection::GenerateShootDir"<<_compensate_value << endl;
    //YuN暂时去掉compensate，之后一定记得加上
    //_compensate_value = 0;

    if (pVision->ball().Vel().mod()<50){
        _compensate_value = 0;
    }
//    if (IS_SIMULATION){
//        _compensate_value = 0;
//    }
    _real_kick_dir= Utils::Normalize(Utils::Normalize(ratio*_compensate_value*PARAM::Math::PI/180)+_raw_kick_dir);
    if(pVision->ball().Vel().mod()<50){
        _real_kick_dir = _raw_kick_dir;
    }
    //cout<<vision->Cycle()<<" "<<ballspeed<<" "<<rawdir<<" "<<_compensate_value<<endl;
//	return;
    if (shootRange.size() > 0)
        return bestRange->getSize();
    else return 0;
}

double CKickDirection::calCompensate(double x, double y)
{
    double compensate=0;
    if (y<=20){
        compensate = 0;
    }else if(y>20 && y<50){
        compensate = 0.01*x+0.01*y+6;
    }else if (y>50){
        compensate = 0.01*x+0.005*y+9.5;
    }
    return compensate;
}

double CKickDirection::calGussiFuncA(double x1, double y1,double x2,double y2){
    double gamma = 0.1088;
    double result=0;
    double sum=(x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);
    result= exp((-1.0)*gamma*sum);
   return result;
}

double CKickDirection::calGussiFuncB(double x1, double y1,double x2,double y2){
    double gamma = 0.0039;
    double result=0;
    double sum=(x1-x2)*(x1-x2)+(y1-y2)*(y1-y2);
    result= exp((-1.0)*gamma*sum);
    return result;
}

//void CKickDirection::getData(double shoot_vel, double shoot_dir, double outdir)
//{
//	CompensateNew::Instance()->getData(shoot_vel, shoot_dir, outdir);
//}


// ------------------------------------------------------

