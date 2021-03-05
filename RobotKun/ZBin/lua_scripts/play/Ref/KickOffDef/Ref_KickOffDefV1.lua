local DSS_FLAG = flag.allow_dss + flag.dodge_ball + flag.avoid_half_field + flag.avoid_stop_ball_circle
local function KICKOFF_DEF_POS(str)
	return function()
		local x, y
		x, y = CKickOffDefPos(str)
		return CGeoPoint:new_local(x,y)
	end
end

gPlayTable.CreatePlay{

firstState = "attacker1",

switch = function()
	if cond.ballMoved() then
		return "exit"
	else
		if enemy.attackNum() < 2 then
			return "attacker1"
		elseif enemy.attackNum() >= 5 then
			return "attacker5"
		else
			return "attacker"..enemy.attackNum()
		end
	end
end,

["attacker1"] = {
	Leader   = task.goSpeciPos(KICKOFF_DEF_POS("left"), _, DSS_FLAG),
	Special  = task.goSpeciPos(KICKOFF_DEF_POS("right"), _, DSS_FLAG),
	Assister = task.goSpeciPos(KICKOFF_DEF_POS("middle"), _, DSS_FLAG),
	Middle   = task.rightBack(),
	Defender = task.leftBack(),
	Fronter  = task.defendHead(DSS_FLAG),
	Center   = task.defendMiddle(_,DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[ADMLSFC]"
},

["attacker2"] = {
	Leader   = task.zmarking("First", DSS_FLAG),
	Special  = task.goSpeciPos(KICKOFF_DEF_POS("right"), _, DSS_FLAG),
	Assister = task.goSpeciPos(KICKOFF_DEF_POS("middle"), _, DSS_FLAG),
	Middle   = task.rightBack(),
	Defender = task.leftBack(),
	Fronter  = task.defendHead(DSS_FLAG),
	Center   = task.goSpeciPos(KICKOFF_DEF_POS("left"), _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[ADMLSFC]"
},

["attacker3"] = {
	Leader   = task.zmarking("First", DSS_FLAG),
	Special  = task.zmarking("Second", DSS_FLAG),
	Assister = task.goSpeciPos(KICKOFF_DEF_POS("middle"), _, DSS_FLAG),
	Middle   = task.goSpeciPos(KICKOFF_DEF_POS("right"), _, DSS_FLAG),
	Defender = task.goSpeciPos(KICKOFF_DEF_POS("left"), _, DSS_FLAG),
	Fronter  = task.singleBack(),
	Center   = task.defendHead(DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[ADMLSFC]"
},

["attacker4"] = {
        Leader   = task.zmarking("First", DSS_FLAG),
        Special  = task.zmarking("Second", DSS_FLAG),
        Assister = task.goSpeciPos(KICKOFF_DEF_POS("middle"), _, DSS_FLAG),
        Middle   = task.rightBack(),
        Defender = task.zmarking("Third", DSS_FLAG),
        Fronter  = task.leftBack(),
        Center   = task.defendHead(DSS_FLAG),
        Goalie   = task.zgoalie(),
        match    = "[ADMLSFC]"
},

["attacker5"] = {
	Leader   = task.zmarking("First", DSS_FLAG),
	Special  = task.zmarking("Second", DSS_FLAG),
	Assister = task.goSpeciPos(KICKOFF_DEF_POS("middle"), _, DSS_FLAG),
	Middle   = task.zmarking("Third", DSS_FLAG),
	Defender = task.leftBack(),
	Fronter  = task.rightBack(),
	Center   = task.zmarking("Fourth", DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[ADMLSFC]"
},

name = "Ref_KickOffDefV1",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
