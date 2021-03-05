-- 角球定位球在一开始就进行站位
-- by zhyaic 2014-04-08
-- yys 2015-06-10
-- 修改到八辆车版本
local STOP_FLAG  = flag.dodge_ball
local STOP_DSS   = bit:_or(STOP_FLAG, flag.allow_dss)
local KICK_POS   = function ()
	return CGeoPoint:new_local(ball.posX() - 59, ball.posY()-ball.antiY()*20)
end

local KICK_DIR  = ball.antiYDir(1.57)

local MIDDLE_POS = ball.antiYPos(CGeoPoint:new_local(450, 300))

local FRONT_POS = ball.antiYPos(CGeoPoint:new_local(290, 180))

local SPECIAL_POS = ball.antiYPos(CGeoPoint:new_local(160,-30))

local CENTER_POS = ball.antiYPos(CGeoPoint:new_local(220, -230))

local LEFT_BACK_POS = ball.antiYPos(CGeoPoint:new_local(-200,120))
local RIGHT_BACK_POS = ball.antiYPos(CGeoPoint:new_local(-200,-120))

local ACC = 400

gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
	switch = function()
		if cond.isGameOn() then
			return "exit"
		end
	end,
	Leader   = task.goCmuRush(KICK_POS, KICK_DIR, ACC, STOP_DSS),
	Special  = task.goCmuRush(SPECIAL_POS, _, ACC, STOP_DSS),
	Middle   = task.goCmuRush(MIDDLE_POS, _, ACC, STOP_DSS),
	Fronter  = task.goCmuRush(FRONT_POS, _, ACC, STOP_DSS),
	Center   = task.goCmuRush(CENTER_POS, _, ACC, STOP_DSS),
	Assister = task.goCmuRush(LEFT_BACK_POS, _, ACC, STOP_DSS),--task.leftBack4Stop(),
	Defender = task.goCmuRush(RIGHT_BACK_POS, _, ACC, STOP_DSS),--task.rightBack4Stop(),
	Goalie   = task.zgoalie(),
	match    = "[ADLSMFC]"
},

name = "Ref_Stop4CornerKick",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}