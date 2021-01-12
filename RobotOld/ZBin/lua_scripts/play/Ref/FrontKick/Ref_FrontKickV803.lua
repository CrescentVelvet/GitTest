-- by hzy 6/29/2016
-- 模拟Immortal打MRL的定位球，挑禁区横向chase
-- FrontKickV1617改版 -- Mark
local function PASS_POS(x,y)
  return ball.refAntiYPos(CGeoPoint:new_local(ball.posX()+x,ball.refAntiY()*y))
end
local DSS_FLAG = flag.allow_dss +  flag.dodge_ball + flag.free_kick
local SHOOT_POS_OFFSET_X = -100
local SHOOT_POS_Y = 270
local FINAL_SHOOT_POS = CGeoPoint:new_local(400, 200)
local SHOOT_POS = ball.refAntiYPos(FINAL_SHOOT_POS)--PASS_POS(SHOOT_POS_OFFSET_X,SHOOT_POS_Y)
local SLOW_GET_BALL_FACE_POS = function()
  return CGeoPoint:new_local(ball.posX()+SHOOT_POS_OFFSET_X,SHOOT_POS_Y)
end
local FrontPosX = 200
local FrontPosY = -300
local Dist = 20

local PRE_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*0, FrontPosY+Dist*5)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*1, FrontPosY+Dist*4)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*2, FrontPosY+Dist*3)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*3, FrontPosY+Dist*2)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*4, FrontPosY+Dist*1)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*5, FrontPosY+Dist*0)),
}

local POS = {
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*0, FrontPosY+Dist*0)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*1, FrontPosY+Dist*1)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*2, FrontPosY+Dist*2)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*3, FrontPosY+Dist*3)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*4, FrontPosY+Dist*4)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*5, FrontPosY+Dist*5)),
}
local STATIC_SHOOT_POS = nil
local function RETURN_SHOOT_POS()
  return STATIC_SHOOT_POS
end

local WAIT_POS3 = ball.refSyntYPos(CGeoPoint:new_local(350, 300))

local USE_ZPASS = gOppoConfig.USE_ZPASS

local CHIP_BUF

gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 20,240) then
        return "start2"
      end
    end,
    Assister = task.staticGetBall(SHOOT_POS,false),
    Leader   = task.goCmuRush(PRE_POS[1],_,_, DSS_FLAG),
    Special  = task.goCmuRush(PRE_POS[2],_,_, DSS_FLAG),
    Middle   = task.goCmuRush(PRE_POS[3],_,_, DSS_FLAG),
    Fronter  = task.goCmuRush(PRE_POS[4],_,_, DSS_FLAG),
    Center   = task.goCmuRush(PRE_POS[5],_,_, DSS_FLAG),
    Defender = task.goCmuRush(PRE_POS[6],_,_, DSS_FLAG),
    Goalie   = task.zgoalie(),
    match    = "{A}[L][MSDFC]"
  },
  ["start2"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 20,20) then
        return "rush"
      end
    end,
    Assister = task.staticGetBall(SHOOT_POS,false),
    Leader   = task.goCmuRush(POS[1],_,_, DSS_FLAG),
    Special  = task.goCmuRush(POS[2],_,_, DSS_FLAG),
    Middle   = task.goCmuRush(POS[3],_,_, DSS_FLAG),
    Fronter  = task.goCmuRush(POS[4],_,_, DSS_FLAG),
    Center   = task.goCmuRush(POS[5],_,_, DSS_FLAG),
    Defender = task.goCmuRush(POS[6],_,_, DSS_FLAG),
    Goalie   = task.zgoalie(),
    match    = "{A}{LMSDFC}"
  },
  ["rush"] = {
    switch = function()
      if bufcnt(true,70) then
        STATIC_SHOOT_POS = SHOOT_POS()
        CHIP_BUF = cp.getFixBuf(SHOOT_POS)
        return "chip"
      end
    end,
    Assister = task.staticGetBall(SHOOT_POS,false),
    Leader   = task.goCmuRush(SHOOT_POS,_, _, DSS_FLAG),
    Special  = task.goCmuRush(POS[2],_, _, DSS_FLAG),
    Middle   = task.goCmuRush(POS[3],_, _, DSS_FLAG),
    Fronter  = task.goCmuRush(POS[4],_, _, DSS_FLAG),
    Center   = task.goCmuRush(POS[5],_, _, DSS_FLAG),
    Defender = task.goCmuRush(POS[6],_, _, DSS_FLAG),
    Goalie   = task.zgoalie(),
    match    = "{AL}[MSDFC]"
  },
  ["chip"] = {
    switch = function()
      if player.kickBall("Assister") or player.toBallDist("Assister")>30 then
        return "fix"
      elseif bufcnt(true,180) then
        return "exit"
      end
    end,
    Assister = task.zget(SHOOT_POS,_,_,flag.kick+flag.chip,true),--task.chipPass(FINAL_SHOOT_POS,_,false,true),
    Leader   = task.goCmuRush(SHOOT_POS,_, _, DSS_FLAG),
    Special  = task.goCmuRush(POS[2],_, _, DSS_FLAG),
    Middle   = task.goCmuRush(POS[3],_, _, DSS_FLAG),
    Fronter  = task.goCmuRush(POS[4],_, _, DSS_FLAG),
    Center   = task.goCmuRush(POS[5],_, _, DSS_FLAG),
    Defender = task.goCmuRush(POS[6],_, _, DSS_FLAG),
    Goalie   = task.zgoalie(),
    match    = "{AL}[MSDFC]"
  },
  ["fix"] = {
    switch = function()
      if bufcnt(true,CHIP_BUF) then
        return "shoot"
      end
    end,
    Assister = task.goCmuRush(WAIT_POS3,_, _, DSS_FLAG),
    Leader   = task.goCmuRush(SHOOT_POS,_, _, DSS_FLAG),
    Fronter  = task.goCmuRush(POS[4],_, _, DSS_FLAG),
    Center   = task.goCmuRush(POS[5],_, _, DSS_FLAG),
    Special  = task.defendHead(),
    Middle   = task.leftBack(),
    Defender = task.rightBack(),
    Goalie   = task.zgoalie(),
    match    = "{AL}[MSDFC]"
  },
  ["shoot"] = {
    switch = function()
        if USE_ZPASS and bufcnt(player.infraredOn("Leader"),3,200) then
                return "zshoot"
        elseif bufcnt(player.kickBall("Leader"), 1, 200) then
          return "exit"
        elseif bufcnt(true,200) then
          return "exit"
        end
    end,
    Assister = task.goCmuRush(WAIT_POS3,_, _, DSS_FLAG),
    Leader   = USE_ZPASS and task.receivePass("Assister",0) or task.zget(_, _, _, flag.kick),
    Fronter  = task.goCmuRush(POS[4],_, _, DSS_FLAG),
    Center   = task.goCmuRush(POS[5],_, _, DSS_FLAG),
    Special  = task.defendHead(),
    Middle   = task.leftBack(),
    Defender = task.rightBack(),
    Goalie   = task.zgoalie(),
    match    = "{AL}[MSDFC]"
  },
  ["zshoot"] = {
    switch = function()
            if bufcnt(player.kickBall("Leader"),1,200) then
                    return "exit"
            end
    end,
    Assister = task.goCmuRush(WAIT_POS3,_, _, DSS_FLAG),
    Leader   = task.zpass(),
    Fronter  = task.goCmuRush(POS[4],_, _, DSS_FLAG),
    Center   = task.goCmuRush(POS[5],_, _, DSS_FLAG),
    Special  = task.defendHead(),
    Middle   = task.leftBack(),
    Defender = task.rightBack(),
    Goalie   = task.zgoalie(),
    match    = "{AL}[SMFCD]"
  },
  name = "Ref_FrontKickV803",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}