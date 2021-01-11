
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


local Left_POS1     = ball.refAntiYPos(CGeoPoint:new_local(400, -40))
local Left_POS2     = ball.refAntiYPos(CGeoPoint:new_local(375, -40))
local Left_POS3     = HALF_TEST and ball.refAntiYPos(CGeoPoint:new_local(200, -40)) or ball.refAntiYPos(CGeoPoint:new_local(-200, -40))
local Right_POS1    = ball.refAntiYPos(CGeoPoint:new_local(400, 40))
local Right_POS2    = ball.refAntiYPos(CGeoPoint:new_local(375, 40))
local Right_POS3    = ball.refAntiYPos(CGeoPoint:new_local(350, 40))

local RUN_POS_1    = {
  ball.refAntiYPos(CGeoPoint:new_local(400, -380)),
  ball.refAntiYPos(CGeoPoint:new_local(400, -60 )),
  ball.refAntiYPos(CGeoPoint:new_local(450, -260)),
}

local RUN_POS_2    = {
  ball.refAntiYPos(CGeoPoint:new_local(375, -380)),
  ball.refAntiYPos(CGeoPoint:new_local(375, -60 )),
  ball.refAntiYPos(CGeoPoint:new_local(425, -380)),
}

local RUN_POS_3    = {
  ball.refAntiYPos(CGeoPoint:new_local(400, 380)),
  ball.refAntiYPos(CGeoPoint:new_local(400, 60 )),
  ball.refAntiYPos(CGeoPoint:new_local(450, 380)),
}

local RUN_POS_4    = {
  ball.refAntiYPos(CGeoPoint:new_local(375, 380)),
  ball.refAntiYPos(CGeoPoint:new_local(375, 60 )),
  ball.refAntiYPos(CGeoPoint:new_local(425, 380)),
}

local RUN_POS_5    = {
  ball.refAntiYPos(CGeoPoint:new_local(350, 380)),
  ball.refAntiYPos(CGeoPoint:new_local(350, 60 )),
  ball.refAntiYPos(CGeoPoint:new_local(400, 380)),
}

local Block_POS_1   = ball.refAntiYPos(CGeoPoint:new_local(40, -140))
local Block_POS_2   = ball.refAntiYPos(CGeoPoint:new_local(250, -30))

local SHOOT_POS =  ball.refAntiYPos(CGeoPoint:new_local(380,100))
local PASS_POS = pos.passForTouch(SHOOT_POS)

local CHIP_BUF

gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Middle") < 20 and
              player.toTargetDist("Special") < 20, 120, 180) then
      return "wait"
    end
  end,
  Assister = task.staticGetBall(PASS_POS),
  Middle   = task.goCmuRush(Left_POS3, _, _, flag.allow_dss +  flag.free_kick),
  Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss +  flag.free_kick) or task.goCmuRush(Left_POS1, _, _, flag.allow_dss +  flag.free_kick),
  Leader   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss +  flag.free_kick) or task.goCmuRush(Left_POS2, _, _, flag.allow_dss +  flag.free_kick),
  Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss +  flag.free_kick) or task.goCmuRush(Right_POS1, _, _, flag.allow_dss +  flag.free_kick),
  Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss +  flag.free_kick) or task.goCmuRush(Right_POS2, _, _, flag.allow_dss +  flag.free_kick),
  Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss +  flag.free_kick) or task.goCmuRush(Right_POS3, _, _, flag.allow_dss +  flag.free_kick),
  Goalie   = task.zgoalie(),
  match    = "(A)(M)[LSDFC]"
},
["wait"] = {
  switch = function()
    if bufcnt(true, 280) then
      return "goalone"
    end
  end,
  Assister = task.staticGetBall(PASS_POS),
  Middle   = task.goCmuRush(Block_POS_1),
  Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss +  flag.free_kick) or task.runMultiPos(RUN_POS_1, false, 25),
  Leader   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss +  flag.free_kick) or task.runMultiPos(RUN_POS_2, false, 10),
  Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss +  flag.free_kick) or task.runMultiPos(RUN_POS_3, false, 40),
  Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss +  flag.free_kick) or task.runMultiPos(RUN_POS_4, false, 55),
  Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss +  flag.free_kick) or task.runMultiPos(RUN_POS_5, false, 60),
  Goalie   = task.zgoalie(),
  match    = "{ASLMDFC}"
},
["goalone"] = {
  switch = function()
    if bufcnt(true, 70) then
      CHIP_BUF = cp.getFixBuf(PASS_POS)
      return "pass"
    end
  end,    
  Assister = task.staticGetBall(PASS_POS),--task.slowGetBall(PASS_POS),--Assister = task.slowGetBall(PASS_POS),
  Middle   = task.goCmuRush(SHOOT_POS, _,_, flag.not_avoid_our_vehicle), --runMultiPos(RUN_POS_2, false, 40),
  Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Leader   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss +  flag.free_kick) or task.continue(), --runMultiPos(RUN_POS_1, false, 30),
  Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Goalie   = task.zgoalie(),
  match    = "{ALMSDFC}"
},    

