#include <cuda.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "cuda_runtime.h"
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <iostream>

#define FRAME_PERIOD (1 / 75.0)
#define ZERO_NUM (1e-8)
#define A_FACTOR (1.5)
#define PI (3.14159265359)
#define G (9.8)
#define SQRT_2 (1.414)
#define PLAYER_SIZE (90.0)

#define MAX_PLAYER (16)
#define THREAD_NUM_PASS (128)
#define BLOCK_X_PASS (16)
#define BLOCK_Y_PASS (MAX_PLAYER * 2)
#define MAX_BALL_SPEED (6500)
#define MIN_BALL_SPEED (1000)
#define BALL_SPEED_UNIT ((MAX_BALL_SPEED - MIN_BALL_SPEED) / BLOCK_X_PASS)

//#define MAX_CHIP_SPEED (400)
//#define MIN_CHIP_SPEED (50)
//#define CHIP_SPEED_UNIT ((MAX_CHIP_SPEED - MIN_CHIP_SPEED) / BLOCK_X_PASS)

#define MIN_DELTA_TIME (0)
#define OUR_RESPONSE_TIME (0.0)
#define THEIR_RESPONSE_TIME (0.0)
#define CAN_NOT_GET_STOP_BALL (true)

//---外部參數---
//场地参数
#define PITCH_LENGTH (9000.0)
#define PITCH_WIDTH (6000.0)
#define PENALTY_LENGTH (1000.0)
#define PENALTY_WIDTH (2000.0)
#define FIELD_BUFFER (300.0)
#define PENALTY_BUFFER (200.0)
//摩擦參數
#define ROLLING_FRACTION_REAL (800)
#define ROLLING_FRACTION_SIM (1520)
#define SLIDING_FRACTION_REAL (ROLLING_FRACTION_REAL * 15.0)
#define SLIDING_FRACTION_SIM (ROLLING_FRACTION_SIM * 15.0)
#define FLAT_ROLL_RATIO (5.0/7.0)
#define FLAT_SLIDE_RATIO (2.0/7.0)
//車球參數
#define PLAYER_CENTER_TO_BALL_CENTER (60)
#define STEP_TIME (0.2)
#define AVOID_DIST (2*PLAYER_SIZE)
//------------

//---CUDA參數---
#define PLAYER_NUM (16)
#define ANGLE_NUM (12)
#define DIST_NUM (8)
#define STEP_DIST (175)
#define SAFE_DIST (3.9*PLAYER_SIZE)
#define IGNORE_DIST (4*PLAYER_SIZE)
#define IGNORE_GOALIE (1)
#define IGNORE_GUARD (1)
//-------------

//时间预测的运动学参数
#define OUR_MAX_SPEED (3000)
#define OUR_MAX_ACC (4500)
#define OUR_MAX_DEC (4500)

#define THEIR_MAX_SPEED (3000)
#define THEIR_MAX_ACC (4500)
#define THEIR_MAX_DEC (4500)

#define OUR_PREDICT_BUFFER_TIME (0.0)
#define THEIR_PREDICT_BUFFER_TIME (0.5)

#define MAX_FLAT_VEL (5000.0)
#define MIN_FLAT_VEL (2000.0)
#define MAX_CHIP_VEL (4000.0)
#define MIN_CHIP_VEL (500.0)

#define True (1)
#define False (0)

//#define CHIP_ENERGY_LEFT_1 (0.34) //挑球后与地面碰撞后的能量剩余比例
//#define CHIP_ENERGY_LEFT_2 (0.97)

//计算点位评分
//#define BLOCK_X_FOR_POS_SCORE (4)
//#define BLOCK_Y_FOR_POS_SCORE (4)
//#define THREAD_X_FOR_POS_SCORE (32)
//#define THREAD_Y_FOR_POS_SCORE (32)
#define INITIAL_VALUE (99999)

// 评估函数各项的阈值
//__constant__ float maxDistToGoal =  900000;//sqrt(pow(PITCH_LENGTH, 2) + pow(PITCH_WIDTH, 2))
//__constant__ float minShootAngle = 0;
//__constant__ float maxShootAngle = PI / 2;
//__constant__ float maxDistToBall = 900000;//sqrt(pow(PITCH_LENGTH, 2) + pow(PITCH_WIDTH, 2))
//__constant__ float minDistToPassLine = 10.0;
//__constant__ float maxDistToPassLine = 900000;//sqrt(pow(PITCH_LENGTH, 2) + pow(PITCH_WIDTH, 2))
//__constant__ float minDistToEnemy = 30.0;
//__constant__ float maxDistToEnemy = 900000;//sqrt(pow(PITCH_LENGTH, 2) + pow(PITCH_WIDTH, 2))
// 评估函数各项的权重
//__constant__ float weight1 = 5;// 1.距离对方球门的距离
//__constant__ float weight2 = 8;// 2.射门有效角度
//__constant__ float weight3 = 0.5;// 3.跟球的距离
//__constant__ float weight4 = 0.3;// 4.对方车到传球线的距离
//__constant__ float weight5 = 0.5;// 5.对方车到接球点的距离
// 挑球模型参数
#define CHIP_FIRST_ANGLE (54.29 / 180.0 * PI)
#define CHIP_SECOND_ANGLE (45.59 / 180.0 * PI)
#define CHIP_VEL_RATIO  (0.6372)
#define MIN_CHIP_DIST (0.5)
#define MAX_CHIP_DIST (4.0)
#define MAX_CHIP_SPEED (50.0 * sqrt(2*G*MAX_CHIP_DIST/sin(2*CHIP_FIRST_ANGLE)))
#define MIN_CHIP_SPEED (50.0 * sqrt(2*G*MIN_CHIP_DIST/sin(2*CHIP_FIRST_ANGLE)))
#define CHIP_SPEED_UNIT ((MAX_CHIP_SPEED - MIN_CHIP_SPEED) / BLOCK_X_PASS)

#define CHIP_DIST_RATIO (0.8)
#define CHIP_LENGTH_RATIO (1.266)
typedef struct {
    float x, y;
} Vector;

typedef struct {
    float x, y;
} Point;

typedef struct {
    Point Pos;
    Vector Vel;
    bool isValid;
} Player;

typedef struct {
    Point interPos;
    bool isValid;
    float interTime;
    float Vel;
    float dir;
    int playerIndex;
    float deltaTime;
    float Q;
} nType;

typedef struct{

} DebugInfor;

typedef struct {
    Point p;
    float score;
} scoreAndPoint;


