local distance = 30
local offset = 4.5
local length = 400
local p_ver = function(n,invert)
  return CGeoPoint:new_local(invert*length,distance*(n-offset))
end
local p_hor = function(n,invert)
  return CGeoPoint:new_local(distance*(n-offset),invert*length)
end

local RUN_FLAG = --flag.allow_dss

gPlayTable.CreatePlay{

firstState = "test1",

["test1"] = {
  switch = function()
    if bufcnt(
      player.toTargetDist("Leader")<20 and
      player.toTargetDist("Special")<20 and
      player.toTargetDist("Assister")<20 and
      player.toTargetDist("Defender")<20 and
      player.toTargetDist("Middle")<20 and
      player.toTargetDist("Center")<20 and
      player.toTargetDist("Breaker")<20 and
      player.toTargetDist("Fronter")<20
      ,40)
     then
      return "test2"
    end
  end,
  Leader   = task.goCmuRush(p_ver(1, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Special  = task.goCmuRush(p_ver(3, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Assister = task.goCmuRush(p_ver(5, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Defender = task.goCmuRush(p_ver(7, 1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Middle   = task.goCmuRush(p_ver(2,-1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Center   = task.goCmuRush(p_ver(4,-1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Breaker  = task.goCmuRush(p_ver(6,-1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  Fronter  = task.goCmuRush(p_ver(8,-1),0,_,RUN_FLAG,_,CVector:new_local(0,0)),
  match    = "[LSADMCBF]"
},
["test2"] = {
  switch = function()
    if bufcnt(true,80) then
      return "test1"
    end
  end,
  Leader   = task.goCmuRush(p_ver(1,-1),0),
  Special  = task.goCmuRush(p_ver(3,-1),0),
  Assister = task.goCmuRush(p_ver(5,-1),0),
  Defender = task.goCmuRush(p_ver(7,-1),0),
  Middle   = task.goCmuRush(p_ver(2, 1),0),
  Center   = task.goCmuRush(p_ver(4, 1),0),
  Breaker  = task.goCmuRush(p_ver(6, 1),0),
  Fronter  = task.goCmuRush(p_ver(8, 1),0),
  match    = "{LSADMCBF}"
},
["test1_2"] = {
  switch = function()
    if bufcnt(
      player.toTargetDist("Leader")<20 and
      player.toTargetDist("Special")<20
      ,40)
     then
      return "test2_2"
    end
  end,
  Leader   = task.goCmuRush(p_ver(1, 1),0),
  Special  = task.goCmuRush(p_ver(2, 1),0),
  match    = "[LS]"
},
["test2_2"] = {
  switch = function()
    if bufcnt(true,90) then
      return "test1_2"
    end
  end,
  Leader   = task.goCmuRush(p_ver(1,-1),0),
  Special  = task.goCmuRush(p_ver(2,-1),0),
  match    = "{LS}"
},
name = "TestMatch",
applicable ={
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}
