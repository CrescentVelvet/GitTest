#include <stdio.h>
#include "analy_log.h"
#include "referee.pb.h"
#include "messages_robocup_ssl_wrapper.pb.h"
#include "messages_robocup_ssl_detection.pb.h"

//修改cpp文件无需编译，直接运行

AnalyEsthetics::AnalyEsthetics(){
    current_frame = 0;
    whole_frame = 0;
    e_ball.setX(-99999);
    e_ball.setY(-99999);
    //初始化小车位置
    for(int i = 0; i < PARAM::ROBOTNUM; i++){
        blue_robot[i].setX(-99999);
        blue_robot[i].setY(-99999);
        yellow_robot[i].setX(-99999);
        yellow_robot[i].setY(-99999);
    }
    //初始化分析参数
    teamname = nullptr;
    analy_status = ANALY::Halt;
    analy_holdMsg = ANALY::Null;
    analy_blueside = ANALY::LEFT;
    analy_yellowside = ANALY::RIGHT;
    //初始化车数
    for(int i = 0; i < 6; i++){
        our_carnum[i] = 0;
        their_carnum[i] = 0;
    }
    //初始化帧数
    for(int i = 0; i < 3; i++){
        real_ourframe[i] = 0;
        real_theirframe[i] = 0;
    }
}

// 分析log主函数
void AnalyEsthetics::analy_frame(void * ptr, int size){
    static SSL_WrapperPacket packet;
//    static int k;
    packet.ParseFromArray(ptr, size);
    if(packet.has_detection()){
        const SSL_DetectionFrame& detection = packet.detection();
        int ballSize = detection.balls_size();
        int blueSize = detection.robots_blue_size();
        int yellowSize = detection.robots_yellow_size();
        // 读取球坐标，若球数不为0，取读到的第一个球为球坐标
        if(ballSize != 0){
            const SSL_DetectionBall& ball = detection.balls(0);
            e_ball.setX(ball.x());
            e_ball.setY(ball.y());
        }
        else{
        }
        // 读取蓝车坐标
        for(int i = 0; i < blueSize; i++){
            const SSL_DetectionRobot& robot = detection.robots_blue(i);
            blue_robot[i].setX(robot.x());
            blue_robot[i].setY(robot.y());
        }
        // 读取黄车坐标
        for(int i = 0; i < yellowSize; i++){
            const SSL_DetectionRobot& robot = detection.robots_yellow(i);
            yellow_robot[i].setX(robot.x());
            yellow_robot[i].setY(robot.y());
        }
        // 判断蓝车左边还是右边
        judgeSide(blue_robot, blueSize);
        // 计算持球信息（即哪一方持球）
        cal_holdMsg();
        // 计算我方敌方帧数车数等详细信息
        if(analy_status != ANALY::Halt || analy_status != ANALY::Stop){
            cal_extrovert();
        }
//        k++;
//        if(k%10000==0){qDebug()<<k;}
    }
    else{
    }
    // 读取到倒数第二帧时结束
    if((current_frame+1) == whole_frame){
        qDebug() << "";
        // 帧数为0时要赋值1，车数均值等于车数除以己方帧数
        for(int i = 0; i < 6; i++) {
            if(real_ourframe[i/2] == 0){real_ourframe[i/2]=1;}
            if(real_theirframe[i/2] == 0){real_theirframe[i/2]=1;}
            our_carnum[i] = our_carnum[i] / real_ourframe[i/2];
            their_carnum[i] = their_carnum[i] / real_theirframe[i/2];
        }
        //将队伍进攻性参数输出为log_data.txt文本
        FILE *fp = fopen("../../log_data000.txt","a");
        // 检查文件是否打开
        if( (fp = fopen("../../log_data000.txt","a")) == NULL){
            printf("Fail to open file!\n");
            exit(0);
        }
        // fprintf(fp,"%f\t",our_carnum[0]);
        // fprintf(fp,"%s\n",teamname);
        // fprintf(fp,"%f\n",our_carnum[2]);
        // fprintf(fp,"%f\n",their_carnum[0]);
        // fprintf(fp,"%f\n",their_carnum[2]);
        fprintf(fp,"hahaha");
        fclose(fp);
        //将GNN训练所需数据输出为gnn_data.txt文本

//        qDebug() << "x1 =" << our_carnum[0];
//        qDebug() << "y1 =" << our_carnum[2];
//        qDebug() << "x2 =" << their_carnum[0];
//        qDebug() << "y2 =" << their_carnum[2];
//        qDebug() << "our attack in ourball    =" << our_carnum[0] << "| their attack in theirball  =" << their_carnum[0];
//        qDebug() << "our defense in ourball   =" << our_carnum[1] << "| their defense in theirball =" << their_carnum[1];
//        qDebug() << "our attack in theirball  =" << our_carnum[2] << "| their attack in ourball    =" << their_carnum[2];
//        qDebug() << "our defense in theirball =" << our_carnum[3] << "| their defense in ourball   =" << their_carnum[3];
//        qDebug() << "our attack in bothball   =" << our_carnum[4] << "| their attack in bothball   =" << their_carnum[4];
//        qDebug() << "our defense in bothball  =" << our_carnum[5] << "| their defense in bothball  =" << their_carnum[5];
        qDebug() << "~~~analy~finish~~~";
    }
}

