--走默认点优先
local BACK_DEF_POS1 = CGeoPoint:new_local(-3300,-1300)
local BACK_DEF_POS2 = CGeoPoint:new_local(-3300,1300)
local BACK_DEF_POS3 = CGeoPoint:new_local(-3400,0)

local BP_FINISH = false
local KICKER_TASK = function()
	return function()
		if cond.ballPlaceFinish() then
			BP_FINISH = true
		end
		if cond.ballPlaceUnfinish() then
			BP_FINISH = false
		end
		if cond.ourBallPlace() and not (BP_FINISH) then
			return task.fetchBall(ball.placementPos,_,true)
		end
		return task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle)--task.defendKick(100,leftUp,rightDown,1)--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area)
	end
end

local DEFEND_POS = function ()
	return function ()
		if ball.posY() >= 0 then
			return BACK_DEF_POS1
		else
			return BACK_DEF_POS2
		end
	end
end

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and
		defenceSquence:attackerAmount() <= 6 and defenceSquence:attackerAmount() > 0 then
		return "attacker"..defenceSquence:attackerAmount()
	elseif cond.ballMoved() then
		return "exit"
	else
		if defenceSquence:attackerAmount() <= param.maxPlayer and defenceSquence:attackerAmount() > 0 then
			return "attacker"..defenceSquence:attackerAmount()
		end
	end
end,

["beginning"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Middle   = task.defendDefault(2),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][DA][S][M]"
},

["attacker1"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.defendHead(flag.avoid_stop_ball_circle),
	Middle   = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][AD][M][S]"
},

["attacker2"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][S][M][DA]"
},

["attacker3"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zback(),
	Goalie   = task.zgoalie(),
	match    = "[L][MS][D][A]"
},

["attacker4"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third",flag.avoid_stop_ball_circle),
	Assister = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Goalie   = task.zgoalie(),
	match    = "[L][SM][A][D]"
},

["attacker5"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third",flag.avoid_stop_ball_circle),
	Assister = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Goalie   = task.zgoalie(),
	match    = "[L][SM][A][D]"
},

["attacker6"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third",flag.avoid_stop_ball_circle),
	Assister = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Goalie   = task.zgoalie(),
	match    = "[L][SM][A][D]"
},

["norPass"] = {
	Leader = KICKER_TASK(),-- task.advance(_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[AD][L][SM]"
},

["norDef"] = {
	Leader = KICKER_TASK(),-- task.advance(_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("Third",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][S][ADM]"
},

name = "Ref_BackDefV1_6vs6",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}