//計算垂點
__device__ Point projectionPointToLine(Point LP1, Point LP2, Point P) {
    Point result;
    if (LP2.x == LP1.x) {
        result.x = LP1.x;
        result.y = P.y;
    } else {
        // 如果该线段不平行于X轴也不平行于Y轴，则斜率存在且不为0。设线段的两端点为pt1和pt2，斜率为：
        float k = (LP2.y - LP1.y) / (LP2.x - LP1.x);
        // 该直线方程为:					y = k* ( x - pt1.x) + pt1.y
        // 其垂线的斜率为 -1/k,垂线方程为:	y = (-1/k) * (x - point.x) + point.y
        // 联立两直线方程解得:
        result.x = (k * k * LP1.x + k * (P.y - LP1.y) + P.x) / (k * k + 1);
        result.y = k * (result.x - LP1.x) + LP1.y;
    }
    return result;
}
//---通用函數---
__device__ bool if_finite(float a) {
    return fabs(a) < INITIAL_VALUE;
}

__device__ bool IsInField(Point p, float buffer=500) {
    return (p.x > -PITCH_LENGTH / 2 + buffer && p.x < PITCH_LENGTH / 2 - buffer
            && p.y < PITCH_WIDTH / 2 - buffer && p.y > -PITCH_WIDTH / 2 + buffer);
}

__device__ bool IsInPenalty(Point p, float buffer=300) {
    return (p.x < -PITCH_LENGTH/2 + PENALTY_LENGTH + buffer && p.x > -PITCH_LENGTH/2 &&
            p.y > -PENALTY_WIDTH/2 - buffer && p.y < PENALTY_WIDTH/2 + buffer) ||
           (p.x > PITCH_LENGTH/2 - PENALTY_LENGTH - buffer && p.x < PITCH_LENGTH/2 &&
            p.y > -PENALTY_WIDTH/2 - buffer && p.y < PENALTY_WIDTH/2 + buffer);
}
__device__ bool IsInEnemy(Point p1, Point p2, float buffer) {
    return ((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y)) < (buffer*buffer);
}
__device__ bool IsOutOfLimit(const float value, const float upLimit, const float lowLimit, const float range){
    return (value > upLimit*(1-range) || value < lowLimit*(1+range));
}
__device__ float Dist(Point p1,Point p2){
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}
//---球模型---
__device__ float flatRollTime(const float startVel, const float rollAcc){
    if(startVel < 0.0)
        return 0.0;
    return (startVel * FLAT_ROLL_RATIO / rollAcc);
}
__device__ float flatSlideTime(const float startVel, const float slideAcc){
    if(startVel < 0.0)
        return 0.0;
    return (startVel * FLAT_SLIDE_RATIO / slideAcc);
}
__device__ float flatRollDist(const float startVel, const float rollAcc){
    if (startVel < 0.0)
        return 0.0;
    return (pow(startVel * FLAT_ROLL_RATIO, 2.0) / (2.0 * rollAcc));
}
__device__ float flatSlideDist(const float startVel, const float slideAcc){
    if (startVel < 0.0)
        return 0.0;
    return ((pow(startVel, 2.0)-pow(startVel * FLAT_ROLL_RATIO, 2.0)) / (2.0 * slideAcc));
}
__device__ float flatStopTime(const float startVel, const float slideAcc, const float rollAcc){
    return flatSlideTime(startVel, slideAcc) + flatRollTime(startVel, rollAcc);
}
__device__ float flatMoveVel(const float startVel, const float time, const float slideAcc, const float rollAcc){
    const float slidingTime = flatSlideTime(startVel, slideAcc);
    const float stopTime = flatStopTime(startVel, slideAcc, rollAcc);
    float ballVel = 0;
    if(time > stopTime)
        ballVel = 0;
    else if (time < slidingTime)
        ballVel = startVel - slideAcc * time;
    else
        ballVel = startVel * FLAT_ROLL_RATIO - rollAcc * (time - slidingTime);
    return ballVel;
}

__device__ float flatMoveDist(const float startVel, const float time, const float slideAcc, const float rollAcc){
    float moveDist = 0;
    float slidingTime = flatSlideTime(startVel, slideAcc);
    float totalTime = flatStopTime(startVel, slideAcc, rollAcc);
    if(time > totalTime)
        moveDist = flatRollDist(startVel, rollAcc) + flatSlideDist(startVel, slideAcc);
    else if(time < slidingTime)
        moveDist = startVel * time - 0.5 * slideAcc * pow(time, 2.0);
    else {
        float slideDist = (pow(startVel, 2.0) - pow(startVel * FLAT_ROLL_RATIO, 2.0)) / (2.0 * slideAcc);
        float rollStartVel = startVel * FLAT_ROLL_RATIO;
        float rollTime = time - slidingTime;
        float rollDist = rollStartVel * rollTime - 0.5 * rollAcc * pow(rollTime, 2.0);
        moveDist = slideDist + rollDist;
    }
    return moveDist;
}
////////////////////////////////////////////////////////////////
/// \brief calculatFlatVel  計算球花給定時間走一段距離所需要的初速度
/// \param distance         距離
/// \param interTime        給定時間
/// \param isSim            是否為仿真
/// \return                 所需初速度
////////////////////////////////////////////////////////////////
__device__ float calculateFlatVel(float distance, float interTime, bool isSim)
{
    //interTime = max(interTime, 0.4);
    float ballSlidAcc = (isSim ? SLIDING_FRACTION_SIM : SLIDING_FRACTION_REAL) / 2;
    float ballRollAcc = (isSim ? ROLLING_FRACTION_SIM : ROLLING_FRACTION_REAL) / 2;
    const float minPassVel = sqrt(ballRollAcc * distance);
    float passVel = max((distance + 1.0/2.0*ballRollAcc*interTime*interTime) / interTime, minPassVel);
    passVel /= FLAT_ROLL_RATIO;
    //passVel = max(min(passVel, MAX_FLAT_VEL), MIN_FLAT_VEL);
    return passVel;
}
__device__ float calculateChipVel(float distance, float interTime, bool isSim)
{
    float passVel = 0;
    float chipDist = distance * CHIP_DIST_RATIO;
    passVel = chipDist / CHIP_LENGTH_RATIO;
    passVel = passVel > MAX_CHIP_VEL ? MAX_CHIP_VEL : passVel;
    passVel = passVel < MIN_CHIP_VEL ? MIN_CHIP_VEL : passVel;
    return passVel;
}
//------------


