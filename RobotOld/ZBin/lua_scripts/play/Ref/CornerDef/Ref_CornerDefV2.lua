--角球防守，增加norPass的状态，保证norPass的时候盯防first的优先级最高
local COR_DEF_POS1 = CGeoPoint:new_local(-100,-150)
local COR_DEF_POS2 = CGeoPoint:new_local(-100,150)
local COR_DEF_POS3 = ball.refAntiYPos(CGeoPoint:new_local(-70,-20))
local DSS_FLAG = flag.allow_dss
gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= param.maxPlayer and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
	elseif cond.ballMoved() then
		return "exit"
	else
		if enemy.situChanged() and 
			enemy.attackNum() <= param.maxPlayer and enemy.attackNum() > 0 then
			return "attacker"..enemy.attackNum()
		end
	end
end,

["beginning"] = {
	Leader   = task.defendKick(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir, DSS_FLAG),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, DSS_FLAG),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Center   = task.defendMiddle(),
	Assister = task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMFC]"
},

["attacker1"] = {
	Leader   = task.defendKick(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir, DSS_FLAG),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, DSS_FLAG),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Center   = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMFC]"
},

["attacker2"] = {
	Leader   = task.defendKick(),
	Special  = task.zmarking("First"),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir, DSS_FLAG),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, DSS_FLAG),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Center   = task.defendMiddle(),
	Assister = task.defendHead(),
	Goalie   = task.zgoalie(),
	match    = "[L][ASDMCF]"
},

["attacker3"] = {
	Leader   = task.defendKick(),
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, DSS_FLAG),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Center   = task.defendMiddle(),
	Assister = task.defendHead(),
	Goalie   = task.zgoalie(),
	match    = "[L][SM][ADCF]"
},

["attacker4"] = {
	Leader   = task.defendKick(),
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.zmarking("Third"),
	Assister = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Fronter  = task.rightBack(),
	Center   = task.leftBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][SDM][ACF]"
},

["attacker5"] = {
	Leader   = task.defendKick(),
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.zmarking("Third"),
	Assister = task.zmarking("Fourth"),
	Fronter  = task.zmarking("Fifth"),
	Center   = task.defendHead(),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDAF][C]"
},

["attacker6"] = {
	Leader   = task.defendKick(),
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.zmarking("Third"),
	Assister = task.zmarking("Fourth"),
	Fronter  = task.zmarking("Fifth"),
	Center   = task.zmarking("Sixth"),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDAFC]"
},

["attacker7"] = {
	Leader   = task.defendKick(),
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.zmarking("Third"),
	Assister = task.zmarking("Fourth"),
	Fronter  = task.zmarking("Fifth"),
	Center   = task.zmarking("Sixth"),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDAFC]"
},

["attacker8"] = {
	Leader   = task.defendKick(),
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.zmarking("Third"),
	Assister = task.zmarking("Fourth"),
	Fronter  = task.zmarking("Fifth"),
	Center   = task.zmarking("Sixth"),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDAFC]"
},

["marking"] = {
	Leader   = task.advance(),
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.leftBack(),--task.zmarking("Fourth"),
	Assister = task.rightBack(),--task.singleBack(),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, DSS_FLAG),
	Center   = task.defendHead(),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMC][F]"--"[L][S][MD][A]"
},

["norPass"] = {
	Leader   = task.advance(),
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Defender = task.zmarking("Third"),
	Assister = task.defendHead(),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[S][L][MD][AFC]"
},

["norDef"] = {
	Leader   = task.advance(),
	Special  = task.zmarking("First"),
	Middle   = task.zmarking("Second"),
	Fronter  = task.zmarking("Third"),
	Center   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][DAC][M][F]"
},

name = "Ref_CornerDefV2",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}