// 读取帧数
void AnalyEsthetics::readFrame(int frac, int fraw, QString name){
    current_frame = frac;
    whole_frame = fraw;
    char *ptrname, *retptrname;
    QByteArray arrayname = name.toLatin1();
    ptrname=arrayname.data();
    while((retptrname=strtok(ptrname,"/"))!=nullptr){
        teamname = retptrname;
        ptrname=nullptr;
    }
}

// 读取裁判盒信息
void AnalyEsthetics::readReferee(void * ptr, int size){
    static SSL_Referee referee;
    referee.ParseFromArray(ptr, size);
//    const auto& stage = referee.stage();
    const auto& command = referee.command();
//    const auto& command_counter = referee.command_counter();
//    const auto& yellow = referee.yellow();
//    const auto& blue = referee.blue();
//    int analy_status_old = analy_status;
    switch(command){
    case 0: analy_status = ANALY::Halt; break;
    case 1: analy_status = ANALY::Stop; break;
    case 2: analy_status = ANALY::Ready; break;
    case 3: analy_status = ANALY::Start; break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17: break;
    default: qDebug() << "error in refereebox : command" << command;
        analy_status = ANALY::Halt; break;
    }
//    if(analy_status_old != analy_status){
//        qDebug() << "";
//        qDebug() << "cmd =" << analy_status;
//        qDebug() << command_counter;
//    }
}

// 判断点在球场内
bool AnalyEsthetics::atPresent(CGeoPoint point){
    return (point.x() >= minX && point.x() <= maxX && point.y() >= minY && point.y() <= maxY);
}

// 极坐标转换直角坐标
CVector AnalyEsthetics::Polar2Vector(double m,double angle){
    return CVector(m*std::cos(angle), m*std::sin(angle));
}

// 分析
CGeoPoint AnalyEsthetics::analy_InterPoint(CGeoPoint enemy, CGeoPoint ball){
    double FRICTION = 80.0;//simmodule = 152
    static const double ballAcc = FRICTION / 2;//球减速度
    constexpr double carAcc = 400;//车加速度
    constexpr double maxBallSpeed = 650;//球最大速度
    double dist = (enemy - ball).mod();//敌人到球的距离
    double delta = maxBallSpeed * maxBallSpeed + 2 * (carAcc - ballAcc) * dist;
    double t = (std::sqrt(delta) - maxBallSpeed) / (carAcc - ballAcc);
    double enemy2point = 0.5 * carAcc * t * t;
    return enemy + Polar2Vector(enemy2point, (ball - enemy).dir());
}