["pass"] = {
  switch = function()
    if player.kickBall("Assister") or player.toBallDist("Assister") > 35 then
      return "fix"
    elseif bufcnt(true, 120) then
      return "exit"
    end
  end,
  Assister = task.zget(PASS_POS,_,_,flag.kick+flag.chip,true),--task.chipPass(PASS_POS),
  Middle   = task.goCmuRush(SHOOT_POS, _,_, flag.allow_dss + flag.free_kick +flag.not_avoid_our_vehicle),--task.goCmuRush(SHOOT_POS),--task.goCmuRush(PASS_POS),
  Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss +  flag.free_kick) or task.leftBack(),
  Leader   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss +  flag.free_kick) or task.rightBack(),
  Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss +  flag.free_kick) or task.defendHead(),
  Goalie   = task.zgoalie(),
  match    = "{ALMF}[CSD]"
},
["fix"] = {
  switch = function()
    if bufcnt(true,CHIP_BUF) then
      return "kick"
    end
  end,
  Assister = HALF_TEST and task.stop() or task.zsupport(),
  Middle   = task.continue(),--task.goCmuRush(SHOOT_POS),
  Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Leader   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Goalie   = task.zgoalie(),
  match    = "{MALF}[CSD]"
},
["kick"] = {
  switch = function()
    if bufcnt(player.infraredOn("Middle"),3,200) then
            return "zshoot"
    elseif bufcnt(true,200) then
        return "exit"
    end
  end,
  Assister = HALF_TEST and task.stop() or task.zsupport(),
  Middle   = task.receivePass("Assister",0),
  Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss +  flag.free_kick) or task.leftBack(),
  Leader   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss +  flag.free_kick) or task.rightBack(),
  Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
  Goalie   = task.zgoalie(),
  match    = "{MALF}[CSD]"
},

["zshoot"] = {
    switch = function()
      if bufcnt(player.kickBall("Leader"),1,200) then
        return "exit"
      end
    end,
    Assister = HALF_TEST and task.stop() or task.zsupport(),
    Middle   = task.zpass(),--("Assister",0),
    Special  = HALF_TEST and task.goCmuRush(HALF_POS[1],_,_,flag.allow_dss +  flag.free_kick) or task.leftBack(),
    Leader   = HALF_TEST and task.goCmuRush(HALF_POS[2],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
    Defender = HALF_TEST and task.goCmuRush(HALF_POS[3],_,_,flag.allow_dss +  flag.free_kick) or task.rightBack(),
    Fronter  = HALF_TEST and task.goCmuRush(HALF_POS[4],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
    Center   = HALF_TEST and task.goCmuRush(HALF_POS[5],_,_,flag.allow_dss +  flag.free_kick) or task.continue(),
    Goalie   = task.zgoalie(),
    match    = "{MALF}[CSD]"
},

name = "Ref_CornerKickV802",
applicable = {
  exp = "a",
  a   = true
},
score = 0,
attribute = "attack",
timeout   = 99999
}