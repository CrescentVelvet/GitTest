local COR_DEF_POS1 = CGeoPoint:new_local(-50*param.lengthRatio,-50*param.widthRatio)
local SIDE_POS, MIDDLE_POS, INTER_POS = pos.refStopAroundBall()

local LEADER_TASK = function()
	if cond.ourBallPlace() then
		return task.fetchBall(ball.placementPos,0,true)
	end
	return task.goCmuRush(pos.LEADER_STOP_POS, player.toBallDir, _, flag.dodge_ball+flag.allow_dss)
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
	Leader   = LEADER_TASK,
	Special  = task.goPassPos("Leader"),
	Middle   = task.goCmuRush(COR_DEF_POS1, player.toBallDir, _, flag.dodge_ball+flag.allow_dss),
	Defender = task.zback(2,1),
	Center   = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "(L)[SMDC]"
},

["attacker1"] = {
	Leader   = LEADER_TASK,
	Special  = task.goPassPos("Leader"),
	Middle   = task.goCmuRush(COR_DEF_POS1, player.toBallDir, _, flag.dodge_ball+flag.allow_dss),
	Defender = task.zback(2,1),
	Center   = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "{L}[SMDC]"
},

["attacker2"] = {
	Leader   = LEADER_TASK,
	Special  = task.goPassPos("Leader"),
	Middle   = task.zmarking("First"), 
	Defender = task.zback(2,1),
	Center   = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "{L}[SMDC]"
},

["attacker3"] = {
	Leader   = LEADER_TASK,
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.zback(2,1),
	Center   = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "{L}[SMDC]"
},

["attacker4"] = {
	Leader   = LEADER_TASK,
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.zmarking("Third"),
	Center   = task.zback(1,1),
	Goalie   = task.zgoalie(),
	match    = "{L}[SMDC]"
},

["attacker5"] = {
	Leader   = LEADER_TASK,
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.zmarking("Third"),
	Center   = task.zmarking("Fourth"),
	Goalie   = task.zgoalie(),
	match    = "{L}[SMDC]"
},

["attacker6"] = {
	Leader   = LEADER_TASK,
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.zmarking("Third"),
	Center   = task.zmarking("Fourth"),
	Goalie   = task.zgoalie(),
	match    = "{L}[SMDC]"
},

name = "Ref_BallPlace2Stop_6vs6",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}