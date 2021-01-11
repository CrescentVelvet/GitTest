local PASS_POS = CGeoPoint:new_local(200, 150)

local ZPASS = true

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

local FIX_BUF = 0

gPlayTable.CreatePlay{
firstState = "start",

["start"] = {
	switch = function ()
		if cond.isNormalStart() then
			return "getball"
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(-20, -5), _, DSS_FLAG),
	Assister = task.goSpeciPos(CGeoPoint:new_local(-15, -240), _, DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(-15, 70), _, DSS_FLAG ),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(-200, 100), _, DSS_FLAG),
	Defender = task.singleBack(),
	Center   = task.goSpeciPos(CGeoPoint:new_local(-200, -100), _, DSS_FLAG),
	Fronter  = task.goSpeciPos(CGeoPoint:new_local(-15, 240), _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "(L)(DMCASF)"
},

-- ["temp1"] = {
-- 	switch = function ()
-- 		if bufcnt(true, 120) then
-- 			return "temp2"
-- 		end
-- 	end,
-- 	Leader   = task.continue(),
-- 	Assister = task.continue(),
-- 	Special  = task.goSpeciPos(CGeoPoint:new_local(-15, 70), _, DSS_FLAG ),--continue(),
-- 	Middle   = task.continue(),
-- 	Defender = task.continue(),
-- 	Center   = task.continue(),
-- 	Fronter   = task.goSpeciPos(CGeoPoint:new_local(-15, 240), _, DSS_FLAG),
-- 	Goalie   = task.zgoalie(),
-- 	match    = "{LASMDCF}"
-- },

-- ["temp2"] = {
-- 	switch = function ()
-- 		if bufcnt(true, 120) then
-- 			return "temp3"
-- 		end
-- 	end,
-- 	Leader   = task.continue(),
-- 	Assister = task.continue(),
-- 	Special  = task.continue(),
-- 	Middle   = task.continue(),
-- 	Defender = task.continue(),
-- 	Center   = task.continue(),
-- 	Fronter   = task.continue(), --goSpeciPos(CGeoPoint:new_local(-15, 90)),
-- 	Goalie   = task.zgoalie(),
-- 	match    = "{LASMDCF}"
-- },

["getball"] = {
	switch = function ()
		if bufcnt(player.infraredCount("Leader") > 20, 1, 100) then
			return "pass"
		end
	end,
	Leader   = task.slowGetBall(PASS_POS, _, false),
	Assister = task.continue(),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Center   = task.continue(),
	Fronter  = task.continue(),
	Goalie   = task.zgoalie(),
	match    = "{LASMDCF}"
},

["pass"] = {
	switch = function ()
		if bufcnt(player.kickBall("Leader"), 1, 80) then
			_, FIX_BUF = cp.toTargetTime(PASS_POS)
			return "fix"
		end
	end,
	Leader   = task.zget(PASS_POS,_,_,flag.kick+flag.chip+flag.dribble),
	Middle   = task.continue(),
	Special  = task.continue(),
	Defender = task.rightBack(),
	Assister = task.leftBack(),
	Center   = task.continue(),
	Fronter  = task.continue(),
	Goalie   = task.zgoalie(),
	match    = "{L}[AD][MSCF]"
},

["fix"] = {
	switch = function ()
		if bufcnt(true, FIX_BUF) then
			return "exit"
		end
	end,
	Leader   = task.stop(), 
	Middle   = task.goSpeciPos(PASS_POS, _, flag.not_avoid_our_vehicle),
	Special  = task.goSpeciPos(CGeoPoint:new_local(250, -200), _, DSS_FLAG),
	Defender = task.rightBack(),
	Assister = task.leftBack(),
	Center   = task.goSpeciPos(CGeoPoint:new_local(250, 200), _, DSS_FLAG),
	Fronter  = task.goSpeciPos(CGeoPoint:new_local(-200, 0), _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{L}[AD][M][SCF]"
},

name = "Ref_KickOffV801",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
