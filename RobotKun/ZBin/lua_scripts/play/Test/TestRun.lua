local testPos  = {
	CGeoPoint:new_local(2000,-2300),
	CGeoPoint:new_local(1000,-300),
	CGeoPoint:new_local(-2000,2300),
	CGeoPoint:new_local(4000,-3000)
}
local vel = CVector:new_local(0, 0)
local maxvel=0
local time = 120
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)
gPlayTable.CreatePlay{

firstState = "test",
["test"] = {
	switch = function()

	end,
	Leader = task.leftBack(),
	Assister = task.rightBack(),
	-- Goalie = task.zgoalie(),
	Kicker = task.zmarking("Zero"),
	-- Kicker = task.zget(testPos[2],_,_,flag.kick),
	-- Tier = task.goCmuRush(testPos[2],0, _, DSS_FLAG),
	-- Kicker = task.goCmuRush(testPos[1],0, _, DSS_FLAG),
	match = "[LA][K]"
},

["test1"] = {
	switch = function()
		if player.kickBall("Tier") then
			return "test"
		end
	end,
	-- Leader = task.leftBack(),
	-- Assister = task.rightBack(),
	-- Goalie = task.zgoalie(),
	-- Leader = task.zmarking("First"),
	Tier = task.zget(testPos[3],_,_,flag.kick),
	Kicker = task.goCmuRush(testPos[3],0, _, DSS_FLAG),
	-- Kicker = task.goCmuRush(testPos[1],0, _, DSS_FLAG),
	match = ""
},

["run1"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Kicker")<10,time) then
			return "run"..2--math.random(4)
		end
	end,
	Kicker = task.goCmuRush(testPos[1],math.pi, _, DSS_FLAG),
	-- Kicker = task.goBezierRush(testPos[3],0, _, DSS_FLAG, _, vel),
	match = ""
},
["run2"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Kicker")<10,time) then
			return "run"..1
		end
	end,
	Kicker = task.goCmuRush(testPos[3],0, _, DSS_FLAG),
	-- Kicker = task.goBezierRush(testPos[4],math.pi, _, DSS_FLAG, _, vel),
	match = ""
},
["run3"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Kicker")<5,time) then
			return "run"..4
		end
	end,
	Kicker = task.goCmuRush(testPos[2],0, _, DSS_FLAG),
	-- Kicker = task.goBezierRush(testPos[1],0, _, DSS_FLAG, _, vel),
	match = ""
},
["run4"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Kicker")<5,time) then
			return "run"..1--math.random(4)
		end
	end,
	Kicker = task.goCmuRush(testPos[4],math.pi, _, DSS_FLAG),
	-- Kicker = task.goBezierRush(testPos[2],math.pi, _, DSS_FLAG, _, vel),
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
