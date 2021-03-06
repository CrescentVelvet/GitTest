--角球防守，增加norPass的状态，保证norPass的时候盯防first的优先级最高
local COR_DEF_POS1 = CGeoPoint:new_local(-1000,-1500)
local COR_DEF_POS2 = CGeoPoint:new_local(-1000,1500)
local COR_DEF_POS3 = ball.refAntiYPos(CGeoPoint:new_local(-700,-200))
local DSS_FLAG = flag.allow_dss + flag.avoid_stop_ball_circle
gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		defenceSquence:attackerAmount() <= 6 and defenceSquence:attackerAmount() > 0 then
		return "attacker"..defenceSquence:attackerAmount()
	elseif cond.ballMoved() then
		return "exit"
	else
		-- if enemy.situChanged() and 
		if defenceSquence:attackerAmount() <= 6 and defenceSquence:attackerAmount() > 0 then
			return "attacker"..defenceSquence:attackerAmount()
		end
	end
end,

["beginning"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.goPassPos("Leader", DSS_FLAG),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir, DSS_FLAG),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][DA][S][M]"
},

["attacker1"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.goPassPos("Leader", DSS_FLAG),
	Middle   = task.zmarking("First", DSS_FLAG),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][M][S][DA]"
},

["attacker2"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.zmarking("First", DSS_FLAG),
	Middle   = task.zmarking("Second", DSS_FLAG),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][S][M][DA]"
},

["attacker3"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.zmarking("First", DSS_FLAG),
	Middle   = task.zmarking("Second", DSS_FLAG),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][S][M][AD]"
},

["attacker4"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.zmarking("First", DSS_FLAG),
	Middle   = task.zmarking("Second", DSS_FLAG),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][S][M][AD]"
},

["attacker5"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.zmarking("First", DSS_FLAG),
	Middle   = task.zmarking("Second", DSS_FLAG),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][S][M][AD]"
},

["attacker6"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.zmarking("First", DSS_FLAG),
	Middle   = task.zmarking("Second", DSS_FLAG),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][S][M][AD]"
},

["marking"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.zmarking("First", DSS_FLAG),
	Middle   = task.zmarking("Second", DSS_FLAG),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][S][M][AD]"
},

["norPass"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.zmarking("First", DSS_FLAG),
	Middle   = task.zmarking("Second", DSS_FLAG),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][S][M][AD]"
},

["norDef"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.zmarking("First", DSS_FLAG),
	Middle   = task.zmarking("Second", DSS_FLAG),
	Defender = task.zback(2,1),
	Assister = task.zback(2,2),
	Goalie   = task.zgoalie(),
	match    = "[L][S][M][AD]"
},

name = "Ref_FrontDef_6vs6",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}