//////////////////////////////////////////////////////////////
/// \brief CUDA_compute_motion_1d   計算一維空間給定速度到點時間
/// \param x0                       目標距離
/// \param v0                       初速度
/// \param v1                       末速度
/// \param a_max                    最大加速度
/// \param d_max                    最大減速度
/// \param v_max                    最大速度
/// \param a_factor                 加速度因子
/// \param traj_time                到點時間
//////////////////////////////////////////////////////////////
__device__ void CUDA_compute_motion_1d(float x0, float v0, float v1, float a_max, float d_max,
                                       float v_max, float a_factor, float &traj_time)
{
    float traj_time_acc, traj_time_dec, traj_time_flat;
    traj_time_acc = traj_time_dec = traj_time_flat = 0.0;
    if ((x0 == 0 && v0 == v1) || !if_finite(x0) || !if_finite(v0) || !if_finite(v1)) {
        traj_time = 0;
        return;
    }

    a_max /= a_factor;
    d_max /= a_factor;

    float accel_dist_to_v1 = fabs((v1 + v0) / 2.0) * fabs(v1 - v0) / a_max;
    float decel_dist_to_v1 = fabs((v0 + v1) / 2.0) * fabs(v0 - v1) / d_max;

    float period = 1 / 40.0;

    if (v0 * x0 > 0 || (fabs(v0) > fabs(v1) && decel_dist_to_v1 > fabs(x0))) {
        float time_to_stop = fabs(v0) / (d_max);
        float x_to_stop = v0 * v0 / (2.0 * d_max);

        CUDA_compute_motion_1d(x0 + copysign(x_to_stop, v0), 0, v1, a_max * a_factor, d_max * a_factor, v_max, a_factor, traj_time);
        traj_time += time_to_stop;
        traj_time /= 1.25;
        return;
    }

    if (fabs(v0) > fabs(v1)) {
        traj_time_acc = (sqrt((d_max * v0 * v0 + a_max * (v1 * v1 + 2 * d_max * fabs(x0))) / (a_max + d_max)) - fabs(v0)) / a_max;

        if (traj_time_acc < 0.0)
            traj_time_acc = 0;
        traj_time_dec = ((fabs(v0) - fabs(v1)) + a_max * traj_time_acc) / d_max;
    }

    else if (accel_dist_to_v1 > fabs(x0)) {
        traj_time_acc = (sqrt(v0 * v0 + 2 * a_max * fabs(x0)) - fabs(v0)) / a_max;
        traj_time_dec = 0.0;
    }

    else {
        traj_time_acc = (sqrt((d_max * v0 * v0 + a_max * (v1 * v1 + 2 * d_max * fabs(x0))) / (a_max + d_max)) - fabs(v0)) / a_max;
        if (traj_time_acc < 0.0)
            traj_time_acc = 0;
        traj_time_dec = ((fabs(v0) - fabs(v1)) + a_max * traj_time_acc) / d_max;
    }


    if (traj_time_acc * a_max + fabs(v0) > v_max) {
        float dist_without_flat = (v_max * v_max - v0 * v0) / (2 * a_max) + (v_max * v_max - v1 * v1) / (2 * d_max);
        traj_time_flat = (fabs(x0) - dist_without_flat) / v_max;
    }
    else {
        traj_time_flat = 0;
    }

    if (FRAME_PERIOD * a_max + fabs(v0) > v_max && traj_time_flat > period) {
        traj_time = traj_time_flat + traj_time_dec;
    }
    else if (traj_time_acc < period && traj_time_dec == 0.0) {
        traj_time = traj_time_acc;
    }
    else if (traj_time_acc < period && traj_time_dec > 0.0) {
        traj_time = traj_time_dec;
    }
    else {
        traj_time = traj_time_acc + traj_time_flat / 1.1 + traj_time_dec / 1.1;
    }
}

//////////////////////////////////////////////////////////////
/// \brief CUDA_predictedTime   計算二維空間給定初始速度零速到點時間
/// \param x0                   出發點X
/// \param y0                   出發點Y
/// \param x1                   目標點X
/// \param y1                   目標點Y
/// \param vx                   初始速度X
/// \param vy                   初始速度Y
/// \param isTheir              是否為己方車
/// \return                     到點時間
//////////////////////////////////////////////////////////////
__device__ float CUDA_predictedTime(float x0, float y0, float x1, float y1, float vx, float vy, bool isTheir) {
    float timeX = 0.0;
    float timeY = 0.0;

    float Angle = atan2(vy+1e-5, vx+1e-5) - atan2(y0 - y1+1e-5, x0 - x1+1e-5);
//    float Angle = atan2(vy, vx) - atan2(y0 - y1, x0 - x1);
    float Speed = sqrt(vx * vx + vy * vy);
    vx = Speed * cospi(Angle / PI); //沿着球线
    vy = Speed * sinpi(Angle / PI); //切着球线

    float maxAcc, maxDec, maxSpeed;
    if(isTheir) {
        maxAcc = THEIR_MAX_ACC;
        maxDec = THEIR_MAX_DEC;
        maxSpeed = THEIR_MAX_SPEED;
    } else {
        maxAcc = OUR_MAX_ACC;
        maxDec = OUR_MAX_DEC;
        maxSpeed = OUR_MAX_SPEED;
    }
    CUDA_compute_motion_1d(sqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)), vx, 0.0, maxAcc, maxDec, maxSpeed, 1.5, timeX);
    CUDA_compute_motion_1d(0, vy, 0.0, maxAcc, maxDec, maxSpeed, 1.5, timeY);
    if (timeX < 1e-5 || timeX > 50) timeX = 0;
    if (timeY < 1e-5 || timeY > 50) timeY = 0;
    //printf("(x0,y0):(%f,%f) (x1,y1):(%f,%f) (vx,vy):(%f,%f) (tX,tY):(%f,%f)\n",x0,y0,x1,y1,vx,vy,timeX,timeY);
    return (timeX > timeY ? timeX : timeY);
}

