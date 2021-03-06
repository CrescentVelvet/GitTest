#include "collisiondetect.h"
#include "messageformat.h"

namespace  {
const int DETECT_DIST = 10; //detect whether the ball in 300mm of vechile
const int SPLIT_THRESHOLD = 20;  //split
const int NEAR_VECHILE_MIN_FRAME = 5; //keep frame to detect ball SLIGHTLY touch vechile
const int LENGTH_THERESHOLD = 5800;
const int WIDTH_THERESHOLD = 4400;
const int MAX_FRAME_NUM = 20;    //split and merge frame num
int LinePoint[MAX_FRAME_NUM];
}
CCollisionDetect::CCollisionDetect() {

}
bool CCollisionDetect::ballIsOnEdge(CGeoPoint ball) {
    if(ball.x() < -LENGTH_THERESHOLD || ball.x() > LENGTH_THERESHOLD ||
            ball.y() < -WIDTH_THERESHOLD || ball.y() > WIDTH_THERESHOLD) return true;
    return false;
}

bool CCollisionDetect::ballCloseEnough2Analyze() {
    for (int color = 0; color < PARAM::TEAMS; color ++) {
        for (int i = 0; i < PARAM::ROBOTNUM; i++) {
            auto& vision = GlobalData::instance()->maintain[0];
            if (vision.robot[color][i].valid && vision.ball[0].pos.dist(vision.robot[color][i].pos) < DETECT_DIST) {
                return true;
            }
        }
    }
    return false;
}

