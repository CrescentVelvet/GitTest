-- by yys 2014-06-10

gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
  switch = function ()
    if cond.isGameOn() then
      return "advance"
    end
  end,
  Leader   = task.goSpeciPos(CGeoPoint:new_local(1500, 0),player.toBallDir,flag.allow_dss),
  Special  = task.goSpeciPos(CGeoPoint:new_local(-2000, 800),player.toBallDir,flag.allow_dss),
  Assister = task.goSpeciPos(CGeoPoint:new_local(-2000, -800),player.toBallDir,flag.allow_dss),
  Middle   = task.goSpeciPos(CGeoPoint:new_local(-2300, 1050),player.toBallDir,flag.allow_dss),
  Defender = task.goSpeciPos(CGeoPoint:new_local(-2300, -1050),player.toBallDir,flag.allow_dss),
  Goalie   = task.penaltyGoalieV2(),
  match    = "{LASMD}"
},

["advance"] = {
  switch = function ()
    if bufcnt(true, 60) then
      return "exit"
    end
  end,
  Leader   = task.goSpeciPos(CGeoPoint:new_local(1500, 0),player.toBallDir,flag.allow_dss),
  Special  = task.goSpeciPos(CGeoPoint:new_local(-2000, 800),player.toBallDir,flag.allow_dss),
  Assister = task.goSpeciPos(CGeoPoint:new_local(-2000, -800),player.toBallDir,flag.allow_dss),
  Middle   = task.goSpeciPos(CGeoPoint:new_local(-2300, 1050),player.toBallDir,flag.allow_dss),
  Defender = task.goSpeciPos(CGeoPoint:new_local(-2300, -1050),player.toBallDir,flag.allow_dss),
  Goalie   = task.zgoalie(),
  match    = "{LASMD}"
},

name = "Ref_PenaltyDefV1_6V6",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}