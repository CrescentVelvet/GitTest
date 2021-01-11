-- local ORIGIN = CGeoPoint:new_local(0, 0)
-- local TARGET = CGeoPoint:new_local(0, -25)
-- local TEST_POS = {
--   CGeoPoint:new_local(-100, -100),
--   CGeoPoint:new_local(100, -100),
--   CGeoPoint:new_local(0, 100),
-- }

gPlayTable.CreatePlay{

firstState = "run1_ad",

["run1_ad"] = {
  switch = function()
    -- if bufcnt(player.infraredOn("Goalie"),3,210) then
    if 1 then
      return "run1_ad"
    end
  end,
  -- Goalie = task.zget(_,_,_,flag.dribbling),
  Goalie = task.advance(),

  match = ""
},

-- ["run2"] = {
--   switch = function()
--     if bufcnt(player.toTargetDist("Goalie")<10,210) then
--       CPrintString("[TestSkill.lua] toTargetDist")
--       return "run1"
--     end
--   end,
--   -- Goalie = task.goCmuRush(ORIGIN,0,_,flag.dribbling),
--   -- Goalie = task.goCmuRush(TEST_POS[2],0,_,flag.dribbling),
--   Goalie = openSpeedCircle(10, 1, math.pi, 1, 0)
--   match = ""
-- },

-- ["run3"] = {
--   switch = function()
--     -- if bufcnt(0,1000,1000) then
--     if bufcnt(player.toTargetDist("Goalie")<10,210) then
--       return "run1"
--     end
--   end,
--   -- Goalie = task.goCmuRush(POS,_,_,flag.dribble),
--   -- Goalie = task.zcirclePass(POS,_,0),
--   -- Goalie = task.zget(POS),
--   Goalie = task.goCmuRush(TEST_POS[3],0,_,flag.dribbling),
--   match = ""
-- },

name = "TestSkill2",
applicable ={
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}