__device__ bool CUDA_predictedFlatInterTime(Point mePoint, Point ballPoint, Vector meVel, Vector ballVel,
                                            const float ballArriveTime, Point* interceptPoint, float* interTime,
                                            float responseTime, bool isTheir, bool isSim) {
    //meVel.x=0;
    //meVel.y=0;
    float ballRollAcc = (isSim ? ROLLING_FRACTION_SIM : ROLLING_FRACTION_REAL) / 2;
    float ballSlideAcc = (isSim ? SLIDING_FRACTION_SIM : SLIDING_FRACTION_REAL) / 2;
    const float originVel = sqrt(ballVel.x * ballVel.x + ballVel.y * ballVel.y);
    const float maxMoveTime = flatStopTime(originVel, ballSlideAcc, ballRollAcc);
    float testMoveDist = 0;
    float testMoveTime = 0;
    Point testPoint = ballPoint;
    bool canInter = false;
    bool safeFlag = true;
    float adjustLength = 0, interT=INITIAL_VALUE;
    for (testMoveTime = 0; testMoveTime < maxMoveTime; testMoveTime += STEP_TIME ) {
        testMoveDist = flatMoveDist(originVel, testMoveTime, ballSlideAcc, ballRollAcc);
        // 计算截球点
        testPoint.x = ballPoint.x + testMoveDist * ballVel.x / originVel;
        testPoint.y = ballPoint.y + testMoveDist * ballVel.y / originVel;
        // 对敌方截球点进行特殊处理
//        if(isTheir) {
//            Vector adjustDir;
//            adjustDir.x = mePoint.x - testPoint.x;
//            adjustDir.y = mePoint.y - testPoint.y;
//            adjustLength = sqrt(adjustDir.x * adjustDir.x + adjustDir.y * adjustDir.y);
//            testPoint.x += adjustDir.x / adjustLength * (AVOID_DIST>adjustLength?adjustLength:AVOID_DIST);
//            testPoint.y += adjustDir.y / adjustLength * (AVOID_DIST>adjustLength?adjustLength:AVOID_DIST);
//        }
        if(IsInPenalty(testPoint, PENALTY_BUFFER))
            continue;
        if (!IsInField(testPoint, FIELD_BUFFER)){
            canInter = false;
            break;
        }
        if(testMoveTime >= STEP_TIME * 3 && isTheir && IsInEnemy(testPoint,mePoint,SAFE_DIST)){
            safeFlag = false;
            break;
        }
        // 计算截球时间
        interT = CUDA_predictedTime(mePoint.x, mePoint.y, testPoint.x, testPoint.y, meVel.x, meVel.y, isTheir);
        if(interT + responseTime < testMoveTime){
            canInter = true;
            break;
        }
    }

    if(!safeFlag){
        interceptPoint->x = INITIAL_VALUE;
        interceptPoint->y = INITIAL_VALUE;
        *interTime = 0;
        return true;
    }
    // 无法截球
    if(!canInter) {
        interceptPoint->x = INITIAL_VALUE;
        interceptPoint->y = INITIAL_VALUE;
        *interTime = INITIAL_VALUE;
        return false;
    }
    // 能够截球计算截球时间和截球点
    *interceptPoint = testPoint;
    *interTime = interT;
    return true;
}

__device__ bool CUDA_FlatSecurityCheck(Point mePoint, Point ballPoint, Point target, Vector meVel, Vector ballVel,
                                       int enemyRole, float responseTime,
                                       float ignoreCloseEnemyDist, bool ignoreTheirGoalie, bool ignoreTheirGuard,
                                       bool isSim){
    Point passLine;
    float ballSpeed = sqrt(ballVel.x * ballVel.x + ballVel.y * ballVel.y);
    passLine.x = target.x - ballPoint.x;
    passLine.y = target.y - ballPoint.y;
    float passLineDist = sqrt(passLine.x * passLine.x + passLine.y * passLine.y);
    float passLineDir = atan2(passLine.y+1e-5, passLine.x+1e-5);
    float ballRollAcc = (isSim ? ROLLING_FRACTION_SIM : ROLLING_FRACTION_REAL) / 2;
    float ballSlideAcc = (isSim ? SLIDING_FRACTION_SIM : SLIDING_FRACTION_REAL) / 2;
    if(Dist(mePoint, ballPoint) < ignoreCloseEnemyDist && !IsInPenalty(mePoint, 0))
        return true;
    Point pedal = projectionPointToLine(ballPoint, target, mePoint);
    if(pedal.x > min(ballPoint.x,target.x) && pedal.x < max(ballPoint.x,target.x)){
        if(Dist(mePoint, pedal) < SAFE_DIST) return false;
    }
    else{
        if(min(Dist(mePoint,ballPoint),Dist(mePoint,target)) < SAFE_DIST) return false;
    }
    if(enemyRole == 1 && ignoreTheirGoalie) return true;
    if(enemyRole == 2 && ignoreTheirGuard) return true;
    float ballMoveTime = 0;
    while(1){
        ballMoveTime += STEP_TIME;
        float ballMoveDist = flatMoveDist(ballSpeed, ballMoveTime, ballSlideAcc, ballRollAcc);
        Point ballPos;
        ballPos.x = ballPoint.x + ballMoveDist * cos(passLineDir);
        ballPos.y = ballPoint.y + ballMoveDist * sin(passLineDir);
        if(!IsInField(ballPos) || ballMoveDist > passLineDist)
            return true;
        float adjustDir = atan2(mePoint.y - ballPos.y+1e-5, mePoint.x - ballPos.x+1e-5);
        Point testPoint;
        testPoint.x = ballPos.x + SAFE_DIST * cos(adjustDir);
        testPoint.y = ballPos.y + SAFE_DIST * sin(adjustDir);
        float enemyTime = CUDA_predictedTime(mePoint.x, mePoint.y, testPoint.x, testPoint.y, meVel.x, meVel.y, True);
        if(enemyTime + responseTime - ballMoveTime < 0)
            return false;
    }
    return true;
}

