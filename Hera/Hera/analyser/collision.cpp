#include "collision.h"

collision::collision()
{

}

//void collision::fake_main(vmData& vm){
//    int tempID_Blue;
//    int tempID_Yellow;
//    CGeoPoint pos_1 ;
//    CGeoPoint pos_2 ;
//    CVector vel_1 ;
//    CVector vel_2 ;
//    for(int i = 0 ; i < vm.maintain[0].robotSize[PARAM::BLUE] ; i++){
//        for(int j = 0 ; j < vm.maintain[0].robotSize[PARAM::YELLOW] ; j++){
//            if(vm.maintain[0].robot[PARAM::BLUE][i].pos.dist(vm.maintain[0].robot[PARAM::YELLOW][j].pos) < min_dist){
//                tempID_Blue = vm.maintain[0].robot[PARAM::BLUE][i].id;
//                tempID_Yellow = vm.maintain[0].robot[PARAM::YELLOW][j].id;
//                pos_1 = vm.maintain[0].robot[PARAM::BLUE][i].pos ;
//                pos_2 = vm.maintain[0].robot[PARAM::YELLOW][j].pos ;
//            }
//        }
//    }
//    for(int i = 0 ; i < vm.maintain[0].robotSize[PARAM::BLUE] ; i++){
//        if(vm.maintain[0].robot[PARAM::BLUE][i].id == tempID_Blue){
//            vel_1 = vm.maintain[-3].robot[PARAM::BLUE][i].velocity;
//        }
//    }
//    for(int i = 0 ; i < vm.maintain[0].robotSize[PARAM::YELLOW] ; i++){
//        if(vm.maintain[0].robot[PARAM::YELLOW][i].id == tempID_Blue){
//            vel_2 = vm.maintain[-3].robot[PARAM::YELLOW][i].velocity;
//        }
//    }
//    collisionDetect(pos_1 , pos_2 , vel_1 , vel_2);
//    judgeCollision(vel_1 , vel_2);
//}


//bool collision::collisionDetect(CGeoPoint& pos_1 , CGeoPoint& pos_2 , CVector vel_1 , CVector vel_2){
//    if(pos_1.dist(pos_2) < min_dist && (vel_1 - vel_2).mod() > min_vel_collision){
//        return true;
//    }
//    return false;
//}

//int collision::judgeCollision(CVector vel_blue , CVector vel_yellow){
//    if(vel_blue.mod() > vel_yellow.mod()){
//        return PARAM::BLUE;
//    }else{
//        return PARAM::YELLOW;
//    }
//}