// 判断黄车与蓝车在左右哪边
void AnalyEsthetics::judgeSide(CGeoPoint blue_car[], int blue_side){
    for(int i = 0; i < blue_side; i++){
        if(blue_car[i].x() < -ANALY::FIELD_WIDTH + ANALY::PENALTY_WIDTH
                && std::fabs(blue_car[i].y()) < ANALY::PENALTY_LENGTH / 2){
            analy_blueside = ANALY::LEFT;
            analy_yellowside = ANALY::RIGHT;
            qDebug() << 0;
            break;
        }
        else if(blue_car[i].x() > ANALY::FIELD_WIDTH - ANALY::PENALTY_WIDTH
                && std::fabs(blue_car[i].y()) < ANALY::PENALTY_LENGTH / 2){
            analy_blueside = ANALY::RIGHT;
            analy_yellowside = ANALY::LEFT;
            qDebug() << 1;
            break;
        }
    }
//    qDebug() << "error in judgeSide";
}

// 确定我方
bool AnalyEsthetics::analy_marking(int team, int num){
    int side = ANALY::LEFT;
    int obside = ANALY::YELLOW;
    CGeoPoint ball = e_ball;
    CGeoPoint me[PARAM::ROBOTNUM];
    CGeoPoint enemy[PARAM::ROBOTNUM];
    if(team == ANALY::BLUE){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = blue_robot[i];
            enemy[i] = yellow_robot[i];
        }
        side = analy_blueside;
        obside = analy_yellowside;
    }
    else if(team == ANALY::YELLOW){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = yellow_robot[i];
            enemy[i] = blue_robot[i];
        }
        side = analy_yellowside;
        obside = analy_blueside;
    }
    else{
        qDebug() << "error in analy_marking : initialize";
        return false;
    }
    if(me[num].x() > -99999 && me[num].y() > -99999
            && enemy[0].x() > -99999 && enemy[0].y() > -99999){
        for(int i = 0; enemy[i].x() < -99999; i++){
            CGeoPoint pass_point = analy_InterPoint(enemy[i], ball);
            double distance_me = me[num].dist(pass_point);
            double distance_enemy = enemy[i].dist(pass_point);
            if(side == ANALY::RIGHT && distance_me < distance_enemy){
                if(enemy[i].x() > 0) { return true; }
            }
            else if(side == ANALY::LEFT && distance_me < distance_enemy){
                if(enemy[i].x() < 0) { return true; }
            }
        }
    }
    else{
        qDebug() << "error in analy_marking : analysis";
        return false;
    }
    return false;
}

// 确定我方防守车
bool AnalyEsthetics::analy_guarding(int team, int num){
    int side = ANALY::LEFT;
    CGeoPoint me[PARAM::ROBOTNUM];
    if(team == ANALY::BLUE){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = blue_robot[i];
        }
        side = analy_blueside;
    }
    else if(team == ANALY::YELLOW){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = yellow_robot[i];
        }
        side = analy_yellowside;
    }
    else{
        qDebug() << "error in analy_guarding : initialize";
        return false;
    }
    if(me[num].x() > -99999 && me[num].y() > -99999){
        if(side == ANALY::LEFT && me[num].x() < ANALY::FIELD_WIDTH/4){
            return true;
        }
        else if(side == ANALY::RIGHT && me[num].x() > ANALY::FIELD_WIDTH/4){
            return true;
        }
    }
    else{
        qDebug() << "error in analy_guarding : analysis";
        return false;
    }
    return false;
}

// 确定我方持球车
bool AnalyEsthetics::analy_goalkeeping(int team, int num){
    int side = ANALY::LEFT;
    CGeoPoint me[PARAM::ROBOTNUM];
    if(team == ANALY::BLUE){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = blue_robot[i];
        }
        side = analy_blueside;
    }
    else if(team == ANALY::YELLOW){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = yellow_robot[i];
        }
        side = analy_yellowside;
    }
    else{
        qDebug() << "error in analy_goalkeeping : initialize";
        return false;
    }
    if(me[num].x() > -99999 && me[num].y() > -99999){
        if(side == ANALY::LEFT
                && me[num].x() < -ANALY::FIELD_WIDTH/2 + ANALY::PENALTY_WIDTH
                && std::fabs(me[num].y()) < ANALY::PENALTY_LENGTH/2){
            return true;
        }
        else if(side == ANALY::RIGHT
                && me[num].x() > ANALY::FIELD_WIDTH/2 - ANALY::PENALTY_WIDTH
                && std::fabs(me[num].y()) < ANALY::PENALTY_LENGTH/2){
            return true;
        }
    }
    else{
        qDebug() << "error in analy_goalkeeping : analysis";
        return false;
    }
    return false;
}

