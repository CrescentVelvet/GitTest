--角球防守，增加norPass的状态，保证norPass的时候盯防first的优先级最高
local COR_DEF_POS1 = CGeoPoint:new_local(-100,-150)
local COR_DEF_POS2 = CGeoPoint:new_local(-100,150)
local COR_DEF_POS3 = ball.refAntiYPos(CGeoPoint:new_local(-70,-20))
local DSS_FLAG = flag.allow_dss + flag.avoid_stop_ball_circle
gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= param.maxPlayer and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
	elseif cond.ballMoved() then
		return "exit"
	elseif gCurrentState == "marking" then
		if bufcnt(not DefendUtils.isPosInOurPenaltyArea(ball.pos()) 
			and ball.velMod() < 150 
			and math.abs(ball.posX()) < param.pitchLength/2,5,100) then
			return "norDef" -- remove norPass
		end
	elseif gCurrentState == "norPass" then
		if bufcnt(ball.velMod() < gNorPass2NorDefBallVel 
			or not enemy.hasReceiver(), 2) then
			return "norDef"
		end
	elseif gCurrentState == "norDef" then
		if bufcnt(cond.canDefenceExit(), 2,60) then
			return "finish"
		end
	else
		if bufcnt(cond.canDefenceExit(), 2) then
			return "finish"
		elseif enemy.situChanged() and 
			enemy.attackNum() <= param.maxPlayer and enemy.attackNum() > 0 then
			return "attacker"..enemy.attackNum()
		elseif bufcnt(cond.isGameOn(), 5) then
			if math.abs(ball.velDir()) > math.pi/4 then
				return "marking"
			else
				return "marking"
			end
		end
	end
end,

["beginning"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.goPassPos("Leader", DSS_FLAG),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir, DSS_FLAG),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, DSS_FLAG),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Center   = task.defendMiddle(_,DSS_FLAG),
	Assister = task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMFC]"
},

["attacker1"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.goPassPos("Leader", DSS_FLAG),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir, DSS_FLAG),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, DSS_FLAG),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Center   = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMFC]"
},

["attacker2"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.marking("First", DSS_FLAG),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir, DSS_FLAG),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, DSS_FLAG),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Center   = task.defendMiddle(_,DSS_FLAG),
	Assister = task.defendHead(DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[L][ASDMCF]"
},

["attacker3"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.marking("First", DSS_FLAG),
	Middle   = task.marking("Second", DSS_FLAG),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, DSS_FLAG),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Center   = task.defendMiddle(_,DSS_FLAG),
	Assister = task.defendHead(DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[L][SM][ADCF]"
},

["attacker4"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.marking("First", DSS_FLAG),
	Middle   = task.marking("Second", DSS_FLAG),
	Defender = task.marking("Third", DSS_FLAG),
	Assister = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Fronter  = task.rightBack(),
	Center   = task.leftBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][SDM][ACF]"
},

["attacker5"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.marking("First", DSS_FLAG),
	Middle   = task.marking("Second", DSS_FLAG),
	Defender = task.marking("Third", DSS_FLAG),
	Assister = task.marking("Fourth", DSS_FLAG),
	Fronter  = task.marking("Fifth", DSS_FLAG),
	Center   = task.defendHead(DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDAF][C]"
},

["attacker6"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.marking("First", DSS_FLAG),
	Middle   = task.marking("Second", DSS_FLAG),
	Defender = task.marking("Third", DSS_FLAG),
	Assister = task.marking("Fourth", DSS_FLAG),
	Fronter  = task.marking("Fifth", DSS_FLAG),
	Center   = task.marking("Sixth", DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDAFC]"
},

["attacker7"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.marking("First", DSS_FLAG),
	Middle   = task.marking("Second", DSS_FLAG),
	Defender = task.marking("Third", DSS_FLAG),
	Assister = task.marking("Fourth", DSS_FLAG),
	Fronter  = task.marking("Fifth", DSS_FLAG),
	Center   = task.marking("Sixth", DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDAFC]"
},

["attacker8"] = {
	Leader   = task.defendKick(_,_,_,_, DSS_FLAG),
	Special  = task.marking("First", DSS_FLAG),
	Middle   = task.marking("Second", DSS_FLAG),
	Defender = task.marking("Third", DSS_FLAG),
	Assister = task.marking("Fourth", DSS_FLAG),
	Fronter  = task.marking("Fifth", DSS_FLAG),
	Center   = task.marking("Sixth", DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDAFC]"
},

["marking"] = {
	Leader   = task.advance(_,DSS_FLAG),
	Special  = task.marking("First", DSS_FLAG),
	Middle   = task.marking("Second", DSS_FLAG),
	Defender = task.leftBack(),--task.marking("Fourth"),
	Assister = task.rightBack(),--task.singleBack(),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Center   = task.defendHead(DSS_FLAG),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMC][F]"--"[L][S][MD][A]"
},

["norPass"] = {
	Leader   = task.advance(_,DSS_FLAG),
	Special  = task.marking("First", DSS_FLAG),
	Middle   = task.marking("Second", DSS_FLAG),
	Defender = task.marking("Third", DSS_FLAG),
	Assister = task.defendHead(DSS_FLAG),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[S][L][MD][AFC]"
},

["norDef"] = {
	Leader   = task.advance(_,DSS_FLAG),
	Special  = task.marking("First", DSS_FLAG),
	Middle   = task.marking("Second", DSS_FLAG),
	Fronter  = task.marking("Third", DSS_FLAG),
	Center   = task.defendMiddle(_,DSS_FLAG),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][DAC][M][F]"
},

name = "Ref_FrontDefV1_m",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}