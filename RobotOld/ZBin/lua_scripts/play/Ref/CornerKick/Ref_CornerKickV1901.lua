local WAIT_POS = ball.refSyntYPos(CGeoPoint:new_local(160, -60))
local WAIT_POS1 = ball.refSyntYPos(CGeoPoint:new_local(120,100))
local WAIT_POS2 = ball.refSyntYPos(CGeoPoint:new_local(150,60))
local FAKE_POS = ball.refSyntYPos(CGeoPoint:new_local(200,100))
local PASS_POS = pos.passForTouch(WAIT_POS)
local CHIP_BUF
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball) + flag.free_kick
gPlayTable.CreatePlay{
firstState = "start",
["start"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Assister")<20,20) then
      CHIP_BUF = cp.getFixBuf(WAIT_POS)
      return "continue"
    end
  end,
  Leader   = task.staticGetBall(PASS_POS,false),
  Assister = task.goCmuRush(WAIT_POS1, _, _, DSS_FLAG),
  Special  = task.goCmuRush(WAIT_POS2, _, _, DSS_FLAG),
  Goalie   = task.zgoalie(),
  match    = "(L)[AS]"
},
["continue"] = {
  switch = function()
    if bufcnt(true,15) then
      return "rush"
    end
  end,
  Leader   = task.staticGetBall(PASS_POS,false),
  Assister = task.goCmuRush(WAIT_POS , _, _, DSS_FLAG),
  Special  = task.goCmuRush(FAKE_POS, _, _, DSS_FLAG),
  Goalie   = task.zgoalie(),
  match    = "{L}[A][S]"
},
["rush"] = {
  switch = function()
    if player.kickBall("Leader") or player.toBallDist("Leader") > 35 then
      return "fix"
    elseif bufcnt(true, 120) then
      return "exit"
    end
  end,
  Leader = task.zget(PASS_POS,_,_,flag.kick+flag.chip,true),
  Assister = task.continue(),
  Special  = task.singleBack(),
  Goalie   = task.zgoalie(),
  match = "{LA}{S}"
},
["fix"] = {
    switch = function()
      if bufcnt(true,CHIP_BUF) then
        return "shoot"
      end
    end,
  Leader = task.rightBack(),
  Assister = task.continue(),
  Special  = task.leftBack(),
  Goalie   = task.zgoalie(),
  match = "{LA}{S}"
},
["shoot"] = {
  switch = function()
    if bufcnt(player.kickBall("Leader"),1,80) then
      return "exit"
    end
  end,
  Leader = task.rightBack(),
  Assister = task.InterTouch(),
  Special  = task.leftBack(),
  Goalie   = task.zgoalie(),
  match = "{LA}{S}"
},
name = "Ref_CornerKickV1901",
applicable = {
  exp = "a",
  a   = true
},
attribute = "attack",
timeout = 99999
}