local pos  = {
	CGeoPoint:new_local(250,150),
	CGeoPoint:new_local(-250,-150)
}
local vel = CVector:new_local(0, 0)
local maxvel = 0
local time = 120
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)
gPlayTable.CreatePlay{

firstState = "run1",
["test"] = {
	switch = function()
	end,
	Leader = task.zmarking("First"),
	match = "[L]"
},

["run1"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Kicker")<5, time) then
			return "run"..2
		end
	end,
	Kicker = task.goCmuRush(pos[1], 0, _, DSS_FLAG),
	match = ""
},

["run2"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Kicker")<5, time) then
			return "run"..1
		end
	end,
	Kicker = task.goCmuRush(pos[2], 0, _, DSS_FLAG),
	match = ""
},

name = "TestFivePass",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999

}
