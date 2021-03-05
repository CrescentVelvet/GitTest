IS_TEST_MODE = false
IS_SIMULATION = CGetIsSimulation()
USE_SWITCH = false
USE_AUTO_REFEREE = false
OPPONENT_NAME = "Other"
SAO_ACTION = CGetSettings("Alert/SaoAction","Int")
IS_YELLOW = CGetSettings("ZAlert/IsYellow","Bool")
IS_RIGHT = CGetSettings("ZAlert/IsRight", "Bool")
DEBUG_MATCH = CGetSettings("Debug/RoleMatch","Bool")

gStateFileNameString = string.format(os.date("%Y%m%d%H%M"))

gTestPlay = "TestRun"

gRoleFixNum = {
        ["Kicker"]   = {},
        ["Goalie"]   = {},
        ["Tier"]     = {},
        -- ["Receiver"] = {} match before rolematch in messi by wangzai
}

-- 用来进行定位球的保持
-- 在考虑智能性时用table来进行配置，用于OurIndirectKick
gOurIndirectTable = {
        -- 在OurIndirectKick控制脚本中可以进行改变的值
        -- 上一次定位球的Cycle
        lastRefCycle = 0
}

gSkill = {
        "SmartGoto",
        "SimpleGoto",
        "RunMultiPos",
        "Stop",
        "GetBallV4",
        "StaticGetBall",
        "GoAndTurnKickV3",
        "SlowGetBall",
        "ChaseKick",
        "ChaseKickV2",
        "AdvanceBall",
        "ReceivePass",
        "OpenSpeed",
        "Speed",
        "PenaltyGoalieV2",
        "PenaltyGoalie2017V1",
        "Marking",
        "ZMarking",
        "ZBlocking",
        "ZGoalie",
        "GotoMatchPos",
        "PenaltyKick2017V1",
        "PenaltyKick2017V2",
        "GoCmuRush",
        "ChinaTecRun",
        "BezierRush",
        "NoneZeroRush",
        "SpeedInRobot",
        "GoAvoidShootLine",
        "ZPass",
        "ZSupport",
        "ZBreak",
        "ZAttack",
        "ZCirclePass",
        "DribbleTurnKick",
        "InterceptTouch",
        "HoldBall",
        "GoAndTurnKickV4",
        "ZDrag",
        "FetchBall",
        "ZBack"
}

gRefPlayTable = {
        "Ref/Ref_HaltV1",
        "Ref/Ref_OurTimeoutV1",
        "Ref/GameStop/Ref_StopZMarking",
        "Ref/GameStop/Ref_StopZMarking_6vs6",
-- BallPlacement
        "Ref/BallPlacement/Ref_BallPlace2Stop",
        "Ref/BallPlacement/Ref_BallPlace2Stop_6vs6",
-- Penalty
        "Ref/PenaltyDef/Ref_PenaltyDefV1",
        "Ref/PenaltyDef/Ref_PenaltyDefV1_6V6",
        "Ref/PenaltyDef/Ref_PenaltyDef2017V1",
        "Ref/PenaltyKick/Ref_PenaltyKickV3",
        "Ref/PenaltyKick/Ref_PenaltyKickV3_6V6",
        "Ref/PenaltyKick/Ref_PenaltyKickV4",
        "Ref/PenaltyKick/Ref_PenaltyKick2017V1",
        "Ref/PenaltyKick/Ref_PenaltyKick2017V2",
-- KickOff
        "Ref/KickOffDef/Ref_KickOffDefV1",
        "Ref/KickOffDef/Ref_KickOffDef_6vs6",
        "Ref/KickOff/Ref_KickOffV801",
        "Ref/KickOff/Ref_KickOffV601",
-- FreeKickDef
        "Ref/CornerDef/Ref_CornerDefV1",
        "Ref/CornerDef/Ref_CornerDefV2",
        "Ref/CornerDef/Ref_CornerDef_6vs6",
        "Ref/FrontDef/Ref_FrontDefV1",
        "Ref/FrontDef/Ref_FrontDef_6vs6",
        "Ref/MiddleDef/Ref_MiddleDefV1",
        "Ref/MiddleDef/Ref_MiddleDef_6vs6",
        "Ref/BackDef/Ref_BackDefV1",
        "Ref/BackDef/Ref_BackDef_6vs6",
-- FreeKick
        "Ref/FrontKick/Ref_FrontKickV801",
        "Ref/FrontKick/Ref_FrontKickV802",
        "Ref/FrontKick/Ref_FrontKickV803",
        "Ref/FrontKick/Ref_FrontKickV804",
        "Ref/FrontKick/Ref_FrontKickV805",
        "Ref/FrontKick/Ref_FrontKickV806",
        -- "Ref/FrontKick/Ref_FrontKickV807",
        "Ref/FrontKick/Ref_FrontKickV1901",

        "Ref/BackKick/Ref_BackKickV801",
        "Ref/BackKick/Ref_BackKickV802",

        "Ref/MiddleKick/Ref_MiddleKickV801",
        "Ref/MiddleKick/Ref_MiddleKickV802",
        "Ref/MiddleKick/Ref_MiddleKickV803",

        -- "Ref/CornerKick/Ref_CornerKickV666",
        "Ref/CornerKick/Ref_CornerKickV801",
        "Ref/CornerKick/Ref_CornerKickV802",
        "Ref/CornerKick/Ref_CornerKickV803",
        "Ref/CornerKick/Ref_CornerKickV804",
        "Ref/CornerKick/Ref_CornerKickV805",
}

gBayesPlayTable = {
        -- "Nor/NormalPlayPP",
        "Nor/NormalPlayZ", 
        "Nor/NormalPlayZ6", 
        "Nor/NormalPlayMessi",
        "Nor/NormalPlayMessi_6vs6"
        -- "Nor/NormalPlay4"
}

gTestPlayTable = {
        -- "Test/RunMilitaryBoxing",
        -- "Test/TestDynamicBackKick",
        -- "Test/TestDynamicBackKick2",
        -- "Test/TestDynamicKick",
        -- "Test/TestDynamicKickJamVersion",
        -- "Test/TestDynamicKickPickVersion",
        -- "Test/TestForCompensate",
        -- "Test/TestForFriction"
        -- "Test/TestTEB",
        "Test/TestForFrictionAuto",
        "Test/TestRun_autoshootfit",
        "Test/RunKickParamTest",
        "Test/TestRun",
        "Test/TestRegulation",
        "Test/TestDemo",
        "Test/TestScript",
        -- "Test/TestMatch",
        "Test/TestDSS",
        "Test/TestFreeKick",
        "Test/TestFreeKick_6vs6",
        "Test/TestFreeKick_6vs6_v2",
        "Test/TestFreeKick_use_chip_predict",
        "Test/TestSkill",
        "Test/TestForChip",
        "Test/TestForBallState",
        -- "Test/TestZbreak",
        "Test/TestKeke"
        -- "Test/TestMultiple",
        -- "Test/TestSkill2",
}