__device__ bool CUDA_predictedChipInterTime(Point mePoint, Point ballPoint, Vector meVel, Vector ballVel,
                                            const float ballArriveTime, Point* interceptPoint, float* interTime,
                                            float responseTime, bool isTheir, bool isSim) {
    float chipVel = sqrt(ballVel.x * ballVel.x + ballVel.y * ballVel.y);
    float meArriveTime = INITIAL_VALUE;
    float ballAcc = (isSim ? ROLLING_FRACTION_SIM : ROLLING_FRACTION_REAL) / 2.0;
    float ballAccSecondJump = 0;
    float stepTime = 0.05;
    float testBallLength = 0;
    Point testPoint = ballPoint;

    // 挑球第一段的时间, 单位s
    float time_1 = 2.0 * chipVel * sin(CHIP_FIRST_ANGLE) / 1000.0 / G;
    // 挑球第一段的距离, 单位m
    float length_1 = 1.0 / 2 * G * time_1 * time_1 / tan(CHIP_FIRST_ANGLE);
    // 挑球第二段的距离, 单位m
    float length_2 = (CHIP_LENGTH_RATIO - 1.0) * length_1;
    // 挑球第二段的时间, 单位s
    float time_2 = sqrt(2 * length_2 * tan(CHIP_SECOND_ANGLE) / G); // 单位s
    // 挑球第一二段的距离, 单位mm
    length_1 *= 1000;
    length_2 *= 1000;
    // 可以开始截球的起始距离
    float jumpDist = 0;
    // 可以开始截球的起始速度
    float moveVel = 0;
    // 可以开始截球的起始时刻
    float ballDropTime = 0;
    // 球滚动的最大时间
    float max_time = 0;
    if(isTheir) {
        jumpDist = length_1;
        moveVel = length_2 / time_2;
        ballDropTime = time_1;
        max_time = time_2 + chipVel * chipVel * CHIP_VEL_RATIO / 980 / ballAcc;
    } else {
        jumpDist = length_1 + length_2;
        moveVel = chipVel * chipVel * CHIP_VEL_RATIO / 980;
        ballDropTime = time_1 + time_2;
        max_time = moveVel / ballAcc;
    }

    bool canInter = true, theirCanTouch = false, isSecondJump = true;
    float afterArrivedTime = 0, secondJumpDist = 0, secondJumpTime = 0, secondJumpVelLeft = 0;
    while (afterArrivedTime < max_time) {
        Vector direc;
        if(isTheir) {
            if(isSecondJump) {
                testBallLength = jumpDist + moveVel * afterArrivedTime;
                if(testBallLength > length_1 + length_2) {
                    secondJumpDist = length_2;
                    secondJumpTime = time_2;
                    secondJumpVelLeft = chipVel * chipVel * CHIP_VEL_RATIO / 980;
                    isSecondJump = false;
                }
            }
            else {
                testBallLength = jumpDist + secondJumpDist + (secondJumpVelLeft * (afterArrivedTime - secondJumpTime) - 0.5 * ballAcc * (afterArrivedTime - secondJumpTime) * (afterArrivedTime - secondJumpTime));
            }
        }
        else {
            testBallLength = jumpDist + (moveVel * afterArrivedTime - 0.5 * ballAcc * afterArrivedTime * afterArrivedTime);
        }

        direc.x = testBallLength * ballVel.x / chipVel;
        direc.y = testBallLength * ballVel.y / chipVel;
        testPoint.x = ballPoint.x + direc.x;
        testPoint.y = ballPoint.y + direc.y;

        if(isTheir) {
            if(sqrt((mePoint.x - testPoint.x) * (mePoint.x - testPoint.x) + (mePoint.y - testPoint.y) * (mePoint.y - testPoint.y)) < PLAYER_SIZE * 1.2) {
                theirCanTouch = true;
                break;
            } else {
                Vector adjustDir;
                adjustDir.x = mePoint.x - testPoint.x;
                adjustDir.y = mePoint.y - testPoint.y;
                float length = sqrt(adjustDir.x * adjustDir.x + adjustDir.y * adjustDir.y);
                adjustDir.x /= length;
                adjustDir.y /= length;
                testPoint.x += adjustDir.x * PLAYER_SIZE;
                testPoint.y += adjustDir.y * PLAYER_SIZE;
            }
        }

        meArriveTime = CUDA_predictedTime(mePoint.x, mePoint.y, testPoint.x, testPoint.y, meVel.x, meVel.y, isTheir);

        if(meArriveTime < 0.10) meArriveTime = 0;

        if(IsInPenalty(testPoint, 200)) {
            afterArrivedTime += stepTime;
            continue;
        }
        if (!IsInField(testPoint)) {
            canInter = false;
            break;
        }
        if(meArriveTime + responseTime < ballDropTime + afterArrivedTime) {
            break;
        }
        afterArrivedTime += stepTime;
    }

    if(!canInter || (CAN_NOT_GET_STOP_BALL && afterArrivedTime >= max_time)){
        interceptPoint->x = INITIAL_VALUE;
        interceptPoint->y = INITIAL_VALUE;
        *interTime = INITIAL_VALUE;
        return false;
    }
    *interceptPoint = testPoint;
    *interTime = CUDA_predictedTime(mePoint.x, mePoint.y, testPoint.x, testPoint.y, meVel.x, meVel.y, isTheir);
    *interTime = *interTime > ballDropTime ? *interTime : ballDropTime;
//    Vector velDir;
//    velDir.x = ballVel.x / chipVel;
//    velDir.y = ballVel.y / chipVel;
////    float meX = 547;
////    float meY = -276;
//    float meX = 444;
//    float meY = -105;

//    float vel = 428;
//    float dir = 4.07;
//    if(!isTheir && mePoint.x < meX + 5 && mePoint.x > meX - 5 && mePoint.y < meY + 5 && mePoint.y > meY - 5 && chipVel > vel - 1.0 && chipVel < vel + 1.0  && atan(ballVel.y / ballVel.x) > dir - 0.1 -  PI && atan(ballVel.y / ballVel.x) < dir + 0.1 - PI) {
//        printf("%lf %lf (%lf, %lf), (%lf, %lf), %lf, %lf, interTime: %f\n", testPoint.x, testPoint.y, ballPoint.x + velDir.x * length_1, ballPoint.y + velDir.y * length_1, ballPoint.x + velDir.x * (length_2 + length_1), ballPoint.y + velDir.y * (length_2 + length_1), chipVel, atan(ballVel.y / ballVel.x), *interTime);
//    }


    if(theirCanTouch){
        *interTime = 0.0;
    }

//    float vel = 202;
//    float dir = 3.28885;
//    if(isTheir && ballPoint.x + velDir.x * length_1 < 0 && chipVel > vel - 1.0 && chipVel < vel + 1.0  && atan(ballVel.y / ballVel.x) > dir - 0.001 - PI && atan(ballVel.y / ballVel.x) < dir + 0.001 - PI) {
//        printf("%lf, %lf, (%lf, %lf)\n", *interTime, testBallLength, (*interceptPoint).x, (*interceptPoint).y);
//    }

    return true;
}

