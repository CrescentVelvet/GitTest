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
  Special  = task.goSpeciPos(CGeoPoint:new_local(-4200, 800),player.toBallDir,flag.allow_dss),
  Assister = task.goSpeciPos(CGeoPoint:new_local(-4200, -800),player.toBallDir,flag.allow_dss),
  Middle   = task.goSpeciPos(CGeoPoint:new_local(-4200, 1050),player.toBallDir,flag.allow_dss),
  Defender = task.goSpeciPos(CGeoPoint:new_local(-4200, -1050),player.toBallDir,flag.allow_dss),
  Fronter  = task.goSpeciPos(CGeoPoint:new_local(-4200, 1300),player.toBallDir,flag.allow_dss),
  Center   = task.goSpeciPos(CGeoPoint:new_local(-4200, -1300),player.toBallDir,flag.allow_dss),
  Goalie   = task.penaltyGoalieV2(),
  match    = "{LASMDFC}"
},

["advance"] = {
  switch = function ()
    if bufcnt(true, 60) then
      return "exit"
    end
  end,
  Leader   = task.goSpeciPos(CGeoPoint:new_local(1500, 0),player.toBallDir,flag.allow_dss),
  Special  = task.goSpeciPos(CGeoPoint:new_local(-4200, 800),player.toBallDir,flag.allow_dss),
  Assister = task.goSpeciPos(CGeoPoint:new_local(-4200, -800),player.toBallDir,flag.allow_dss),
  Middle   = task.goSpeciPos(CGeoPoint:new_local(-4200, 1050),player.toBallDir,flag.allow_dss),
  Defender = task.goSpeciPos(CGeoPoint:new_local(-4200, -1050),player.toBallDir,flag.allow_dss),
  Fronter  = task.goSpeciPos(CGeoPoint:new_local(-4200, 1300),player.toBallDir,flag.allow_dss),
  Center   = task.goSpeciPos(CGeoPoint:new_local(-4200, -1300),player.toBallDir,flag.allow_dss),
  Goalie   = task.zgoalie(),
  match    = "{LASMDFC}"
},

name = "Ref_PenaltyDefV1",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}