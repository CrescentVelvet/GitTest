local USE_ZPASS = gOppoConfig.USE_ZPASS

local WAIT_POS = ball.refSyntYPos(CGeoPoint:new_local(485, -135))

local FAKE_POS1 = ball.refSyntYPos(CGeoPoint:new_local(40, -120))
local FAKE_POS2 = ball.refSyntYPos(CGeoPoint:new_local(320, -280))
local FAKE_POS3 = ball.refSyntYPos(CGeoPoint:new_local(595, -170))

local PULLING_POS1 = ball.refSyntYPos(CGeoPoint:new_local(320, -330))
local PULLING_POS2 = ball.refSyntYPos(CGeoPoint:new_local(580, -300))

local STATIC_POS1 = ball.refSyntYPos(CGeoPoint:new_local(400, 165))
local STATIC_POS2 = ball.refSyntYPos(CGeoPoint:new_local(345, 300))
local STATIC_POS3 = ball.refSyntYPos(CGeoPoint:new_local(345, 0))
local STATIC_POS4 = ball.refSyntYPos(CGeoPoint:new_local(300, -170))

local WAIT_POS3 = ball.refSyntYPos(CGeoPoint:new_local(350, 300))

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball) + flag.free_kick
local PASS_POS = pos.passForTouch(WAIT_POS)

