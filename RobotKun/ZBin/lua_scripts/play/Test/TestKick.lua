------------------ 不许改！！！！！ mark说的
local sPos = ball.antiYPos(CGeoPoint:new_local(3200,1300))
local pPos = pos.passForTouch(sPos)
local fPos = {
	ball.antiYPos(CGeoPoint:new_local(3000,0)),
	-- ball.antiYPos(CGeoPoint:new_local(-500,2400)) -- [1]
	ball.antiYPos(CGeoPoint:new_local(1200,-2900)) -- [2]

}

gPlayTable.CreatePlay{

firstState = "goto",
["goto"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Assister")<200,20) then
			return "goto2"
		end
	end,
	Leader = task.staticGetBall(sPos,false),
	Assister = task.goCmuRush(fPos[1],player.toTheirGoalDir),
	match  = "(LA)"
},
["goto2"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Assister")<200,20) then
			return "wait"
		end
	end,
	Leader = task.slowGetBall(sPos,false),
	Assister = task.goCmuRush(fPos[2],player.toTheirGoalDir),
	match  = "{LA}"
},
["wait"] = {  
	switch = function()
		if bufcnt(player.toTargetDist("Assister")<2930,2) then
			return "pass"
		end
	end,
	Leader = task.slowGetBall(sPos,false),
	Assister = task.goCmuRush(sPos,player.toTheirGoalDir),
	match = "{LA}"
},
["pass"] = {
	switch = function()
		if bufcnt(player.kickBall("Leader"),1,200) then
			return "fix"
		end
	end,
	Leader = task.zget(pPos,nil,nil, flag.kick + flag.chip + flag.dribble + flag.safe),
	Assister = task.goCmuRush(sPos,player.toTheirGoalDir),
	match = "{LA}"
},
["fix"] = {
	switch = function()
		if bufcnt(true,50) then
			return "kick"
		end
	end,
	Leader = task.stop(),
	Assister = task.goCmuRush(sPos,player.toTheirGoalDir),
	match = "{LA}"
},
["kick"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister"),1,200) then
			return "finish"
		end
	end,
	Assister = task.InterTouch(sPos,false,6500),
	Leader = task.stop(),
	match  = "{LA}"
},

name = "TestKick",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
