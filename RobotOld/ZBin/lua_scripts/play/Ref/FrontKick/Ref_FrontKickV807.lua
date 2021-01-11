-- prototype
-- 三车 两传一射（chip & touch & touch )
--by Wang 

local PASS_POS1 = CGeoPoint:new_local(500, -360)
local WAIT_POS2 = ball.refAntiYPos(CGeoPoint:new_local(350, 90))
local PASS_POS2 = pos.passForTouch(WAIT_POS2)
local WAIT_POS1 = ball.refAntiYPos(PASS_POS1)
--local PASS_POS3 = ball.refSyntYPos(CGeoPoint:new_local(350, 120))
local PASS_POS2_TMP = ball.refSyntYPos(CGeoPoint:new_local(-110, 0))
local FAKE_POS1 = ball.refSyntYPos(CGeoPoint:new_local(330, 250))
local FAKE_POS2 = ball.refSyntYPos(CGeoPoint:new_local(560, 180))
local FAKE_POS3 = ball.refSyntYPos(CGeoPoint:new_local(550, 140))

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

gPlayTable.CreatePlay{
firstState = "start",

["start"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Leader") < 20, 20) then
			return "fakeAction"
		end
	end,
	Assister = task.staticGetBall(PASS_POS1),--task.goBackBallV2(0, 10),
	Leader   = task.goSpeciPos(WAIT_POS1, player.toPlayerDir("Assister"), DSS_FLAG),
	Breaker  = task.goSpeciPos(PASS_POS2_TMP, _, DSS_FLAG),
	match    = "{A}[LB]"
},

["fakeAction"] = {
	switch = function()
		if  bufcnt(true, 30) then
			return "pass1"
		end
	end,
	Assister = task.staticGetBall(PASS_POS1),--task.goBackBallV2(0, 10),
	Leader   = task.goSpeciPos(WAIT_POS1, player.toPlayerDir("Assister")),
	Breaker  = task.continue(),
	match    = "{A}{LB}"
},

["pass1"] = {
	switch = function()
		if  bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 30, 1, 150) then
			return "pass2"
		end
	end,
	Assister = task.getBallV4(PASS_POS1, _, _, flag.kick+flag.chip),--task.chipPass(PASS_POS1),
	Leader   = task.goSpeciPos(WAIT_POS1, player.toPlayerDir("Assister"),  DSS_FLAG),
	Breaker  = task.continue(),--task.goSpeciPos(WAIT_POS2, _,  DSS_FLAG),
	match    = "{A}{LB}"
},

["fix1"] = {
	switch = function()
		if  bufcnt(true, 30) then--if  bufcnt(true, cp.getFixBuf(PASS_POS1)) then
			return "pass2"
		end
	end,
	Assister = task.goSpeciPos(FAKE_POS1, _, DSS_FLAG),
	Leader   = task.goSpeciPos(WAIT_POS1, player.toPlayerDir("Assister"), flag.not_avoid_our_vehicle),
	Breaker  = task.goSpeciPos(WAIT_POS2, _,  DSS_FLAG),
	match    = "{A}{LB}"
},

["pass2"] = {
	switch = function()
		if  bufcnt(player.kickBall("Leader"), 1) then
		--if  bufcnt(player.kickBall("Leader") or player.toBallDist("Leader") > 30, 1, 150) then
			return "shoot"
		elseif bufcnt(true,150) then
			return "exit"
		end
	end,
	Assister = task.goSpeciPos(FAKE_POS1, _, DSS_FLAG),
	Leader   = task.getBallV4(PASS_POS2, gRolePos["Leader"], _, flag.kick),--task.InterTouch(_,PASS_POS2),--task.InterTouch(_,PASS_POS2,400),
	Breaker  = task.goSpeciPos(WAIT_POS2, _, flag.not_avoid_our_vehicle),
	match    = "{A}{LB}"
},

["shoot"] = {
    switch = function ()
		if bufcnt(player.kickBall("Breaker"), 1, 150) then
			return "exit"
		end
	end,
	Assister = task.goSpeciPos(FAKE_POS1, _, DSS_FLAG),
	Leader   = task.defendMiddle(),
	Breaker  = task.getBallV4(_, gRolePos["Breaker"], _, flag.kick),--task.InterTouch(),
	match    = "{A}{LB}"
},

name = "Ref_FrontKickV807",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
