local DSS = bit:_or(flag.allow_dss,flag.dodge_ball)--avoid_ball_and_shoot_line)
local STATIC_POS = {
	CGeoPoint:new_local(180, 0),
	CGeoPoint:new_local(420,-220),
	CGeoPoint:new_local(420, 220)
}
-- local passPos = CGeoPoint:new_local(0,0)
-- local getPassPos = function()
-- 	local p = function ()
-- 		return passPos
-- 	end
-- 	return p
-- end
local fixBuf
local fixTime
local passPos

function getPassPos()
	local rPos = function ()
		local passPos = messi:passPos()
		return pos.passForTouch(passPos)--passPos
		-- return CGeoPoint:new_local(param.pitchLength/8, -param.pitchWidth/8)
	end
	return rPos
end

function getFakeReceivePos()
	local rPos = function ()
		return messi:receiverPos() + CVector:new_local(-100, 0)
	end
	return rPos
end

function getFakeOtherPos(i)
	local oPos = function ()
		return messi:otherPos(i) + CVector:new_local(150, 0)
	end
	return oPos
end

local leaderNum = function ()
	return messi:leaderNum()
end
local receiverNum = function ()
	return messi:receiverNum()
end
function getFlag()
	local flag = function ()
		if messi:isFlat() then
			return flag.kick + flag.dribble + flag.safe 
		else
			return flag.kick + flag.chip + flag.dribble + flag.safe
		end
	end
	return flag
end
function getPassVel()
	local vel = function ()
		local speed = messi:passVel()
		if messi:isFlat() then
			speed = speed*0.9
			if speed < 1 then
				speed = 1
			end
		else
			speed = speed * 1.15
		end
		-- print("speed: ", speed)
		return speed
	end
	return vel
end

gPlayTable.CreatePlay{
firstState = "start",

["test"] = {
	switch = function ()
	return
	end,
	Leader   = task.stop(),
	Receiver = task.stop(),
	Middle   = task.stop(),
	Defender = task.stop(),
	Assister = task.stop(),
	Fronter  = task.stop(),
	Center   = task.stop(),
	Goalie   = task.stop(),
	match    = "{L}[RMDAFC]"
},

["start"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 25, 30) then
			messi:setMessiRun(true)
			messi:setFreeKickMode(true)
			messi:generateFreeKickPos()
			return "Compute"
		end
	end,
	Leader   = task.staticGetBall(),
	Receiver = task.goCmuRush(STATIC_POS[1],player.toBallDir,_,DSS),
	Assister = task.goCmuRush(STATIC_POS[2],player.toBallDir,_,DSS),
	Middle   = task.goCmuRush(STATIC_POS[3],player.toBallDir,_,DSS),
	Defender = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{L}[FC][R][D][MA]"
},

["Compute"] = {
	switch = function()
		local leader = leaderNum() + 1
		local receiver = receiverNum() + 1
		if leader == receiver then
			gRoleNum["Leader"] = leaderNum() + 1
			return "SpecailCompute"
		else
			gRoleNum["Leader"] = leaderNum() + 1
			gRoleNum["Receiver"] = receiverNum() + 1
		end
		if bufcnt(player.toTargetDist("Receiver") < 30, 20) then
			return "fakeAction1"
		end
	end,
	Leader   = task.staticGetBall(getPassPos()),
	Receiver = task.goCmuRush(pos.getReceivePos(),player.toBallDir,_,DSS),
	Assister = task.zdrag(pos.getOtherPos(1)),--task.goCmuRush(pos.getOtherPos(1),player.toBallDir,_,DSS),
	Middle   = task.goCmuRush(pos.getOtherPos(2),player.toBallDir,_,DSS),
	Defender = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{LR}[FC][D][MA]"
},

["SpecailCompute"] = {
	switch = function()
		messi:setMessiRun(true)
		messi:setFreeKickMode(true)
		local leader = leaderNum() + 1
		local receiver = receiverNum() + 1
		if leader == receiver then
			gRoleNum["Leader"] = leaderNum() + 1
		else
			gRoleNum["Leader"] = leaderNum() + 1
			gRoleNum["Receiver"] = receiverNum() + 1
			return "Compute"
		end
	end,
	Leader   = task.staticGetBall(getPassPos()),
	Receiver = task.goCmuRush(pos.getOtherPos(1),player.toBallDir,_,DSS),
	Assister = task.goCmuRush(pos.getOtherPos(2),player.toBallDir,_,DSS),
	Middle   = task.goCmuRush(pos.getOtherPos(3),player.toBallDir,_,DSS),
	Defender = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{L}[R][FC][D][MA]"
},

["fakeAction1"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Receiver") < 20, 30) then
			return "Pass"
		end
	end,
	Leader   = task.staticGetBall(getPassPos()),
	Receiver = task.goCmuRush(getFakeReceivePos(),player.toBallDir,_,DSS),
	Assister = task.goCmuRush(getFakeOtherPos(1),player.toBallDir,_,DSS),
	Middle   = task.goCmuRush(getFakeOtherPos(2),player.toBallDir,_,DSS),
	Defender = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{L}[R][FC][D][MA]"
},

["Pass"] = {
	switch = function ()
		if bufcnt(player.kickBall("Leader"), 1) then
			local chipVel = getPassVel()()
			fixTime = -0.0000015498 * chipVel * chipVel + 0.0025344180 * chipVel + 0.2463515283
			fixBuf = fixTime * 75
			gRoleNum["Leader"] = leaderNum() + 1
			messi:setMessiRun(false)
			messi:setFreeKickMode(false)
			return "fix"
		end
	end,
	Leader   = task.zget(pos.getPassPos(), _, _, flag.kick+flag.chip, true),
	Receiver = task.goCmuRush(pos.getReceivePos(),player.toBallDir,_,DSS),
	Middle   = task.goCmuRush(pos.getOtherPos(1),player.toBallDir,_,DSS),
	Fronter  = task.goCmuRush(pos.getOtherPos(2),player.toBallDir,_,DSS),
	Center   = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{LR}[DA][MFC]"
},

["fix"] = {
	switch = function ()
		if bufcnt(true, fixBuf) then
			return "Shoot"
		end
	end,
	Leader   = task.goCmuRush(pos.getLeaderWaitPos(),player.toBallDir,_,flag.allow_dss),
	Receiver = task.goCmuRush(pos.getOtherPos(1),player.toBallDir,_,DSS),
	Middle   = task.goCmuRush(pos.getOtherPos(2),player.toBallDir,_,DSS),
	Fronter  = task.goCmuRush(pos.getOtherPos(3),player.toBallDir,_,DSS),
	Center   = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{L}[R][DA][MFC]"
},

["Shoot"] = {
	switch = function ()
		if bufcnt(player.kickBall("Leader"), 1, 250)then
			return "exit"
		end
	end,
	Leader   = task.InterTouch(),
	Receiver = task.goCmuRush(pos.getOtherPos(1),player.toBallDir,_,DSS),
	Middle   = task.goCmuRush(pos.getOtherPos(2),player.toBallDir,_,DSS),
	Fronter  = task.goCmuRush(pos.getOtherPos(3),player.toBallDir,_,DSS),
	Center   = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{L}[R][DA][MFC]"
},

name = "TestMessiFreeKick",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}