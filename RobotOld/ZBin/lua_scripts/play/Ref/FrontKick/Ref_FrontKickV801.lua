-- FrontKickV4大场版 --Wang

local HALF_POS = {
  ball.refSyntYPos(CGeoPoint:new_local(40, 90)),
  ball.refSyntYPos(CGeoPoint:new_local(40, 60)),
  ball.refSyntYPos(CGeoPoint:new_local(40, 30)),
  ball.refSyntYPos(CGeoPoint:new_local(40, 0)),
  ball.refSyntYPos(CGeoPoint:new_local(40, -30)),
  ball.refSyntYPos(CGeoPoint:new_local(40, -60)),
  ball.refSyntYPos(CGeoPoint:new_local(40, -90))
}
local HALF_TEST = gOppoConfig.IfHalfField

local TMP_POS = ball.refSyntYPos(CGeoPoint:new_local(220*0.5, 130*0.5))
local TMP_POS2 = ball.refSyntYPos(CGeoPoint:new_local(400*0.5, 130*0.5))
local TMP_POS3 = ball.refAntiYPos(CGeoPoint:new_local(300*0.5, 280*0.5))
local SYNT_CORNER_POS = ball.refSyntYPos(CGeoPoint:new_local(400*0.5, 310*0.5))
local SYNT_CORNER_POS2 = ball.refSyntYPos(CGeoPoint:new_local(460*0.5, 230*0.5))
local ANTI_CORNER_POS = ball.refAntiYPos(CGeoPoint:new_local(480*0.5, 250*0.5))
local ANTI_CORNER_POS2 = ball.refAntiYPos(CGeoPoint:new_local(505*0.5, 255*0.5))
local TOUCH_POS = ball.refAntiYPos(CGeoPoint(param.pitchLength/2.0, 15))

local LEFT_STOP = CGeoPoint:new_local(-550*0.5, -400*0.5)
local RIGHT_STOP = CGeoPoint:new_local(-550*0.5, 400*0.5)
local GOAL_POINT = CGeoPoint:new_local(600*0.5, -250*0.5)
local START_POINT = CGeoPoint:new_local(40*0.5, -200*0.5)

local BACK_DIR = function(d)
	return function()
		return ball.antiY()*d
	end
end

local DSS_FLAG = flag.allow_dss + flag.dodge_ball + flag.free_kick

