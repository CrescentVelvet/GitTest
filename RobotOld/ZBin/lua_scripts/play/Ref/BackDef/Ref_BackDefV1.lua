--走默认点优先
local BACK_DEF_POS1 = CGeoPoint:new_local(0, -150)
local BACK_DEF_POS2 = CGeoPoint:new_local(0, 150)
local BACK_DEF_POS3 = CGeoPoint:new_local(100, -50)


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

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
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
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.goPassPos("Leader", flag.dodge_ball+flag.allow_dss+flag.avoid_stop_ball_circle),
	Middle   = task.defendDefault(2),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Fronter  = task.goSpeciPos(BACK_DEF_POS1, player.toBallDir, flag.avoid_stop_ball_circle),
	Center   = task.goSpeciPos(BACK_DEF_POS2, player.toBallDir, flag.avoid_stop_ball_circle),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMFC]"
},

["attacker1"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.goPassPos("Leader",flag.dodge_ball+flag.allow_dss+flag.avoid_stop_ball_circle),
	Middle   = task.defendDefault(2),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Fronter  = task.defendMiddle(_,flag.avoid_stop_ball_circle),
	Center   = task.defendHead(flag.avoid_stop_ball_circle),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMFC]"
},

["attacker2"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.defendDefault(2),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Fronter  = task.defendMiddle(_,flag.avoid_stop_ball_circle),
	Center   = task.defendHead(flag.avoid_stop_ball_circle),
	Goalie   = task.zgoalie(),
	match    = "[L][S][DA][M][FC]"
},

["attacker3"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.defendDefault(2),
	Assister = task.leftBack(),
	Fronter  = task.rightBack(),
	Center   = task.defendHead(flag.avoid_stop_ball_circle),
	Goalie   = task.zgoalie(),
	match    = "[L][S][A][D][M][F][C]"
},

["attacker4"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third",flag.avoid_stop_ball_circle),
	Assister = task.leftBack(),
	Fronter  = task.rightBack(),
	Center   = task.defendHead(flag.avoid_stop_ball_circle),
	Goalie   = task.zgoalie(),
	match    = "[L][S][A][MD][FC]"
},

["attacker5"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third",flag.avoid_stop_ball_circle),
	Assister = task.zmarking("Fourth",flag.avoid_stop_ball_circle),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][AMD][FC]"
},

["attacker6"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third",flag.avoid_stop_ball_circle),
	Assister = task.zmarking("Fourth",flag.avoid_stop_ball_circle),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][AMD][FC]"
},

["attacker7"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third",flag.avoid_stop_ball_circle),
	Assister = task.zmarking("Fourth",flag.avoid_stop_ball_circle),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][AMD][FC]"
},

["attacker8"] = {
	Leader = KICKER_TASK(),-- task.defendKick(_,_,_,_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third",flag.avoid_stop_ball_circle),
	Assister = task.zmarking("Fourth",flag.avoid_stop_ball_circle),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][AMD][FC]"
},

["norPass"] = {
	Leader = KICKER_TASK(),-- task.advance(_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third",flag.avoid_stop_ball_circle),
	Assister = task.zmarking("Fourth",flag.avoid_stop_ball_circle),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[S][L][ADM][FC]"
},

["norDef"] = {
	Leader = KICKER_TASK(),-- task.advance(_,flag.avoid_stop_ball_circle),
	Special  = task.zmarking("First",flag.avoid_stop_ball_circle),
	Middle   = task.zmarking("Second",flag.avoid_stop_ball_circle),
	Defender = task.zmarking("Third",flag.avoid_stop_ball_circle),
	Assister = task.zmarking("Fourth",flag.avoid_stop_ball_circle),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][ADM][FC]"
},

name = "Ref_BackDefV1",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}