__device__ bool CUDA_ChipSecurityCheck(Point mePoint, Point ballPoint, Point target, Vector meVel, Vector ballVel,
                                        int enemyRole, float responseTime,
                                        bool ignoreTheirGuard, bool isSim){
    float BALL_ACC = (isSim ? ROLLING_FRACTION_SIM : ROLLING_FRACTION_REAL) / 2.0;

    Point passLine;
    passLine.x = target.x - ballPoint.x;
    passLine.y = target.y - ballPoint.y;
    float passLineDist = sqrt(passLine.x * passLine.x + passLine.y * passLine.y) - PLAYER_SIZE;
    float passLineDir = atan2(passLine.y+1e-5, passLine.x+1e-5);

    float chipLength1 = sqrt(ballVel.x * ballVel.x + ballVel.y * ballVel.y) / 1000;
    float chipTime1 = sqrt(2.0 * chipLength1 * tan(CHIP_FIRST_ANGLE) / G);
    float chipLength2 = (CHIP_LENGTH_RATIO - 1) * chipLength1;
    float chipTime2 = sqrt(2 * chipLength2 * tan(CHIP_SECOND_ANGLE) / G);
    chipLength1 *= 1000;
    chipLength2 *= 1000;
    Point startPos;
    startPos.x = ballPoint.x + (chipLength1 + chipLength2) * cos(passLineDir);
    startPos.y = ballPoint.y + (chipLength1 + chipLength2) * sin(passLineDir);
    float startTime = chipTime1 + chipTime2;
    float startVel = pow(chipTime1 * 1000 * G / (2 * sin(CHIP_FIRST_ANGLE)), 2) * CHIP_VEL_RATIO / 9800;
    float startDist = Dist(startPos, ballPoint);
    Point pedal = projectionPointToLine(ballPoint, target, mePoint);
    if(pedal.x > min(startPos.x,target.x) && pedal.x < max(startPos.x,target.x)){
        if(Dist(mePoint, pedal) < SAFE_DIST) return false;
    }
    else{
        if(min(Dist(mePoint,startPos),Dist(mePoint,target)) < SAFE_DIST) return false;
    }
    if(enemyRole == 2 && ignoreTheirGuard) return true;
    Point ballPos = startPos;
    float ballSpeed = startVel;
    float ballMoveTime = startTime;
    float ballMoveDist = startDist;
    while(1){
        ballMoveTime += STEP_TIME;
        ballSpeed = startVel - BALL_ACC * (ballMoveTime - startTime);
        if(ballSpeed < 0) ballSpeed = 0;
        ballMoveDist = startDist + (ballSpeed + startVel) / 2 * (ballMoveTime - startTime);
        ballPos.x = ballPoint.x + ballMoveDist * cos(passLineDir);
        ballPos.y = ballPoint.y + ballMoveDist * sin(passLineDir);
        if(!IsInField(ballPos) || ballMoveDist > passLineDist)
            return true;
        float adjustDir = atan2(mePoint.y - ballPos.y+1e-5, mePoint.x - ballPos.x+1e-5);
        Point testPoint;
        testPoint.x = ballPos.x + SAFE_DIST * cos(adjustDir);
        testPoint.y = ballPos.y + SAFE_DIST * sin(adjustDir);
        float enemyTime = CUDA_predictedTime(mePoint.x, mePoint.y, testPoint.x, testPoint.y, meVel.x, meVel.y, True);
        if(enemyTime + responseTime - ballMoveTime < 0)
            return false;
    }
    return true;
}

__global__ void calculateAllInterInfoV10(Player* players, Point* ballPos, nType* bestPass, int leader, double theirFlatResponseTime, double theirChipResponseTime, bool isSim) {

    //printf("bushibawokao");

    int catcherIndex = blockIdx.x;
    int angleIndex = blockIdx.y;
    int distanceIndex =  threadIdx.x / PLAYER_NUM;
    int blockerIndex = threadIdx.x % PLAYER_NUM;
    bool flatSecurity = true;
    bool chipSecurity = true;
    bool canInter;
    float interTime = 0.0;
    Point interPoint;

    Point catchPoint;
    catchPoint.x = players[catcherIndex].Pos.x + (distanceIndex + 2) * STEP_DIST * cospi(2.0 * angleIndex / ANGLE_NUM);
    catchPoint.y = players[catcherIndex].Pos.y + (distanceIndex + 2) * STEP_DIST * sinpi(2.0 * angleIndex / ANGLE_NUM);

    if(!IsInField(catchPoint) || IsInPenalty(catchPoint) || !players[catcherIndex].isValid){
        flatSecurity = chipSecurity = false;
    }

    double selfPassBuffer = 1500.0;
    if(catcherIndex == leader && IsInPenalty(catchPoint, selfPassBuffer) && !IsInField(catchPoint, selfPassBuffer)) flatSecurity = chipSecurity = false;

    if(catcherIndex == leader && distanceIndex != 2) flatSecurity = chipSecurity = false;
    float catchtime = CUDA_predictedTime(players[catcherIndex].Pos.x, players[catcherIndex].Pos.y, catchPoint.x, catchPoint.y, players[catcherIndex].Vel.x, players[catcherIndex].Vel.y, False);
    float ballLineDist = sqrt((catchPoint.x - ballPos->x) * (catchPoint.x - ballPos->x) + (catchPoint.y - ballPos->y) * (catchPoint.y - ballPos->y));
    float ballLineAngle = atan2(catchPoint.y - ballPos->y, catchPoint.x - ballPos->x);
    Vector ballFlatVel, ballChipVel;
    float ballFlatSpeed = calculateFlatVel(ballLineDist, catchtime + OUR_PREDICT_BUFFER_TIME, isSim);
//    if (catcherIndex == leader) ballFlatSpeed = MIN_FLAT_VEL;
//    if(leader != catcherIndex) ballFlatSpeed = calculateFlatVel(ballLineDist, catchtime + OUR_PREDICT_BUFFER_TIME, isSim);
//    else ballFlatSpeed = calculateFlatVel(ballLineDist, catchtime + OUR_PREDICT_BUFFER_TIME + 0.5, isSim);
//    printf("minspeed:%f\n", ballFlatSpeed);
    if(players[catcherIndex].isValid && IsOutOfLimit(ballFlatSpeed, MAX_FLAT_VEL, MIN_FLAT_VEL, 0.05)){
        //flatSecurity = false;
        if(ballFlatSpeed > MAX_FLAT_VEL){
            ballFlatSpeed = MAX_FLAT_VEL;
            ballFlatVel.x = ballFlatSpeed * cos(ballLineAngle);
            ballFlatVel.y = ballFlatSpeed * sin(ballLineAngle);
            CUDA_predictedFlatInterTime(players[catcherIndex].Pos,*ballPos,players[catcherIndex].Vel,ballFlatVel,
                                        INITIAL_VALUE, &catchPoint,&catchtime,OUR_RESPONSE_TIME,false,isSim);
        }
    }
//    if (catcherIndex == leader) ballFlatSpeed = 1000.0;
    float ballChipSpeed = calculateChipVel(ballLineDist, catchtime + OUR_PREDICT_BUFFER_TIME, isSim);
    if(IsOutOfLimit(ballChipSpeed, MAX_CHIP_VEL, MIN_CHIP_VEL, 0.05)){
        chipSecurity = false;
    }
    ballFlatVel.x = ballFlatSpeed * cos(ballLineAngle);
    ballFlatVel.y = ballFlatSpeed * sin(ballLineAngle);
    ballChipVel.x = ballChipSpeed * cos(ballLineAngle);
    ballChipVel.y = ballChipSpeed * sin(ballLineAngle);

    int enemyRole = 0;
    if(IsInPenalty(players[blockerIndex + PLAYER_NUM].Pos, 0))
        enemyRole = 1;
    else
        if(IsInPenalty(players[blockerIndex + PLAYER_NUM].Pos, 300))
            enemyRole = 2;
    if(players[blockerIndex + PLAYER_NUM].isValid == 1 && flatSecurity){
          flatSecurity = CUDA_FlatSecurityCheck(players[blockerIndex + PLAYER_NUM].Pos,*ballPos,catchPoint,players[blockerIndex + PLAYER_NUM].Vel,ballFlatVel,
                         enemyRole,theirFlatResponseTime,IGNORE_DIST,IGNORE_GOALIE,IGNORE_GUARD,isSim);
    }

    if(players[blockerIndex + PLAYER_NUM].isValid == 1 && chipSecurity){
          chipSecurity = CUDA_ChipSecurityCheck(players[blockerIndex + PLAYER_NUM].Pos,*ballPos,catchPoint,players[blockerIndex + PLAYER_NUM].Vel,ballChipVel,
                         enemyRole,theirChipResponseTime,IGNORE_GUARD,isSim);
    }

    int offset = catcherIndex * ANGLE_NUM * DIST_NUM * PLAYER_NUM + angleIndex * DIST_NUM * PLAYER_NUM + distanceIndex * PLAYER_NUM + blockerIndex;

    bestPass[offset].interPos = catchPoint;
    bestPass[offset].interTime = catchtime;
    bestPass[offset].playerIndex = catcherIndex;
    bestPass[offset].dir = ballLineAngle;
    bestPass[offset].Vel = flatSecurity? ballFlatSpeed: 0;
    bestPass[offset].isValid = flatSecurity;

    offset += PLAYER_NUM * ANGLE_NUM * DIST_NUM * PLAYER_NUM;

    bestPass[offset].interPos = catchPoint;
    bestPass[offset].interTime = catchtime;
    bestPass[offset].playerIndex = catcherIndex;
    bestPass[offset].dir = ballLineAngle;
    bestPass[offset].Vel = chipSecurity? ballChipSpeed: 0;
    bestPass[offset].isValid = chipSecurity;
}

