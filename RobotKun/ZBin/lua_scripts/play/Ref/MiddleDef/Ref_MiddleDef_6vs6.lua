--防robodragon的markingFront脚本
local leftUp = CGeoPoint:new_local(0,-param.pitchWidth/2)
local rightDown = CGeoPoint:new_local(-param.pitchLength/2,param.pitchWidth/2)
-- local left = CGeoPoint:new_local(0,-150)
-- local right =CGeoPoint:new_local(-300,110)
local KICKDEFTASK = task.defendKick(1000,leftUp,rightDown,1)
-- local protectPos = function ()
-- 	return ball.pos()+Utils.Polar2Vector(50,Utils.Normalize(ball.velDir()-math.pi/2))
-- end
-- local left = CGeoPoint:new_local(-param.pitchLength/2,-param.goalWidth/2-10)
-- local right = CGeoPoint:new_local(-param.pitchLength/2,param.goalWidth/2+10)

-- local KICKDEFTASK = task.defendKick(150,left,right,2)

local MID_DEF_POS1 = CGeoPoint:new_local(-3300,-1300)
local MID_DEF_POS2 = CGeoPoint:new_local(-3300,1300)
local MID_DEF_POS3 = CGeoPoint:new_local(-3400,0)

local DEFEND_POS = function ()
	return function ()
		if ball.posY() >= 0 then
			return MID_DEF_POS1
		else
			return MID_DEF_POS2
		end
	end
end

local FRONT_POS = function()
	return CGeoPoint:new_local(2000, -1700*ball.refAntiY())
end
local checkPosX = 0
local checkPosY = 0
local checkDir = 0
local bestEnemy = 0
local kickPos = CGeoPoint:new_local(0,0)


local DSS_FLAG = flag.allow_dss
local markFrontOut = function ()
	return vision:ballVelValid() and ball.velMod() < 50
end

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
		return task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle)
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
		-- if enemy.situChanged() and 
		if defenceSquence:attackerAmount() <= 6 and defenceSquence:attackerAmount() > 0 then
			return "attacker"..defenceSquence:attackerAmount()
		end
	end
end,

["beginning"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.goPassPos("Leader"),
	Middle   = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][DA][M]S]"
},

["attacker1"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.zmarking("First", flag.avoid_stop_ball_circle),
	Middle   = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][AM][S][D]"
},

["attacker2"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Middle   = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zback(2,1),
	Defender = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][AD][M][S]"
},

["attacker3"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Defender = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zback(2,1),
	Middle   = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][AM][D][S]"
},

["attacker4"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Defender = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zback(2,1),
	Middle   = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][AM][D][S]"
},

["attacker5"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Defender = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zback(2,1),
	Middle   = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][AM][D][S]"
},

["attacker6"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Defender = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zback(2,1),
	Middle   = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][AM][D][S]"
},

["attackDef"] = { 
	Leader = KICKER_TASK(),--task.shoot(),-- task.goSpeciPos(FRONT_POS),   -- gai wei shoot(), ke neng you wen ti
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Assister = task.continue(),
	Goalie   = task.zgoalie(),
	match    = "{LDAMS}"
},

["norPass"] = {
	Leader = KICKER_TASK(),--task.shoot(),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[S][L][AD][M]"
},

["norDef"] = {
	Leader = KICKER_TASK(),--task.shoot(),
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][AD][SM]"
},

name = "Ref_MiddleDef_6vs6",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}