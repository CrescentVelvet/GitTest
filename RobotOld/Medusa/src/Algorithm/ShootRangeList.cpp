#include <VisionModule.h>
#include <RobotCapability.h>
#include <TaskMediator.h>
#include "ShootRangeList.h"
#include "PlayInterface.h"
#include "staticparams.h"	//  [5/16/2011 zhanfei]
//namespace{
//    bool IGNORE_OPP_GOALIE = false;
//}
CShootRangeList::CShootRangeList(const CVisionModule* pVision, const int player, const CGeoPoint pos)
{
	if (pVision->ourPlayer(player).Valid()) {
//        if (lastCycle != pVision->Cycle() || player != lastPlayer/* || lastPoint.dist(pos) > 5*/) { // 重新计算 gty/wyk 19/5/14
			lastCycle = pVision->getCycle();
			lastPlayer = player;
			lastPoint = pos;

			lastShootList.clear(); // 清空
			lastBothKickList.clear();
			lastChipKickList.clear();
			const CGeoPoint theirGoal(PARAM::Field::PITCH_LENGTH / 2, 0);

			CGeoPoint startPos;
            if (std::abs(pos.x()) > 1000) {
				startPos = pVision->ball().Pos();
			}
			else {
				startPos = pos;
			}
			const double startPos2TheirGoalDist = (theirGoal - startPos).mod();
			const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(pVision->getSide(), player);
//			const double minShootAngleWidth = robotCap->minShootAngleWidth();
			const CGeoPoint leftPost(PARAM::Field::PITCH_LENGTH / 2, -PARAM::Field::GOAL_WIDTH / 2); // 左门柱
			const CGeoPoint rightPost(PARAM::Field::PITCH_LENGTH / 2, PARAM::Field::GOAL_WIDTH / 2); // 右门柱
			const double leftPostAngle = (leftPost - startPos).dir();
			const double rightPostAngle = (rightPost - startPos).dir();
			const double leftPostDist = (leftPost - startPos).mod();
			const double rightPostDist = (rightPost - startPos).mod();
			bool shootAngleIsTooNarrow = false;
			if (std::abs(startPos.y()) > PARAM::Field::GOAL_WIDTH / 2) {
				// 两个门柱
				//const double minShootAngle = (std::min)(std::abs(leftPostAngle), std::abs(rightPostAngle));
				const double minShootWidth = 2 * std::sin(std::abs(leftPostAngle - rightPostAngle) / 2)*(std::min)(leftPostDist, rightPostDist); // --- LIUNIAN

                shootAngleIsTooNarrow = minShootWidth < 1.5*robotCap->minShootAngleWidth();//射门宽度小于1.5*1.5*ballSize  gty/wyk 2019.5.14
			}
			if (!shootAngleIsTooNarrow) {
				const double minChipKickObstacleDist = 26/*robotCap->minChipKickObstacleDist()*/;
				const double maxChipKickDist = (std::min)(robotCap->maxChipShootDist(), startPos2TheirGoalDist - robotCap->minChipShootExtra(startPos2TheirGoalDist) - PARAM::Field::MAX_PLAYER_SIZE);

				//lastShootList.add(CValueRange(leftPostAngle, rightPostAngle)); // 初始化
				//lastBothKickList.add(CValueRange(leftPostAngle, rightPostAngle));
				lastShootList.add(CValueRange(leftPostAngle, rightPostAngle, leftPostDist, rightPostDist)); // 初始化
				lastBothKickList.add(CValueRange(leftPostAngle, rightPostAngle, leftPostDist, rightPostDist));
				for (int i = 0; i < PARAM::Field::MAX_PLAYER; ++i) {
					if (pVision->theirPlayer(i).Valid() && pVision->theirPlayer(i).X() > startPos.x()) {
						const CGeoPoint& playerPos = pVision->theirPlayer(i).Pos();

                        // delete by wyk, not use anymore 2019.5.14
//						// 对付CMU，可能需要忽略他的守门员
//						if (IGNORE_OPP_GOALIE) {
//							if (playerPos.dist(theirGoal) < PARAM::Field::GOAL_WIDTH / 2 + 30)
//								continue;
//						}

						const CVector startPos2player = playerPos - startPos;
						const double playerDist = startPos2player.mod() - PARAM::Field::MAX_PLAYER_SIZE / 2;
						const double playerDir = startPos2player.dir();
						const double angleRange = (playerDist <= PARAM::Field::MAX_PLAYER_SIZE / 2 ? PARAM::Math::PI / 2 : std::asin(PARAM::Field::MAX_PLAYER_SIZE / 2 / playerDist)); // 挡住的角度
						lastShootList.removeAngleRange(playerDir, angleRange, playerDist);
                        if (playerDist < 0.001 || (playerDist > minChipKickObstacleDist && playerDist < maxChipKickDist)) {
							continue; // 挡不住或者防止除法越界
						}
						lastBothKickList.removeAngleRange(playerDir, angleRange, playerDist);
					}
				}
                for( int i=0; i<PARAM::Field::MAX_PLAYER; ++i ){
                    if( pVision->ourPlayer(i).Valid() && pVision->ourPlayer(i).X() > startPos.x() && i!=player){
                        //自己挡住的当然不算
                        const CGeoPoint& playerPos = pVision->ourPlayer(i).Pos();
                        const CVector startPos2player = playerPos - startPos;
                        const double playerDist = startPos2player.mod() - PARAM::Field::MAX_PLAYER_SIZE/2;
                        const double playerDir = startPos2player.dir();
                        const double angleRange = (playerDist <= PARAM::Field::MAX_PLAYER_SIZE/2 ? PARAM::Math::PI/2 : std::asin( PARAM::Field::MAX_PLAYER_SIZE/2 / playerDist)); // 挡住的角度
                        lastShootList.removeAngleRange(playerDir, angleRange, playerDist);
                        if( playerDist < 0.001 || (playerDist > minChipKickObstacleDist && playerDist < maxChipKickDist)){
                            continue; // 挡不住或者防止除法越界
                        }
                        lastBothKickList.removeAngleRange(playerDir, angleRange, playerDist);
                    }
                }
			}
			lastChipKickList = lastBothKickList - lastShootList;
//		}
	}
}