extern "C" void BestPass(Player* players, Point* ball, nType* result, int leader, double theirFlatResponseTime, double theirChipResponseTime, bool isSim) {
    nType *pointInfor;
    cudaMallocManaged((void**)&pointInfor, 2*PLAYER_NUM*ANGLE_NUM*DIST_NUM*PLAYER_NUM*sizeof(nType));
//    cudaEvent_t start, stop;
//    cudaEventCreate(&start);
//    cudaEventCreate(&stop);
//    cudaEventRecord(start);

    //new CUDA
    //printf("angleNum%d,playerNum%d,distanceNum%d\n",params->angleNum,params->playerNum,params->distanceNum);

    dim3 blocks(PLAYER_NUM, ANGLE_NUM);
    calculateAllInterInfoV10 <<<blocks, DIST_NUM * PLAYER_NUM>>> (players,ball,pointInfor, leader, theirFlatResponseTime, theirChipResponseTime,isSim);
    cudaDeviceSynchronize();
    cudaError_t cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess){
        printf("CUDA ERROR: %d\n", (int)cudaStatus);
        printf("Error Name: %s\n", cudaGetErrorName(cudaStatus));
        printf("Description: %s\n", cudaGetErrorString(cudaStatus));
    }
    for(int i = 0; i < 2*PLAYER_NUM*ANGLE_NUM*DIST_NUM*PLAYER_NUM; ++i)
        result[i] = pointInfor[i];

//    cudaEventRecord(stop);
//    cudaEventSynchronize(stop);

//    float milliseconds = 0;
//    cudaEventElapsedTime(&milliseconds, start, stop);
//    printf("Time: %.5f ms\n", milliseconds);
    cudaFree(pointInfor);
}

//extern "C" void PosScore(Player* players, Point* ballPos, Point* bestPositions) {
//    scoreAndPoint *allScore;
//    cudaMallocManaged((void**)&allScore, BLOCK_X_FOR_POS_SCORE * BLOCK_Y_FOR_POS_SCORE * THREAD_X_FOR_POS_SCORE * THREAD_Y_FOR_POS_SCORE * sizeof(scoreAndPoint));
////    cudaEvent_t start, stop;
////    cudaEventCreate(&start);
////    cudaEventCreate(&stop);
////    cudaEventRecord(start);

//    dim3 blocks(BLOCK_X_FOR_POS_SCORE, BLOCK_Y_FOR_POS_SCORE);
//    dim3 threads(THREAD_X_FOR_POS_SCORE, THREAD_Y_FOR_POS_SCORE);
//    calculateAllPosScore<<< blocks, threads >>> (players, ballPos, allScore);
//    cudaDeviceSynchronize();

//    sortPosScore<<< blocks, threads >>> (allScore);
//    cudaDeviceSynchronize();

//    cudaError_t cudaStatus = cudaGetLastError();
//    if (cudaStatus != cudaSuccess){
//        printf("CUDA ERROR: %d\n", (int)cudaStatus);
//        printf("Error Name: %s\n", cudaGetErrorName(cudaStatus));
//        printf("Description: %s\n", cudaGetErrorString(cudaStatus));
//    }
////    cudaEventRecord(stop);
////    cudaEventSynchronize(stop);
////    float milliseconds = 0;
////    cudaEventElapsedTime(&milliseconds, start, stop);
////    printf("Time: %.5f ms\n", milliseconds);

//    for(int i = 0; i < BLOCK_X_FOR_POS_SCORE * BLOCK_Y_FOR_POS_SCORE; i++) {
//        bestPositions[i] = allScore[i * THREAD_X_FOR_POS_SCORE * THREAD_Y_FOR_POS_SCORE].p;
////        printf("(%lf, %lf)\n", allScore[i * THREAD_X_FOR_POS_SCORE * THREAD_Y_FOR_POS_SCORE].p.x, allScore[i * THREAD_X_FOR_POS_SCORE * THREAD_Y_FOR_POS_SCORE].p.y);
//    }
//    cudaFree(allScore);
//    return;
//}

