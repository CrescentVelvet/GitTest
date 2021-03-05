local waitPos = CGeoPoint:new_local(2400, 1800)
local passPos = pos.passForTouch(waitPos)
local goalPoint = CGeoPoint:new_local(param.pitchLength/2,0)
local inFlag = true 	-- 用于记录入角的标记位，在球靠近到距离40的时候只记录一次数据
local outFlag = true 	-- 用于记录出角的标记位
local player2Goal
local testFlag = false	-- true是开启补偿。检查补偿是否正确
local count = 0

local random_x = 0.5
local random_y = 0.5
local random_p = 0.5
local X_S = 2000
local X_E = 4000
local Y_S = -2700
local Y_E = 200
local P_S = 3000
local P_E = 6000
local random_start = function()
	local x = X_S + (X_E-X_S)*random_x
	local y = Y_S + (Y_E-Y_S)*random_y
	return CGeoPoint:new_local(x,y)
end
local random_power = function()
	return P_S + (P_E-P_S)*random_p
end

-- 以下数据为需要记录的值
local ballSpeed = 0
local inAngel = 0
local outAngel = 0
local p = function()
	local kicker2ball = (ball.pos() - player.pos("Kicker")):dir()
	local pos = ball.pos() + Utils.Polar2Vector(800,kicker2ball)
	return pos
end
local d = function()
	local ball2kicker = (player.pos("Kicker") - ball.pos()):dir()
	return ball2kicker
end
gPlayTable.CreatePlay{
switch = function()
	debugEngine:gui_debug_msg(CGeoPoint:new_local(300,0),string.format("%s = %f","ballSpeed",ballSpeed),3)
	debugEngine:gui_debug_msg(CGeoPoint:new_local(700,0),string.format("%s = %f","Alpha",inAngel),3)
	debugEngine:gui_debug_msg(CGeoPoint:new_local(1100,0),string.format("%s = %f","Theta",outAngel),3)
	if bufcnt(not ball.valid(),100,9999) then
		return "stop"
	end
	if true then
		local ballDir = ball.velDir()
		local ballVel = ball.velMod()
		local shootDir = player.toPointDir(goalPoint,"Kicker")
		local ball2Player = ball.toPlayerDir("Kicker")
		if not gCurrentState == "fetch_ball" and bufcnt(ballVel > 1000 
			and math.abs(Utils.Normalize(Utils.Normalize(ballDir) - Utils.Normalize(shootDir)))>math.pi/9
			and math.abs(Utils.Normalize(Utils.Normalize(ballDir) - Utils.Normalize(ball2Player)))>math.pi/9,30,9999) then
			return "stop"
		end
	end
	if gCurrentState == "fetch_ball" then
		if bufcnt(player.toTargetDist("Kicker")< 400 and player.toPointDist("Tier",random_start())<200, 40,9999) then
			return "goto"
		end
	end
	if gCurrentState ~= "fetch_ball" and gCurrentState ~= "goto" and gCurrentState ~= "pass" and bufcnt(ball.valid() and ball.velMod() < 1000 and ball.toPlayerDist("Kicker") > 1500,40,9999) then					
		random_x = math.random()
		random_y = math.random()
		random_p = math.random()
		while Utils.InTheirPenaltyArea(random_start(),200) do
			random_x = math.random()
			random_y = math.random()
		end
		return "fetch_ball"
	end
	if gCurrentState == "goto" then
		if bufcnt(player.toTargetDist("Kicker")< 400 and player.toTargetDist("Tier")<500, 40,9999) then
			return "pass"
		end
	elseif gCurrentState == "pass" then
		if bufcnt(player.kickBall("Tier"), 1, 360)then      
			return "kick"
		end
	elseif gCurrentState == "kick" then
		debugEngine:gui_debug_line(player.pos("Kicker"),player.pos("Kicker")+Utils.Polar2Vector(10000,player.dir("Kicker")))
		if bufcnt(player.kickBall("Kicker"), "fast") then	
			player2Goal = player.toPointDir(goalPoint,"Kicker")
			return "record"
		end
		if inFlag and player.toBallDist("Kicker") < 500 then
			inFlag = false
			ballSpeed = ball.velMod()
			inAngel = Utils.Normalize(player.dir("Kicker") - (ball.velDir() + math.pi))
		end
	elseif gCurrentState == "record" then
		if bufcnt(true,20) then
			inFlag = true
			outFlag = true
			return "stop"
		end
		if player.toBallDist("Kicker") > 800 and ball.velMod() > 3000 and outFlag then
			outFlag = false
			outAngel = Utils.Normalize(ball.velDir() - player2Goal) -- 射出角。 	
			count = count + 1
			print("Record!!!!!!  ",count)
			if not testFlag then
				local recordfile = io.open("data/compensate.txt","a")
				if inAngel < 0 then
					recordfile:write(ballSpeed,"  ",-inAngel,"  ",-outAngel,"\n")
				else
					recordfile:write(ballSpeed,"  ",inAngel,"  ",outAngel,"\n")
				end
				recordfile:close()
			else -- testFlag == true
				outAng = ball.velDir() -- 实际射出速度方向
				expOutAng =  player.toPointDir(goalPoint,"Kicker") -- 期望射出方向
				--getluadata:getAdjustData(outAng, expOutAng, ballSpeed, math.abs(inAngel))
				print(outAng,"  ",expOutAng," ", player.dir("Kicker"),"\n")
			end
		end
	end
end,

firstState = "fetch_ball",
["fetch_ball"] = {
	Kicker = task.goCmuRush(waitPos, _, _, flag.allow_dss),
	Tier   = task.fetchBall(random_start,0,true),
	match  = ""
},
["goto"] = {		
	Kicker = task.goCmuRush(waitPos, _, _, flag.allow_dss),
	Tier   = task.goCmuRush(p,d,_,flag.dodge_ball),
	match  = ""
},	

["pass"] = {  
	Kicker = task.goCmuRush(waitPos, player.toPointDir(goalPoint), _, flag.allow_dss),
	Tier   = task.zget(passPos,_, random_power, flag.kick + flag.safe+flag.dribble),
	match = ""
},

["kick"] = {
	-- debugEngine:gui_debug_msg(CGeoPoint:new_local(30,0),string.format("%s = %f","ballSpeed",ballSpeed),3)
	-- debugEngine:gui_debug_msg(CGeoPoint:new_local(70,0),string.format("%s = %f","realdirection",realdirection),3)
	-- debugEngine:gui_debug_msg(CGeoPoint:new_local(110,0),string.format("%s = %f","targetdirection",targetdirection),3)
	Kicker = task.InterTouch(waitPos,testFlag,6500),--task.zget(waitPos,_,_, flag.kick),--false为测试补偿数据专用
	Tier   = task.stop(),
	match  = ""
},

["record"] = {
	Kicker = task.stop(),
	Tier   = task.stop(),
	match  = ""
},
["stop"] = { 	
	Kicker = task.goCmuRush(waitPos, _, _, flag.allow_dss),
	Tier   = task.stop(),
	match  = ""
},	

name = "TestForCompensate",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

