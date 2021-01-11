local PASS_POS = pos.chipPassForTouch(CGeoPoint:new_local(100, 80))

local ZPASS = true

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

gPlayTable.CreatePlay{
firstState = "start",

["start"] = {
	switch = function ()
		if cond.isNormalStart() then
			return "temp1"
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(-20, -5), _, DSS_FLAG),
	Receiver = task.goSpeciPos(CGeoPoint:new_local(-20, 100), _, DSS_FLAG),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(-20, -100), _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{LRM}"
},

["temp1"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Receiver") < 100, 60) then
			return "temp3"
		end
	end,
	Leader   = task.continue(),
	Receiver = task.goSpeciPos(CGeoPoint:new_local(-10, 100), _, DSS_FLAG),--continue(),
	Middle   = task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "{LRM}"
},

-- ["temp2"] = {
-- 	switch = function ()
-- 		if bufcnt(true, 120) then
-- 			return "pass"
-- 		end
-- 	end,
-- 	Leader   = task.continue(),
-- 	Receiver = task.continue(),
-- 	Middle   = task.continue(),
-- 	Goalie   = task.zgoalie(),
-- 	match    = "{LRM}"
-- },

["temp3"] = {
	switch = function ()
		if bufcnt(player.kickBall("Leader"), 1, 80) then
			return "pass"
		end
	end,
	Leader   = task.zget(PASS_POS,_,_,flag.kick+flag.chip+flag.dribble),
	Receiver = task.continue(),
	Middle   = task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "{LRM}"
},

["pass"] = {
	switch = function ()
		if bufcnt(true, 30) then
			return "exit"
		end
	end,
	Leader   = task.goPassPos("Middle"),
	Receiver = task.zget(_,_,_,flag.kick+flag.dribble),
	-- Receiver = task.goSpeciPos(pos.passForTouch(PASS_POS), dir.evaluateTouch(CGeoPoint:new_local(param.pitchLength/2, 0)), flag.not_avoid_our_vehicle),
	Middle   = task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "{LRM}"
},

-- ["fix"] = {
-- 	switch = function ()
-- 		if bufcnt(true, cp.getFixBuf(PASS_POS)) then
-- 			return "kick"
-- 		end
-- 	end,
-- 	Leader   = task.stop(), 
-- 	Receiver = task.goSpeciPos(PASS_POS, dir.evaluateTouch(CGeoPoint:new_local(param.pitchLength/2, 0)), flag.not_avoid_our_vehicle),
-- 	Middle   = task.singleBack(),
-- 	Goalie   = task.zgoalie(),
-- 	match    = "{LRM}"
-- },

-- ["kick"] = {
--     switch = function ()
-- 		if bufcnt(player.kickBall("Receiver"), 1, 60) then
-- 			return "exit"
-- 		end
-- 	end,
-- 	Leader   = task.goPassPos("Receiver"),
-- 	Receiver = ZPASS and task.zpass() or task.zget(_,_,_,flag.kick),
-- 	Middle   = task.singleBack(),
-- 	Goalie   = task.zgoalie(),
-- 	match    = "{LRM}"
-- },

name = "Ref_KickOffV401",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
