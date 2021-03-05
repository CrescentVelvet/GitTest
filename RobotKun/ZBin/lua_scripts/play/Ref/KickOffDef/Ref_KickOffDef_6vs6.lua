local DSS_FLAG = flag.allow_dss + flag.dodge_ball + flag.avoid_half_field + flag.avoid_stop_ball_circle
local function KICKOFF_DEF_POS(str)
	return function()
		local x, y
		x, y = CKickOffDefPos(str)
		return CGeoPoint:new_local(x,y)
	end
end

local MiddlePos = CGeoPoint:new_local(-2500,0)
local DefenderPos = CGeoPoint:new_local(-3300,0)

gPlayTable.CreatePlay{

firstState = "attacker1",

switch = function()
	if cond.ballMoved() then
		return "exit"
	else
		-- if defenceSquence:attackerAmount() < 2 then
		-- 	return "attacker1"
		-- elseif defenceSquence:attackerAmount() >= 5 then
		-- 	return "attacker5"
		-- else
		-- 	return "attacker"..defenceSquence:attackerAmount()
		-- end
		return "attacker1"
	end
end,

["attacker1"] = {
	Leader   = task.goSpeciPos(KICKOFF_DEF_POS("left"), _, DSS_FLAG),
	Special  = task.goSpeciPos(KICKOFF_DEF_POS("right"), _, DSS_FLAG),
	Assister = task.goSpeciPos(KICKOFF_DEF_POS("middle"), _, DSS_FLAG),
	Middle   = task.goSpeciPos(MiddlePos, _, DSS_FLAG),
	Defender = task.goSpeciPos(DefenderPos, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[ADMLS]"
},

["attacker2"] = {
	Leader   = task.zmarking("First", DSS_FLAG),
	Special  = task.zmarking("Second", DSS_FLAG),
	Assister = task.goSpeciPos(KICKOFF_DEF_POS("middle"), _, DSS_FLAG),
	Middle   = task.zback(1,1,_,flag.avoid_stop_ball_circle),
	Defender = task.defendMiddle(_,DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[MD][ALS]"
},

["attacker3"] = {
	Leader   = task.zmarking("First", DSS_FLAG),
	Special  = task.zmarking("Second", DSS_FLAG),
	Assister = task.goSpeciPos(KICKOFF_DEF_POS("middle"), _, DSS_FLAG),
	Middle   = task.zback(2,1,_,flag.avoid_stop_ball_circle),
	Defender = task.zback(2,2,_,flag.avoid_stop_ball_circle),
	Goalie   = task.zgoalie(),
	match    = "[DM][ALS]"
},

["attacker4"] = {
    Leader   = task.zmarking("First", DSS_FLAG),
    Special  = task.zmarking("Second", DSS_FLAG),
    Assister = task.zmarking("Third", DSS_FLAG),
	Middle   = task.zback(2,1,_,flag.avoid_stop_ball_circle),
	Defender = task.zback(2,2,_,flag.avoid_stop_ball_circle),
    Goalie   = task.zgoalie(),
    match    = "[DM][ALS]"
},

["attacker5"] = {
    Leader   = task.zmarking("First", DSS_FLAG),
    Special  = task.zmarking("Second", DSS_FLAG),
    Assister = task.zmarking("Third", DSS_FLAG),
	Middle   = task.zback(2,1,_,flag.avoid_stop_ball_circle),
	Defender = task.zback(2,2,_,flag.avoid_stop_ball_circle),
    Goalie   = task.zgoalie(),
    match    = "[DM][ALS]"
},

["attacker6"] = {
    Leader   = task.zmarking("First", DSS_FLAG),
    Special  = task.zmarking("Second", DSS_FLAG),
    Assister = task.zmarking("Third", DSS_FLAG),
	Middle   = task.zback(2,1,_,flag.avoid_stop_ball_circle),
	Defender = task.zback(2,2,_,flag.avoid_stop_ball_circle),
    Goalie   = task.zgoalie(),
    match    = "[DM][ALS]"
},

name = "Ref_KickOffDef_6vs6",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
