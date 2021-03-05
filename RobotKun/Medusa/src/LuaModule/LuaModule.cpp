#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "skill/Factory.h"
#include "LuaModule.h"
#include "geometry.h"
#include "TaskMediator.h"
#include "BufferCounter.h"
#include "Global.h"
#include "Compensate.h"
#include "KickOffDefPosV2.h"
#include "MarkingPosV2.h"
#include "PenaltyPosCleaner.h"
#include "defence/EnemyDefendTacticAnalys.h"
#include "WaitKickPos.h"
#include "TouchKickPos.h"
#include "MarkingTouchPos.h"
#include "SkillUtils.h"
#include <QString>

#ifndef _WIN32
#include <libgen.h>
#endif

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "tolua++.h"
TOLUA_API int  tolua_zeus_open (lua_State* tolua_S);

extern "C" {
	typedef struct 
	{
		const char *name;
		int (*func)(lua_State *);
	}luaDef;
}

extern luaDef GUIGlue[];

namespace{
	bool IS_SIMULATION = false;
    bool LUA_DEBUG = true;
}

CLuaModule::CLuaModule():m_pScriptContext(nullptr)
{
	m_pErrorHandler = NULL;

	m_pScriptContext = lua_open();
	luaL_openlibs(m_pScriptContext);
	tolua_zeus_open(m_pScriptContext);
    InitLuaGlueFunc();
    ZSS::ZParamManager::instance()->loadParam(LUA_DEBUG,"Debug/A_LuaDebug",true);

}

void CLuaModule::InitLuaGlueFunc()
{
	for(int i=0; GUIGlue[i].name; i++) {
		AddFunction(GUIGlue[i].name, GUIGlue[i].func);
	}
}

CLuaModule::~CLuaModule()
{
	if(m_pScriptContext)
		lua_close(m_pScriptContext);
}

static std::string findScript(const char *pFname)
{
	FILE *fTest;
#if defined(_WIN32)
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
    _splitpath( pFname, drive, dir, fname, ext );
    std::string scriptsString = "Scripts\\";
#else
    std::string pname = pFname;
    int l = pname.length();
    char name[l+1];
    strcpy(name,pname.c_str());
    std::string drive = "";
    std::string dir = dirname(name);
    std::string fname = basename(name);// Mark : not work /error
    std::string ext = "";
    std::string scriptsString = "Scripts/";
#endif
    std::string strTestFile(pFname);
    fTest = fopen(strTestFile.c_str(), "r");
    if(fTest == NULL)
    {
        //not that one...
        strTestFile = (std::string) drive + dir + scriptsString + fname + ".lua";
        fTest = fopen(strTestFile.c_str(), "r");
    }
	if(fTest == NULL)
	{
		//not that one...
        strTestFile = (std::string) drive + dir + scriptsString + fname + ".LUA";
		fTest = fopen(strTestFile.c_str(), "r");
	}

	if(fTest == NULL)
	{
		//not that one...
		strTestFile = (std::string) drive + dir + fname + ".LUB";
		fTest = fopen(strTestFile.c_str(), "r");
	}

	if(fTest == NULL)
	{
		//not that one...
		//not that one...
		strTestFile = (std::string) drive + dir + fname + ".LUA";
		fTest = fopen(strTestFile.c_str(), "r");
	}

	if(fTest != NULL)
	{
		fclose(fTest);
	}

	return strTestFile;
}

bool CLuaModule::RunScript(const char *pFname)
{
    std::string strFilename = findScript(pFname);
    const char *pFilename = strFilename.c_str();

	if (0 != luaL_loadfile(m_pScriptContext, pFilename))
    {
        double x = (ZSS::ZParamManager::instance()->value("ZAlert/IsRight").toBool()?1:-1)*(PARAM::Field::PITCH_LENGTH/2-50);
        qDebug() << QString("Lua Error - Script Run\nScript Name:%1\nError Message:%2\n").arg(pFilename).arg(luaL_checkstring(m_pScriptContext, -1));
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(x,-200),QString("Lua Error - Script Load").toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(x,-400),QString("Name:%1").arg(pFilename).toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(x,-600),QString("Error Message:%1").arg(luaL_checkstring(m_pScriptContext, -1)).toLatin1());
        GDebugEngine::Instance()->send(!ZSS::ZParamManager::instance()->value("ZAlert/IsYellow").toBool());
        if(LUA_DEBUG){
            std::cout << "Press enter to continue ...";
            std::cin.get();
        }
            return false;
    }
	if (0 != lua_pcall(m_pScriptContext, 0, LUA_MULTRET, 0))
    {
        double x = (ZSS::ZParamManager::instance()->value("ZAlert/IsRight").toBool()?1:-1)*(PARAM::Field::PITCH_LENGTH/2-50);
        qDebug() << QString("Lua Error - Script Run\nScript Name:%1\nError Message:%2\n").arg(pFilename).arg(luaL_checkstring(m_pScriptContext, -1));
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(x,-200),QString("Lua Error - Script Load").toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(x,-400),QString("Name:%1").arg(pFilename).toLatin1());
        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(x,-600),QString("Error Message:%1").arg(luaL_checkstring(m_pScriptContext, -1)).toLatin1());
        GDebugEngine::Instance()->send(!ZSS::ZParamManager::instance()->value("ZAlert/IsYellow").toBool());
        if(LUA_DEBUG){
            std::cout << "Press enter to continue ...";
            std::cin.get();
        }
        return false;
    }
	return true;

}

bool CLuaModule::RunString(const char *pCommand)
{
	if (0 != luaL_loadbuffer(m_pScriptContext, pCommand, strlen(pCommand), NULL))
	{
		if(m_pErrorHandler)
		{
			char buf[256];
			sprintf(buf, "Lua Error - String Load\nString:%s\nError Message:%s\n", pCommand, luaL_checkstring(m_pScriptContext, -1));
			m_pErrorHandler(buf);
		}

		return false;
	}
	if (0 != lua_pcall(m_pScriptContext, 0, LUA_MULTRET, 0))
	{
		if(m_pErrorHandler)
		{
			char buf[256];
			sprintf(buf, "Lua Error - String Run\nString:%s\nError Message:%s\n", pCommand, luaL_checkstring(m_pScriptContext, -1));
			m_pErrorHandler(buf);
		}

		return false;
	}
	return true;
}

