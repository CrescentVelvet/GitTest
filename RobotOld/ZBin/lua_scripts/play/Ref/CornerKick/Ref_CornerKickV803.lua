--一传一射 by Wang
local HALF_POS = {
  ball.refSyntYPos(CGeoPoint:new_local(40, 90)),
  ball.refSyntYPos(CGeoPoint:new_local(40, 60)),
  ball.refSyntYPos(CGeoPoint:new_local(40, 30)),
  ball.refSyntYPos(CGeoPoint:new_local(40, 0)),
  ball.refSyntYPos(CGeoPoint:new_local(40, -30)),
  ball.refSyntYPos(CGeoPoint:new_local(40, -60)),
  ball.refSyntYPos(CGeoPoint:new_local(40, -90))
}
local HALF_TEST = gOppoConfig.IfHalfField

local USE_ZPASS = false

local FAKE_POS_Y = 205
local FrontPosX = 300
local FrontPosY = 100
local Dist = 20

local FAKE_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*1, FrontPosY+Dist*4)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*2, FrontPosY+Dist*3)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*3, FrontPosY+Dist*2)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*4, FrontPosY+Dist*1)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*5, FrontPosY+Dist*0)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX+Dist*0, FrontPosY+Dist*5)),
}

local START_POS={
  ball.refSyntYPos(CGeoPoint:new_local(50,  50)),
  ball.refSyntYPos(CGeoPoint:new_local(50,  30)),
  ball.refSyntYPos(CGeoPoint:new_local(50,  10)),
  ball.refSyntYPos(CGeoPoint:new_local(50, -10)),
  ball.refSyntYPos(CGeoPoint:new_local(50, -30)),
  ball.refSyntYPos(CGeoPoint:new_local(50, -50)),
}
local PULLING_POS = ball.refSyntYPos(CGeoPoint:new_local(595, -280))

local WAIT_POS = ball.refSyntYPos(CGeoPoint:new_local(500, -150))--ball.refSyntYPos(CGeoPoint:new_local(500, -140))
local PASS_POS = pos.passForTouch(WAIT_POS)

local CHIP_BUF

gPlayTable.CreatePlay{

firstState = "init",
["init"] = {
  switch = function()
    CHIP_BUF = cp.getFixBuf(PASS_POS)
    return "start"
  end,
  Assister = task.staticGetBall(PASS_POS),
  Special  = task.goCmuRush(WAIT_POS, _, _, flag.allow_dss + flag.free_kick),
  Leader   = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[2], _, _, flag.allow_dss + flag.free_kick),
  Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[3], _, _, flag.allow_dss + flag.free_kick),
  Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[4], _, _, flag.allow_dss + flag.free_kick),
  Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[5], _, _, flag.allow_dss + flag.free_kick),
  Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[1], _, _, flag.allow_dss + flag.free_kick),
  Goalie   = task.zgoalie(),
  match    = "(A)[S][LMDFC]"
},
["start"] = {
  switch = function()
    if  bufcnt(player.toTargetTime(WAIT_POS,"Special")*param.frameRate - CHIP_BUF < 0, 5) then
      return "pass"
    end
  end,
  Assister = task.staticGetBall(PASS_POS),
  Special  = task.goCmuRush(WAIT_POS, _, _, flag.allow_dss + flag.free_kick),
  Leader   = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[2], _, _, flag.allow_dss + flag.free_kick),
  Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[3], _, _, flag.allow_dss + flag.free_kick),
  Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[4], _, _, flag.allow_dss + flag.free_kick),
  Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[5], _, _, flag.allow_dss + flag.free_kick),
  Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[1], _, _, flag.allow_dss + flag.free_kick),
  Goalie   = task.zgoalie(),
  match    = "(A)[S][LMDFC]"
},
["pass"] = {
  switch = function()
    if  bufcnt(player.kickBall("Assister"), 1, 120) then
      return "fix"
    end
  end,
  Assister = task.zget(PASS_POS,_,_,flag.kick+flag.chip,true),--task.chipPass(PASS_POS,_,false,false),
  Special  = task.goCmuRush(WAIT_POS, _, _, flag.allow_dss + flag.free_kick),
  Leader   = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[2], _, _, flag.allow_dss + flag.free_kick),
  Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[3], _, _, flag.allow_dss + flag.free_kick),
  Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[4], _, _, flag.allow_dss + flag.free_kick),
  Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[5], _, _, flag.allow_dss + flag.free_kick),
  Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(FAKE_POS[1], _, _, flag.allow_dss + flag.free_kick),
  Goalie   = task.zgoalie(),
  match    = "{AS}[MLDFC]"
},

["fix"] = {
  switch = function()
    if bufcnt(true, CHIP_BUF) then
      return "shoot"
    end
  end,
  Assister = HALF_TEST and task.stop() or task.defendMiddle(),
  Special  = task.goCmuRush(WAIT_POS, _, _, flag.allow_dss + flag.free_kick),
  Leader   = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss + flag.free_kick) or task.leftBack(),
  Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss + flag.free_kick) or task.rightBack(),
  Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss + flag.free_kick) or task.defendHead(),
  Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(CGeoPoint:new_local(-350, -230), _, _, flag.allow_dss + flag.free_kick),
  Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss + flag.free_kick) or task.goCmuRush(CGeoPoint:new_local(-350, 230), _, _, flag.allow_dss + flag.free_kick),
  Goalie   = task.zgoalie(),
  match    = "{S}[AMLDFC]"
},

["shoot"] = {
  switch = function ()
      if USE_ZPASS and bufcnt(player.infraredOn("Special"),3,200) then
          return "zshoot"
      elseif bufcnt(player.kickBall("Special"), 1, 200) then
        return "exit"
      end 
  end,
  Assister = HALF_TEST and task.stop() or task.defendMiddle(),
  Special  = USE_ZPASS and task.receivePass("Assister",0) or task.shoot(),
  Leader   = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss + flag.free_kick) or task.continue(),
  Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss + flag.free_kick) or task.continue(),
  Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss + flag.free_kick) or task.continue(),
  Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss + flag.free_kick) or task.continue(),
  Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss + flag.free_kick) or task.continue(),
  Goalie   = task.zgoalie(),
  match    = "{S}[AMLDFC]"
},
["zshoot"] = {
    switch = function()
        if bufcnt(player.kickBall("Special"),1,200) then
            return "exit" 
        end
    end,
    Assister = HALF_TEST and task.stop() or task.defendMiddle(),
    Special  = task.zpass(),--USE_ZPASS and task.receivePass("Assister",0) or task.zget(_, _, _, flag.kick),
    Leader   = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss + flag.free_kick) or task.continue(),
    Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss + flag.free_kick) or task.continue(),
    Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss + flag.free_kick) or task.continue(),
    Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss + flag.free_kick) or task.continue(),
    Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss + flag.free_kick) or task.continue(),
    Goalie   = task.zgoalie(),
    match    = "{S}[ASMFCD]"
},
name = "Ref_CornerKickV803",
applicable = {
  exp = "a",
  a   = true
},
score = 0,
attribute = "attack",
timeout   = 99999
}