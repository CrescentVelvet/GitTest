local testPos  = {
	CGeoPoint:new_local(3000,-2000),
	CGeoPoint:new_local(3000,2000),
	CGeoPoint:new_local(-3000,2000),
	CGeoPoint:new_local(-3000,-2000)
}
local vel = CVector:new_local(0, 0)
local maxvel=0
local time = 100
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

local startPos = CGeoPoint:new_local(0,0)
local targetPos = CGeoPoint:new_local(0,0)
local lastTargetPos = targetPos

local drawDebug = function()
	lastTargetPos = targetPos
	targetPos = gRolePos["Kicker"]()
	local dist = (lastTargetPos - targetPos):mod()
	if dist > 5 then
		startPos = CGeoPoint:new_local(player.pos("Kicker"))
	end
	debugEngine:gui_debug_line(startPos,targetPos)
	debugEngine:gui_debug_msg(startPos,"start")
	debugEngine:gui_debug_msg(targetPos,"target")
end

local speed_mod = 0
local speed_angle = (math.random()-0.5)*math.pi*2
local lastSpeedMod = 0
local speedMod = 0
local CMP = 0 -- camparesion of target speed and real speed

local SUM_CMP = 0
local COUNT = 0

local AVG = 0
local ANG = 0

local target_dspeed = 80

local origin_pos = CGeoPoint:new_local(1000,0)

local init = function()
	speed_mod = 0
	speed_angle = (math.random()-0.5)*math.pi*2
	lastSpeedMod = 0
	speedMod = 0
	CMP = 0
	SUM_CMP = 0
	COUNT = 0
end

local process = function()
	local recordfile = io.open("data/constraint.txt","a")
	ANG = math.floor(speed_angle/math.pi*180)
	AVG = target_dspeed - math.floor(SUM_CMP/COUNT)
	recordfile:write(ANG," ",AVG,"\n")
end

local sX = function()
	return speed_mod*math.cos(speed_angle)
end

local sY = function()
	return speed_mod*math.sin(speed_angle)
end

gPlayTable.CreatePlay{

firstState = "goback",

["open_test"] = {
	switch = function()
		speed_mod = speed_mod + target_dspeed
		lastSpeedMod = speedMod
		speedMod = player.rawVelMod("Kicker") + 0.001
		CMP = target_dspeed - (speedMod - lastSpeedMod)
		if speedMod > 1000 and speedMod < 2800 then
			SUM_CMP = SUM_CMP + CMP
			COUNT = COUNT + 1
			debugEngine:gui_debug_msg(CGeoPoint:new_local(0,0),"cmp : " .. math.floor(CMP))
			debugEngine:gui_debug_msg(player.pos("Kicker")+CVector(300,300),math.floor(speedMod))
		end
		if COUNT > 10 then
			debugEngine:gui_debug_msg(CGeoPoint:new_local(0,1000),"avg : " .. math.floor(SUM_CMP/COUNT))
			if speedMod > 2000 then
				process()
				return "goback"
			end
		end

		if speed_mod > 6000 then
			return "goback"
		end
		debugEngine:gui_debug_msg(CGeoPoint:new_local(0,-1000),"LAST : " .. AVG .. " " .. ANG)
		debugEngine:gui_debug_line(origin_pos,origin_pos + CVector:new_local(1000*math.cos(speed_angle),1000*math.sin(speed_angle)))
	end,
	Kicker = task.openSpeed(sX,sY,0),
	match = ""
},
["goback"] = {
	switch = function()
		debugEngine:gui_debug_msg(CGeoPoint:new_local(0,-1000),"LAST : " .. AVG .. " " .. ANG)
		if bufcnt(player.toPointDist("Kicker",origin_pos) < 20,100) then
			init()
			return "open_test"
		end
	end,
	Kicker = task.goCmuRush(origin_pos,0),
	match = ""
},

name = "TestConstraint",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
