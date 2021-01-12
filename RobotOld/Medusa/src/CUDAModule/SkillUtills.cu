#include <cuda.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "cuda_runtime.h"
#include <math.h>
#include <stdio.h>

#define FRAME_PERIOD (1 / 75.0)
#define ZERO_NUM (1e-8)
#define A_FACTOR (1.5)
#define PI (3.14159265359)
#define G (9.8)
#define SQRT_2 (1.414)
#define PLAYER_SIZE (9.0)

#define PLAYER_CENTER_TO_BALL_CENTER (60)
#define MAX_PLAYER_NUM (16)
#define THREAD_NUM_PASS (128)
#define BLOCK_X_PASS (16)
#define BLOCK_Y_PASS (MAX_PLAYER_NUM * 2)
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

//场地参数
#define PITCH_LENGTH (12000)
#define PITCH_WIDTH (9000)
#define PENALTY_LENGTH (1200)
#define PENALTY_WIDTH (2400)

//时间预测的运动学参数
#define OUR_MAX_SPEED (3000)
#define OUR_MAX_ACC (4500)
#define OUR_MAX_DEC (4500)

#define THEIR_MAX_SPEED (3000)
#define THEIR_MAX_ACC (4500)
#define THEIR_MAX_DEC (4500)

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
#define CHIP_LENGTH_RATIO (1.266)
#define CHIP_VEL_RATIO  (0.6372)
#define MIN_CHIP_DIST (0.5)
#define MAX_CHIP_DIST (4.0)
#define MAX_CHIP_SPEED (50.0 * sqrt(2*G*MAX_CHIP_DIST/sin(2*CHIP_FIRST_ANGLE)))
#define MIN_CHIP_SPEED (50.0 * sqrt(2*G*MIN_CHIP_DIST/sin(2*CHIP_FIRST_ANGLE)))
#define CHIP_SPEED_UNIT ((MAX_CHIP_SPEED - MIN_CHIP_SPEED) / BLOCK_X_PASS)
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
    float interTime;
    float Vel;
    float dir;
    int playerIndex;
    float deltaTime;
    float Q;
} rType;

typedef struct {
    Point p;
    float score;
} scoreAndPoint;

__device__ float Min(float a, float b) { return a > b ? b : a; }

__device__ float Max(float a, float b) { return a > b ? a : b; }

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

__device__ bool if_finite(float a) {
    return fabs(a) < INITIAL_VALUE;
}

__device__ bool IsInField(Point p, float buffer=1000) {
    return (p.x > -PITCH_LENGTH / 2 + buffer && p.x < PITCH_LENGTH / 2 - buffer
            && p.y < PITCH_WIDTH / 2 - buffer && p.y > -PITCH_WIDTH / 2 + buffer);
}

__device__ bool IsInPenalty(Point p, float buffer=2000) {
    return (p.x < -PITCH_LENGTH/2 + PENALTY_LENGTH + buffer && p.x > -PITCH_LENGTH/2 && p.y > -PENALTY_WIDTH/2 - buffer && p.y < PENALTY_WIDTH/2 + buffer)
            || (p.x > PITCH_LENGTH/2 - PENALTY_LENGTH - buffer && p.x < PITCH_LENGTH/2 && p.y > -PENALTY_WIDTH/2 - buffer && p.y < PENALTY_WIDTH/2 + buffer);
}

