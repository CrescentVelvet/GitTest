-- 两传一射 by Wang
-- 适合x<320的前场球

local HALF_TEST = gOppoConfig.IfHalfField

local CHIP_BUF

local PASS_POS1 = CGeoPoint:new_local(420, -120)
local WAIT_POS1 = ball.refAntiYPos(PASS_POS1)
local WAIT_POS2 = ball.refAntiYPos(CGeoPoint:new_local(280, 0))
local WAIT_POS3 = ball.refSyntYPos(CGeoPoint:new_local(350, 300))
local PASS_POS2 = pos.passForTouch(WAIT_POS3)

--local PASS_POS3 = ball.refSyntYPos(CGeoPoint:new_local(350, 120))
local PASS_POS2_TMP = ball.refSyntYPos(CGeoPoint:new_local(100, -120))

local FAKE_POS2 = ball.refSyntYPos(CGeoPoint:new_local(560, 180))
local FAKE_POS3 = ball.refSyntYPos(CGeoPoint:new_local(550, 140))
local Middle_POS1 = ball.refSyntYPos(CGeoPoint:new_local(350, 120))
local Middle_POS2 = ball.refSyntYPos(CGeoPoint:new_local(440, -320))
local Center_POS1 = ball.refSyntYPos(CGeoPoint:new_local(380, -40))
local Center_POS2 = ball.refSyntYPos(CGeoPoint:new_local(500, -170))

local DSS_FLAG = flag.allow_dss + flag.dodge_ball + flag.free_kick


gPlayTable.CreatePlay{
firstState = "start",

["start"] = {
        switch = function()
                if  bufcnt(player.toTargetDist("Leader") < 20, 20) then
                        return "fakeAction"
                end
        end,
        Assister = task.staticGetBall(PASS_POS1),--task.goBackBallV2(0, 10),
        Leader   = task.goSpeciPos(WAIT_POS1, player.toPlayerDir("Assister"), DSS_FLAG),
        Breaker  = task.goSpeciPos(PASS_POS2_TMP, _, DSS_FLAG),
        Special  = task.goSpeciPos(FAKE_POS2, _, DSS_FLAG),
        Middle   = task.goSpeciPos(Middle_POS1, _, DSS_FLAG),
        Center   = task.goSpeciPos(Center_POS1, _, DSS_FLAG),
        Defender = task.singleBack(),
        Goalie   = task.zgoalie(),
        match    = "{A}[LB][SDMC]"
},

["fakeAction"] = {
        switch = function()
                if  bufcnt(true, 30) then
                        CHIP_BUF = cp.getFixBuf(WAIT_POS1)
                        return "pass1"
                end
        end,
        Assister = task.staticGetBall(PASS_POS1),--task.goBackBallV2(0, 10),
        Leader   = task.goSpeciPos(WAIT_POS1, player.toPlayerDir("Assister"), DSS_FLAG),
        Breaker  = task.continue(),
        Special  = task.goSpeciPos(FAKE_POS2, _, DSS_FLAG),
        Middle   = task.goSpeciPos(Middle_POS1, _, DSS_FLAG),
        Center   = task.goSpeciPos(Center_POS1, _, DSS_FLAG),
        Defender = task.singleBack(),
        Goalie   = task.zgoalie(),
        match    = "{ALB}{SDMC}"
},

["pass1"] = {
        switch = function()
                if  bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 30, 1, 150) then
                        return "fix1"
                end
        end,
        Assister = task.zget(WAIT_POS1,_,_,flag.kick+flag.chip+flag.dribble),--task.chipPass(PASS_POS1),
        Leader   = task.goSpeciPos(WAIT_POS1, player.toPlayerDir("Assister"),  DSS_FLAG),
        Breaker  = task.continue(),--task.goSpeciPos(WAIT_POS2, _,  DSS_FLAG),
        Special  = task.goSpeciPos(FAKE_POS2, _,  DSS_FLAG),
        Middle   = task.goSpeciPos(Middle_POS1, _,   DSS_FLAG),
        Center   = task.goSpeciPos(Center_POS1, _, DSS_FLAG),
        Defender = task.singleBack(),
        Goalie   = task.zgoalie(),
        match    = "{ALB}{SDMC}"
},

["fix1"] = {
        switch = function()
                if  bufcnt(true, CHIP_BUF) then
                        return "pass2"
                end
        end,
        Assister = task.goSpeciPos(WAIT_POS3, _, DSS_FLAG),
        Leader   = task.goSpeciPos(WAIT_POS1, player.toPlayerDir("Assister"), flag.allow_dss + flag.free_kick + flag.not_avoid_our_vehicle),
        Breaker  = task.continue(),--task.goSpeciPos(WAIT_POS2, _,  DSS_FLAG),
        Special  = task.goSpeciPos(FAKE_POS2, _,  DSS_FLAG),
        Middle   = task.goSpeciPos(Middle_POS1, _,   DSS_FLAG),
        Center   = task.goSpeciPos(Center_POS1, _, DSS_FLAG),
        Defender = task.singleBack(),
        Goalie   = task.zgoalie(),
        match    = "{ALB}{SDMC}"
},

["pass2"] = {
        switch = function()
                if  bufcnt(player.kickBall("Leader"),1,150) then
                --if  bufcnt(player.kickBall("Leader") or player.toBallDist("Leader") > 30, 1, 150) then
                        return "shoot"
                elseif bufcnt(true,150) then
                        return "exit"
                end
        end,
        Assister = task.goSpeciPos(WAIT_POS3, _, DSS_FLAG),
        Leader   = task.zget(PASS_POS2,_,_,flag.kick),
        Breaker  = task.goSpeciPos(WAIT_POS2, _, flag.allow_dss + flag.free_kick + flag.not_avoid_our_vehicle),
        Special  = task.goSpeciPos(FAKE_POS2, _, DSS_FLAG),
        Middle   = task.goSpeciPos(Middle_POS2, _,   DSS_FLAG),
        Center   = task.leftBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "{ALB}{SDMC}"
},

["fix2"] = {
        switch = function()
                if  bufcnt(true, cp.getFixBuf(PASS_POS2)) then
                        return "shoot"
                end
        end,
        Assister = task.goSpeciPos(WAIT_POS3, _, DSS_FLAG),
        Leader   = task.goSpeciPos(FAKE_POS3, _, DSS_FLAG),
        Breaker  = task.goSpeciPos(WAIT_POS2, _, flag.allow_dss + flag.free_kick + flag.not_avoid_our_vehicle),
        Special  = task.goSpeciPos(FAKE_POS2, _, DSS_FLAG),
        Middle   = task.goSpeciPos(Middle_POS1, _,   DSS_FLAG),
        Center   = task.leftBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "{ALB}{SDMC}"
},

["shoot"] = {
    switch = function ()
                if bufcnt(player.kickBall("Breaker"), 1, 150) then
                        return "exit"
                end
        end,
        Assister = task.zget(_,_,_,flag.kick),
        Leader   = task.goSpeciPos(FAKE_POS3, _, DSS_FLAG),
        Breaker  = task.goSpeciPos(WAIT_POS2, _, flag.allow_dss + flag.free_kick + flag.not_avoid_our_vehicle),
        Special  = task.goSpeciPos(FAKE_POS2, _, DSS_FLAG),
        Middle   = task.goSpeciPos(Middle_POS1, _,   DSS_FLAG),
        Center   = task.leftBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "{ALB}{SDMC}"
},

name = "Ref_CornerKickV805",
applicable = {
        exp = "a",
        a = true
},
attribute = "attack",
timeout   = 99999
}
