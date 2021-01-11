-- ftq 2018.4.1 immortal kick 的中场

local FreeKick_ImmortalStart_Pos = function ()
	local pos
	local ball2goal = CVector:new_local(CGeoPoint:new_local(param.pitchLength/2, 0) - ball.pos())
	pos = ball.pos() + Utils.Polar2Vector(100, ball2goal:dir()) -- 球距离球门100cm
	local tempPos = ball.pos()+ Utils.Polar2Vector(65, ball2goal:dir())
	for i = 1, param.maxPlayer do
		if vision:TheirPlayer(i):Valid() then
			if vision:TheirPlayer(i):Pos():dist(tempPos) < 20 then
				local dir = (CGeoPoint:new_local(450, 30 * ball.antiY()) - vision:TheirPlayer(i):Pos()):dir()
				pos = vision:TheirPlayer(i):Pos() + Utils.Polar2Vector(30, dir)
				break
			end
		else
			pos = tempPos
		end
	end
	return pos
end

local Field_RobotFoward_Pos = function (role)
	local pos
	local UpdatePos = function ()
		local roleNum = player.num(role)
		if Utils.PlayerNumValid(roleNum) then
			local carPos = vision:OurPlayer(roleNum):Pos()
			pos = carPos + Utils.Polar2Vector(1, vision:OurPlayer(roleNum):Dir())
		end
		return pos
	end
	return UpdatePos
end

local FREEKICKPOS
local FIELDROBOTPOS
FREEKICKPOS = FreeKick_ImmortalStart_Pos

PreGoPos = function (role)
	return function ()
		return player.pos(role) + Utils.Polar2Vector(5, player.dir(role))
	end
end

local DSS_FLAG = flag.allow_dss + flag.dodge_ball + flag.free_kick

gPlayTable.CreatePlay{

firstState = "startball",

["startball"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Special") < 30 or
		          player.toTargetDist("Defender") < 30, 70, 120)then
			return "changepos"
		end
	end,
	Assister = task.staticGetBall(CGeoPoint:new_local(param.pitchLength/2, 0)),
	Leader   = task.defendMiddle(),
	Middle   = task.rightBack(),
	Center   = task.leftBack(),
	Fronter  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(0, 300)),_,DSS_FLAG),
	Special  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(0, 230)),_,DSS_FLAG),
	Defender = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(0, 150)),_,DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{AL}[MSDFC]"
},

["changepos"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 30, 30, 120)then
			return "getball"
		end
	end,
	Assister = task.slowGetBall(CGeoPoint:new_local(param.pitchLength/2, 0)),	
	Middle   = task.leftBack(),
	Center   = task.rightBack(),
	Special  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(350, -150)),_,DSS_FLAG),
	Fronter  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(500, -260)),_,DSS_FLAG),
	Defender = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(-200, -120)),_,DSS_FLAG),
	Leader   = task.goSpeciPos(FREEKICKPOS),
	Goalie   = task.zgoalie(),
	match    = "{LMAC}[SDF]"
},

["getball"] = {
	switch = function ()
		if bufcnt(player.InfoControlled("Assister") and player.toTargetDist("Leader") < 30, 30, 120) then
			return "chipball"
		end
	end,
	Assister = task.slowGetBall(CGeoPoint:new_local(param.pitchLength/2, 0)),
	Special  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(350, 200)),_,DSS_FLAG),
	Fronter  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(500, 200)),_,DSS_FLAG),
	Middle   = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(200, 150)),_,DSS_FLAG),		
	Leader   = task.goSpeciPos(Field_RobotFoward_Pos("Leader")),	
	Defender = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{LASFC}[DM]"
},	

["chipball"] = {
    switch = function ()
		if bufcnt(player.kickBall("Assister") or
				  player.toBallDist("Assister") > 20, "fast") then
			return "shootball"
		elseif  bufcnt(true,60) then
			return "exit"
		end
	end,
	Assister = task.chipPass(CGeoPoint:new_local(param.pitchLength/2, 20), 80), --踢球车
	Leader   = task.stop(),
	Special  = task.continue();--goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(330, 50))),
	Middle   = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(480, 10)),_,DSS_FLAG),
	Fronter  = task.continue(),--goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(240, -150))),
	Defender = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{ALSDCFM}"
},
--[[
["waitball"] = {
    switch = function ()
		if bufcnt( math.abs(Utils.Normalize((player.dir("Leader") - player.toBallDir("Leader"))))< math.pi / 4, "normal") then
			return "shootball"
		elseif  bufcnt(true, 40) then
			return "exit"
		end
	end,
	Leader   = task.goSpeciPos(PreGoPos("Leader")),
	Assister = task.stop(),
	Special  = task.goSupportPos("Leader"),
	Defender = task.leftBack(),
	Middle   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{ALS}[DM]"
},
]]--
["shootball"] = {
    switch = function ()
		if bufcnt(false, 1, 120) then
			return "exit"
		end
	end,
	Leader   = task.flatPass(CGeoPoint(param.pitchLength/2, 0)),
	Special  = task.zsupport(),
	Fronter  = task.continue(), --goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(30, 150))),
	Center   = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(230, 150)),_,DSS_FLAG),
	Assister = task.sideBack(),
	Defender = task.leftBack(),
	Middle   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{ALS}[DMFC]"
},

name = "Ref_MiddleKickV801",
applicable ={
	exp = "a",
	a   = true
},
attribute = "attack",
timeout = 99999
}
