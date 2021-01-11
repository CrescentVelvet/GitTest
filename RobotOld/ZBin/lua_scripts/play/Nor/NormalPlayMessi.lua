local DSS = bit:_or(flag.allow_dss,flag.dodge_ball)--avoid_ball_and_shoot_line)
USE_ZGET = CGetSettings("Messi/USE_ZGET", "Bool")
local leaderNum = function ()
	return messi:leaderNum()
end
local receiverNum = function ()
	return messi:receiverNum()
end
function getPassPos()
	local rPos = function ()
		local passPos = messi:passPos()
		return passPos
	end
	return rPos
end

function goaliePassPos()
	local gPos = function ()
		return messi:goaliePassPos()
	end
	return gPos
end

function getPassVel()
	local vel = function ()
		local speed = messi:passVel()
		-- if not IS_SIMULATION then
		-- 	if messi:isFlat() then
		-- 		speed = speed*1
		-- 		if speed < 1 then
		-- 			speed = 1
		-- 		end
		-- 	else
		-- 		speed = speed*1
		-- 	end
		-- end
		-- print("speed: ", speed)
		return speed
	end
	return vel
end

function getAttackerNum(i)
	return function ()
		return messi:attackerNum(i)
	end
end

function getFlag()
	local flag = function ()
		local f = flag.dribble + flag.safe
		if messi:needChip() then
			f = f + flag.chip
		end
		if messi:needKick() then
			f = f + flag.kick
		end
		return f
	end
	return flag
end

local function ourBallJumpCond()
	local state = messi:nextState()
	local attackerAmount = messi:attackerAmount()
	if state == "Pass" then 
		--绕过match直接赋值
		local leader = leaderNum() + 1
		local receiver = receiverNum() + 1
		-- print("valid:", receiver, player.valid(receiver))
		if leader ~= receiver and player.valid(receiver) then
			gRoleNum["Leader"] = leader
			gRoleNum["Receiver"] = receiver
			return "Pass"..attackerAmount
		else
			gRoleNum["Leader"] = leader
			return "SpecialPass"..attackerAmount
		end
	elseif state == "GetBall" then
		gRoleNum["Leader"] = leaderNum() + 1
		return state..attackerAmount
	elseif state == "fix" then
		gRoleNum["Leader"] = leaderNum() + 1
		return "fix"
	end
end

function ourBallJumpCondTest()
	local leader = leaderNum() + 1
	local receiver = receiverNum() + 1
	local state = messi:nextState()
	if state == "fix" then
		gRoleNum["Leader"] = leader
		return "testFix"
	end
	if leader ~= receiver and player.valid(receiver) then
		gRoleNum["Leader"] = leader
		gRoleNum["Receiver"] = receiver
		return "test4"
	else
		gRoleNum["Leader"] = leader
		return "test3"
	end
end

function leaderTask (t, w, p, f)
	return function()
		-- local leaderState = messi:leaderState()
		-- if leaderState == "PASS" then
		-- 	return task.zattack(pos.getPassPos(), _, getPassVel(), getFlag())
		-- elseif leaderState == "SELFPASS" then
		-- 	return task.zattack(pos.getPassPos(), _, getPassVel(), getFlag())
		-- elseif leaderState == "COMPUTE" then
		-- 	return task.zattack(pos.getPassPos(), _, getPassVel(), getFlag())
		-- end
		-- print("USE_ZGET: ", USE_ZGET)
		if USE_ZGET then
			return task.zget(pos.getPassPos(), _, getPassVel(), getFlag())
		else
			return task.zattack(pos.getPassPos(), _, getPassVel(), getFlag())
		end
	end
end

function receiverTask (p,d)
	return function ()
		local state = messi:nextState()
		if state == "GetBall" then
			return task.zsupport()
		end
		local leaderState = messi:leaderState()
		if leaderState == "PASS" then
			return task.goCmuRush(pos.getReceivePos(),player.toBallDir,_,DSS)
		elseif leaderState == "SELFPASS" then
			return task.zsupport()
		elseif leaderState == "COMPUTE" then
			return task.zdrag(pos.getReceivePos())
		end
	end
end

