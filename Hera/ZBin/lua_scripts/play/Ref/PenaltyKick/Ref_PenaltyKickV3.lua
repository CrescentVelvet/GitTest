local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)
local  gInitKickDirection = 1
local  gKickDirection = 1
local  stateKeepNum0=0
local  stateKeepNum1=0
local  targetPosX = 520
local function getKickPos()
	local targetPos = CGeoPoint:new_local(param.pitchLength/2,gKickDirection*targetPosX)
	return targetPos
end 

local function getAntiKickPos()
	local targetPos = CGeoPoint:new_local(param.pitchLength/2,-gKickDirection*targetPosX)
	return targetPos
end 

-- local  function enemyKickStyle()
-- 	local theirGoalieNum = skillUtils:getTheirGoalie()
-- 	if enemy.velMod(theirGoalieNum)<15 then
-- 		return "still"
-- 	else 
-- 		return "move"
-- 	end
-- end
local function calPredictFactor(goalieVelY)
	local gPredictFactor =10/60
	if goalieVelY>1000 then
		gPredictFactor = 10/60
	elseif goalieVelY>600 then
		gPredictFactor = 18/60
	elseif goalieVelY>300 then
		gPredictFactor = 6/60
	elseif goalieVelY>100 then
		gPredictFactor = 4/60
	end
	return gPredictFactor
end

local function generateKickDirection()
	local kickDirection=0
	local theirGoalieNum = skillUtils:getTheirGoalie()
	if theirGoalieNum==0 then
		return 1
	end
	local goalieY=enemy.posY(theirGoalieNum)
	--local goalieVelY=enemy.vel(theirGoalieNum):y()*2
	local lastGoalieY = vision:theirPlayer(vision:getLastCycle(),theirGoalieNum):Y()
	local lastLastGoalieY = vision:theirPlayer(vision:getLastCycle()-1,theirGoalieNum):Y()
	local lastGoalieVelY=goalieY-lastGoalieY
	local lastLastGoalieVelY=lastGoalieY-lastLastGoalieY
	local acc = lastGoalieVelY-lastLastGoalieVelY
	local goalieVelY=lastGoalieVelY+acc
	--local predictFactor=calPredictFactor(goalieVelY)
	predictFactor=1
	if goalieVelY<100 then
		local predictGoalPosY=goalieY+goalieVelY*predictFactor
		if predictGoalPosY>50 then
			kickDirection=-1
		end
		if predictGoalPosY<0 then
			kickDirection=1
		end
	else
		local predictGoalPosY=goalieY+goalieVelY*predictFactor
		if predictGoalPosY>120 then
			kickDirection=-1
		end
		if predictGoalPosY<-50 then
			kickDirection=1
		end
	end
	return kickDirection
	
end




gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
		math.randomseed(os.time()) 
		if cond.isNormalStart() then
			stateKeepNum0=math.random(0,30)
			return "getBall"
		-- elseif cond.isGameOn() then
		-- 	return "exit"
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(param.pitchLength/2 - param.penaltyDepth - 200,0),0, DSS_FLAG),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-1000),player.toBallDir,DSS_FLAG),
	Fronter  = task.goSpeciPos(CGeoPoint:new_local(4200, 2200),player.toBallDir,DSS_FLAG),
	Center   = task.goSpeciPos(CGeoPoint:new_local(4200, -2200),player.toBallDir,DSS_FLAG),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][ASDMFC]"
},


["getBall"] = {
	switch = function ()
		if  bufcnt(true,15+stateKeepNum0) then
			stateKeepNum1=math.random(0,40)
			return "prepare"
		end
	end,
	Leader   = {SlowGetBall{pos = ball.pos, dir = player.toPointDir(CGeoPoint:new_local(param.pitchLength/2,targetPosX))}},
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-1000),player.toBallDir,DSS_FLAG),
	Fronter  = task.goSpeciPos(CGeoPoint:new_local(4200, 2200),player.toBallDir,DSS_FLAG),
	Center   = task.goSpeciPos(CGeoPoint:new_local(4200, -2200),player.toBallDir,DSS_FLAG),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{LASDMFC}"
},


["prepare"] = {
	switch = function ()
		if  bufcnt(true,60+stateKeepNum1) then
			return "slowGoto"
		elseif  bufcnt(generateKickDirection()==1,3) then
			return "direct"
		end
	end,
	Leader   = {SlowGetBall{pos = ball.pos, dir = player.toPointDir(CGeoPoint:new_local(param.pitchLength/2,targetPosX))}},
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-1000),player.toBallDir,DSS_FLAG),
	Fronter  = task.goSpeciPos(CGeoPoint:new_local(4200, 2200),player.toBallDir,DSS_FLAG),
	Center   = task.goSpeciPos(CGeoPoint:new_local(4200, -2200),player.toBallDir,DSS_FLAG),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{LASDMFC}"
},


["slowGoto"] = {
	switch = function ()
		if  bufcnt(generateKickDirection()==1,3,10) then
			return "direct"
		elseif bufcnt(generateKickDirection()==-1,3,10) then
			return "turn"
		elseif  bufcnt(generateKickDirection()==0,3,10) then
			return "direct"
		end
	end,
	Leader   = {SlowGetBall{pos = ball.pos, dir = player.toPointDir(CGeoPoint:new_local(param.pitchLength/2,targetPosX))}},
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-1000),player.toBallDir,DSS_FLAG),
	Fronter  = task.goSpeciPos(CGeoPoint:new_local(4200, 2200),player.toBallDir,DSS_FLAG),
	Center   = task.goSpeciPos(CGeoPoint:new_local(4200, -2200),player.toBallDir,DSS_FLAG),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{LASDMFC}"
},

["direct"] = {
	switch = function ()
		if  bufcnt( player.kickBall("Leader"), "normal", 10) then
			return "exit"
		end
	end,
	Leader   = task.zget(getAntiKickPos,_,_,flag.force_kick),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-1000),player.toBallDir,DSS_FLAG),
	Fronter  = task.goSpeciPos(CGeoPoint:new_local(4200, 2200),player.toBallDir,DSS_FLAG),
	Center   = task.goSpeciPos(CGeoPoint:new_local(4200, -2200),player.toBallDir,DSS_FLAG),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{LASDMFC}"
},

["turn"] = {
	switch = function ()
		if  bufcnt(true, 10) then
			return "kick"
		end
	end,
	Leader   = task.penaltyTurn(dir.specified(-30), false),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-1000),player.toBallDir,DSS_FLAG),
	Fronter  = task.goSpeciPos(CGeoPoint:new_local(4200, 2200),player.toBallDir,DSS_FLAG),
	Center   = task.goSpeciPos(CGeoPoint:new_local(4200, -2200),player.toBallDir,DSS_FLAG),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{LASDMFC}"
},

["kick"] = {
	switch = function ()
		if  bufcnt( player.kickBall("Leader"), "normal", 10) then
			return "exit"
		end
	end,
	Leader   = task.zget(getKickPos,_,_,flag.force_kick),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-1000),player.toBallDir,DSS_FLAG),
	Fronter  = task.goSpeciPos(CGeoPoint:new_local(4200, 2200),player.toBallDir,DSS_FLAG),
	Center   = task.goSpeciPos(CGeoPoint:new_local(4200, -2200),player.toBallDir,DSS_FLAG),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{LASDMFC}"
},

name = "Ref_PenaltyKickV3",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
