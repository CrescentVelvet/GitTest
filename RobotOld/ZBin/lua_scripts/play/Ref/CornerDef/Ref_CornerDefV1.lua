--角球防守，增加norPass的状态，保证norPass的时候盯防first的优先级最高
--防守头球，defendHead,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area实时有防开球车
--可以修改的地方有attacker5,attacker6,可以改变防头球车的匹配优先级，或者去掉防开球车，加一盯人车
local COR_DEF_POS1 = CGeoPoint:new_local(-170,-50)
local COR_DEF_POS2 = CGeoPoint:new_local(-170,50)
local COR_DEF_POS3 = CGeoPoint:new_local(-170,0)

local PreBallPos = function()
	return ball.pos() + Utils.Polar2Vector(ball.velMod()/10, ball.velDir())
end

local BP_FINISH = false
local KICKER_TASK = function()
	return function()
		if cond.ballPlaceFinish() then
			BP_FINISH = true
		end
		if cond.ballPlaceUnfinish() then
			BP_FINISH = false
		end
		if cond.ourBallPlace() and not (BP_FINISH) then
			return task.fetchBall(ball.placementPos,_,true)
		end
		return task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area)
	end
end

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
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
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.goPassPos("Leader", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Center   = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMFC]"
},

["attacker1"] = {
	Leader   = KICKER_TASK(),--,
	Special  = task.defendHead(flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),--task.goPassPos("Leader"),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Center   = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][DASMFC]"
},

["attacker2"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zmarking("First", flag.allow_dss + flag.avoid_stop_ball_circle+ flag.avoid_their_ballplacement_area),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Fronter  = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Center   = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "(AFC)[L][S][DM]"
},

["attacker3"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zmarking("First", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Middle   = task.zmarking("Second", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.defendHead(flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "(AFC)[L][S][MD]"
},

["attacker4"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zmarking("First", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Middle   = task.zmarking("Second", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.zmarking("Third", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.defendHead(flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "(AFC)[L][S][MD]"
},

["attacker5"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zmarking("First", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Middle   = task.zmarking("Second", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.zmarking("Third", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Fronter  = task.zmarking("Fourth", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Center   = task.defendHead(flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "(CA)[L][S][MDF]"
},

["attacker6"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zmarking("First", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Middle   = task.zmarking("Second", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.zmarking("Third", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Fronter  = task.zmarking("Fourth", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Center   = task.defendHead(flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.singleBack(),
	Goalie   = task.zgoalie(),
	match    = "(CA)[L][S][MDF]"
},

["attacker7"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zmarking("First", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Middle   = task.zmarking("Second", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.zmarking("Third", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Fronter  = task.zmarking("Fourth", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zmarking("Fifth", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Center   = task.defendHead(flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Goalie   = task.zgoalie(),
	match    = "(C)[L][S][MDAF]"
},

["attacker8"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zmarking("First", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Middle   = task.zmarking("Second", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.zmarking("Third", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Fronter  = task.zmarking("Fourth", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zmarking("Fifth", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Center   = task.defendHead(flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Goalie   = task.zgoalie(),
	match    = "(C)[L][S][MDAF]"
},



["marking"] = {
	Leader   = KICKER_TASK(),--task.markingDir("First",player.toBallDir),
	Special  = task.markingDir("Second",player.toBallDir),
	Middle   = task.zmarking("Third", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.advance(_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),--task.zmarking("Fourth", flag.allow_dss + flag.avoid_stop_ball_circle),
	Fronter  = task.defendHead(flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "(FAC)[D][LS][M]"--"[L][S][A][MD]"
},

["norPass"] = {
	Leader   = KICKER_TASK(),--task.advance(_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Defender = task.markingDir("Third",player.toBallDir),
	Assister = task.defendHead(flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Fronter  = task.leftBack(),
	Center   = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[S][L][MD][AFC]"
},

["norDef"] = {
	Leader   = KICKER_TASK(),--task.advance(_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Fronter  = task.markingDir("Third",player.toBallDir) ,
	Center   = task.defendMiddle(_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.zgoalie(),
	match    = "[L][S][DAC][M][F]"
},

name = "Ref_CornerDefV1",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}