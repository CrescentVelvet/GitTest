-- FrontKick1704 TestFuckBackKick改版 可以在前场边侧以及中间使用
-- Mark
local TMP_POS = ball.refSyntYPos(CGeoPoint:new_local(230*0.5, 150*0.5))
local SYNT_CORNER_POS = ball.refSyntYPos(CGeoPoint:new_local(430*0.5, 290*0.5))
local ANTI_CORNER_POS = ball.refAntiYPos(CGeoPoint:new_local(430*0.5, 290*0.5))
local SIDE_POS = ball.refSyntYPos(CGeoPoint:new_local(-350*0.5, 330*0.5))
local ASS_SIDE_POS = ball.refSyntYPos(CGeoPoint:new_local(-400*0.5, 330*0.5))
local LEA_SIDE_POS = ball.refSyntYPos(CGeoPoint:new_local(-180*0.5, 330*0.5))
local BACK_DIR = function(d)
	return function()
		return ball.antiY()*d
	end
end

local DSS_FLAG = flag.allow_dss +  flag.dodge_ball + flag.free_kick

local function magicPos()
	return function ()
		return CGeoPoint:new_local(ball.refPosX() - 120, ball.refPosY() + 60 * ball.refAntiY())
	end
end

local HALF_TEST = gOppoConfig.IfHalfField

gPlayTable.CreatePlay{

firstState = "tmpState",

["tmpState"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Special") < 20, 20, 180) then
			return "oneReady"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(0, 300), 30),
	Leader   = task.goBackBall(0, 50),
	Special  = task.goCmuRush(magicPos(), _, _, DSS_FLAG),
	Middle   = task.goCmuRush(TMP_POS, _, _, DSS_FLAG),
	Fronter  = task.sideBack(),
	Center   = task.leftBack(),
	Defender = HALF_TEST and task.stop() or task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "(A)(D){LSCMF}"
},

["oneReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Assister") < 5, "normal", 180) then
			return "twoReady"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(0, 300), 30),
	Leader   = task.goBackBall(0, 50),
	Special  = task.goCmuRush(magicPos(), _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Fronter  = task.sideBack(),
	Center   = task.leftBack(),
	Defender = HALF_TEST and task.stop() or task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{A}{LSDCMF}"
},

["twoReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Leader") < 5, "normal", 180) then
			return "chooseKicker"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(0, 50), 30),
	Leader   = task.goBackBall(CGeoPoint:new_local(300, 200), 20),
	Special  = task.goCmuRush(magicPos(), _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Fronter  = task.sideBack(),
	Center   = task.leftBack(),
	Defender = HALF_TEST and task.stop() or task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{ALSCDMF}"
},

["chooseKicker"] = {
	switch = function()
		-- 由于CMU三车盯人，因此坚定地跑掉一辆车
		if  bufcnt(true, 60) then
			return "continueGo"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(0, 50), 30),
	Leader   = task.goBackBall(CGeoPoint:new_local(300, 200), 20),
	Special  = task.goCmuRush(ball.goRush(), _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Fronter  = task.sideBack(),
	Center   = task.leftBack(),
	Defender = HALF_TEST and task.stop() or task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "{ALSCDMF}"
},

