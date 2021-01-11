--防robodragon的markingFront脚本
local leftUp = CGeoPoint:new_local(0,-param.pitchWidth/2)
local rightDown = CGeoPoint:new_local(-param.pitchLength/2,param.pitchWidth/2)
-- local left = CGeoPoint:new_local(0,-150)
-- local right =CGeoPoint:new_local(-300,110)
local KICKDEFTASK = task.defendKick(100,leftUp,rightDown,1)
-- local protectPos = function ()
-- 	return ball.pos()+Utils.Polar2Vector(50,Utils.Normalize(ball.velDir()-math.pi/2))
-- end
-- local left = CGeoPoint:new_local(-param.pitchLength/2,-param.goalWidth/2-10)
-- local right = CGeoPoint:new_local(-param.pitchLength/2,param.goalWidth/2+10)

-- local KICKDEFTASK = task.defendKick(150,left,right,2)

local MID_DEF_POS1 = CGeoPoint:new_local(0, -150)
local MID_DEF_POS2 = CGeoPoint:new_local(0, 150)
local MID_DEF_POS3 = CGeoPoint:new_local(100, -50)

local FRONT_POS = function()
	return CGeoPoint:new_local(200, -170*ball.refAntiY())
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
		return task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area)
	end
end

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= param.maxPlayer and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
	elseif cond.ballMoved() then
		return "exit"
	else
		if enemy.situChanged() and 
			enemy.attackNum() <= param.maxPlayer and enemy.attackNum() > 0 then
			return "attacker"..enemy.attackNum()
		end
	end
end,

["beginning"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.goPassPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Fronter  = task.goSpeciPos(MID_DEF_POS1, player.toBallDir, DSS_FLAG),
	Center   = task.goSpeciPos(MID_DEF_POS2, player.toBallDir, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMFC]"
},

["attacker1"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.goPassPos("Leader"),
	Fronter  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Center   = task.goSpeciPos(MID_DEF_POS3, player.toBallDir, DSS_FLAG),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMFC]"
},

["attacker2"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Fronter  = task.shoot(),
	Center   = task.zmarking("Second", flag.avoid_stop_ball_circle),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][M][DA][C][F]"
},

["attacker3"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle),
	Fronter  = task.defendHead(),
	Center   = task.zmarking("Third", flag.avoid_stop_ball_circle),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][M][DA][F][C]"
},

["attacker4"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third", flag.avoid_stop_ball_circle),
	Assister = task.leftBack(),
	Center   = task.rightBack(),
	Fronter  = task.defendHead(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][AC][MD][F]"
},

["attacker5"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third", flag.avoid_stop_ball_circle),
	Assister = task.zmarking("Fourth", flag.avoid_stop_ball_circle),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][AMD][FC]"
},

["attacker6"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third", flag.avoid_stop_ball_circle),
	Assister = task.zmarking("Fourth", flag.avoid_stop_ball_circle),
	Fronter  = task.zmarking("Fifth", flag.avoid_stop_ball_circle),
	Center   = task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][AMD][C][F]"
},

["attacker7"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third", flag.avoid_stop_ball_circle),
	Assister = task.zmarking("Fourth", flag.avoid_stop_ball_circle),
	Fronter  = task.zmarking("Fifth", flag.avoid_stop_ball_circle),
	Center   = task.zmarking("Sixth", flag.avoid_stop_ball_circle),
	Goalie   = task.zgoalie(),
	match    = "[L][S][AMD][C][F]"
},

["attacker8"] = {
	Leader = KICKER_TASK(),--KICKDEFTASK,
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third", flag.avoid_stop_ball_circle),
	Assister = task.zmarking("Fourth", flag.avoid_stop_ball_circle),
	Fronter  = task.zmarking("Fifth", flag.avoid_stop_ball_circle),
	Center   = task.zmarking("Sixth", flag.avoid_stop_ball_circle),
	Goalie   = task.zgoalie(),
	match    = "[L][S][AMD][C][F]"
},

["attackDef"] = { 
	Leader = KICKER_TASK(),--task.shoot(),-- task.goSpeciPos(FRONT_POS),   -- gai wei shoot(), ke neng you wen ti
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Assister = task.continue(),
	Fronter  = task.continue(),
	Center   = task.continue(),
	Goalie   = task.zgoalie(),
	match    = "{LDAMSFC}"
},

["norPass"] = {
	Leader = KICKER_TASK(),--task.shoot(),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Fronter  = task.markingDir("Third", player.toBallDir),
	Center   = task.defendHead(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[S][L][AD][M][C][F]"
},

["norDef"] = {
	Leader = KICKER_TASK(),--task.shoot(),
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle),
	Fronter  = task.zmarking("Third", flag.avoid_stop_ball_circle),
	Center   = task.defendHead(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][AD][SMF][C]"
},

name = "Ref_MiddleDefV1",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}