gPlayTable.CreatePlay{

firstState = "tmpState",

["tmpState"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Special") < 20, 20, 180) then
			return "oneReady"
		end
	end,
	Assister = task.goBackBallV2(CGeoPoint:new_local(450*0.5, -100*0.5), 20),
        Defender = HALF_TEST and task.stop() or task.singleBack(),--task.goCmuRush(START_POINT, _, 600, DSS_FLAG),
	Leader   = HALF_TEST and task.goSpeciPos(HALF_POS[1],_,_,DSS_FLAG) or task.goBackBallV2(0, 50),
	Special  = HALF_TEST and task.goSpeciPos(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(ball.goRush(), _, _, DSS_FLAG),
	Middle   = HALF_TEST and task.goSpeciPos(HALF_POS[3],_,_,DSS_FLAG) or task.goCmuRush(TMP_POS, _, _, DSS_FLAG),
	Breaker  = HALF_TEST and task.goSpeciPos(HALF_POS[4],_,_,DSS_FLAG) or task.stop(),--task.goCmuRush(LEFT_STOP, _, 600, DSS_FLAG),
	Center   = HALF_TEST and task.goSpeciPos(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(TMP_POS2, _, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "(ADS)[LMBC]"
},

["oneReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Special") < 5, "normal", 180) then
			return "twoReady"
		end
	end,
	Assister = task.goBackBallV2(CGeoPoint:new_local(600*0.5, -100*0.5), 20),
        Defender = HALF_TEST and task.stop() or task.singleBack(),--task.goCmuRush(START_POINT, _, 600, DSS_FLAG),
	Leader   = HALF_TEST and task.goSpeciPos(HALF_POS[1],_,_,DSS_FLAG) or task.goBackBallV2(0, 50),
	Special  = HALF_TEST and task.goSpeciPos(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(ball.goRush(), _, _, DSS_FLAG),
	Middle   = HALF_TEST and task.goSpeciPos(HALF_POS[3],_,_,DSS_FLAG) or task.goCmuRush(TMP_POS, _, _, DSS_FLAG),
	Breaker  = HALF_TEST and task.goSpeciPos(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Center   = HALF_TEST and task.goSpeciPos(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(TMP_POS2, _, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{AD}{SLMBC}"
},

["twoReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Leader") < 5, "normal", 180) then
			return "chooseKicker"
		end
	end,
	Assister = task.goBackBallV2("Special", 25),
        Defender = HALF_TEST and task.stop() or task.singleBack(),--task.goCmuRush(START_POINT, _, 600, DSS_FLAG),
	Leader   = HALF_TEST and task.goSpeciPos(HALF_POS[1],_,_,DSS_FLAG) or task.goBackBallV2(CGeoPoint:new_local(600, -100), 20),
	Special  = HALF_TEST and task.goSpeciPos(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(ball.goRush(), _, _, DSS_FLAG),
	Middle   = HALF_TEST and task.goSpeciPos(HALF_POS[3],_,_,DSS_FLAG) or task.goCmuRush(TMP_POS, _, _, DSS_FLAG),
	Breaker  = HALF_TEST and task.goSpeciPos(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Center   = HALF_TEST and task.goSpeciPos(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(TMP_POS2, _, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{AD}{LSMBC}"
},

["chooseKicker"] = {--change car to kick?
	switch = function()
		-- 由于CMU三车盯人，因此坚定地跑掉一辆车
		if  bufcnt(true, 120) then
			return "continueGo"
		end
	end,
	Assister = task.goBackBallV2("Special", 25),
        Defender = HALF_TEST and task.stop() or task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(-240, 220)), _, _, DSS_FLAG),--task.goCmuRush(START_POINT, _, 600, DSS_FLAG),
	Leader   = HALF_TEST and task.goSpeciPos(HALF_POS[1],_,_,DSS_FLAG) or task.goBackBallV2(CGeoPoint:new_local(600, -100), 20),
	Special  = HALF_TEST and task.goSpeciPos(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(ball.goRush(), _, _, DSS_FLAG),
	Middle   = HALF_TEST and task.goSpeciPos(HALF_POS[3],_,_,DSS_FLAG) or task.goCmuRush(TMP_POS, _, _, DSS_FLAG),
	Breaker  = HALF_TEST and task.goSpeciPos(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Center   = HALF_TEST and task.goSpeciPos(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(TMP_POS2, _, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{AD}{LSMBC}"
},

["continueGo"] = {--Defender go to position--OK!
	switch = function()
		if  bufcnt(player.toTargetDist("Defender") < 250, 1, 150) then--need to change for Defender to get the ball 	
			return "continuePass"
		end
	end,
	Assister = task.goBackBallV2("Special", 25),
        Defender = task.goCmuRush(ball.goRush(), _, _, DSS_FLAG),
	Leader   = HALF_TEST and task.goSpeciPos(HALF_POS[1],_,_,DSS_FLAG) or task.goBackBallV2(CGeoPoint:new_local(600*0.5, -100*0.5), 20),
	Special  = HALF_TEST and task.goSpeciPos(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(ANTI_CORNER_POS2, _, _, DSS_FLAG),
	Middle   = HALF_TEST and task.goSpeciPos(HALF_POS[3],_,_,DSS_FLAG) or task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Breaker  = HALF_TEST and task.goSpeciPos(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(SYNT_CORNER_POS2, _, _, DSS_FLAG),
	Center   = HALF_TEST and task.goSpeciPos(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{AD}{LSMBC}"
},

["continuePass"] = {--pass the ball to Defender
	switch = function()
		if  player.kickBall("Assister") or player.isBallPassed("Assister","Defender") then
			return "continueShoot"
		elseif  bufcnt(true, 60) then--90) then
			return "exit"
			--return "tmpState"
		end
	end,
	Assister = task.zget(pos.passForTouch(ball.goRush()),_,_,flag.kick),--task.passToPos(pos.passForTouch(ball.goRush())),
    Defender = task.goCmuRush(ball.goRush()),
    Leader   = HALF_TEST and task.goSpeciPos(HALF_POS[1],_,_,DSS_FLAG) or task.singleBack(),
	Special  = HALF_TEST and task.goSpeciPos(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(ANTI_CORNER_POS2, _, _, DSS_FLAG),
    Middle   = HALF_TEST and task.goSpeciPos(HALF_POS[3],_,_,DSS_FLAG) or task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Breaker  = HALF_TEST and task.goSpeciPos(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(SYNT_CORNER_POS2, _, _, DSS_FLAG),
	Center   = HALF_TEST and task.goSpeciPos(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{AD}{SLMBC}"
},

["continueShoot"] = {--shoot!!!
	switch = function()
        if bufcnt(player.infraredOn("Defender"),3,200) then
                return "zshoot"
        elseif bufcnt(true,200) then
                return "exit"
        end
	end,
    Assister = task.leftBack(),
	Defender = task.receivePass("Assister",0),
    Leader   = HALF_TEST and task.goSpeciPos(HALF_POS[1],_,_,DSS_FLAG) or task.rightBack(),
	Special  = HALF_TEST and task.goSpeciPos(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(ANTI_CORNER_POS2, _, _, DSS_FLAG),--task.goCmuRush(TMP_POS3, _, 600, DSS_FLAG),
	Middle   = HALF_TEST and task.goSpeciPos(HALF_POS[3],_,_,DSS_FLAG) or task.defendMiddle(),
	Breaker  = HALF_TEST and task.goSpeciPos(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(SYNT_CORNER_POS2, _, _, DSS_FLAG),
	Center   = HALF_TEST and task.goSpeciPos(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{D}{S}[LAMBC]"
},
  ["zshoot"] = {
    switch = function()
            if bufcnt(player.kickBall("Defender"),1,200) then
                    return "exit"
            end
    end,
    Assister = task.leftBack(),
	Defender = task.zpass(),
    Leader   = HALF_TEST and task.goSpeciPos(HALF_POS[1],_,_,DSS_FLAG) or task.rightBack(),
	Special  = HALF_TEST and task.goSpeciPos(HALF_POS[2],_,_,DSS_FLAG) or task.goCmuRush(ANTI_CORNER_POS2, _, _, DSS_FLAG),--task.goCmuRush(TMP_POS3, _, 600, DSS_FLAG),
	Middle   = HALF_TEST and task.goSpeciPos(HALF_POS[3],_,_,DSS_FLAG) or task.defendMiddle(),
	Breaker  = HALF_TEST and task.goSpeciPos(HALF_POS[4],_,_,DSS_FLAG) or task.goCmuRush(SYNT_CORNER_POS2, _, _, DSS_FLAG),
	Center   = HALF_TEST and task.goSpeciPos(HALF_POS[5],_,_,DSS_FLAG) or task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{D}{S}[LAMBC]"
  },

name = "Ref_FrontKickV801",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