gPlayTable.CreatePlay{
firstState = "test",

["test"] = {
	switch = function ()
		gRoleNum["Leader"] = leaderNum() + 1
		gRoleNum["Receiver"] = receiverNum() + 1
	end,
	Leader   = task.stop(),
	Receiver = task.stop(),
	Middle   = task.stop(),
	Defender = task.stop(),
	Assister = task.stop(),
	Fronter  = task.stop(),
	Center   = task.stop(),
	Goalie   = task.stop(),
	match    = "{LR}[MDAFC]"
},
["test1"] = {
	switch = function ()
		gRoleNum["Leader"] = leaderNum() + 1
		gRoleNum["Receiver"] = receiverNum() + 1
		local isValid = messi:isReceiverPosValid()
		if isValid then
			return "test2"
		end
		return 
	end,
	Leader   = leaderTask(pos.getPassPos(), pos.getLeaderWaitPos(), getPassVel(), getFlag()),
	Special  = task.zdrag(pos.getOtherPos(1)),
	match    = "{L}[S]"
},
["test2"] = {
	switch = function ()
		gRoleNum["Leader"] = leaderNum() + 1
		gRoleNum["Receiver"] = receiverNum() + 1
		local isValid = messi:isReceiverPosValid()
		if not isValid then
			return "test2"
		end
		return 
	end,
	Leader   = leaderTask(pos.getPassPos(), pos.getLeaderWaitPos(), getPassVel(), getFlag()),
	Receiver = receiverTask(), --task.goCmuRush(pos.getReceivePos(),player.toBallDir,_,DSS),
	match    = "{LR}"
},
["test3"] = {
	switch = function ()
		return ourBallJumpCondTest()
	end,
	Leader   = leaderTask(),--task.zattack(pos.getPassPos(), _, getPassVel(), getFlag()),
	Receiver = receiverTask(), --task.goCmuRush(pos.getOtherPos(1),player.toBallDir,_,DSS),
	Middle   = task.zsupport(),
	Fronter  = task.zdrag(pos.getOtherPos(1)),
	-- Center   = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	-- match    = "{L}[R][MFC]"
	match    = "{L}[R][F][M]"
},
["test4"] = {
	switch = function ()
		return ourBallJumpCondTest()
	end,
	Leader   = leaderTask(),--task.zattack(pos.getPassPos(), _, getPassVel(), getFlag()),
	Receiver = receiverTask(), --task.goCmuRush(pos.getReceivePos(),player.toBallDir,_,DSS),
	Middle   = task.zsupport(),
	Fronter  = task.zdrag(pos.getOtherPos(1)),
	-- Center   = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	-- match    = "{LR}[MFC]"
	match    = "{LR}[F][M]"
},
["testFix"] = {
	switch = function ()
		local nextState = ourBallJumpCondTest()
		if nextState ~= "testFix" then
			return nextState
		end
	end,
	Leader   = task.goCmuRush(pos.getLeaderWaitPos(),player.toBallDir,_,flag.allow_dss),
	Receiver = receiverTask(),
	Middle   = task.zdrag(pos.getOtherPos(1)),
	Fronter  = task.zdrag(pos.getOtherPos(2)),
	Center   = task.zdrag(pos.getBackPos()),
	match    = "{L}[R][MFC]"
},

["initState"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = task.zget(),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Fronter  = task.markingFront("First"),
	Center   = task.markingFront("Second"),
	Goalie   = task.zgoalie(),
	match    = "[L][FCADMS]"
},
["Pass0"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zdrag(pos.getOtherPos(2)),
	Fronter  = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Center   = task.zdrag(pos.getOtherPos(1)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "{LR}[DA][M][F][C]"
},
["SpecialPass0"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zdrag(pos.getOtherPos(2)),
	Fronter  = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Center   = task.zdrag(pos.getOtherPos(1)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "{L}[DA][R][M][F][C]"
},
["Pass1"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zdrag(pos.getOtherPos(2)),
	Fronter  = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Center   = task.zmarking("Zero",_,getAttackerNum(0)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "{LR}[DA][C][MF]"
},
["SpecialPass1"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zdrag(pos.getOtherPos(2)),
	Fronter  = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Center   = task.zmarking("Zero",_,getAttackerNum(0)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "{L}[DA][R][C][MF]"
},
["Pass2"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zdrag(pos.getOtherPos(2)),
	Fronter  = task.zmarking("Zero",_,getAttackerNum(0)),
	Center   = task.zmarking("First",_,getAttackerNum(1)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "{LR}[DA][FC][M]"
},
["SpecialPass2"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zdrag(pos.getOtherPos(2)),
	Fronter  = task.zmarking("Zero",_,getAttackerNum(0)),
	Center   = task.zmarking("First",_,getAttackerNum(1)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "{L}[DA][R][FC][M]"
},
["Pass3"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zmarking("Zero",_,getAttackerNum(0)),
	Fronter  = task.zmarking("First",_,getAttackerNum(1)),
	Center   = task.zmarking("Second",_,getAttackerNum(2)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "{LR}[DA][MFC]"
},
["SpecialPass3"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zmarking("Zero",_,getAttackerNum(0)),
	Fronter  = task.zmarking("First",_,getAttackerNum(1)),
	Center   = task.zmarking("Second",_,getAttackerNum(2)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "{L}[DA][R][MFC]"
},
["GetBall0"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zdrag(pos.getOtherPos(2)),
	Fronter  = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Center   = task.zmarking("First"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "[L][DA][C][MF][R]"
},
["GetBall1"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zdrag(pos.getOtherPos(2)),
	Fronter  = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
	Center   = task.zmarking("Zero",_,getAttackerNum(0)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "[L][DA][C][MF][R]"
},
["GetBall2"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zdrag(pos.getOtherPos(2)),
	Fronter  = task.zmarking("Zero",_,getAttackerNum(0)),
	Center   = task.zmarking("First",_,getAttackerNum(1)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "[L][DA][FC][M][R]"
},
["GetBall3"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = receiverTask(),
	Middle   = task.zmarking("Zero",_,getAttackerNum(0)),
	Fronter  = task.zmarking("First",_,getAttackerNum(1)),
	Center   = task.zmarking("Second",_,getAttackerNum(2)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "[L][DA][MFC][R]"
},
["GetBall4"] = {
	switch = function ()
		return ourBallJumpCond()
	end,
	Leader   = leaderTask(),
	Receiver = task.zmarking("Zero",_,getAttackerNum(0)),
	Middle   = task.zmarking("First",_,getAttackerNum(1)),
	Fronter  = task.zmarking("Second",_,getAttackerNum(2)),
	Center   = task.zmarking("Third",_,getAttackerNum(3)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(goaliePassPos()),
	match    = "[L][DA][RMFC]"
},
["fix"] = {
	switch = function ()
		local nextState = ourBallJumpCond()
		if nextState ~= "fix" then
			return nextState
		end
	end,
	Leader   = task.goCmuRush(pos.getLeaderWaitPos(),player.toBallDir,_,flag.allow_dss),
	Receiver = receiverTask(),
	Middle   = task.continue(),
	Fronter  = task.continue(),
	Center   = task.continue(),
	Defender = task.continue(),
	Assister = task.continue(),
	Goalie   = task.continue(),
	match    = "{L}[R][DA][MFC]"
},

name = "NormalPlayMessi",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}