// 计算
int AnalyEsthetics::cal_nearest(int team){
    int side = ANALY::LEFT;
    CGeoPoint ball = e_ball;
    CGeoPoint me[PARAM::ROBOTNUM];
    if(team == ANALY::BLUE){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = blue_robot[i];
        }
        side = analy_blueside;
    }
    else if(team == ANALY::YELLOW){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = yellow_robot[i];
        }
        side = analy_yellowside;
    }
    else{
        qDebug() << "error in cal_nearest : initialize";
        return -1;
    }
    int nearest_num = 0;
    double nearest_dis = me[nearest_num].dist(ball);
    double temp_nearest_dis;
    if(me[0].x() > -99999 && me[0].y() > -99999){
        for(int i = 0; me[i].x() < -99999; i++){
            temp_nearest_dis = me[i].dist(ball);
            if(temp_nearest_dis < nearest_dis){
                nearest_num = i;
                nearest_dis = temp_nearest_dis;
            }
        }
    }
    else{
        qDebug() << "error in cal_nearest : analysis";
    }
    return nearest_num;
}

// 计算当前持球状态
void AnalyEsthetics::cal_holdMsg(){
    int side = ANALY::LEFT;
    int obside = ANALY::YELLOW;
    int team = ANALY::BLUE;
    int opteam = ANALY::YELLOW;
    CGeoPoint ball = e_ball;
    CGeoPoint me[PARAM::ROBOTNUM];
    CGeoPoint enemy[PARAM::ROBOTNUM];
    if(team == ANALY::BLUE){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = blue_robot[i];
            enemy[i] = yellow_robot[i];
        }
        side = analy_blueside;
        obside = analy_yellowside;
    }
    else if(team == ANALY::YELLOW){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = yellow_robot[i];
            enemy[i] = blue_robot[i];
        }
        side = analy_yellowside;
        obside = analy_blueside;
    }
    else{
        qDebug() << "error in cal_holdMsg : initialize";
    }
    if(me[0].x() > -99999 && me[0].y() > -99999
            && enemy[0].x() > -99999 && enemy[0].y() > -99999){
        double our_nearest = me[cal_nearest(team)].dist(ball);
        double their_nearest = enemy[cal_nearest(opteam)].dist(ball);
        if(our_nearest < 150 && their_nearest > 200){
            analy_holdMsg = ANALY::OurHolding;
//            qDebug() << "OurHolding";
        }
        else if(our_nearest > 200 && their_nearest < 150){
            analy_holdMsg = ANALY::TheirHolding;
//            qDebug() << "TheirHolding";
        }
        else if(our_nearest < 150 || their_nearest < 150){
            analy_holdMsg = ANALY::BothHolding;
//            qDebug() << "BothHolding";
        }
        else{
            if(our_nearest - their_nearest < -200){
                analy_holdMsg = ANALY::Our;
//                qDebug() << "Our";
            }
            else if(our_nearest - their_nearest > 200){
                analy_holdMsg = ANALY::Their;
//                qDebug() << "Their";
            }
            else{
                analy_holdMsg = ANALY::Both;
//                qDebug() << "Both";
            }
        }
    }
    else{
        analy_holdMsg = ANALY::Null;
        qDebug() << "error in cal_holdMsg : analysis";
    }
}

