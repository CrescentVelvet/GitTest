local LEFTUP_POS = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(100, 280*ball.refAntiY())
	else
		return CGeoPoint:new_local(100, 90*ball.refAntiY())
	end
end

local RIGHTDOWN_POS = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(-150, 90*ball.refAntiY())
	else
		return CGeoPoint:new_local(-150, 280*ball.refAntiY())
	end
end

local function getKickDir()
	local targetPos = CGeoPoint:new_local(0, 0)
	return (targetPos-ball.pos()):dir()
end 

local passposnew = CGeoPoint:new_local(0, 0)
local holdBallPos = CGeoPoint:new_local(0,0)

gPlayTable.CreatePlay{

firstState = "holdBall",

["advanceBallV2"] = { 
    switch   = function()
    end,
    Assister = task.advance(),
    Goalie   = task.zgoalie(),
    match    = "[A]"
},

-- ["markingTouch"] = {
--     switch   = function()
--     end,
-- 	Middle   = task.markingTouch(0,LEFTUP_POS,RIGHTDOWN_POS,"vertical"),
-- 	match    = "{M}"
-- },

-- ["receivePass"] = {
-- 	switch   = function()
-- 	end,
-- 	Leader   = task.receivePass("Special"),
-- 	Special  = task.goSupportPos("Leader"),
-- 	match    = "{LS}"
-- },        

-- ["slowGetBall"] = {
-- 	switch   = function()
-- 	end,
-- 	Assister = task.slowGetBall(passposnew),
-- 	match    = "[A]"
-- },

["holdBall"] = {
	switch   = function()
	end,
	Leader = task.holdball(holdBallPos),
	match    = "[L]"
},

["ChaseKick"] = {
	switch   = function()
	end,
	Leader   = task.chaseNew(),
	Assister = task.chaseNew(),
	Special = task.chaseNew(),
	match    = "{LAS}"
},

['oppo'] = {
	switch   = function()
	end,
	Leader   = task.advance(),
	Assister = task.zget(),
	match    = "{LA}"
},

['smartgoto'] = {
	switch    = function()
	end,
	Leader    = task.goSpeciPos(CGeoPoint:new_local(-15, 90)),
	match     = "[L]"
},

name = "TestMultiple",
applicable ={
        exp = "a",
        a = true
},
attribute = "attack",
timeout   = 99999
}