__device__ void CUDA_compute_motion_1d(float x0, float v0, float v1, float a_max, float d_max, float v_max, float a_factor, float &traj_time)
{
    float traj_time_acc, traj_time_dec, traj_time_flat;
    traj_time_acc = traj_time_dec = traj_time_flat = 0.0;
    if ((x0 == 0 && v0 == v1) || !if_finite(x0) || !if_finite(v0) || !if_finite(v1)) {
        traj_time = 0;
        return;
    }

    a_max /= a_factor;
    d_max /= a_factor;

    float accel_time_to_v1 = fabs(v1 - v0) / a_max;
    float accel_dist_to_v1 = fabs((v1 + v0) / 2.0) * accel_time_to_v1;
    float decel_time_to_v1 = fabs(v0 - v1) / d_max;
    float decel_dist_to_v1 = fabs((v0 + v1) / 2.0) * decel_time_to_v1;

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

__device__ float CUDA_predictedTime(float x0, float y0, float x1, float y1, float vx, float vy, bool isTheir) {
    float timeX = 0.0;
    float timeY = 0.0;
    float x = x0 - x1;
    float y = y0 - y1;

    float newVelAngle = atan2(vy, vx) - atan2(y, x);
    float velLength = sqrt(vx * vx + vy * vy);
    vx = velLength * cospi(newVelAngle / PI);
    vy = velLength * sinpi(newVelAngle / PI);

    x = sqrt(x * x + y * y);
    y = 0;

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
    CUDA_compute_motion_1d(x, vx, 0.0, maxAcc, maxDec, maxSpeed, 1.5, timeX);
    CUDA_compute_motion_1d(y, vy, 0.0, maxAcc, maxDec, maxSpeed, 1.5, timeY);
    if (timeX < 1e-5 || timeX > 50) timeX = 0;
    if (timeY < 1e-5 || timeY > 50) timeY = 0;
    return (timeX > timeY ? timeX : timeY);
}

__device__ bool CUDA_predictedInterTime(Point mePoint, Point ballPoint, Vector meVel, Vector ballVel, Point* interceptPoint, float* interTime, float responseTime, bool isTheir, float* rollingFraction, float* slidingFraction) {
    float ballSlidAcc = (*slidingFraction) / 2;
    float ballRollAcc = (*rollingFraction) / 2;
    static const float stepTime = 0.02;
    static const float PASS_VEL_DECAY = 5.0 / 7.0;
    static const float FIELD_BUFFER = 300.0;
    static const float PENALTY_BUFFER = 200.0;
    static const float AVOID_DIST = 4 * PLAYER_SIZE;
    // 初始化球速、加速度、球移动的距离、球初始位置
    const float originVel = sqrt(ballVel.x * ballVel.x + ballVel.y * ballVel.y);
    const float maxMoveTime = /*originVel * 2.0 / 7.0 / ballSlidAcc + */originVel * PASS_VEL_DECAY / ballRollAcc;
    float testVel = originVel;
    float ballAcc = ballSlidAcc;
    float testMoveDist = 0;
    float ballMoveTime = 0;
    Point testPoint = ballPoint;

///for debug
//    Vector velDir;
//    velDir.x = ballVel.x / originBallVel;
//    velDir.y = ballVel.y / originBallVel;
////    float meX = 547;
////    float meY = -276;
//    float meX = 564;
//    float meY = 159;

//    float vel = 236;
//    float dir = 4.41;
//    if(!isTheir && mePoint.x < meX + 5 && mePoint.x > meX - 5 && mePoint.y < meY + 5 && mePoint.y > meY - 5 && originBallVel > vel - 1.0 && originBallVel < vel + 1.0  && atan(ballVel.y / ballVel.x) > dir -  PI - 0.1 && atan(ballVel.y / ballVel.x) < dir -  PI + 0.1) {
//        printf("%lf %lf, %lf, %lf, interTime: %f\n", testPoint.x, testPoint.y, originBallVel, atan(ballVel.y / ballVel.x), ballSlidAcc);
//    }

    Vector ballDirec;
//    float slidingDist = 0, slidingTime = 0, slindingVel = 0;
    bool canInter = true, /*isSliding = true,*/ theirCanTouch = false;
    for (ballMoveTime = 0; ballMoveTime < maxMoveTime; ballMoveTime += stepTime ) {
//        if(isSliding) {
//            testVel = originVel - ballAcc * ballMoveTime;//v_0-at
//            if(testVel < 5.0 * originVel / 7.0) {
//                isSliding = false;
//                ballMoveTime = slidingTime = originVel * 2.0 / 7.0 / ballSlidAcc;
//                slidingDist = 12 * originVel * originVel / 49.0 / ballAcc;
//                testVel = slindingVel = originVel * 5.0 / 7.0;
//                ballAcc = ballRollAcc;
//            }
//            testMoveDist = PLAYER_CENTER_TO_BALL_CENTER + (originVel + testVel) * ballMoveTime / 2;
//        }
//        else {
//            testVel = slindingVel - ballAcc * (ballArriveTime - slidingTime);//v_0-at
//            testBallLength = PLAYER_CENTER_TO_BALL_CENTER + (slindingVel + testVel) * (ballArriveTime - slidingTime) / 2 + slidingDist;
//        }
        // 计算球速、球移动的距离
        testVel = originVel * PASS_VEL_DECAY - ballRollAcc * ballMoveTime;
        if(testVel < 0) testVel = 0;
        testMoveDist = (originVel * PASS_VEL_DECAY + testVel) * ballMoveTime / 2;
        // 计算截球点
        ballDirec.x = testMoveDist * ballVel.x / originVel;
        ballDirec.y = testMoveDist * ballVel.y / originVel;
        testPoint.x = ballPoint.x + ballDirec.x;
        testPoint.y = ballPoint.y + ballDirec.y;
        // 对敌方截球点进行特殊处理
        if(isTheir) {
            Vector adjustDir;
            adjustDir.x = mePoint.x - testPoint.x;
            adjustDir.y = mePoint.y - testPoint.y;
            float length = sqrt(adjustDir.x * adjustDir.x + adjustDir.y * adjustDir.y);
            adjustDir.x /= length;
            adjustDir.y /= length;
            testPoint.x += adjustDir.x * AVOID_DIST;
            testPoint.y += adjustDir.y * AVOID_DIST;
//            // 在球线上认为可以截球
//            if(sqrt((mePoint.x - testPoint.x) * (mePoint.x - testPoint.x) + (mePoint.y - testPoint.y) * (mePoint.y - testPoint.y)) < AVOID_DIST) {
//                theirCanTouch = true;
//                break;
//            }
        }
        // 计算截球时间
        float meArriveTime = CUDA_predictedTime(mePoint.x, mePoint.y, testPoint.x, testPoint.y, meVel.x, meVel.y, isTheir);

        if(IsInPenalty(testPoint, PENALTY_BUFFER))
            continue;
        if (!IsInField(testPoint, FIELD_BUFFER)) {
            canInter = false;
            break;
        }
        if(meArriveTime + responseTime < ballMoveTime){
            break;
        }
    }
//    if(testVel < 0)
//        printf("%lf, %lf, %lf, %lf \n", testVel, ballArriveTime, max_time, slidingTime);

//    if(originBallVel > 400 && ballArriveTime >= max_time)
//        printf("testBallLength: %lf  %lf   %lf\n", testBallLength, originBallVel, testBallLength);
    // 无法截球
    if(!canInter || (CAN_NOT_GET_STOP_BALL && ballMoveTime >= maxMoveTime)) {
        interceptPoint->x = INITIAL_VALUE;
        interceptPoint->y = INITIAL_VALUE;
        *interTime = INITIAL_VALUE;
        return false;
    }
    // 能够截球计算截球时间和截球点
    *interceptPoint = testPoint;
    *interTime = CUDA_predictedTime(mePoint.x, mePoint.y, interceptPoint->x, interceptPoint->y, meVel.x, meVel.y, isTheir);
//    if(theirCanTouch ) *interTime = 0.0;
    return true;
}

__device__ bool CUDA_predictedChipInterTime(Point mePoint, Point ballPoint, Vector meVel, Vector ballVel, Point* interceptPoint, float* interTime, float responseTime, bool isTheir, float* rollingFraction) {
    float chipVel = sqrt(ballVel.x * ballVel.x + ballVel.y * ballVel.y);
    float meArriveTime = INITIAL_VALUE;
    float ballAcc = (*rollingFraction) / 2.0;
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

//        if(chipVel > 303 && chipVel < 305 && mePoint.x > 304 && mePoint.y < -240 && atan(ballVel.y / ballVel.x) > 5.74 - 2 * PI && atan(ballVel.y / ballVel.x) < 5.75 - 2 * PI) {
//            printf("%lf, %lf, %lf, %lf\n", moveVel, ballAccSecondJump, 2 * length_2 / time_2, time_2);
//        }
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
    *interTime = max(*interTime, ballDropTime);

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

//    if((*interceptPoint).x > -85 && (*interceptPoint).x < -75 && (*interceptPoint).y < -5 && (*interceptPoint).y > -15)
//        printf("%lf \n", atan(ballVel.y / ballVel.x));

//    float vel = 202;
//    float dir = 3.28885;
//    if(isTheir && ballPoint.x + velDir.x * length_1 < 0 && chipVel > vel - 1.0 && chipVel < vel + 1.0  && atan(ballVel.y / ballVel.x) > dir - 0.001 - PI && atan(ballVel.y / ballVel.x) < dir + 0.001 - PI) {
//        printf("%lf, %lf, (%lf, %lf)\n", *interTime, testBallLength, (*interceptPoint).x, (*interceptPoint).y);
//    }

    return true;
}

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
//    for (int i=0; i < MAX_PLAYER_NUM; i++) {
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
//    for (int i=0; i < MAX_PLAYER_NUM; i++) {
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

__global__ void calculateAllInterInfo(Player* players, Point* ballPos, rType* bestPass, float* rollingFraction, float* slidingFraction) {
    int angleIndex = threadIdx.x;
    int speedIndex = blockIdx.x;
    int playerNum =  blockIdx.y;

    int offset = blockIdx.y + gridDim.y * (threadIdx.x + blockIdx.x * blockDim.x);
    bool isTheir = playerNum < MAX_PLAYER_NUM ? false : true;
    float responseTime = isTheir ? THEIR_RESPONSE_TIME : OUR_RESPONSE_TIME;
    Vector ballVel;
    float interTime;
    Point interPoint;
//    if(playerNum == 2)
//        printf("pos: (%f, %f)\n", players[playerNum].Pos.x, players[playerNum].Pos.y);
//        printf("vel: (%f, %f)\n", players[playerNum].Vel.x, players[playerNum].Vel.y);
//    if(playerNum == 0)
//        printf("valid: %d\n", players[playerNum].isValid);

    ballVel.x = (speedIndex * BALL_SPEED_UNIT + MIN_BALL_SPEED) * cospi( 2.0 * angleIndex / THREAD_NUM_PASS);
    ballVel.y = (speedIndex * BALL_SPEED_UNIT + MIN_BALL_SPEED) * sinpi( 2.0 * angleIndex / THREAD_NUM_PASS);

    interTime = INITIAL_VALUE;
    interPoint.x = INITIAL_VALUE;
    interPoint.y = INITIAL_VALUE;

    if(players[playerNum].isValid)
         CUDA_predictedInterTime(players[playerNum].Pos, *ballPos, players[playerNum].Vel, ballVel, &interPoint, &interTime, responseTime, isTheir, rollingFraction, slidingFraction);
//    if(playerNum == 3 && interPoint.x > 512 && interPoint.y > -167 && interTime < 10)
//        printf("interTime: %f\n", interTime);

    bestPass[offset].interPos = interPoint;
    bestPass[offset].interTime = interTime;
    bestPass[offset].playerIndex = playerNum;
    bestPass[offset].dir = 2.0 * PI * angleIndex / THREAD_NUM_PASS;
    bestPass[offset].Vel = speedIndex * BALL_SPEED_UNIT + MIN_BALL_SPEED;


//    /***************** chip *******************/
    interTime = INITIAL_VALUE;
    interPoint.x = INITIAL_VALUE;
    interPoint.y = INITIAL_VALUE;
    ballVel.x = (speedIndex * CHIP_SPEED_UNIT + MIN_CHIP_SPEED) * cospi(2.0 * angleIndex / THREAD_NUM_PASS);
    ballVel.y = (speedIndex * CHIP_SPEED_UNIT + MIN_CHIP_SPEED) * sinpi(2.0 * angleIndex / THREAD_NUM_PASS);

    if(players[playerNum].isValid)
         CUDA_predictedChipInterTime(players[playerNum].Pos, *ballPos, players[playerNum].Vel, ballVel, &interPoint, &interTime, responseTime, isTheir, rollingFraction);

//        if(bestPass[offset].interPos.x > 299 && bestPass[offset].interPos.x < 300 && bestPass[offset].interPos.y > 248 && bestPass[offset].interPos.y < 249)
//            printf("%d\n", bestPass[offset].playerIndex);


    offset += BLOCK_X_PASS * BLOCK_Y_PASS * THREAD_NUM_PASS;
    bestPass[offset].interPos = interPoint;
    bestPass[offset].interTime = interTime;
    bestPass[offset].playerIndex = playerNum;
    bestPass[offset].dir = 2.0 * PI * angleIndex / THREAD_NUM_PASS;
    bestPass[offset].Vel = speedIndex * CHIP_SPEED_UNIT + MIN_CHIP_SPEED;
    __syncthreads();
}

__global__ void getBest(rType* passPoints) {
    __shared__ rType iP[BLOCK_Y_PASS];
    int blockId = blockIdx.y * gridDim.x + blockIdx.x;
    int playerNum = threadIdx.x;
    rType temp;
    bool even = true;
    int i;
    iP[playerNum] = passPoints[blockId * blockDim.x + playerNum];
    __syncthreads();
    for(i = 0; i < blockDim.x; i++) {
        if(playerNum < blockDim.x - 1 && even && iP[playerNum].interTime > iP[playerNum + 1].interTime) {
            temp = iP[playerNum + 1];
            iP[playerNum + 1] = iP[playerNum];
            iP[playerNum] = temp;
        }
        else if(playerNum > 0 && !even && iP[playerNum].interTime < iP[playerNum - 1].interTime) {
            temp = iP[playerNum];
            iP[playerNum] = iP[playerNum - 1];
            iP[playerNum - 1] = temp;
        }
        even = !even;
        __syncthreads();
    }
    passPoints[blockId * blockDim.x + playerNum] = iP[playerNum];
    __syncthreads();
    /************************/
    iP[playerNum] = passPoints[blockId * blockDim.x + playerNum + BLOCK_X_PASS * BLOCK_Y_PASS * THREAD_NUM_PASS];
    __syncthreads();
    even = true;
    for(i = 0; i < blockDim.x; i++) {
        if(playerNum < blockDim.x - 1 && even && iP[playerNum].interTime > iP[playerNum + 1].interTime) {
            temp = iP[playerNum + 1];
            iP[playerNum + 1] = iP[playerNum];
            iP[playerNum] = temp;
        }
        else if(playerNum > 0 && !even && iP[playerNum].interTime < iP[playerNum - 1].interTime) {
            temp = iP[playerNum];
            iP[playerNum] = iP[playerNum - 1];
            iP[playerNum - 1] = temp;
        }
        even = !even;
        __syncthreads();
    }

//    float interPointX = 370;
//    float interPointY = -108;

//    float meX = 444;
//    float meY = -105;

//    float vel = 428;
//    float dir = 4.07;
//    if(iP[playerNum].playerIndex == 1 && iP[playerNum].interPos.x < interPointX + 1.0 && iP[playerNum].interPos.x > interPointX - 1.0 && iP[playerNum].interPos.y < interPointY + 1.0 && iP[playerNum].interPos.y > interPointY - 1.0) {
//        printf("this: %f\n fast: %f\n fastIdx: %d \n\n", iP[playerNum].interTime, iP[playerNum - 2].interTime, iP[playerNum - 2].playerIndex);
//    }

    passPoints[blockId * blockDim.x + playerNum + BLOCK_X_PASS * BLOCK_Y_PASS * THREAD_NUM_PASS] = iP[playerNum];
    __syncthreads();

}

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
//        allScore[allScoreIndex].score = CUDA_evaluateFunc(allScore[allScoreIndex].p, *ballPos, Players, Players[MAX_PLAYER_NUM]);
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

extern "C" void BestPass(Player* players, Point* ball, rType* result, float* rollingFraction, float* slidingFraction) {
    rType *bestPass;

    cudaMallocManaged((void**)&bestPass, 2 * BLOCK_X_PASS * BLOCK_Y_PASS * THREAD_NUM_PASS * sizeof(rType));

//    cudaEvent_t start, stop;
//    cudaEventCreate(&start);
//    cudaEventCreate(&stop);
//    cudaEventRecord(start);

    dim3 bolcks(BLOCK_X_PASS, BLOCK_Y_PASS);
    calculateAllInterInfo <<< bolcks, THREAD_NUM_PASS >>> (players, ball, bestPass, rollingFraction, slidingFraction);
    cudaDeviceSynchronize();
    dim3 blocks2(BLOCK_X_PASS, THREAD_NUM_PASS);
    getBest<<< blocks2, BLOCK_Y_PASS >>> (bestPass);
    cudaDeviceSynchronize();
    cudaError_t cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess){
        printf("CUDA ERROR: %d\n", (int)cudaStatus);
        printf("Error Name: %s\n", cudaGetErrorName(cudaStatus));
        printf("Description: %s\n", cudaGetErrorString(cudaStatus));
    }

//    cudaEventRecord(stop);
//    cudaEventSynchronize(stop);

//    float milliseconds = 0;
//    cudaEventElapsedTime(&milliseconds, start, stop);
//    printf("Time: %.5f ms\n", milliseconds);

    rType defaultPlayer;
    defaultPlayer.dir = INITIAL_VALUE;
    defaultPlayer.interPos.x = INITIAL_VALUE;
    defaultPlayer.interPos.y = INITIAL_VALUE;
    defaultPlayer.interTime = INITIAL_VALUE;
    defaultPlayer.Vel = INITIAL_VALUE;
    defaultPlayer.deltaTime = -INITIAL_VALUE;
    defaultPlayer.playerIndex = INITIAL_VALUE;
    for(int i = 0; i < BLOCK_X_PASS * BLOCK_Y_PASS * THREAD_NUM_PASS; i += BLOCK_Y_PASS) {
        int playerNum = 0;
        for(int j = 0; j < MAX_PLAYER_NUM; j++) {
            if(bestPass[i + j].playerIndex >= MAX_PLAYER_NUM ) {
                while(playerNum < MAX_PLAYER_NUM) {
                    result[i / 2 + playerNum] = defaultPlayer;
                    playerNum++;
                }
                for(int k = 0; k < j; k++) {
                    result[i / 2 + k].deltaTime = bestPass[i + j].interTime - result[i / 2 + k].interTime;
                    if(result[i / 2 + k].deltaTime < MIN_DELTA_TIME)
                        result[i / 2 + k] = defaultPlayer;
                }
                break;
            }
            else {
                result[i / 2 + playerNum] = bestPass[i + j];
                playerNum++;
            }
        }
    }


    for(int i = BLOCK_X_PASS * BLOCK_Y_PASS * THREAD_NUM_PASS; i < 2 * BLOCK_X_PASS * BLOCK_Y_PASS * THREAD_NUM_PASS; i += BLOCK_Y_PASS) {
        int playerNum = 0;
        for(int j = 0; j < MAX_PLAYER_NUM; j++) {
//            float interPointX = 370;
//            float interPointY = -108;

//            float meX = 444;
//            float meY = -105;

//            float vel = 428;
//            float dir = 4.07;
//            if(bestPass[i + j].playerIndex == 1 && bestPass[i + j].interPos.x < interPointX + 1.0 && bestPass[i + j].interPos.x > interPointX - 1.0 && bestPass[i + j].interPos.y < interPointY + 1.0 && bestPass[i + j].interPos.y > interPointY - 1.0) {
//                printf("this: %f\n fast: %f\n fastIdx: %d \n\n", bestPass[i + j].interTime, bestPass[i + 1].interTime, bestPass[i + 1].playerIndex);
//            }

            if(bestPass[i + j].playerIndex >= MAX_PLAYER_NUM) {
                while(playerNum < MAX_PLAYER_NUM) {
                    result[i / 2 + playerNum] = defaultPlayer;
                    playerNum++;
                }
                for(int k = 0; k < j; k++) {
                    result[i / 2 + k].deltaTime = bestPass[i + j].interTime - result[i / 2 + k].interTime;

                    if(result[i / 2 + k].deltaTime < MIN_DELTA_TIME)
                        result[i / 2 + k] = defaultPlayer;
                }
                break;
            }
            else {
                result[i / 2 + playerNum] = bestPass[i + j];
                playerNum++;
            }
        }
    }
    cudaFree(bestPass);
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
