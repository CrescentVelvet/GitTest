local ORIGIN = CGeoPoint:new_local(0, 0)
local TARGET = CGeoPoint:new_local(600, 0)
local POS = CGeoPoint:new_local(300, 170)
-- local TEST_POS = {
--   CGeoPoint:new_local(-100, -100),
--   CGeoPoint:new_local(100, -100),
--   CGeoPoint:new_local(0, 100),
-- }

local USE_ZPASS = gOppoConfig.USE_ZPASS

gPlayTable.CreatePlay{

firstState = "run1_blue",

["run1_blue"] = {
  switch = function()
    -- if bufcnt(player.infraredOn("Goalie"),3,210) then
    if 1 then
      return "run1_blue"
    end
  end,
  -- Goalie = task.zget(_,_,_,flag.dribbling),
  -- Goalie = task.zpass(_,0),
  -- Goalie = task.cmuTurnKickV1(TARGET,_,flag.kick),
  -- Goalie = task.goAndTurnKickV4(TARGET,_,flag.kick),
  -- Goalie = task.getBallV4(ORIGIN,POS),
  Goalie = task.zget(),

  match = ""
},

name = "TestSkill",
applicable ={
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}