void CCollisionDetect::analyzeData(ReceiveVisionMessage& result) {
    //first test near ball
    int OurTouchNum = -1, TheirTouchNum = -1;
    for (int roboId = 0; roboId < PARAM::ROBOTNUM; roboId++) {
        bool foundBlue = true, foundYellow = true;
        for (int frame = 0; frame > -NEAR_VECHILE_MIN_FRAME; frame--) {
            bool foundBlueCar = false, foundYellowCar = false;
            Ball ball = GlobalData::instance()->maintain[frame].ball[0];
//            for (int i = 0; i < GlobalData::instance()->maintain[frame].robotSize[PARAM::BLUE]; i++)
              if (GlobalData::instance()->maintain[frame].robot[PARAM::BLUE][roboId].valid
//                        GlobalData::instance()->maintain[frame].robot[PARAM::BLUE][i].id == roboId      //change by lzx
                        && GlobalData::instance()->maintain[frame].robot[PARAM::BLUE][roboId].pos.dist(ball.pos) >= 85
                        && GlobalData::instance()->maintain[frame].robot[PARAM::BLUE][roboId].pos.dist(ball.pos) <= 125)
                    foundBlueCar = true;
//            for (int i = 0; i < GlobalData::instance()->maintain[frame].robotSize[PARAM::YELLOW]; i++)
            // for (int i = 0; i <  PARAM::ROBOTNUM; i++)
              if (GlobalData::instance()->maintain[frame].robot[PARAM::YELLOW][roboId].valid
                   //     GlobalData::instance()->maintain[frame].robot[PARAM::YELLOW][i].id == roboId
                        && GlobalData::instance()->maintain[frame].robot[PARAM::YELLOW][roboId].pos.dist(ball.pos) >= 85
                        && GlobalData::instance()->maintain[frame].robot[PARAM::YELLOW][roboId].pos.dist(ball.pos) <= 125)
                    foundYellowCar = true;
            if (!foundBlueCar) foundBlue = false;
            if (!foundYellowCar) foundYellow = false;
        }
        if (foundBlue) {
            OurTouchNum = roboId;
        }
        if (foundYellow) {
            TheirTouchNum = roboId;
        }
    }

    if (OurTouchNum != -1 && TheirTouchNum == -1) {
        result.ball[0].ball_state_machine.ballState = _touched;
        GlobalData::instance()->lastTouch = PARAM::BLUE * PARAM::ROBOTNUM + OurTouchNum;
        return;
        //std::cout<<"the ball touch OUR robot:num:"<<OurTouchNum<<" id:"<<GlobalData::instance()->maintain[LinePoint[j]].robot[PARAM::BLUE][OurTouchNum].id<<std::endl;
    } else if (TheirTouchNum != -1 && OurTouchNum == -1) {
        result.ball[0].ball_state_machine.ballState = _touched;
        GlobalData::instance()->lastTouch = PARAM::YELLOW * PARAM::ROBOTNUM + TheirTouchNum;
        //std::cout<<"the ball touch THEIR robot:num:"<<TheirTouchNum<<" id:"<<GlobalData::instance()->maintain[LinePoint[j]].robot[PARAM::YELLOW][TheirTouchNum].id<<std::endl;
        return;
    } else if (OurTouchNum != -1 && TheirTouchNum != -1) {
        //争球
        result.ball[0].ball_state_machine.ballState = _struggle;
        GlobalData::instance()->lastTouch = -1;
        //std::cout << "CANNOT decide the ball between OUR player no." << OurTouchNum << " and THEIR player No." << TheirTouchNum << std::endl;
        return;
    }

    //use split and merge algorithm
    LinePoint[1] = -MAX_FRAME_NUM + 1;
    LinePoint[2] = 0;
    PointN = 2;
    while (true) {
        int pi = 1, maxi = 1, i = -MAX_FRAME_NUM + 1;
        double maxDis = -1;
        while (pi < PointN) {
            const auto &p1 = GlobalData::instance()->maintain[LinePoint[pi]];
            const auto &p2 = GlobalData::instance()->maintain[LinePoint[pi + 1]];
            const auto& q = GlobalData::instance()->maintain[i];
            CGeoPoint p1BallPos(p1.ball[0].pos.x(), p1.ball[0].pos.y());
            CGeoPoint p2BallPos(p2.ball[0].pos.x(), p2.ball[0].pos.y());
            CGeoPoint qBallPos(q.ball[0].pos.x(), q.ball[0].pos.y());
            CGeoSegment PassLine(p1BallPos, p2BallPos);
            double d = qBallPos.dist(PassLine.projection(qBallPos));
            //------------------IMPOTANT------------
            //CGeoSegment PassLine(p1.BallPos, p2.BallPos);
            //d = q.BallPos.dist(PassLine.projection(q.BallPos));
            //std::cout << "ball" << i << " @" << qBallPos<< "\t with d:" << d << std::endl;
            if (d > maxDis && d > SPLIT_THRESHOLD && i != LinePoint[pi] && i != LinePoint[pi + 1]) {
                maxi = i;
                maxDis = d;
            }
            if (i == LinePoint[pi + 1]) pi++;
            i++;
        }
        //std::cout << "-----------------\n";
        if (maxi == 1) break;
        for (i = PointN; i > 0; i--) {
            if (LinePoint[i] < maxi) break;
            LinePoint[i + 1] = LinePoint[i];
        }
        PointN++;
        LinePoint[i + 1] = maxi;
    }
    if (PointN > 2) {
        int OurTouchNum = -1, TheirTouchNum = -1, j = PointN - 1;
        double OurTouchDis = 200.0, TheirTouchDis = 200.0;
        //std::cout << "found PointN=" << PointN << "\tball pos" << GlobalData::instance()->maintain[LinePoint[j]].ball[0].pos.x << std::endl;
        CGeoPoint ballpos(GlobalData::instance()->maintain[LinePoint[j]].ball[0].pos);
//        for (int i = 0; i < GlobalData::instance()->maintain[LinePoint[j]].robotSize[PARAM::BLUE]; i++) {
          for(int i = 0; i < PARAM::ROBOTNUM; i++) {
              if(!GlobalData::instance()->maintain[LinePoint[j]].robot[PARAM::BLUE][i].valid) {
                  continue;
              }//change by lzx
            CGeoPoint tempos(GlobalData::instance()->maintain[LinePoint[j]].robot[PARAM::BLUE][i].pos);
            if (ballpos.dist(tempos) < OurTouchDis) {
                OurTouchDis = ballpos.dist(tempos);
                OurTouchNum = i;
            }
        }
//        for (int i = 0; i < GlobalData::instance()->maintain[LinePoint[j]].robotSize[PARAM::YELLOW]; i++) {
          for(int i = 0; i < PARAM::ROBOTNUM; i++) {
              if(!GlobalData::instance()->maintain[LinePoint[j]].robot[PARAM::YELLOW][i].valid) {
                  continue;
              }//change by lzx
            CGeoPoint tempos(GlobalData::instance()->maintain[LinePoint[j]].robot[PARAM::YELLOW][i].pos);
            if (ballpos.dist(tempos) < TheirTouchDis) {
                TheirTouchDis = ballpos.dist(tempos);
                TheirTouchNum = i;
            }
        }
        if (OurTouchNum != -1 && ((TheirTouchNum != -1 && OurTouchDis <= TheirTouchDis - 5) || TheirTouchNum == -1)) {
            result.ball[0].ball_state_machine.ballState = _touched;
            //LastTouch = 1;
            GlobalData::instance()->lastTouch = PARAM::BLUE * PARAM::ROBOTNUM + OurTouchNum; //change by lzx //+GlobalData::instance()->maintain[LinePoint[j]].robot[PARAM::BLUE][OurTouchNum].id;
            //std::cout<<"the ball touch OUR robot:num:"<<OurTouchNum<<" id:"<<GlobalData::instance()->maintain[LinePoint[j]].robot[PARAM::BLUE][OurTouchNum].id<<std::endl;
        } else if (TheirTouchNum != -1 && ((OurTouchNum != -1 && TheirTouchDis <= OurTouchDis - 5) || OurTouchNum == -1)) {
            result.ball[0].ball_state_machine.ballState = _touched;
            //LastTouch = 2;
            GlobalData::instance()->lastTouch = PARAM::YELLOW * PARAM::ROBOTNUM + TheirTouchNum;//change by lzx //+GlobalData::instance()->maintain[LinePoint[j]].robot[PARAM::YELLOW][TheirTouchNum].id;
            //std::cout<<"the ball touch THEIR robot:num:"<<TheirTouchNum<<" id:"<<GlobalData::instance()->maintain[LinePoint[j]].robot[PARAM::YELLOW][TheirTouchNum].id<<std::endl;
        } else if (OurTouchNum != -1 && TheirTouchNum != -1 && abs(OurTouchDis - TheirTouchDis) <= 5) {
            //争球
            result.ball[0].ball_state_machine.ballState = _struggle;
            //LastTouch = 0;
            GlobalData::instance()->lastTouch = -1;
            //std::cout << "CANNOT decide the ball between OUR player no." << OurTouchNum << " and THEIR player No." << TheirTouchNum << std::endl;
        } else if (ballIsOnEdge(GlobalData::instance()->maintain[LinePoint[j]].ball[0].pos)) {
            //LastTouch = 3;
            GlobalData::instance()->lastTouch = -1;
            //std::cout << "the ball touch WALL" << std::endl;
        }
    }
}

void  CCollisionDetect::visionAlart() {


}
