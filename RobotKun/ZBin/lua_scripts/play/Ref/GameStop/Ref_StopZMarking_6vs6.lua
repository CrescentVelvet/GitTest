local STOP_POS1 = pos.LEADER_STOP_POS
local WAIT_POS = CGeoPoint:new_local(0, 0)
local SIDE_POS, MIDDLE_POS, INTER_POS = pos.refStopAroundBall()
local LEADER_TASK = function ()
	if ball.posX() < 0 then
		return task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.dodge_ball+flag.allow_dss+flag.free_kick)
	else
		return task.zmarking("Third", flag.avoid_stop_ball_circle+flag.free_kick)
	end
end
gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()	
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
	else
		if cond.isGameOn() then
			return "finish"
		elseif enemy.situChanged() and
			enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
			return "attacker"..enemy.attackNum()
		end
	end
end,
-- headback sideback defend middle
["beginning"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.goPassPos("Leader", flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Middle   = task.goCmuRush(WAIT_POS, player.toBallDir, _, flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L](DA)[SM]"
},

["attacker1"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.goPassPos("Leader", flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Middle   = task.goCmuRush(INTER_POS, player.toBallDir, _, flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "(L)(DA)[SM]"
},

["attacker2"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.goPassPos("Leader", flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender = task.zmarking("Third", flag.avoid_stop_ball_circle+flag.free_kick),
	Assister = task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "[L](A)[DSM]"
},

["attacker3"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle+flag.free_kick),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender  = task.zmarking("Third", flag.avoid_stop_ball_circle+flag.free_kick),
	Assister = task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "(L)(A)[SMD]"
},

["attacker4"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.zmarking("Second", flag.avoid_stop_ball_circle+flag.free_kick),
	Middle   = task.zmarking("Third", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L](DA)[SM]"
},

["attacker5"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.zmarking("Second", flag.avoid_stop_ball_circle+flag.free_kick),
	Middle   = task.zmarking("Third", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L](DA)[SM]"
},

["attacker6"] = {
	Leader   = LEADER_TASK,
	Special  = task.zmarking("Second", flag.avoid_stop_ball_circle+flag.free_kick),
	Middle   = task.zmarking("Third", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "(L)(DA)[SM]"
},



name = "Ref_StopZMarking_6vs6",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}