// 计算
void AnalyEsthetics::cal_extrovert(){
    int side = ANALY::LEFT;
    int obside = ANALY::RIGHT;
    int team = ANALY::BLUE;
    int obteam = ANALY::YELLOW;
    CGeoPoint ball = e_ball;
    CGeoPoint me[PARAM::ROBOTNUM];
    CGeoPoint enemy[PARAM::ROBOTNUM];
    // 若为蓝队，则将蓝车设置为我车，黄车设置为敌车
    if(team == ANALY::BLUE){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = blue_robot[i];
            enemy[i] = yellow_robot[i];
        }
        side = analy_blueside;
        obside = analy_yellowside;
    }
    // 若为黄队，则将黄车设置为我车，蓝车设置为敌车
    else if(team == ANALY::YELLOW){
        for(int i = 0; i < PARAM::ROBOTNUM; i++){
            me[i] = yellow_robot[i];
            enemy[i] = blue_robot[i];
        }
        side = analy_yellowside;
        obside = analy_blueside;
    }
    else{
        qDebug() << "error in cal_extrovert : initialize";
    }
    if(me[0].x() > -99999 && me[0].y() > -99999
            && enemy[0].x() > -99999 && enemy[0].y() > -99999){
        for(int i = 0; me[i].x() > -99999 && me[i].y() > -99999; i++){
            // 如果我方持球，增加我方帧数[0]
            if(analy_holdMsg == ANALY::OurHolding){
                real_ourframe[0]++;
                // 如果我方一车到球的距离小于持球距离，判断为持球，增加我方车数[0]
                if(me[i].dist(ball) < ANALY::HAS_BALL){
                    our_carnum[0]++;
                }
                // 如果我方一车为盯人或后卫或守门，增加我方车数[1]
                else if(analy_marking(team, i)
                        || analy_guarding(team, i)
                        || analy_goalkeeping(team, i)){
                    our_carnum[1]++;
                }
                // 其余我方一车，增加我方车数[0]
                else{
                    our_carnum[0]++;
                }
            }
            // 如果敌方持球，增加我方帧数[1]
            else if(analy_holdMsg == ANALY::TheirHolding){
                real_ourframe[1]++;
                // 如果我方一车为盯人或后卫或守门，增加我方车数[3]
                if(analy_marking(team, i)
                        || analy_guarding(team, i)
                        || analy_goalkeeping(team, i)){
                    our_carnum[3]++;
                }
                // 其余我方一车，增加我方车数[2]
                else{
                    our_carnum[2]++;
                }
            }
            // 如果我方持球或敌方持球或双方持球，增加我方帧数[2]
            else if(analy_holdMsg == ANALY::Our
                    || analy_holdMsg == ANALY::Their
                    || analy_holdMsg == ANALY::Both
                    || analy_holdMsg == ANALY::BothHolding){
                real_ourframe[2]++;
                // 如果我方一车为盯人或后卫或守门，增加我方车数[5]
                if(analy_marking(team, i)
                        || analy_guarding(team, i)
                        || analy_goalkeeping(team, i)){
                    our_carnum[5]++;
                }
                // 其余我方一车，增加我方车数[4]
                else{
                    our_carnum[4]++;
                }
            }
            else{
            }
        }
        for(int i = 0; enemy[i].x() > -99999 && enemy[i].y() > -99999; i++){
            if(analy_holdMsg == ANALY::TheirHolding){
                real_theirframe[0]++;
                if(enemy[i].dist(ball) < ANALY::HAS_BALL){
                    their_carnum[0]++;
                }
                else if(analy_marking(obteam, i)
                        || analy_guarding(obteam, i)
                        || analy_goalkeeping(obteam, i)){
                    their_carnum[1]++;
                }
                else{
                    their_carnum[0]++;
                }
            }
            else if(analy_holdMsg == ANALY::OurHolding){
                real_theirframe[1]++;
                if(analy_marking(obteam, i)
                        || analy_guarding(obteam, i)
                        || analy_goalkeeping(obteam, i)){
                    their_carnum[3]++;
                }
                else{
                    their_carnum[2]++;
                }
            }
            else if(analy_holdMsg == ANALY::Our
                    || analy_holdMsg == ANALY::Their
                    || analy_holdMsg == ANALY::Both
                    || analy_holdMsg == ANALY::BothHolding){
                real_theirframe[2]++;
                if(analy_marking(obteam, i)
                        || analy_guarding(obteam, i)
                        || analy_goalkeeping(obteam, i)){
                    their_carnum[5]++;
                }
                else{
                    their_carnum[4]++;
                }
            }
            else{
            }
        }
    }
    else{
        qDebug() << "error in cal_extrovert : analysis";
    }
}
