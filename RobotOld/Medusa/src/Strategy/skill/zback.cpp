#include "zback.h"
#include "VisionModule.h"
#include "skill/Factory.h"
#include "guardpos.h"
#include "SkillUtils.h"
#include "KickStatus.h"
#include "parammanager.h"
#include "GDebugEngine.h"
namespace {
const CGeoPoint OUR_GOAL = CGeoPoint(-PARAM::Field::PITCH_LENGTH/2, 0);
const double MIN_DIST_TO_PENALTY = PARAM::Vehicle::V2::PLAYER_SIZE*2;
const double DSS_DIST_FROM_PENALTY = PARAM::Vehicle::V2::PLAYER_SIZE*8;
bool DEBUG_ZBACK = false;
}

CZBack::CZBack()
{
ZSS::ZParamManager::instance()->loadParam(DEBUG_ZBACK, "Debug/ZBack", false);
}

void CZBack::plan(const CVisionModule *pVision)
{
    // basic info
    int player = task().executor;
    int flag = task().player.flag;
    int index = task().player.rotdir;
    int guardNum = task().player.kick_flag;
    double power = task().player.chipkickpower;
    bool needKick = flag & PlayerStatus::KICK;
    bool isChip = flag & PlayerStatus::CHIP;
    int bestEnemyNum = ZSkillUtils::instance()->getTheirBestPlayer();
    const PlayerVisionT& bestEnemy = pVision->theirPlayer(bestEnemyNum);
    const MobileVisionT& ball = pVision->ball();
    const PlayerVisionT& me = pVision->ourPlayer(player);
    // task
    TaskT GuardTask(task());
    CGeoPoint guardPos = GuardPos::Instance()->backPos(guardNum, index);
    // 当存在后卫失位时，调整位置
    int validGuard = GuardPos::Instance()->checkValidNum(guardNum);
    int missingGuard = guardNum - validGuard;
    if (missingGuard > 0 && GuardPos::Instance()->validBackPos(guardPos, player)){
        int missingCnt = 0;
        for (int i = 1; i <= missingGuard; ++i) {
            if (GuardPos::Instance()->missingBackIndex(i) < index)
                missingCnt++;
        }
        int newindex = index - missingCnt;
        guardPos = GuardPos::Instance()->backPos(validGuard, newindex);
    }
    if (DEBUG_ZBACK){
        GDebugEngine::Instance()->gui_debug_x(guardPos, COLOR_CYAN);
        GDebugEngine::Instance()->gui_debug_msg(guardPos, QString("  %1 %2").arg(player).arg(index).toLatin1(), COLOR_CYAN);
    }
    GuardTask.player.pos = guardPos;
    // 朝向
    if (Utils::InOurPenaltyArea(ball.Pos(), MIN_DIST_TO_PENALTY))
        GuardTask.player.angle = (bestEnemy.Pos() - OUR_GOAL).dir();
    else
        GuardTask.player.angle = (ball.Pos() - OUR_GOAL).dir();

    // 距离禁区较远时开DSS
    if (!Utils::InOurPenaltyArea(me.Pos(), DSS_DIST_FROM_PENALTY)) GuardTask.player.flag = PlayerStatus::ALLOW_DSS;
    setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(GuardTask));

    if (needKick){
        if(!isChip) KickStatus::Instance()->setKick(player, power);
        else KickStatus::Instance()->setChipKick(player, power);
    }

    _lastCycle = pVision->getCycle();
    CStatedTask::plan(pVision);
}

CPlayerCommand* CZBack::execute(const CVisionModule *pVision)
{
    if(subTask()) return subTask()->execute(pVision);
    return nullptr;
}