["continueGo"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Defender") < 50, 1, 180) then			
			return "continuePass"
		end
	end,
	Assister = task.stop(),
	Leader   = task.goBackBall(CGeoPoint:new_local(300, 200), 20),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Fronter  = task.goCmuRush(ball.goRush(), _, _, DSS_FLAG),
	Center   = task.goCmuRush(ball.goRush(-100,250), _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Defender = task.goCmuRush(magicPos(), _, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{ALSDMFC}"
},

["chip"] = {
	switch = function()
		if  bufcnt(true, 60) then
			return "continueChipPass"
		end
	end,
	Assister = task.slowGetBall(magicPos(),false,false),
	Leader   = task.goBackBall(CGeoPoint:new_local(450, -100), 20),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Fronter  = task.goCmuRush(ball.goRush(), _, _, DSS_FLAG),
	Center   = task.goCmuRush(ball.goRush(-100,250), _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Defender = task.goCmuRush(magicPos(), _, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{ALSDMFC}"
},

["continuePass"] = {
	switch = function()
		if  player.kickBall("Assister") or 
			player.isBallPassed("Assister","Defender") then
			return "continueShoot"
		elseif  bufcnt(true, 90) then
			return "exit"
		end
	end,
	Assister = task.passToPos(magicPos()),--task.passToPos(pos.passForTouch(ball.goRush())),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Fronter  = task.goCmuRush(ball.goRush(), _, _, DSS_FLAG),
	Center   = task.goCmuRush(ball.goRush(-100,250), _, _, DSS_FLAG),
	Defender = task.goCmuRush(magicPos()),
	Leader   = HALF_TEST and task.stop() or task.stop(),--task.goCmuRush(SIDE_POS),
	Goalie   = task.zgoalie(),
	match    = "{ASDLMFC}"
},

["continueChipPass"] = {
	switch = function()
		if  player.kickBall("Assister") or 
			player.isBallPassed("Assister","Defender") then
			return "fix"
		elseif  bufcnt(true, 90) then
			return "exit"
		end
	end,
	Assister = task.chipPass(pos.passForTouch(magicPos()),100,false),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Fronter  = task.goCmuRush(ball.goRush(), _, _, DSS_FLAG),
	Center   = task.goCmuRush(ball.goRush(-100,250), _, _, DSS_FLAG),
	Defender = task.goCmuRush(magicPos()),
	Leader   = HALF_TEST and task.stop() or task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "{ASDLMFC}"
},

["fix"] = {
	switch = function()
		if bufcnt(true,30) then
			return "waitForShoot"
		end
	end,
	Assister = HALF_TEST and task.stop() or task.goCmuRush(ASS_SIDE_POS, _, _, DSS_FLAG),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Fronter  = task.goCmuRush(ball.goRush(), _, _, DSS_FLAG),
	Center   = task.goCmuRush(ball.goRush(-100,250), _, _, DSS_FLAG),
	Defender = task.goCmuRush(magicPos()),
	Leader   = HALF_TEST and task.stop() or task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "{ASDLMFC}"
},

["waitForShoot"] = {
	switch = function()
		if  bufcnt(true,40) then
			return "continueShoot"
		end
	end,
	Assister = HALF_TEST and task.stop() or task.goCmuRush(ASS_SIDE_POS, _, _, DSS_FLAG),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Defender = task.zget(magicPos(), _, _, flag.kick),
	Leader   = HALF_TEST and task.stop() or task.singleBack(),
	Fronter  = task.sideBack(),
	Center   = task.goCmuRush(ball.goRush(-100,250), _, _, DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "{ASDLMFC}"
},

["continueShoot"] = {
	switch = function()
		if  bufcnt(true, 80) then
			print (magicPos)
			return "shoot"
		end
	end,
	Defender = task.zget(_, _, _, flag.kick),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Assister = HALF_TEST and task.stop() or task.goCmuRush(ASS_SIDE_POS, _, _, DSS_FLAG),
	Leader   = HALF_TEST and task.stop() or task.rightBack(),
	Fronter  = task.sideBack(),
	Center   = task.leftBack(),
	Goalie   = task.zgoalie(),
	match    = "{SDA}[LMFC]"
},
["shoot"] = {
	switch = function()
		if  bufcnt(player.kickBall("Defender"), 1, 120) then
			return "exit"
		end
	end,
	Defender = task.zget(_, _, _, flag.kick),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Middle   = task.defendMiddle(),
	Assister = HALF_TEST and task.stop() or task.goCmuRush(ASS_SIDE_POS, _, _, DSS_FLAG),
	Center   = task.leftBack(),
	Leader   = HALF_TEST and task.stop() or task.rightBack(),
	Fronter  = task.sideBack(),
	Goalie   = task.zgoalie(),
	match    = "{SDA}[LMFC]"
},

name = "Ref_FrontKickV802",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
