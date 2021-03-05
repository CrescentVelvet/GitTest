-- middle
-- local distance = 45
-- local offset = 4.5
-- local length = 150
-- local xOffset = 120
-- local yOffset = -120

--left
-- local distance = 45
-- local offset = 4.5
-- local length = 150
-- local xOffset = -120
-- local yOffset = -120

--down middle
-- local distance = 45
-- local offset = 4.5
-- local length = 150
-- local xOffset = 120
-- local yOffset = 120

--up middle
-- local distance = 45
-- local offset = 4.5
-- local length = 150
-- local xOffset = 120
-- local yOffset = -260

--right
local distance = 45
local offset = 4.5
local length = 150
local xOffset = 260
local yOffset = -120

local p_ver = function(n,invert)
  return CGeoPoint:new_local(invert*length - xOffset + distance * 2, distance*(n-offset) - yOffset)
end
local p_hor = function(n,invert)
  return CGeoPoint:new_local(distance*(n-offset) + xOffset, invert*length + yOffset + distance * 2)
end
local time = 5
local dist = 20
local RUN_FLAG = flag.allow_dss

gPlayTable.CreatePlay{

firstState = "test1_1",
["test1_1"] = {
  switch = function()
    if bufcnt(
      player.toTargetDist("Leader")<dist and
      player.toTargetDist("Special")<dist and
      player.toTargetDist("Assister")<dist and
      player.toTargetDist("Defender")<dist
      ,time)
     then
      return "test2_1"
    end
  end,
  Leader   = IS_YELLOW and task.goCmuRush(p_ver(1, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(1, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Special  = IS_YELLOW and task.goCmuRush(p_ver(2, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(2, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Assister = IS_YELLOW and task.goCmuRush(p_ver(3, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(3, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Defender = IS_YELLOW and task.goCmuRush(p_ver(4, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(4, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  match    = "{LSAD}"
},
["test2_1"] = {
  switch = function()
    if bufcnt(
      player.toTargetDist("Leader")<dist and
      player.toTargetDist("Special")<dist and
      player.toTargetDist("Assister")<dist and
      player.toTargetDist("Defender")<dist
      ,time) then
      return "test1_1"
    end
  end,
  Leader   = IS_YELLOW and task.goCmuRush(p_ver(1, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(1, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Special  = IS_YELLOW and task.goCmuRush(p_ver(2, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(2, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Assister = IS_YELLOW and task.goCmuRush(p_ver(3, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(3, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Defender = IS_YELLOW and task.goCmuRush(p_ver(4, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(4, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  match    = "{LSAD}"
},

["test1_2"] = {
  switch = function()
    if bufcnt(
      player.toTargetDist("Leader")<dist and
      player.toTargetDist("Special")<dist and
      player.toTargetDist("Assister")<dist and
      player.toTargetDist("Defender")<dist
      ,time)
     then
      return "test2_2"
    end
  end,
  Leader   = IS_YELLOW and task.goCmuRush(p_ver(1, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(1, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Special  = IS_YELLOW and task.goCmuRush(p_ver(2, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(2, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Assister = IS_YELLOW and task.goCmuRush(p_ver(3, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(3, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Defender = IS_YELLOW and task.goCmuRush(p_ver(4, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(4, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  match    = "{LSAD}"
},
["test2_2"] = {
  switch = function()
    if bufcnt(
      player.toTargetDist("Leader")<dist and
      player.toTargetDist("Special")<dist and
      player.toTargetDist("Assister")<dist and
      player.toTargetDist("Defender")<dist
      ,time) then
      return "test1_2"
    end
  end,
  Leader   = IS_YELLOW and task.goCmuRush(p_ver(4, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(4, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Special  = IS_YELLOW and task.goCmuRush(p_ver(3, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(3, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Assister = IS_YELLOW and task.goCmuRush(p_ver(2, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(2, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Defender = IS_YELLOW and task.goCmuRush(p_ver(1, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)) or task.goCmuRush(p_hor(1, -1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  match    = "{LSAD}"
},

name = "TestDSS",
applicable ={
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}
