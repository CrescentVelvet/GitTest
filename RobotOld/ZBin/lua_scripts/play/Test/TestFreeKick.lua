local DSS_FLAG = flag.allow_dss + flag.dodge_ball + flag.free_kick
local AFTER_KICK_FLAG = flag.allow_dss + flag.dodge_ball
local pass_pos = messi:freeKickPos();--flatPassPos();--flatShootPos();--chipPassPos();--chipShootPos();
local wait_pos = messi:freeKickWaitPos() --messi:freeKickWaitPos()
local fake_pos = pos.getOtherPos(2)() --CGeoPoint:new_local(440, 180)
local small_r  = 75
local line_x   = 30
local count    = 0
local Rcount   = 0
local dirFlag  = 1
local TO_TARGET_CNT = 0
local CHIP_CNT      = 0
local HOLDING_CNT   = 0
local FIX_CNT       = 0
local BUFFER_TIME   = 20
local FREEKICK_FLAG
local BACK_KICK   = false
local MIDDLE_KICK = false
local FREE_DEBUG  = true
local CAN_DIRECT_SHOOT = false
local PASS_POS = function ()
	return pass_pos
end
local WAIT_POS = function ()
	return wait_pos
end
local FAKE_POS = function ()
	return fake_pos
end
local initVariable = function()
	pass_pos = messi:freeKickPos()
	wait_pos = messi:freeKickWaitPos()
	if math.abs(pass_pos:y()) < 1e-5 then
		dirFlag  = math.abs(pass_pos:y()) / 1e-5
	else
		dirFlag  = math.abs(pass_pos:y()) / pass_pos:y()
	end
	fake_pos = pos.getOtherPos(2)()
	small_r  = 75
	line_x   = 40
	Rcount   = 0
	BACK_KICK     = false
	MIDDLE_KICK   = false
	CAN_DIRECT_SHOOT = false
	TO_TARGET_CNT = 0
	CHIP_CNT      = 0
	FIX_CNT       = 0
	HOLDING_CNT   = 0
	if wait_pos:x() < param.pitchLength / 6 then -- 0
		BACK_KICK = true
	elseif wait_pos:x() < param.pitchLength / 3 then -- param.pitchLength / 4 
		MIDDLE_KICK = true
	end
end
local function circlePosMove( number )
    return function ()
        return wait_pos + Utils.Polar2Vector(small_r +20*Rcount,math.pi*2/6*(number-3+dirFlag*(count-2)))
    end
end
local function linePos (number)
	return function ()
		return wait_pos + Utils.Polar2Vector(line_x * (number - 3) , dirFlag * math.pi / 2) --math.pi / 4 (pass_pos - fake_pos):dir() +
	end
end

local STOP_BALL_PLACE = false
local BP_FINISH = false
local KICKER_TASK = function(pos)
	return function()
		if cond.ballPlaceFinish() then
			BP_FINISH = true
		end
		if cond.ballPlaceUnfinish() then
			BP_FINISH = false
		end
		if cond.ourBallPlace() and not (STOP_BALL_PLACE and BP_FINISH) then
			-- debugEngine:gui_debug_msg(ball.pos()+CVector:new_local(30,30),"Fetch " .. (BP_FINISH and "T" or "F"),3)
			return task.fetchBall(ball.placementPos,_,true)
		end
		-- debugEngine:gui_debug_msg(ball.pos()+CVector:new_local(30,30),"Static " .. (BP_FINISH and "T" or "F"),4)
		return task.staticGetBall(pos, _, flag.free_kick + flag.allow_dss)
	end
end
local LEFTBACK_TASK = function ()
	return function ()
		return BACK_KICK and task.leftBack() or task.goCmuRush(linePos(4), player.toBallDir, _, DSS_FLAG)
	end
end
local RIGHTBACK_TASK = function ()
	return function ()
		return BACK_KICK and task.rightBack() or MIDDLE_KICK and task.singleBack() or task.goCmuRush(linePos(5), player.toBallDir, _, DSS_FLAG)
	end
end
local function freeKickDebug ()
	if not FREE_DEBUG then
		return nil
	end
	if STOP_BALL_PLACE then
		debugEngine:gui_debug_msg(ball.pos(),"Ready",7)
	else
		debugEngine:gui_debug_msg(ball.pos(),"Preparing",1)
	end
    debugEngine:gui_debug_msg(wait_pos, "FREE")
	debugEngine:gui_debug_msg(pass_pos, "SHOOT")
	debugEngine:gui_debug_msg(fake_pos, "FAKE")
	debugEngine:gui_debug_msg(CGeoPoint:new_local(-400, -400) * (IS_RIGHT and -1 or 1), "toTargetTime: "..TO_TARGET_CNT.. "  chipTime: ".. CHIP_CNT.."  fixTime: "..FIX_CNT)
