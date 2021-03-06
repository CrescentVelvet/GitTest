--暂停脚本，我方暂停与敌方暂停都会调用，左右需要side调整
local TargetPosG  = CGeoPoint:new_local(-3000,-500)
local TargetPos1  = CGeoPoint:new_local(-2500, -500)
local TargetPos2  = CGeoPoint:new_local(-2000,-500)
local TargetPos3  = CGeoPoint:new_local(-2300, -160)
local TargetPos4  = CGeoPoint:new_local(-2700, 170)
local TargetPos5  = CGeoPoint:new_local(-3000, 500)
local TargetPos6  = CGeoPoint:new_local(-2500,500)
local TargetPos7  = CGeoPoint:new_local(-2000, 500)
gPlayTable.CreatePlay{

firstState = "imuset",
-- firstState = "move",

["imuset"] = {
	switch = function ()
	end,
	Leader   = task.goCmuRush(TargetPos1,math.pi/2,1500,flag.allow_dss),
	Special  = task.goCmuRush(TargetPos2,math.pi/2,1500,flag.allow_dss),
	Middle   = task.goCmuRush(TargetPos3,math.pi/2,1500,flag.allow_dss),
	Defender = task.goCmuRush(TargetPos4,math.pi/2,1500,flag.allow_dss),
	Assister = task.goCmuRush(TargetPos5,math.pi/2,1500,flag.allow_dss),
	Goalie   = task.goCmuRush(TargetPosG,math.pi/2,1500,flag.allow_dss),
	Breaker  = task.goCmuRush(TargetPos6,math.pi/2,1500,flag.allow_dss),
	Center   = task.goCmuRush(TargetPos7,math.pi/2,1500,flag.allow_dss),
	match = "[LSMDACB]"
},

["move"] = {
	switch = function ()
		if bufcnt(
			player.toTargetDist("Assister") <20 and 
			player.toTargetDist("Leader")   <20 and 
			player.toTargetDist("Special")  <20 and 
			player.toTargetDist("Defender") <20 and 
			player.toTargetDist("Middle")   <20 and 
			player.toTargetDist("Goalie")   <20 and
			player.toTargetDist("Breaker")  <20 and
			player.toTargetDist("Center")   <20
			,20,999) then
			return "turn"
		end
	end,
	Leader   = task.goCmuRush(TargetPos1,0,150,flag.allow_dss),
	Special  = task.goCmuRush(TargetPos2,0,150,flag.allow_dss),
	Middle   = task.goCmuRush(TargetPos3,0,150,flag.allow_dss),
	Defender = task.goCmuRush(TargetPos4,0,150,flag.allow_dss),
	Assister = task.goCmuRush(TargetPos5,0,150,flag.allow_dss),
	Breaker  = task.goCmuRush(TargetPos6,0,150,flag.allow_dss),
	Center   = task.goCmuRush(TargetPos7,0,150,flag.allow_dss),
	Goalie   = task.goCmuRush(TargetPosG,0,150,flag.allow_dss),
	match = "[LSMDACB]"
},

["turn"] = {
	switch = function ()
		if bufcnt(true,150) then
			return "turn"
		end
	end,
	Leader   = task.goCmuRush(TargetPos1,0,150,flag.allow_dss),
	Special  = task.goCmuRush(TargetPos2,0,150,flag.allow_dss),
	Middle   = task.goCmuRush(TargetPos3,0,150,flag.allow_dss),
	Defender = task.goCmuRush(TargetPos4,0,150,flag.allow_dss),
	Assister = task.goCmuRush(TargetPos5,0,150,flag.allow_dss),
	Goalie   = task.goCmuRush(TargetPosG,0,150,flag.allow_dss),
	Breaker  = task.goCmuRush(TargetPos6,0,150,flag.allow_dss),
	Center   = task.goCmuRush(TargetPos7,0,150,flag.allow_dss),
	match = "[LSMDACB]"
},


["halt"] = {
	switch = function()

	end,
	["Leader"]   = task.stop(),
	["Special"]  = task.stop(),
	["Assister"] = task.stop(),
	["Defender"] = task.stop(),
	["Middle"]   = task.stop(),
	["Center"]   = task.stop(),
	["Breaker"]  = task.stop(),
	["Goalie"]   = task.stop(),
	match = "[LSADMCB]"
},

name = "Ref_OurTimeoutV1",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
