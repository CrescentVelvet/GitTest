#include "out_event.h"

out_event::out_event() /*: status(0)*/{

}

//void out_event::judgeOutEvent(vmData& vmdata){
//    if(Utils::instance()->inArea(vmdata.maintain[0].ball[0].pos, Param::Field::PITCH_LENGTH/2, Param::Field::PITCH_LENGTH/2+Param::Field::GOAL_DEPTH, -Param::Field::GOAL_WIDTH/2, Param::Field::GOAL_WIDTH/2)
//            || Utils::instance()->inArea(vmdata.maintain[0].ball[0].pos, -Param::Field::PITCH_LENGTH/2, -Param::Field::PITCH_LENGTH/2-Param::Field::GOAL_DEPTH, -Param::Field::GOAL_WIDTH/2, Param::Field::GOAL_WIDTH/2)){
//        status = goal;
//    }else if(! Utils::instance()->inArea(vmdata.maintain[0].ball[0].pos, -Param::Field::PITCH_LENGTH/2, Param::Field::PITCH_LENGTH/2, -Param::Field::PITCH_WIDTH/2, Param::Field::PITCH_WIDTH/2)){
//        status = out_of_field;
//    }
//}

//void out_event::run(vmData& vmdata){
//    judgeOutEvent(vmdata);
//}
