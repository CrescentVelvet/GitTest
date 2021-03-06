local DSS_FLAG = flag.allow_dss + flag.dodge_ball + flag.our_ball_placement
local  gInitKickDirection = 1
local  gKickDirection = 1
local  stateKeepNum0=0
local  stateKeepNum1=0
local  targetPosX = param.goalWidth/2 - 100
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

local DirInFrontOfBall = function()
	return (CGeoPoint:new_local(param.pitchLength/2,targetPosX) - ball.pos()):dir()
end

local PosInFrontOfBall = function()
	return ball.pos() + Utils.Polar2Vector(-200,DirInFrontOfBall())
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
	Leader   = task.goSpeciPos(PosInFrontOfBall,DirInFrontOfBall, DSS_FLAG),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(3000,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(3000,-1000),player.toBallDir,DSS_FLAG),
	Defender = task.zback(2, 1),
	Assister = task.zback(2, 2),
	Goalie   = task.zgoalie(),
	match    = "[L][ASDM]"
},


["getBall"] = {
	switch = function ()
		if  bufcnt(true,15+stateKeepNum0) then
			stateKeepNum1=math.random(0,40)
			return "prepare"
		end
	end,
	Leader   = {SlowGetBall{pos = ball.pos, dir = player.toPointDir(CGeoPoint:new_local(param.pitchLength/2,targetPosX))}},
	Middle   = task.goSpeciPos(CGeoPoint:new_local(3000,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(3000,-1000),player.toBallDir,DSS_FLAG),
	Defender = task.zback(2, 1),
	Assister = task.zback(2, 2),
	Goalie   = task.zgoalie(),
	match    = "{LASDM}"
},


["prepare"] = {
	switch = function ()
		-- if  bufcnt(true,60+stateKeepNum1) then
		if  bufcnt(player.infraredOn("Leader"),60+stateKeepNum1) then
			return "slowGoto"
		elseif  bufcnt(player.infraredOn("Leader") and generateKickDirection()==1,3) then
			return "direct"
		end
	end,
	Leader   = {SlowGetBall{pos = ball.pos, dir = player.toPointDir(CGeoPoint:new_local(param.pitchLength/2,targetPosX))}},
	Middle   = task.goSpeciPos(CGeoPoint:new_local(3000,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(3000,-1000),player.toBallDir,DSS_FLAG),
	Defender = task.zback(2, 1),
	Assister = task.zback(2, 2),
	Goalie   = task.zgoalie(),
	match    = "{LASDM}"
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
	Middle   = task.goSpeciPos(CGeoPoint:new_local(3000,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(3000,-1000),player.toBallDir,DSS_FLAG),
	Defender = task.zback(2, 1),
	Assister = task.zback(2, 2),
	Goalie   = task.zgoalie(),
	match    = "{LASDM}"
},

["direct"] = {
	switch = function ()
		if  bufcnt( player.kickBall("Leader"), "normal", 10) then
			return "exit"
		end
	end,
	Leader   = task.zget(getAntiKickPos,_,_,flag.force_kick + flag.norush),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(3000,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(3000,-1000),player.toBallDir,DSS_FLAG),
	Defender = task.zback(2, 1),
	Assister = task.zback(2, 2),
	Goalie   = task.zgoalie(),
	match    = "{LASDM}"
},

["turn"] = {
	switch = function ()
		if  bufcnt(true, 20) then
			return "kick"
		end
	end,
	Leader   = task.penaltyTurn(dir.specified(-30), false),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(3000,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(3000,-1000),player.toBallDir,DSS_FLAG),
	Defender = task.zback(2, 1),
	Assister = task.zback(2, 2),
	Goalie   = task.zgoalie(),
	match    = "{LASDM}"
},

["kick"] = {
	switch = function ()
		if  bufcnt( player.kickBall("Leader"), "normal", 10) then
			return "exit"
		end
	end,
	Leader   = task.zget(getKickPos,_,_,flag.force_kick+flag.norush),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(3000,1000),player.toBallDir,DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(3000,-1000),player.toBallDir,DSS_FLAG),
	Defender = task.zback(2, 1),
	Assister = task.zback(2, 2),
	Goalie   = task.zgoalie(),
	match    = "{LASDM}"
},

name = "Ref_PenaltyKickV3_6V6",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
