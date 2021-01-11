local pos  = {
	CGeoPoint:new_local(280,280),
	CGeoPoint:new_local(-280,280),
	CGeoPoint:new_local(-280,-280),
	CGeoPoint:new_local(280,-280)
}

local ChinaOpenStart = CGeoPoint:new_local(0, 200)
local ChinaOpenEnd = CGeoPoint:new_local(220, 0)

-- local mode = task.goCmuRush
local mode = task.goChinaTecRush	
-- local mode = task.goBezierRush

local maxvel= 0
local time = 20
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)
local IMMORTAL_FLAG = bit:_or(bit:_or(flag.allow_dss, flag.use_immortal), flag.dodge_ball)

gPlayTable.CreatePlay{

firstState = "run1",

["run1"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Goalie")<5, time) then
			return "run"..2--math.random(4)
		end
	end,

	Goalie = task.goCmuRush(ChinaOpenStart, -math.pi*1/2, _, DSS_FLAG),
	match = ""
},

["run2"] = {
	switch = function()
		-- if bufcnt(player.toTargetDist("Kicker")<5,time) then
		return "run"..2
		-- end
	end,
	Goalie = mode(ChinaOpenEnd, ChinaOpenStart),

	match = ""
},

name = "TestChinaTech",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