local CHIP_BUF

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
gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
  switch = function()
    if  bufcnt(player.toTargetDist("Leader") < 20, 20) then
      return "fakeAction1"
    end
  end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(FAKE_POS1 , _, _, DSS_FLAG),
    Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS1, _, _, DSS_FLAG),
    Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS2, _, _, DSS_FLAG),
    Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS3, _, _, DSS_FLAG),
    Center   = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS4, _, _, DSS_FLAG),
    Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(PULLING_POS1, _, _, DSS_FLAG),
    Goalie   = task.zgoalie(),
    match    = "(AL)(F)[SMDC]"
  },

  ["fakeAction1"] = {
  switch = function()
    if  bufcnt(player.toTargetDist("Leader") < 20, 30) then
      return "fakeAction2"
    end
  end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(FAKE_POS2 , _, _, DSS_FLAG),
    Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS1, _, _, DSS_FLAG),
    Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS2, _, _, DSS_FLAG),
    Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS3, _, _, DSS_FLAG),
    Center   = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS4, _, _, DSS_FLAG),
    Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(PULLING_POS2, _, _, DSS_FLAG),
    Goalie   = task.zgoalie(),
    match    = "{AL}{F}[SMDC]"
  },

  ["fakeAction2"] = {
  switch = function()
    if  bufcnt(player.toTargetDist("Leader") < 30, 10) then
      CHIP_BUF = cp.getFixBuf(WAIT_POS)
      return "waitchip"
    end
  end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(FAKE_POS3 , _, _, flag.allow_dss + flag.not_avoid_our_vehicle + flag.free_kick),
    Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS1, _, _, DSS_FLAG),
    Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS2, _, _, DSS_FLAG),
    Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS3, _, _, DSS_FLAG),
    Center   = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS4, _, _, DSS_FLAG),
    Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(PULLING_POS2, _, _, flag.allow_dss + flag.not_avoid_our_vehicle + flag.free_kick),
    Goalie   = task.zgoalie(),
    match    = "{AL}{F}{SMDC}"
  },
  ["waitchip"] = {
    switch = function()
      if bufcnt(true,5) then
        return "pass"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(FAKE_POS3 , _, _, flag.allow_dss + flag.not_avoid_our_vehicle + flag.free_kick),
    Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS1, _, _, DSS_FLAG),
    Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS2, _, _, DSS_FLAG),
    Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS3, _, _, DSS_FLAG),
    Center   = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS4, _, _, DSS_FLAG),
    Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(PULLING_POS2, _, _, flag.allow_dss + flag.not_avoid_our_vehicle + flag.free_kick),
    Goalie   = task.zgoalie(),
    match    = "{AL}{F}{SMDC}"
  },

  ["pass"] = {
  switch = function()
    if  bufcnt(player.kickBall("Assister"), 1) then
      return "fix"
    elseif bufcnt(true,150) then
      return "exit"
    end
  end,
    Assister = task.zget(WAIT_POS,_,_,flag.kick+flag.chip,true),--task.chipPass(WAIT_POS,_,false,false),
    Leader   = task.goCmuRush(FAKE_POS3 , _, _, flag.allow_dss + flag.not_avoid_our_vehicle + flag.free_kick),
    Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS1, _, _, DSS_FLAG),
    Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS2, _, _, DSS_FLAG),
    Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS3, _, _, DSS_FLAG),
    Center   = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS4, _, _, DSS_FLAG),
    Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(PULLING_POS2, _, _, flag.allow_dss + flag.not_avoid_our_vehicle + flag.free_kick),
    Goalie   = task.zgoalie(),
    match    = "{AL}{F}{SMDC}"
  },

  ["fix"] = {
  switch = function()
    if  bufcnt(true, CHIP_BUF) then
      return "shoot"
    end
  end,
    Assister = HALF_TEST and task.stop() or task.goSpeciPos(WAIT_POS3, _, DSS_FLAG),
    Leader   = task.goCmuRush(WAIT_POS),
    Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS1, _, _, DSS_FLAG),
    Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,DSS_FLAG) or task.leftBack(),--goCmuRush(STATIC_POS2, _, _, DSS_FLAG),
    Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,DSS_FLAG) or task.rightBack(),--goCmuRush(STATIC_POS3, _, _, DSS_FLAG),
    Center   = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS4, _, _, DSS_FLAG),
    Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,DSS_FLAG) or task.defendHead(),
    Goalie   = task.zgoalie(),
    match    = "{AL}[FSMDC]"
  },

  ["shoot"] = {
    switch = function ()
        if USE_ZPASS and bufcnt(player.infraredOn("Leader"),3,200) then
                return "zshoot"
        elseif bufcnt(player.kickBall("Leader"), 1, 200) then
          return "exit"
        elseif bufcnt(true,200) then
          return "exit"
        end
    end,
    Assister = HALF_TEST and task.stop() or task.goSpeciPos(WAIT_POS3, _, DSS_FLAG),
    Leader   = USE_ZPASS and task.receivePass("Assister",0) or task.InterTouch(),
    Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS1, _, _, DSS_FLAG),
    Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,DSS_FLAG) or task.leftBack(),--goCmuRush(STATIC_POS2, _, _, DSS_FLAG),
    Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,DSS_FLAG) or task.rightBack(),--goCmuRush(STATIC_POS3, _, _, DSS_FLAG),
    Center   = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS4, _, _, DSS_FLAG),
    Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,DSS_FLAG) or task.defendHead(),
    Goalie   = task.zgoalie(),
    match    = "{AL}{F}[SMDC]"
  },
["zshoot"] = {
        switch = function()
                if bufcnt(player.kickBall("Leader"),1,200) then
                        return "exit"
                end
        end,
    Assister = HALF_TEST and task.stop() or task.goSpeciPos(WAIT_POS3, _, DSS_FLAG),
    Leader   = task.zpass(),
    Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS1, _, _, DSS_FLAG),
    Middle   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,DSS_FLAG) or task.leftBack(),--goCmuRush(STATIC_POS2, _, _, DSS_FLAG),
    Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,DSS_FLAG) or task.rightBack(),--goCmuRush(STATIC_POS3, _, _, DSS_FLAG),
    Center   = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(STATIC_POS4, _, _, DSS_FLAG),
    Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,DSS_FLAG) or task.defendHead(),
    Goalie   = task.zgoalie(),
    match    = "{AL}[SMFCD]"
},

  name = "Ref_CornerKickV801",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}