end
gPlayTable.CreatePlay{

firstState = "initState",
["initState"] = {
	switch = function()
		STOP_BALL_PLACE = false
		BP_FINISH = false
		initVariable()
		freeKickDebug()
		return "GoLine"
	end,
	Leader   = task.stop(),
	Receiver = task.stop(),
    Assister = task.stop(),
    Middle   = task.stop(),
    Fronter  = task.stop(),
    Special  = task.stop(),
    Defender = task.stop(),
    Goalie   = task.zgoalie(),
	match = "[LRAMSFD]"
},
-- ["GoCircle"] = { --用的时候需要仿照goline修改
-- 	switch = function()
-- 		freeKickDebug()
-- 		if bufcnt(
-- 			player.toTargetDist("Assister") <20 and 
-- 			player.toTargetDist("Fronter")  <20 and 
-- 			player.toTargetDist("Special")  <20 and 
-- 			player.toTargetDist("Defender") <20 and 
-- 			player.toTargetDist("Middle")   <20 and
-- 			player.toTargetDist("Receiver") <20
-- 			,20) then
-- 			pass_pos = messi:freeKickPos()
-- 			fake_pos = pos.getOtherPos(2)()
-- 			small_r = 280
-- 			return "comfirmReceiver"
-- 		end
-- 	end,
-- 	Leader   = KICKER_TASK(WAIT_POS),
-- 	Receiver = task.goSpeciPos(circlePosMove(6),player.toPointDir(WAIT_POS)),
--     Assister = task.goSpeciPos(circlePosMove(1),player.toPointDir(WAIT_POS)),
--     Middle   = task.goSpeciPos(circlePosMove(2),player.toPointDir(WAIT_POS)),
--     Fronter  = task.goSpeciPos(circlePosMove(3),player.toPointDir(WAIT_POS)),
--     Special  = task.goSpeciPos(circlePosMove(4),player.toPointDir(WAIT_POS)),
--     Defender = task.goSpeciPos(circlePosMove(5),player.toPointDir(WAIT_POS)),
--     Goalie   = task.zgoalie(),
-- 	match = "(L)[RAMFSD]"
-- },
["GoLine"] = {
	switch = function()
		freeKickDebug()
		if bufcnt(
            player.toTargetDist("Assister") <20 and 
            player.toTargetDist("Fronter")  <20 and 
            player.toTargetDist("Special")  <20 and 
            player.toTargetDist("Defender") <20 and 
            player.toTargetDist("Middle")   <20 and
            player.toTargetDist("Receiver") <20
            ,20 ,100) then
			STOP_BALL_PLACE = true
		end
		if STOP_BALL_PLACE and not cond.ourBallPlace() then
			pass_pos = messi:freeKickPos()
			if math.abs(pass_pos:y()) < 1e-5 then
				dirFlag  = math.abs(pass_pos:y()) / 1e-5
			else
				dirFlag  = math.abs(pass_pos:y()) / pass_pos:y()
			end
			fake_pos = pos.getOtherPos(2)()
			-- fake_pos = CGeoPoint:new_local(pass_pos:x(), pass_pos:y() + ball.posY() > 0 and -200 or 200)
			line_x   = 80
			local rush_dist = BACK_KICK and 50 or MIDDLE_KICK and 150 or 100
			if wait_pos:y() > 0 then
				wait_pos = wait_pos + Utils.Polar2Vector(rush_dist, math.pi * (-1 / 4))
			else
				wait_pos = wait_pos + Utils.Polar2Vector(rush_dist, math.pi * ( 1 / 4))
			end
			return "comfirmReceiver"
		end
	end,
	Leader   = KICKER_TASK(WAIT_POS),
	Receiver = task.goCmuRush(linePos(0), player.toBallDir, _, DSS_FLAG),
	Assister = task.goCmuRush(linePos(1), player.toBallDir, _, DSS_FLAG),
	Middle   = task.goCmuRush(linePos(2), player.toBallDir, _, DSS_FLAG),
	Fronter  = task.goCmuRush(linePos(3), player.toBallDir, _, DSS_FLAG),
	Special  = LEFTBACK_TASK(),
	Defender = RIGHTBACK_TASK(),
	Goalie   = task.zgoalie(),
	match = "(L)[SD][RAMF]"
},
["comfirmReceiver"] = {
	switch = function()
		freeKickDebug()
		if bufcnt(true, 1) then
			return "Judge"
		end
	end,
	Leader   = task.staticGetBall(WAIT_POS),
	Receiver = task.goCmuRush(PASS_POS, player.toTheirGoalDir, _, DSS_FLAG),
	Assister = task.continue(),
	Middle   = task.continue(),
	Fronter  = task.continue(),
	Special  = LEFTBACK_TASK(),
	Defender = RIGHTBACK_TASK(),
	Goalie   = task.zgoalie(),
	match = "{L}[R][SD][AMF]"
},
["Judge"] = {
	switch = function()
		freeKickDebug()
		CAN_DIRECT_SHOOT = messi:canDirectKick()
		print("canshoot: ", CAN_DIRECT_SHOOT)
		if bufcnt(player.infraredCount("Leader") > 20, 1, 100) then --bufcnt(player.toBallDist("Leader") < 12, 50, 100)
			BUFFER_TIME = BACK_KICK and 10 or MIDDLE_KICK and 15 or 20
			TO_TARGET_CNT = player.toTargetTime(PASS_POS, "Receiver") + BUFFER_TIME
			CHIP_CNT = cp.toTargetTime(PASS_POS)
			if CAN_DIRECT_SHOOT then
				return "DirectKick"
			elseif TO_TARGET_CNT > CHIP_CNT then
				HOLDING_CNT = TO_TARGET_CNT - CHIP_CNT
				return "Holding"
			else 
				return "FixPass"
			end
		end
	end,
	Leader   = task.slowGetBall(PASS_POS, flag.dribble, false),
	Receiver = task.goCmuRush(linePos(6), player.toBallDir, _, DSS_FLAG),
	Assister = task.continue(),
	Middle   = task.continue(),
	Fronter  = task.continue(),
	Special  = LEFTBACK_TASK(),
	Defender = RIGHTBACK_TASK(),
	Goalie   = task.zgoalie(),
	match = "{LR}[SD][AMF]"
},
["DirectKick"] = {
	switch = function()
		freeKickDebug()
		if bufcnt(player.kickBall("Leader"), 1,150) then
		    return "finish" 
		end
	end,
	Leader   = task.zget(_, _, _, flag.kick + flag.safe),
	Receiver = task.zsupport(),
	Assister = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Fronter  = task.goCmuRush(pos.getOtherPos(2), player.toBallDir, _, AFTER_KICK_FLAG),
	Special  = task.leftBack(),
	Defender = task.rightBack(),
	Goalie   = task.zgoalie(),
	match = "[L][SD][RAMF]"
},
["Holding"] = {
	switch = function()
		freeKickDebug()
		if bufcnt(true, HOLDING_CNT) then
			return "FixPass"
		end
	end,
	Leader   = task.slowGetBall(PASS_POS, flag.dribble, false),
	Receiver = task.goCmuRush(PASS_POS, player.player.toTheirGoalDir),
	Assister = task.goCmuRush(FAKE_POS, player.toBallDir, _, DSS_FLAG),
	Middle   = task.continue(),
	Fronter  = task.continue(),
	Special  = LEFTBACK_TASK(),
	Defender = RIGHTBACK_TASK(),
	Goalie   = task.zgoalie(),
	match = "{LR}[SD][AMF]"
},
["FixPass"] = {
	switch = function()
		freeKickDebug()
		if bufcnt(player.kickBall("Leader"), 1, 100) then
			_,FIX_CNT = cp.toTargetTime(PASS_POS)
			if FIX_CNT <= 0 then
				FIX_CNT = 1
			end
			FIX_CNT = BACK_KICK and FIX_CNT * 0.4 or MIDDLE_KICK and FIX_CNT * 0.75 or FIX_CNT
			-- fake_pos = CGeoPoint:new_local(pass_pos:x(), (param.penaltyWidth / 2 + 12) * -dirFlag)
			return "Fix"
		end
	end,
	Leader   = task.zget(PASS_POS, _, _, flag.kick + flag.chip + flag.dribble + flag.safe),
	Receiver = task.goCmuRush(PASS_POS, player.toTheirGoalDir),
	Assister = task.goCmuRush(FAKE_POS, player.toBallDir, _, DSS_FLAG),
	Middle   = task.continue(),
	Fronter  = task.continue(),
	Special  = LEFTBACK_TASK(),
	Defender = RIGHTBACK_TASK(),
	Goalie   = task.zgoalie(),
	match = "{LR}[SD][AMF]"
},
["Fix"] = {
	switch = function()
		freeKickDebug()
		if bufcnt(true, FIX_CNT) then
			return "Shoot"
		end
	end,
	Leader   = task.zget(PASS_POS, _, _, flag.kick + flag.chip + flag.dribble + flag.safe),
	Receiver = task.goCmuRush(PASS_POS, player.toBallDir),
	Assister = task.goCmuRush(FAKE_POS, player.toBallDir, _, AFTER_KICK_FLAG),
	Middle   = task.goCmuRush(linePos(2), player.toBallDir, _, AFTER_KICK_FLAG),
	Fronter  = task.goCmuRush(linePos(3), player.toBallDir, _, AFTER_KICK_FLAG),
	Special  = task.leftBack(),
	Defender = task.rightBack(),
	Goalie   = task.zgoalie(),
	match = "{LR}[SD][AMF]"
},
["Shoot"] = {
	switch = function()
		freeKickDebug()
		if bufcnt(player.kickBall("Receiver"), 1,150) then
		    return "finish" 
		end
	end,
	Leader   = task.zsupport(),
	Receiver = task.zattack(_, _, _, flag.kick + flag.safe),--task.zget(_, _, _, flag.kick + flag.safe),
	Assister = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Fronter  = task.goCmuRush(pos.getOtherPos(2), player.toBallDir, _, AFTER_KICK_FLAG),
	Special  = task.leftBack(),
	Defender = task.rightBack(),
	Goalie   = task.zgoalie(),
	match = "{R}[SD][LAMF]"
},

name = "TestFreeKick",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}