const char *CLuaModule::GetErrorString(void)
{
	return luaL_checkstring(m_pScriptContext, -1);
}


bool CLuaModule::AddFunction(const char *pFunctionName, LuaFunctionType pFunction)
{
	lua_register(m_pScriptContext, pFunctionName, pFunction);
	return true;
}

const char *CLuaModule::GetStringArgument(int num, const char *pDefault)
{
	return luaL_optstring(m_pScriptContext, num, pDefault);

}

double CLuaModule::GetNumberArgument(int num, double dDefault)
{
	return luaL_optnumber(m_pScriptContext, num, dDefault);
}

bool CLuaModule::GetBoolArgument(int num)
{
	return lua_toboolean(m_pScriptContext, num);
}

CGeoPoint* CLuaModule::GetPointArgument(int num)
{
	return (CGeoPoint*)(lua_touserdata(m_pScriptContext, num));
}

void CLuaModule::PushString(const char *pString)
{
	lua_pushstring(m_pScriptContext, pString);
}

void CLuaModule::PushNumber(double value)
{
	lua_pushnumber(m_pScriptContext, value);
}

void CLuaModule::PushBool(bool value)
{
	lua_pushboolean(m_pScriptContext, value);
}

extern "C" int Skill_SmartGotoPoint(lua_State *L)
{
	TaskT playerTask;
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	playerTask.executor = runner;
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	playerTask.player.pos = CGeoPoint(x,y);
	playerTask.player.rotvel = 0;
	playerTask.player.angle = LuaModule::Instance()->GetNumberArgument(4, NULL);
	playerTask.player.flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
	playerTask.ball.Sender = LuaModule::Instance()->GetNumberArgument(6,NULL);
	playerTask.player.max_acceleration = LuaModule::Instance()->GetNumberArgument(7,NULL);
    double vx = LuaModule::Instance()->GetNumberArgument(8, 0.0);
    double vy = LuaModule::Instance()->GetNumberArgument(9, 0.0);
    playerTask.player.vel = CVector(vx,vy);

	CPlayerTask* pTask = TaskFactoryV2::Instance()->SmartGotoPosition(playerTask);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	
	return 0;
}