// attack threat Evaluation Function for Run Pos
// # attack
// 1.距离对方球门的距离 2.射门有效角度 3.跟球的距离 4.对方车到传球线的距离 5.对方车到接球点的距离
// # defence
// !!!!!!!!!!!!!!!!!!!!!! 可以根據場上形式使用不同的公式
//__device__ float CUDA_evaluateFunc(Point candidate, Point ballPos, Player* enemy, Player receiver)
//{
//    float score = -INITIAL_VALUE;
//    // 1.距离对方球门的距离
//    Point goal;
//    goal.x = 600;
//    goal.y = 0;
//    float distToGoal = sqrt((candidate.x - goal.x) * (candidate.x - goal.x) + (candidate.y - goal.y) * (candidate.y - goal.y));

//    // 2.射门有效角度
//    Point leftGoalPost;
//    Point rightGoalPost;
//    leftGoalPost.x = rightGoalPost.x = 600;
//    leftGoalPost.y = -60;
//    rightGoalPost.y = 60;
//    float leftDir = atan2((candidate.y - leftGoalPost.y) , (candidate.x - leftGoalPost.x));
//    float rightDir = atan2((candidate.y - rightGoalPost.y) , (candidate.x - rightGoalPost.x));
//    float shootAngle = fabs(leftDir - rightDir);
//    shootAngle = shootAngle > PI ? 2*PI - shootAngle : shootAngle;

//    // 3.跟球的距离
//    float distToBall = sqrt((candidate.x - ballPos.x) * (candidate.x - ballPos.x) + (candidate.y - ballPos.y) * (candidate.y - ballPos.y));
//    // 4.对方车到传球线的距离
//    float distToPassLine = INITIAL_VALUE;
//    for (int i=0; i < MAX_PLAYER; i++) {
//        if(enemy[i].isValid){
//            Point projection = projectionPointToLine(candidate, ballPos, enemy[i].Pos);
//            // 判断是否在线段之间
//            if(projection.x > Min(ballPos.x, candidate.x) && projection.x < Max(ballPos.x, candidate.x)){
//                float dist = sqrt((projection.x - enemy[i].Pos.x) * (projection.x - enemy[i].Pos.x) + (projection.y - enemy[i].Pos.y) * (projection.y - enemy[i].Pos.y));
//                if(dist < distToPassLine)
//                    distToPassLine = dist;
//            }
//        }
//    }
//    // 5.对方车到接球点的距离
//    float distToEnemy = INITIAL_VALUE;
//    for (int i=0; i < MAX_PLAYER; i++) {
//        if(enemy[i].isValid){
//            float dist = sqrt((candidate.x - enemy[i].Pos.x) * (candidate.x - enemy[i].Pos.x) + (candidate.y - enemy[i].Pos.y) * (candidate.y - enemy[i].Pos.y));
//            if(dist < distToEnemy)
//                distToEnemy = dist;
//        }
//    }

//    // 当满足最低要求时计算得分
//    if(distToGoal < maxDistToGoal && shootAngle >= minShootAngle && distToBall < maxDistToBall
//            && distToPassLine >= minDistToPassLine && distToEnemy >= minDistToEnemy){
//        // 归一化处理
//        distToGoal = 1 - distToGoal/maxDistToGoal;
//        shootAngle = shootAngle/maxShootAngle;
//        distToBall = 1 - distToBall/maxDistToBall;
//        distToPassLine = distToPassLine/maxDistToPassLine;
//        distToEnemy = distToEnemy/maxDistToEnemy;

//        // 计算得分
//        score = weight1*distToGoal + weight2*shootAngle + weight3*distToBall + weight4*distToPassLine + weight5*distToEnemy;
//    }
//    return score;
//}

//__global__ void calculateAllPosScore(Player* Players, Point* ballPos, scoreAndPoint* allScore) {
//    float blockLength = PITCH_LENGTH / gridDim.x;
//    float blockWidth = PITCH_WIDTH / gridDim.y;
//    float threadLength = blockLength / blockDim.x;
//    float threadWidth = blockWidth / blockDim.y;
//    int blockIndex = gridDim.x * blockIdx.y + blockIdx.x;
//    int threadIndex = blockDim.x * threadIdx.y + threadIdx.x;
//    int allScoreIndex = blockIndex * blockDim.x * blockDim.y + threadIndex;
//    __syncthreads();
//    allScore[allScoreIndex].p.x = blockLength * blockIdx.x + threadLength / 2 + threadLength * threadIdx.x - PITCH_LENGTH / 2;
//    allScore[allScoreIndex].p.y = blockWidth * blockIdx.y + threadWidth / 2 + threadWidth * threadIdx.y - PITCH_WIDTH / 2;
//    __syncthreads();
//    if(IsInPenalty(allScore[allScoreIndex].p))
//        allScore[allScoreIndex].score = INITIAL_VALUE;
//    else
//        allScore[allScoreIndex].score = CUDA_evaluateFunc(allScore[allScoreIndex].p, *ballPos, Players, Players[MAX_PLAYER]);
//    __syncthreads();
//}

//__global__ void sortPosScore(scoreAndPoint *allScore) {
//    __shared__ scoreAndPoint scoreBlock[THREAD_X_FOR_POS_SCORE * THREAD_Y_FOR_POS_SCORE];
//    int blockIndex = gridDim.x * blockIdx.y + blockIdx.x;
//    int threadIndex = blockDim.x * threadIdx.y + threadIdx.x;
//    int allScoreIndex = blockIndex * blockDim.x * blockDim.y + threadIndex;
//    scoreAndPoint temp;
//    scoreBlock[threadIndex] = allScore[allScoreIndex];
//    __syncthreads();
//    //并行地按照从大到小的次序进行排列
//    bool even = true;
//    for(int i = 0; i < THREAD_X_FOR_POS_SCORE * THREAD_Y_FOR_POS_SCORE; i++) {
//        if(threadIndex < THREAD_X_FOR_POS_SCORE * THREAD_Y_FOR_POS_SCORE - 1 && even && scoreBlock[threadIndex].score < scoreBlock[threadIndex + 1].score) {
//            temp = scoreBlock[threadIndex + 1];
//            scoreBlock[threadIndex + 1] = scoreBlock[threadIndex];
//            scoreBlock[threadIndex] = temp;
//        }
//        else if(threadIndex > 0 && !even && scoreBlock[threadIndex].score > scoreBlock[threadIndex - 1].score) {
//            temp = scoreBlock[threadIndex];
//            scoreBlock[threadIndex] = scoreBlock[threadIndex - 1];
//            scoreBlock[threadIndex - 1] = temp;
//        }
//        even = !even;
//        __syncthreads();
//    }
//    allScore[allScoreIndex] = scoreBlock[threadIndex];
//    __syncthreads();
//}
