local PASS_POS = CGeoPoint:new_local(300, 1000)

local ZPASS = true

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

local FIX_BUF = 0

local noname = 0

local RECEIEVE_POS = function ()
	local chip_pos = ball.chipPredictPos()
	if chip_pos:x() > 9000 then
		return PASS_POS
	else
		return chip_pos
	end
end

gPlayTable.CreatePlay{
firstState = "start",

["start"] = {
	switch = function ()
		if cond.isNormalStart() then
			return "getdir"
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(-200, -200), _, DSS_FLAG),
	Assister = task.goSpeciPos(CGeoPoint:new_local(-150, -2000), _, DSS_FLAG),
	Special  = task.goSpeciPos(CGeoPoint:new_local(-150, 2000), _, DSS_FLAG ),
	Center   = task.goSpeciPos(CGeoPoint:new_local(-2500, -500), _, DSS_FLAG),
	Fronter  = task.goSpeciPos(CGeoPoint:new_local(-2500, 500), _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "(L)(ACSF)"
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
["getdir"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 200, 20)then
			return "getball"
		end
	end,
	Leader = task.staticGetBall(PASS_POS,false),
	Assister = task.continue(),
	Special  = task.continue(),
	Center   = task.continue(),
	Fronter  = task.continue(),
	Goalie   = task.zgoalie(),
	match    = "{LACSF}"
},

["getball"] = {
	switch = function ()
		if player.infraredOn("Leader") then
			return "pass"
		end
	end,
	Leader   = {SlowGetBall{pos = PASS_POS, dir = player.toPointDir(PASS_POS)}},
	Assister = task.continue(),
	Special  = task.continue(),
	Center   = task.continue(),
	Fronter  = task.continue(),
	Goalie   = task.zgoalie(),
	match    = "{LACSF}"
},

["pass"] = {
	switch = function ()
		if bufcnt(player.kickBall("Leader"), 1, 20) then
			noname, FIX_BUF = cp.toTargetTime(PASS_POS)
			return "fix"
		end
	end,
	Leader   = task.zget(PASS_POS,_,_,flag.kick+flag.norush+flag.force_kick),
	Assister = task.goSpeciPos(CGeoPoint:new_local(-150, -2000), _, DSS_FLAG),
	Special  = task.continue(),
	Center   = task.zback(2,1),
	Fronter  = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "{L}[CF][AS]"
},

["fix"] = {
	switch = function ()
		if bufcnt(true, FIX_BUF*0.5) then
			return "exit"
		end
	end,
	Leader   = task.stop(), 
	Assister = task.goSpeciPos(PASS_POS, _, flag.not_avoid_our_vehicle),
	Special  = task.goSpeciPos(CGeoPoint:new_local(2500, -2000), _, DSS_FLAG),
	Center   = task.zback(2,1),
	Fronter  = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "{L}[CF][A][S]"
},

name = "Ref_KickOffV601",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