extern "C" int Skill_GoAvoidShootLine(lua_State *L)
{
	TaskT playerTask;
	playerTask.player.is_specify_ctrl_method = true;
	playerTask.player.specified_ctrl_method = CMU_TRAJ;
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	playerTask.executor = runner;
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	playerTask.player.pos = CGeoPoint(x,y);
	playerTask.player.angle = LuaModule::Instance()->GetNumberArgument(4, NULL);
	playerTask.player.flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
	playerTask.ball.Sender = LuaModule::Instance()->GetNumberArgument(6,NULL);

	CPlayerTask* pTask = TaskFactoryV2::Instance()->GoAvoidShootLine(playerTask);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

extern "C" int Skill_GoCmuRush(lua_State *L)
{
	TaskT playerTask;
	playerTask.player.is_specify_ctrl_method = true;
    playerTask.player.specified_ctrl_method = CMU_TRAJ;
    int runner = int(LuaModule::Instance()->GetNumberArgument(1, 0));
	playerTask.executor = runner;
    double x = LuaModule::Instance()->GetNumberArgument(2, 0);
    double y = LuaModule::Instance()->GetNumberArgument(3, 0);
	playerTask.player.pos = CGeoPoint(x,y);
    playerTask.player.angle = LuaModule::Instance()->GetNumberArgument(4, 0);
    playerTask.player.flag = int(LuaModule::Instance()->GetNumberArgument(5, 0));
    playerTask.ball.Sender = int(LuaModule::Instance()->GetNumberArgument(6,0));
    playerTask.player.max_acceleration = LuaModule::Instance()->GetNumberArgument(7,0);
    playerTask.player.needdribble = int(LuaModule::Instance()->GetNumberArgument(8,0));
    double vx = LuaModule::Instance()->GetNumberArgument(9,0.0);
    double vy = LuaModule::Instance()->GetNumberArgument(10,0.0);
    playerTask.player.vel = CVector(vx,vy);

	CPlayerTask* pTask = TaskFactoryV2::Instance()->SmartGotoPosition(playerTask);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

extern "C" int Skill_BezierRush(lua_State *L)
{
    TaskT playerTask;
    playerTask.player.is_specify_ctrl_method = true;
    playerTask.player.specified_ctrl_method = CMU_TRAJ;
    int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
    playerTask.executor = runner;
    double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
    double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
    playerTask.player.pos = CGeoPoint(x, y);
    playerTask.player.angle = LuaModule::Instance()->GetNumberArgument(4, NULL);
    playerTask.player.flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
    playerTask.ball.Sender = LuaModule::Instance()->GetNumberArgument(6, NULL);
    playerTask.player.max_acceleration = LuaModule::Instance()->GetNumberArgument(7, NULL);
    playerTask.player.needdribble = LuaModule::Instance()->GetNumberArgument(8, NULL);

    CPlayerTask* pTask = TaskFactoryV2::Instance()->GotoPositionNew(playerTask);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

    return 0;
}

extern "C" int Skill_NoneZeroGoCmuRush(lua_State *L)
{
	TaskT playerTask;
	playerTask.player.is_specify_ctrl_method = true;
	playerTask.player.specified_ctrl_method = CMU_TRAJ;
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	playerTask.executor = runner;
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	playerTask.player.pos = CGeoPoint(x, y);
	playerTask.player.angle = LuaModule::Instance()->GetNumberArgument(4, NULL);
	playerTask.player.flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
	playerTask.ball.Sender = LuaModule::Instance()->GetNumberArgument(6, NULL);
	playerTask.player.max_acceleration = LuaModule::Instance()->GetNumberArgument(7, NULL);
	playerTask.player.needdribble = LuaModule::Instance()->GetNumberArgument(8, NULL);
	double velX = LuaModule::Instance()->GetNumberArgument(9, NULL);
	double velY = LuaModule::Instance()->GetNumberArgument(10, NULL);
    playerTask.player.vel = CVector(velX, velY);

    CPlayerTask* pTask = TaskFactoryV2::Instance()->SmartGotoPosition(playerTask);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

extern "C" int Skill_SimpleGotoPoint(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(4, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
	CPlayerTask* pTask = PlayerRole::makeItSimpleGoto(runner, CGeoPoint(x, y), angle, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

extern "C" int Skill_Stop(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	CPlayerTask* pTask = PlayerRole::makeItStop(runner);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

extern "C" int Register_Role(lua_State *L)
{
	int num = LuaModule::Instance()->GetNumberArgument(1, NULL);
	string role = LuaModule::Instance()->GetStringArgument(2, NULL);
	TaskMediator::Instance()->setRoleInLua(num, role);
	return 0;
}

extern "C" int Skill_GetBallV4(lua_State *L)
{
    int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
    //double angle = LuaModule::Instance()->GetNumberArgument(2, NULL);
    double px = LuaModule::Instance()->GetNumberArgument(2, NULL);
    double py = LuaModule::Instance()->GetNumberArgument(3, NULL);
    double wpx = LuaModule::Instance()->GetNumberArgument(4, NULL);
    double wpy = LuaModule::Instance()->GetNumberArgument(5, NULL);
    double power = LuaModule::Instance()->GetNumberArgument(6, NULL);
    int kick_flag = LuaModule::Instance()->GetNumberArgument(7, NULL);
    double precision = LuaModule::Instance()->GetNumberArgument(8, NULL);
    CGeoPoint pos = CGeoPoint(px, py);
    CGeoPoint waitPos = CGeoPoint(wpx, wpy);

    CPlayerTask* pTask = PlayerRole::makeItGetBallV4(runner, kick_flag, pos, waitPos, power, precision);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int Skill_StaticGetBall(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(3,NULL); 
	CPlayerTask* pTask = PlayerRole::makeItStaticGetBallNew(runner, angle, CVector(0,0), flag, 12, CMU_TRAJ);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_GoAndTurnKickV3(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double preci = LuaModule::Instance()->GetNumberArgument(3,NULL);
	int circleNum = LuaModule::Instance()->GetNumberArgument(4,NULL);
	double fixAngle = LuaModule::Instance()->GetNumberArgument(5,NULL);
	double maxAcc = LuaModule::Instance()->GetNumberArgument(6,NULL);
	int radius = LuaModule::Instance()->GetNumberArgument(7,NULL);
	int numPerCir = LuaModule::Instance()->GetNumberArgument(8,NULL);
	double gotoPre = LuaModule::Instance()->GetNumberArgument(9,NULL);
	double gotoDist = LuaModule::Instance()->GetNumberArgument(10,NULL);
	double adjustPre = LuaModule::Instance()->GetNumberArgument(11,NULL);
	CPlayerTask* pTask = PlayerRole::makeItGoAndTurnKickV3(runner, angle, circleNum, fixAngle,maxAcc,radius,numPerCir,gotoPre,gotoDist,adjustPre,preci);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_SlowGetBall(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	CPlayerTask* pTask = PlayerRole::makeItSlowGetBall(runner, angle, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ChaseKick(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(3,NULL);
	CPlayerTask* pTask = PlayerRole::makeItChaseKickV1(runner, angle, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ChaseKickV2(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(3,NULL);
	CPlayerTask* pTask = PlayerRole::makeItChaseKickV2(runner, angle, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

// 现在使用新的抢球，by dxh 2013.5.20
extern "C" int Skill_AdvanceBallV2(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	int tendemNum = LuaModule::Instance()->GetNumberArgument(3, NULL);
	CPlayerTask* pTask = PlayerRole::makeItAdvanceBallV2(runner, flag, tendemNum);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ZPass(lua_State *L)
{
    int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
    double px = LuaModule::Instance()->GetNumberArgument(2, NULL);
    double py = LuaModule::Instance()->GetNumberArgument(3, NULL);
    double power = LuaModule::Instance()->GetNumberArgument(4, NULL);
    int kick_flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
    double precision = LuaModule::Instance()->GetNumberArgument(6, NULL);
    CGeoPoint target = CGeoPoint(px, py);
    CPlayerTask* pTask = PlayerRole::makeItZPass(runner, target, power, kick_flag, precision);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int Skill_FetchBall(lua_State *L)
{
    int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
    double px = LuaModule::Instance()->GetNumberArgument(2, NULL);
    double py = LuaModule::Instance()->GetNumberArgument(3, NULL);
    double power = LuaModule::Instance()->GetNumberArgument(4, NULL);
    int kick_flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
    CGeoPoint target = CGeoPoint(px, py);
    CPlayerTask* pTask = PlayerRole::makeItFetchBall(runner, target, power, kick_flag);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int Skill_ZSupport(lua_State *L)
{
    int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
    CPlayerTask* pTask = PlayerRole::makeItZSupport(runner);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int Skill_ZBreak(lua_State *L)
{
    int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
    double px = LuaModule::Instance()->GetNumberArgument(2, NULL);
    double py = LuaModule::Instance()->GetNumberArgument(3, NULL);
    double power = LuaModule::Instance()->GetNumberArgument(4, NULL);
    int kick_flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
    double precision = LuaModule::Instance()->GetNumberArgument(6, NULL);
    CGeoPoint target = CGeoPoint(px, py);
    CPlayerTask* pTask = PlayerRole::makeItZBreak(runner, target, power, kick_flag, precision);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int Skill_ZAttack(lua_State *L)
{
    int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
    double px = LuaModule::Instance()->GetNumberArgument(2, NULL);
    double py = LuaModule::Instance()->GetNumberArgument(3, NULL);
    double wpx = LuaModule::Instance()->GetNumberArgument(4, NULL);
    double wpy = LuaModule::Instance()->GetNumberArgument(5, NULL);
    double power = LuaModule::Instance()->GetNumberArgument(6, NULL);
    int flag = LuaModule::Instance()->GetNumberArgument(7, NULL);
    double precision = LuaModule::Instance()->GetNumberArgument(8, NULL);
    CGeoPoint target = CGeoPoint(px, py);
    CGeoPoint waitPos = CGeoPoint(wpx, wpy);
    CPlayerTask* pTask = PlayerRole::makeItZAttack(runner, target, waitPos, power, flag, precision);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int Skill_ZCirclePass(lua_State *L)
{
    int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
    double px = LuaModule::Instance()->GetNumberArgument(2, NULL);
    double py = LuaModule::Instance()->GetNumberArgument(3, NULL);
    double power = LuaModule::Instance()->GetNumberArgument(4, NULL);
    int flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
    CGeoPoint target = CGeoPoint(px, py);
    CPlayerTask* pTask = PlayerRole::makeItZCirclePass(runner, target, power, flag);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int Skill_DribbleTurnKick(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double finalDir = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double turnRotVel = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double kickPower = LuaModule::Instance()->GetNumberArgument(4,NULL);
	CPlayerTask* pTask = PlayerRole::makeItDribbleTurnKick(runner, finalDir, turnRotVel,kickPower);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ReceivePass(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(3, NULL);
	CPlayerTask* pTask = PlayerRole::makeItReceivePass(runner, angle, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ZGoalie(lua_State *L){
    int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
    double px = LuaModule::Instance()->GetNumberArgument(2, NULL);
    double py = LuaModule::Instance()->GetNumberArgument(3, NULL);
    int power = LuaModule::Instance()->GetNumberArgument(4, NULL);
    int flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
    CGeoPoint pos = CGeoPoint(px, py);
    CPlayerTask* pTask = PlayerRole::makeItZGoalie(runner, pos, power, flag);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int Skill_PenaltyGoalieV2(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyGoalieV2(runner, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_PenaltyGoalie2017V2(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyGoalie2017V2(runner, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_Speed(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double speedX = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double speedY = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double rotSpeed = LuaModule::Instance()->GetNumberArgument(4,NULL);
	CPlayerTask* pTask = PlayerRole::makeItRun(runner, speedX, speedY, rotSpeed, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}
extern "C" int Skill_OpenSpeed(lua_State *L)
{
    int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
    double speedX = LuaModule::Instance()->GetNumberArgument(2,NULL);
    double speedY = LuaModule::Instance()->GetNumberArgument(3,NULL);
    double rotSpeed = LuaModule::Instance()->GetNumberArgument(4,NULL);
    CPlayerTask* pTask = PlayerRole::makeItOpenRun(runner, speedX, speedY, rotSpeed, 0);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int Skill_Marking(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	int pri    = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double x = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double y = LuaModule::Instance()->GetNumberArgument(4,NULL);
	int flag   = LuaModule::Instance()->GetNumberArgument(5,NULL);
	bool front = LuaModule::Instance()->GetBoolArgument(6);
	double dir = LuaModule::Instance()->GetNumberArgument(7,NULL);
	int bestEnemy = DefenceInfo::Instance()->getAttackOppNumByPri(0);
	if (DefenceInfo::Instance()->getOppPlayerByNum(bestEnemy)->isTheRole("RReceiver")) {
		if (pri > 0) {
			pri -= 1;
		}		
	}
	pri = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(pri);
	CPlayerTask* pTask = PlayerRole::makeItMarkEnemy(runner, pri, front, flag, CGeoPoint(x,y),dir);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ZMarking(lua_State *L)
{
    int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
    int pri    = LuaModule::Instance()->GetNumberArgument(2,NULL);
    int flag   = LuaModule::Instance()->GetNumberArgument(3,NULL);
    int num    = LuaModule::Instance()->GetNumberArgument(4,NULL);
    int bestEnemy = DefenceInfo::Instance()->getAttackOppNumByPri(0);
    if (DefenceInfo::Instance()->getOppPlayerByNum(bestEnemy)->isTheRole("RReceiver")) {
        if (pri > 0) {
            pri -= 1;
        }
    }
    int oppNum = num >= 0 ? num : DefenceInfo::Instance()->getSteadyAttackOppNumByPri(pri);
    CPlayerTask* pTask = PlayerRole::makeItZMarkEnemy(runner, oppNum, pri, flag);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int Skill_HoldBall(lua_State *L){
    int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
    double px = LuaModule::Instance()->GetNumberArgument(2, NULL);
    double py = LuaModule::Instance()->GetNumberArgument(3, NULL);
    CGeoPoint target = CGeoPoint(px, py);
    CPlayerTask* pTask = PlayerRole::makeItHoldBall(runner, target);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int Skill_ZBlocking(lua_State *L)
{
    int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
    int pri    = LuaModule::Instance()->GetNumberArgument(2,NULL);
    double x = LuaModule::Instance()->GetNumberArgument(3,NULL);
    double y = LuaModule::Instance()->GetNumberArgument(4,NULL);
    int bestEnemy = DefenceInfo::Instance()->getAttackOppNumByPri(0);
    if (DefenceInfo::Instance()->getOppPlayerByNum(bestEnemy)->isTheRole("RReceiver")) {
        if (pri > 0) {
            pri -= 1;
        }
    }
    int defNum = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(pri);
    CGeoPoint runPoint(x, y);
    CPlayerTask* pTask = PlayerRole::makeItAMarkEnemy(runner, defNum, runPoint);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    return 0;
}

extern "C" int FUNC_GetMarkingPos(lua_State *L)
{	
	int pri = LuaModule::Instance()->GetNumberArgument(1,NULL);
	bool front = LuaModule::Instance()->GetBoolArgument(2);
	CGeoPoint p;
	int bestEnemy = DefenceInfo::Instance()->getAttackOppNumByPri(0);
	//当receiver为最高优先级的时候，这句话可以理解为场上对方是否有receiver
	if (DefenceInfo::Instance()->getOppPlayerByNum(bestEnemy)->isTheRole("RReceiver")){
		if (pri > 0) {
			pri -= 1;
		}		
	}
	int oppNum = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(pri);
	const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
	static bool kickOffSide = false;
	bool checkKickOffArea = false;
	if ("TheirIndirectKick" == refMsg || "TheirDirectKick" == refMsg){
		if (vision->ball().Y() >0){
			kickOffSide = false;
		}else{
			kickOffSide = true;
		}
	}
	if (kickOffSide == false){
		CGeoPoint leftUp = CGeoPoint(PARAM::Field::PITCH_LENGTH/2,30+20);
		CGeoPoint rightDown = CGeoPoint(80,PARAM::Field::PITCH_WIDTH/2);
		if (DefenceInfo::Instance()->checkInRecArea(oppNum,vision,MarkField(leftUp,rightDown))){
			checkKickOffArea = true;
		}
	}else{
		CGeoPoint leftUp = CGeoPoint(PARAM::Field::PITCH_LENGTH/2,-PARAM::Field::PITCH_WIDTH/2);
		CGeoPoint rightDown = CGeoPoint(80,-30-20);
		if (DefenceInfo::Instance()->checkInRecArea(oppNum,vision,MarkField(leftUp,rightDown))){
			checkKickOffArea = true;
		}
	}
	if (front && vision->theirPlayer(oppNum).Pos().x()>-190 && false == checkKickOffArea) {
		double dir = (vision->ball().Pos() - vision->theirPlayer(oppNum).Pos()).dir();
		p = vision->theirPlayer(oppNum).Pos() + Utils::Polar2Vector(35,dir);
	} else{
		p = MarkingPosV2::Instance()->getMarkingPos(vision, pri);
	}
	LuaModule::Instance()->PushNumber(p.x());
	LuaModule::Instance()->PushNumber(p.y());
	return 2;
}



extern "C" int FUNC_GetZMarkingPos(lua_State *L){
    int robotNum = LuaModule::Instance()->GetNumberArgument(1,NULL);
    int pri = LuaModule::Instance()->GetNumberArgument(2,NULL);
    int flag = LuaModule::Instance()->GetNumberArgument(3,NULL);
    int num = LuaModule::Instance()->GetNumberArgument(4, NULL);
    CGeoPoint markPos;
    int bestEnemy = DefenceInfo::Instance()->getAttackOppNumByPri(0);
    //当receiver为最高优先级的时候，这句话可以理解为场上对方是否有receiver
    if (DefenceInfo::Instance()->getOppPlayerByNum(bestEnemy)->isTheRole("RReceiver")){
        if (pri > 0) {
            pri -= 1;
        }
    }
    int oppNum = num >= 0 ? num : DefenceInfo::Instance()->getSteadyAttackOppNumByPri(pri);
    //GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(400*pri,200*robotNum),QString("%1 %2 %3").arg(robotNum).arg(pri).arg(oppNum).toLatin1());
    markPos = ZSkillUtils::instance()->getZMarkingPos(vision, robotNum, oppNum, pri, flag);
    LuaModule::Instance()->PushNumber(markPos.x());
    LuaModule::Instance()->PushNumber(markPos.y());
    return 2;
}

extern "C" int FUNC_GetZBlockingPos(lua_State *L){

    int pri = LuaModule::Instance()->GetNumberArgument(1,NULL);
    double defPos_x = LuaModule::Instance()->GetNumberArgument(2,NULL);
    double defPos_y = LuaModule::Instance()->GetNumberArgument(3,NULL);

    CGeoPoint markPos;
    int bestEnemy = DefenceInfo::Instance()->getAttackOppNumByPri(0);
    //当receiver为最高优先级的时候，这句话可以理解为场上对方是否有receiver
    if (DefenceInfo::Instance()->getOppPlayerByNum(bestEnemy)->isTheRole("RReceiver")){
        if (pri > 0) {
            pri -= 1;
        }
    }
    int oppNum = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(pri);
    CGeoPoint enemyPos = vision->theirPlayer(oppNum).Pos();
    const MobileVisionT& ball = vision->ball();

    markPos = ZSkillUtils::instance()->getMarkingPoint(enemyPos, vision->theirPlayer(oppNum).Vel(), 450, 450, 450, 300, CGeoPoint(defPos_x, defPos_y));

    if(!vision->theirPlayer(oppNum).Valid()){
        markPos = CGeoPoint(-2500, 0);
    }
    //防止车出场
    if (!Utils::IsInField(markPos, 180))
        markPos = Utils::MakeInField(markPos, 180);
    //防止车进入禁区
    if (Utils::InOurPenaltyArea(markPos, 180))
        markPos = Utils::MakeOutOfOurPenaltyArea(markPos, 180);
    //防止禁区前车挤车,绕前marking
    if ((markPos - enemyPos).mod() < 230)
        markPos = enemyPos + Utils::Polar2Vector(200, (ball.RawPos()- enemyPos).dir());

    LuaModule::Instance()->PushNumber(markPos.x());
    LuaModule::Instance()->PushNumber(markPos.y());
    return 2;
}

extern "C" int FUNC_ZGetBallPos(lua_State *L){
    int robotNum = LuaModule::Instance()->GetNumberArgument(1,0);
    if(robotNum == 0)
        robotNum = ZSkillUtils::instance()->getOurBestPlayer();
    CGeoPoint matchPos;
    matchPos = ZSkillUtils::instance()->getOurInterPoint(robotNum);
    LuaModule::Instance()->PushNumber(matchPos.x());
    LuaModule::Instance()->PushNumber(matchPos.y());
    return 2;
}

extern "C" int FUNC_GetBlockingPos(lua_State *L)
{	
	int pri = LuaModule::Instance()->GetNumberArgument(1,NULL);
	CGeoPoint p = MarkingPosV2::Instance()->getMarkingPosByAbsolutePri(vision,pri);
	LuaModule::Instance()->PushNumber(p.x());
	LuaModule::Instance()->PushNumber(p.y());
	return 2;
}

extern "C" int FUNC_TimeOut(lua_State* L)
{
	bool cond = LuaModule::Instance()->GetBoolArgument(1);
	int buf = LuaModule::Instance()->GetNumberArgument(2, NULL);
	int cnt = LuaModule::Instance()->GetNumberArgument(3, 9999);

	if(BufferCounter::Instance()->isClear(vision->getCycle())){
		BufferCounter::Instance()->startCount(vision->getCycle(), cond, buf, cnt);
	}

	if(BufferCounter::Instance()->timeOut(vision->getCycle(), cond)){
		LuaModule::Instance()->PushNumber(1);
	} else{
		LuaModule::Instance()->PushNumber(0);
	}
	return 1;
}

//extern "C" int FUNC_GetRealNum(lua_State* L)
//{
//	int num = LuaModule::Instance()->GetNumberArgument(1, NULL);
//	LuaModule::Instance()->PushNumber(PlayInterface::Instance()->getRealIndexByNum(num));
//	return 1;
//}

//extern "C" int FUNC_GetStrategyNum(lua_State* L)
//{
//	int num = LuaModule::Instance()->GetNumberArgument(1, NULL);
//	LuaModule::Instance()->PushNumber(num);
//	return 1;
//}

extern "C" int FUNC_GetIsSimulation(lua_State* L)
{
    ZSS::ZParamManager::instance()->loadParam(IS_SIMULATION,"Alert/IsSimulation",false);

	LuaModule::Instance()->PushBool(IS_SIMULATION);
	return 1;
}

extern "C" int FUNC_GetSettings(lua_State* L){
    QString key(LuaModule::Instance()->GetStringArgument(1, NULL));
    QString type(LuaModule::Instance()->GetStringArgument(2, NULL));
    if(type == "Bool"){
        bool temp;
        ZSS::ZParamManager::instance()->loadParam(temp,key);
        LuaModule::Instance()->PushBool(temp);
    }else if(type == "Int"){
        int temp;
        ZSS::ZParamManager::instance()->loadParam(temp,key);
        LuaModule::Instance()->PushNumber(temp);
    }else if(type == "Double"){
        double temp;
        ZSS::ZParamManager::instance()->loadParam(temp,key);
        LuaModule::Instance()->PushNumber(temp);
    }else{
        QString temp;
        ZSS::ZParamManager::instance()->loadParam(temp,key);
        LuaModule::Instance()->PushString(temp.toLatin1());
    }
    return 1;
}

extern "C" int FUNC_GetKSettings(lua_State* L){
    QString key(LuaModule::Instance()->GetStringArgument(1, NULL));
    QString type(LuaModule::Instance()->GetStringArgument(2, NULL));
    if(type == "Bool"){
        bool temp;
        ZSS::KParamManager::instance()->loadParam(temp,key);
        LuaModule::Instance()->PushBool(temp);
    }else if(type == "Int"){
        int temp;
        ZSS::KParamManager::instance()->loadParam(temp,key);
        LuaModule::Instance()->PushNumber(temp);
    }else if(type == "Double"){
        double temp;
        ZSS::KParamManager::instance()->loadParam(temp,key);
        LuaModule::Instance()->PushNumber(temp);
    }else{
        QString temp;
        ZSS::KParamManager::instance()->loadParam(temp,key);
        LuaModule::Instance()->PushString(temp.toLatin1());
    }
    return 1;
}

extern "C" int FUNC_PrintString(lua_State* L) {
    const char* str = LuaModule::Instance()->GetStringArgument(1, NULL);
    printf("%s\n",str);
    fflush(stdout);
    return 0;
}

extern "C" int FUNC_GetKickOffDefPos(lua_State* L)
{
	string str = LuaModule::Instance()->GetStringArgument(1,NULL);
	CGeoPoint pos;
	if ( "left" == str) {
		pos = KickOffDefPosV2::Instance()->GetLeftPos(vision);
	}

	if ( "right" == str) {
		pos = KickOffDefPosV2::Instance()->GetRightPos(vision);
	}

	if ( "middle" == str) {
		pos = KickOffDefPosV2::Instance()->GetMidPos(vision);
	}
	LuaModule::Instance()->PushNumber(pos.x());
	LuaModule::Instance()->PushNumber(pos.y());

	return 2;
}

extern "C" int FUNC_CalCompensate(lua_State*L){
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double targetX = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double targetY = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double realDir;
	realDir = Compensate::Instance()->getKickDir(runner,CGeoPoint(targetX,targetY));
	LuaModule::Instance()->PushNumber(realDir);
	return 1;
}

extern "C" int FUNC_AddPenaltyCleaner(lua_State*L){
	string role = LuaModule::Instance()->GetStringArgument(1,NULL);
	int num = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double targetX = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double targetY = LuaModule::Instance()->GetNumberArgument(4,NULL);
	PenaltyPosCleaner::Instance()->add(role,num, CGeoPoint(targetX,targetY));
	return 0;
}

extern "C" int FUNC_GetPenaltyCleaner(lua_State*L){
	string role = LuaModule::Instance()->GetStringArgument(1,NULL);
	CGeoPoint p = PenaltyPosCleaner::Instance()->get(role);
	LuaModule::Instance()->PushNumber(p.x());
	LuaModule::Instance()->PushNumber(p.y());
	return 2;
}

extern "C" int FUNC_CleanPenalty(lua_State*L){
	PenaltyPosCleaner::Instance()->clean(vision);
	return 0;
}

extern "C" int FUNC_GetResetGroup(lua_State*L){
	GroupStatus* status = PenaltyPosCleaner::Instance()->getGroupStatus();
	char str[20] = "";

	for(map<string, SGroup>::iterator iter = status->begin(); iter != status->end(); iter++){
		char tmpStr[8] = "[";
		for(int j = 0; j < iter->second._names.size(); j++){
			strcat(tmpStr, iter->second._names[j].c_str());
		}
		strcat(tmpStr, "]");
		strcat(str, tmpStr);
	}
	LuaModule::Instance()->PushString(str);
	return 1;
}

extern "C" int Skill_PenaltyKick2017V1(lua_State*L) {
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyKick2017V1(runner, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_PenaltyKick2017V2(lua_State*L) {
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyKick2017V2(runner, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int FUNC_CGetDefNumByRolename(lua_State* L){
	string rolename = LuaModule::Instance()->GetStringArgument(1, NULL);
	string str = EnemyDefendTacticAnalys::Instance()->doAnalys(rolename,vision);
	LuaModule::Instance()->PushString(str.c_str());
	return 1;
}

extern "C" int FUNC_CGetDefRolenameByNum(lua_State* L){
	int num = LuaModule::Instance()->GetNumberArgument(1, NULL);
	string str = EnemyDefendTacticAnalys::Instance()->doAnalys(num,vision);
	LuaModule::Instance()->PushString(str.c_str());
	return 1;
}

extern "C" int FUNC_EnemyHasReceiver(lua_State* L){
	bool istrue = false;
	for(int i = 0; i < PARAM::Field::MAX_PLAYER; i++) {
		if(DefenceInfo::Instance()->getOppPlayerByNum(i)->isTheRole("RReceiver")){
			istrue = true;
		}
	}
	LuaModule::Instance()->PushBool(istrue);
	return 1;
}

//extern "C" int FUNC_SetRoleAndNum(lua_State* L){
//	string rolename = LuaModule::Instance()->GetStringArgument(1, NULL);
//	int num = LuaModule::Instance()->GetNumberArgument(2, NULL);
//	PlayInterface::Instance()->setRoleNameAndNum(rolename, num);
//	return 0;
//}

extern "C" int Skill_SpeedInRobot(lua_State* L){
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double speedX = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double speedY = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double rotSpeed = LuaModule::Instance()->GetNumberArgument(4,NULL);
	CVector localVel(speedX, speedY);
	CVector globalVel = localVel.rotate(vision->ourPlayer(runner).Dir());
	DribbleStatus::Instance()->setDribbleCommand(runner,3);
	CPlayerTask* pTask = PlayerRole::makeItRun(runner, globalVel.x(), globalVel.y(), rotSpeed, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

//当出现receiver时，找到离球最近的receiver，并看他是否被绕前盯人
extern "C" int FUNC_IsNearestBallReceiverBeDenied(lua_State* L){
	bool bl = MarkingPosV2::Instance()->isNearestBallReceiverBeDenied(vision);
	LuaModule::Instance()->PushBool(bl);
	return 1;
}

//在NormalPlay中MarkingX值最靠前的车的匹配点
extern "C" int FUNC_MarkingXFirstPos(lua_State* L){
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	CGeoPoint pos = MarkingPosV2::Instance()->getMarkingPosByNum(vision,runner);

	LuaModule::Instance()->PushNumber(pos.x());
	LuaModule::Instance()->PushNumber(pos.y());

	return 2;
}

//在NormalPlay中Markingx值最靠前的车
extern "C" int FUNC_MarkingXFirstNum(lua_State* L){
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	int enemy = LuaModule::Instance()->GetNumberArgument(2,NULL);
	
	CPlayerTask* pTask = PlayerRole::makeItMarkEnemy(runner, enemy);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

// 从Lua中注册开球车朝向供Touch使用，防止球看不见时车会上前拿球
extern "C" int FUNC_SetPassDir(lua_State* L){
	double dir = LuaModule::Instance()->GetNumberArgument(1,NULL);
	TouchKickPos::Instance()->setPassDir(vision->getCycle(), dir);

	return 0;
}

extern "C" int FUNC_GetMarkingTouchPos(lua_State* L){
	int mAreaNum = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double x1 = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double y1 = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double x2 = LuaModule::Instance()->GetNumberArgument(4,NULL);
	double y2 = LuaModule::Instance()->GetNumberArgument(5,NULL);
	//cout<<x1<<" "<<y1<<" "<<x2<<" "<<y2<<endl;
	int markingDirection = LuaModule::Instance()->GetNumberArgument(6,NULL);
	CGeoPoint p = CGeoPoint(0,0);
	if (markingDirection == 1){
		p = MarkingTouchPos::Instance()->caculMarkingTouchPos(mAreaNum,CGeoPoint(x1,y1),CGeoPoint(x2,y2),true);
	}else{
		p = MarkingTouchPos::Instance()->caculMarkingTouchPos(mAreaNum,CGeoPoint(x1,y1),CGeoPoint(x2,y2),false);
	}
	//GDebugEngine::Instance()->gui_debug_msg(p,"P",COLOR_WHITE);
	LuaModule::Instance()->PushNumber(p.x());
	LuaModule::Instance()->PushNumber(p.y());

	return 2;
}

extern "C" int Skill_InterceptTouch(lua_State* L) {
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double touchDir = LuaModule::Instance()->GetNumberArgument(4, NULL);
	double power = LuaModule::Instance()->GetNumberArgument(5, NULL);
	double buffer = LuaModule::Instance()->GetNumberArgument(6, NULL);
	int useChip = LuaModule::Instance()->GetNumberArgument(7, NULL);
	bool testMode = LuaModule::Instance()->GetBoolArgument(8);
    double target_x = LuaModule::Instance()->GetNumberArgument(9, NULL);
    double target_y = LuaModule::Instance()->GetNumberArgument(10, NULL);
    CPlayerTask* pTask = PlayerRole::makeItInterceptTouch(runner, CGeoPoint(x, y), touchDir, power, buffer, useChip,testMode, CGeoPoint(target_x, target_y));
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_GoAndTurnKickV4(lua_State* L){
    double runner = LuaModule::Instance()->GetNumberArgument(1, 0.0);
    double x      = LuaModule::Instance()->GetNumberArgument(2, 0.0);
    double y      = LuaModule::Instance()->GetNumberArgument(3, 0.0);
    double flag   = LuaModule::Instance()->GetNumberArgument(4, 0.0);
    double power  = LuaModule::Instance()->GetNumberArgument(5, 0.0);
    double angle  = LuaModule::Instance()->GetNumberArgument(6, 0.0);
    int runner_int = static_cast<int>( runner );
    int flag_int   = static_cast<int>( flag );
    CGeoPoint pos = CGeoPoint(x, y);
    CPlayerTask* pTask = PlayerRole::makeItGoAndTurnKickV4(runner_int, pos,
                                                           flag_int, power,
                                                           angle);
    TaskMediator::Instance()->setPlayerTask(runner_int, pTask, 1);
    return 0;
}

extern "C" int Skill_ZDrag(lua_State* L){
    double runner = LuaModule::Instance()->GetNumberArgument(1, 0.0);
    double x      = LuaModule::Instance()->GetNumberArgument(2, 0.0);
    double y      = LuaModule::Instance()->GetNumberArgument(3, 0.0);
    double tx     = LuaModule::Instance()->GetNumberArgument(4, 0.0);
    double ty     = LuaModule::Instance()->GetNumberArgument(5, 0.0);
    int runner_int = static_cast<int>( runner );
    CGeoPoint pos = CGeoPoint(x, y);
    CGeoPoint target = CGeoPoint(tx, ty);
    CPlayerTask* pTask = PlayerRole::makeItZDrag(runner_int, pos, target);
    TaskMediator::Instance()->setPlayerTask(runner_int, pTask, 1);
    return 0;
}

extern "C" int Skill_ZBack(lua_State* L){
    int runner = LuaModule::Instance()->GetNumberArgument(1, 0.0);
    int guardNum = LuaModule::Instance()->GetNumberArgument(2, 0.0);
    int index = LuaModule::Instance()->GetNumberArgument(3, 0.0);
    double power = LuaModule::Instance()->GetNumberArgument(4, 0.0);
    int flag = LuaModule::Instance()->GetNumberArgument(5, 0.0);
    int defendNum = LuaModule::Instance()->GetNumberArgument(6, 0.0);
    if(defendNum < 0) defendNum = -1;
    CPlayerTask* pTask = PlayerRole::makeItZBack(runner, guardNum, index, power, flag, defendNum);
    TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
    GuardPos::Instance()->setBackNum(runner, index);
    return 0;
}

luaDef GUIGlue[] = 
{
    {"SmartGotoPos",		Skill_SmartGotoPoint},
	{"SimpleGotoPos",		Skill_SimpleGotoPoint},
    {"StopRobot",			Skill_Stop},
    {"CRegisterRole",		Register_Role},
    {"CGetBallV4",			Skill_GetBallV4},
    {"CZGetBallPos",        FUNC_ZGetBallPos},
    {"CStaticGetBall",		Skill_StaticGetBall},
    {"CSlowGetBall",        Skill_SlowGetBall},
	{"CChaseKick",			Skill_ChaseKick},
    {"CChaseKickV2",		Skill_ChaseKickV2},
    {"CZGoalie",			Skill_ZGoalie },
    {"CAdvanceBall",		Skill_AdvanceBallV2},
	{"CReceivePass",		Skill_ReceivePass},
	{"CMarking",			Skill_Marking },
	{"CZMarking",			Skill_ZMarking},
    {"CZBlocking",           Skill_ZBlocking},
	{"CTimeOut",			FUNC_TimeOut},
//	{"CGetRealNum",			FUNC_GetRealNum},
	{"CGetIsSimulation",	FUNC_GetIsSimulation},
    {"CGetSettings",        FUNC_GetSettings},
    {"CGetKSettings",       FUNC_GetKSettings},      //wyd test 9.21
    {"CPenaltyGoalieV2",    Skill_PenaltyGoalieV2},
	{"CPenaltyGoalie2017V2",Skill_PenaltyGoalie2017V2},
    {"CSpeed",				Skill_Speed},
    {"COpenSpeed",			Skill_OpenSpeed},
	{"CGoAndTurnKickV3",	Skill_GoAndTurnKickV3},
    {"CCalCompensateDir",	FUNC_CalCompensate},
	{"CKickOffDefPos",		FUNC_GetKickOffDefPos},
    {"CGetMarkingPos",      FUNC_GetMarkingPos},
    {"CGetZMarkingPos",     FUNC_GetZMarkingPos},
	{"CGetMarkingTouchPos",	FUNC_GetMarkingTouchPos},
    {"CGetZBlockingPos",     FUNC_GetZBlockingPos},
	{"CAddPenaltyCleaner",  FUNC_AddPenaltyCleaner},
	{"CGetPenaltyCleaner",  FUNC_GetPenaltyCleaner},
	{"CCleanPenalty",       FUNC_CleanPenalty},
	{"CGetResetMatchStr",	FUNC_GetResetGroup},
//    {"CSetRoleAndNum",		FUNC_SetRoleAndNum},
	{"CPenaltyKick2017V1",	Skill_PenaltyKick2017V1},
	{"CPenaltyKick2017V2",	Skill_PenaltyKick2017V2},
	{"CGoCmuRush",			Skill_GoCmuRush},
    {"CBezierRush",			Skill_BezierRush},
	{"CNoneZeroGoCmuRush",	Skill_NoneZeroGoCmuRush},
    {"CGoAvoidShootLine",	Skill_GoAvoidShootLine},
	{"CGetDefendNumByName", FUNC_CGetDefNumByRolename},
	{"CGetDefendNameByNum", FUNC_CGetDefRolenameByNum},
    {"CEnemyHasReceiver",	FUNC_EnemyHasReceiver},
	{"CGetBlockingPos",		FUNC_GetBlockingPos},
	{"CSpeedInRobot",		Skill_SpeedInRobot},
	{"CShouldAdvanceReceiver",FUNC_IsNearestBallReceiverBeDenied},
	{"CMarkingXFirstPos",	FUNC_MarkingXFirstPos},
	{"CMarkingXFirstNum",	FUNC_MarkingXFirstNum},
    {"CSetPassDir",			FUNC_SetPassDir},
    {"CZPass",          	Skill_ZPass},
    {"CFetchBall",          Skill_FetchBall},
    {"CZBreak",          	Skill_ZBreak},
    {"CZAttack",          	Skill_ZAttack},
    {"CZSupport",          	Skill_ZSupport},
    {"CHoldBall",           Skill_HoldBall},
    {"CZCirclePass",        Skill_ZCirclePass},
	{"CDribbleTurnKick",    Skill_DribbleTurnKick},
//    {"CGetStrategyNum",     FUNC_GetStrategyNum},
    {"CInterceptTouch",     Skill_InterceptTouch},
    {"CPrintString",        FUNC_PrintString},
    {"CGoAndTurnKickV4",    Skill_GoAndTurnKickV4},
    {"CZDrag",              Skill_ZDrag},
    {"CZBack",              Skill_ZBack},
	{NULL, NULL}
};
