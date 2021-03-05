gPlayTable.CreatePlay{

firstState = "testState1",
["testState1"] = { 
	switch   = function()
		if bufcnt(ball.valid(),15) then
			return "testState2"
		end
	end,
	match = ""
},
["testState2"] = { 
	switch   = function()
		local dist = ball.velMod()*ball.velMod()/105--152simulation
		debugEngine:gui_debug_x(ball.pos() + Utils.Polar2Vector(dist, ball.velDir()))
		--print(ball.pos() + Utils.Polar2Vector(dist, ball.velDir())
		if bufcnt(not ball.valid(),10) then
			return "testState1"
		end
		-- if bufcnt(player.kickBall("Goalie"), 1,400) then
		-- 	return "testState3"
		-- end
	end,
	match    = ""
},

name = "TestForFriction",
applicable ={
        exp = "a",
        a = true
},
attribute = "attack",
timeout   = 99999
}
