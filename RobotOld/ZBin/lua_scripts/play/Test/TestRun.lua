local pos  = {
	CGeoPoint:new_local(200,100),
	CGeoPoint:new_local(-200,100),
	CGeoPoint:new_local(-200,-100),
	CGeoPoint:new_local(200,-100)
}
local vel = CVector:new_local(0, 0)
local maxvel=0
local time = 120
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)
gPlayTable.CreatePlay{

firstState = "run1",
["test"] = {
	switch = function()
	end,
	-- Leader = task.leftBack(),
	-- Assister = task.rightBack(),
	-- Goalie = task.zgoalie(),
	Leader = task.zmarking("First"),
	match = "[L]"
},

["run1"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Kicker")<5,time) then
			return "run"..2--math.random(4)
		end
	end,
	Kicker = task.goCmuRush(pos[1],0, _, DSS_FLAG),
	-- Kicker = task.goBezierRush(pos[3],0, _, DSS_FLAG, _, vel),
	match = ""
},
["run2"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Kicker")<5,time) then
			return "run"..3
		end
	end,
	Kicker = task.goCmuRush(pos[3],math.pi, _, DSS_FLAG),
	-- Kicker = task.goBezierRush(pos[4],math.pi, _, DSS_FLAG, _, vel),
	match = ""
},
["run3"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Kicker")<5,time) then
			return "run"..4
		end
	end,
	Kicker = task.goCmuRush(pos[2],0, _, DSS_FLAG),
	-- Kicker = task.goBezierRush(pos[1],0, _, DSS_FLAG, _, vel),
	match = ""
},
["run4"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Kicker")<5,time) then
			return "run"..1--math.random(4)
		end
	end,
	Kicker = task.goCmuRush(pos[4],math.pi, _, DSS_FLAG),
	-- Kicker = task.goBezierRush(pos[2],math.pi, _, DSS_FLAG, _, vel),
	match = ""
},

name = "TestRun",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
