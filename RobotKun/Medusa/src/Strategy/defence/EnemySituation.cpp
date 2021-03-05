#include "EnemySituation.h"
#include "staticparams.h"

CEnemySituation::CEnemySituation()
{
	ballKeeper.num = 1;
	ballKeeper.pos = CGeoPoint(0,0);
	ballKeeper.dir = PARAM::Math::PI;
	ballKeeper.ballTaked = false;
	ballKeeper.backPass = false;
	ballKeeper.passMe = false;
	receiver = 2;
	receiverDir = PARAM::Math::PI;
	receiverExist = false;
	attackerNum = 0;
	keeperChange = false;
	attackerNumChange = false;
	attackerChange = false;
	defSituationChange = false;
	for(int i = 0;i < PARAM::Field::MAX_PLAYER;i++)isMarked[i] = false;
	for(int i = 0;i < PARAM::Field::MAX_PLAYER;i++)markCycle[i] = 0;
	for(int i = 0;i < PARAM::Field::MAX_PLAYER;i++)steadyAttackArray[i] = 0;
	markList.reserve(6);
	markList.clear();
}