CShootRangeList::CShootRangeList(const CVisionModule* pVision,const bool defence,const int player,const CGeoPoint pos)
{
	//SR_DEFENCE 为重载此函数的重要标志
	if (SR_DEFENCE != defence)
	{
		cout<<"call function ERROR in shootRangeList,please check your codes "<<endl;
		return;
	}
	if( lastCycle != pVision->getCycle() || player != lastPlayer || lastPoint.dist(pos)>5){ // 重新计算
		lastCycle = pVision->getCycle();
		lastPlayer = player;
		lastPoint = pos;

		lastShootList.clear(); // 清空
		lastBothKickList.clear();
		lastChipKickList.clear();
		const CGeoPoint myGoal(PARAM::Field::PITCH_LENGTH/2, 0);

		CGeoPoint startPos;
		if (std::abs(pos.x())>1000){
			startPos = CGeoPoint(-1*pVision->ball().Pos().x(),-1*pVision->ball().Pos().y());
		}
		else{
			startPos = CGeoPoint(-1*pos.x(),-1*pos.y());
		}
		const double startPos2myGoalDist = (myGoal - startPos).mod();
		const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(pVision->getSide(), 1);
//		const double minShootAngleWidth = robotCap->minShootAngleWidth();
		const CGeoPoint leftPost(PARAM::Field::PITCH_LENGTH/2, -PARAM::Field::GOAL_WIDTH/2); // 左门柱
		const CGeoPoint rightPost(PARAM::Field::PITCH_LENGTH/2, PARAM::Field::GOAL_WIDTH/2); // 右门柱
		const double leftPostAngle = (leftPost - startPos).dir();
		const double rightPostAngle = (rightPost - startPos).dir();
		const double leftPostDist = (leftPost - startPos).mod();
		const double rightPostDist = (rightPost - startPos).mod();
		bool shootAngleIsTooNarrow = false;
		if( std::abs(startPos.y()) > PARAM::Field::GOAL_WIDTH/2 ){
			// 两个门柱
			//const double minShootAngle = (std::min)(std::abs(leftPostAngle), std::abs(rightPostAngle));
			const double minShootWidth = 2*std::sin(std::abs(leftPostAngle-rightPostAngle)/2)*(std::min)(leftPostDist,rightPostDist); // --- LIUNIAN

			shootAngleIsTooNarrow = minShootWidth < 1.5*robotCap->minShootAngleWidth();
		}
		if( !shootAngleIsTooNarrow ){
			const double minChipKickObstacleDist = 26/*robotCap->minChipKickObstacleDist()*/;
			const double maxChipKickDist = (std::min)(robotCap->maxChipShootDist(), startPos2myGoalDist - robotCap->minChipShootExtra(startPos2myGoalDist) - PARAM::Field::MAX_PLAYER_SIZE );

			//lastShootList.add(CValueRange(leftPostAngle, rightPostAngle)); // 初始化
			//lastBothKickList.add(CValueRange(leftPostAngle, rightPostAngle));
			lastShootList.add(CValueRange(leftPostAngle, rightPostAngle,leftPostDist, rightPostDist)); // 初始化
			lastBothKickList.add(CValueRange(leftPostAngle, rightPostAngle, leftPostDist, rightPostDist));
			for( int i=0; i<PARAM::Field::MAX_PLAYER; ++i ){
				if (player == i)//排除掉我想要排除的车
				{
					continue;
				}
				if( pVision->ourPlayer(i).Valid() && pVision->ourPlayer(i).X() * -1 > startPos.x() - 10){// -10为了防止底线射门时计算错误
					const CGeoPoint& playerPos = CGeoPoint(-1*pVision->ourPlayer(i).X(),-1*pVision->ourPlayer(i).Y());

					const CVector startPos2player = playerPos - startPos;
					const double playerDist = startPos2player.mod() - PARAM::Field::MAX_PLAYER_SIZE/2;
					const double playerDir = startPos2player.dir();
					const double angleRange = (playerDist <= PARAM::Field::MAX_PLAYER_SIZE/2 ? PARAM::Math::PI/2 : std::asin( PARAM::Field::MAX_PLAYER_SIZE/2 / playerDist)); // 挡住的角度
					lastShootList.removeAngleRange(playerDir, angleRange, playerDist);
                    if( playerDist < 0.001 || (playerDist > minChipKickObstacleDist && playerDist < maxChipKickDist)){
						continue; // 挡不住或者防止除法越界
					}
					lastBothKickList.removeAngleRange(playerDir, angleRange, playerDist);
				}
			}
		}
		lastChipKickList = lastBothKickList - lastShootList;
	}
}

const CValueRangeList& CShootRangeList::get()
{
	return lastBothKickList;
}
const CValueRangeList& CShootRangeList::getShootRange()
{
	return lastShootList;
}

const CValueRangeList& CShootRangeList::getChipKickRange()
{
	return lastChipKickList;
}
