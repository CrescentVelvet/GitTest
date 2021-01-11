local STOP_POS1 = pos.LEADER_STOP_POS

local COR_DEF_POS1 = CGeoPoint:new_local(-50*param.lengthRatio,-50*param.widthRatio)
local SIDE_POS, MIDDLE_POS, INTER_POS = pos.refStopAroundBall()
gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()	
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 8 and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
	else
		if cond.isGameOn() then
			return "finish"
		elseif enemy.situChanged() and
			enemy.attackNum() <= 8 and enemy.attackNum() > 0 then
			return "attacker"..enemy.attackNum()
		end
	end
end,
-- headback sideback defend middle
["beginning"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.goPassPos("Leader", flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Middle   = task.goCmuRush(COR_DEF_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Fronter  = task.goCmuRush(INTER_POS, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Center   = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][DA][SMFC]"
},

["attacker1"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.goPassPos("Leader", flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Middle   = task.goCmuRush(COR_DEF_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Fronter  = task.goCmuRush(INTER_POS, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Center   = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][DA][SMFC]"
},

["attacker2"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.goPassPos("Leader", flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Middle   = task.goCmuRush(COR_DEF_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Center   = task.zmarking("First", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender = task.leftBack(),
	Fronter  = task.defendHead(flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][DA][SMFC]"
},

["attacker3"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle+flag.free_kick),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender = task.goCmuRush(COR_DEF_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Center   = task.leftBack(),
	Fronter  = task.rightBack(),
	Assister = task.defendHead(flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Goalie   = task.zgoalie(),
	match    = "[L][CF][SMDA]"
},

["attacker4"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle+flag.free_kick),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender = task.zmarking("Third", flag.avoid_stop_ball_circle+flag.free_kick),
	Center   = task.leftBack(),
	Fronter  = task.rightBack(),
	Assister = task.defendHead(flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Goalie   = task.zgoalie(),
	match    = "[L][CF][SMDA]"
},

["attacker5"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle+flag.free_kick),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender = task.zmarking("Third", flag.avoid_stop_ball_circle+flag.free_kick),
	Center   = task.zmarking("Fourth", flag.avoid_stop_ball_circle+flag.free_kick),
	Fronter  = task.singleBack(),
	Assister = task.defendHead(flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDFCA]"
},

["attacker6"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle+flag.free_kick),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender = task.zmarking("Third", flag.avoid_stop_ball_circle+flag.free_kick),
	Center   = task.zmarking("Fourth", flag.avoid_stop_ball_circle+flag.free_kick),
	Fronter  = task.zmarking("Fifth", flag.avoid_stop_ball_circle+flag.free_kick),
	Assister = task.defendHead(flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDFCA]"
},

["attacker7"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle+flag.free_kick),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender = task.zmarking("Third", flag.avoid_stop_ball_circle+flag.free_kick),
	Center   = task.zmarking("Fourth", flag.avoid_stop_ball_circle+flag.free_kick),
	Fronter  = task.zmarking("Fifth", flag.avoid_stop_ball_circle+flag.free_kick),
	Assister = task.zmarking("Sixth", flag.avoid_stop_ball_circle+flag.free_kick),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDFCA]"
},

["attacker8"] = {
	Leader   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Special  = task.zmarking("First", flag.avoid_stop_ball_circle+flag.free_kick),
	Middle   = task.zmarking("Second", flag.avoid_stop_ball_circle+flag.free_kick),
	Defender = task.zmarking("Third", flag.avoid_stop_ball_circle+flag.free_kick),
	Center   = task.zmarking("Fourth", flag.avoid_stop_ball_circle+flag.free_kick),
	Fronter  = task.zmarking("Fifth", flag.avoid_stop_ball_circle+flag.free_kick),
	Assister = task.defendHead(flag.slowly+flag.dodge_ball+flag.allow_dss+flag.free_kick),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDFCA]"
},

name = "Ref_StopZMarking",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}