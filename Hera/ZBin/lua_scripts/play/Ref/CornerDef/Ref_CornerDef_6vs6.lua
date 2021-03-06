--角球防守，增加norPass的状态，保证norPass的时候盯防first的优先级最高
--防守头球，defendHead,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area实时有防开球车
--可以修改的地方有attacker5,attacker6,可以改变防头球车的匹配优先级，或者去掉防开球车，加一盯人车
local COR_DEF_POS1 = CGeoPoint:new_local(-3300,-1300)
local COR_DEF_POS2 = CGeoPoint:new_local(-3300,1300)
local COR_DEF_POS3 = CGeoPoint:new_local(-3300,0)


function getAttackerNum(i)
    return function ()
        return defenceSquence:getAttackNum(i)
    end
end

local PreBallPos = function()
	return ball.pos() + Utils.Polar2Vector(ball.velMod()/10, ball.velDir())
end

local DEFEND_POS = function ()
	return function ()
		if ball.posY() >= 0 then
			return COR_DEF_POS1
		else
			return COR_DEF_POS2
		end
	end
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
		defenceSquence:attackerAmount() <= 6 and defenceSquence:attackerAmount() > 0 then
			return "attacker"..defenceSquence:attackerAmount() 
	elseif gCurrentState == "busdefence" and bufcnt(ball.posX() > -1/6 * param.pitchLength or player.infraredOn("Leader"), 1, 200) then
		return "exit"
	elseif (ball.pos()-ball.refPos()):mod() > 150 * 10 then
		return "busdefence"
	-- elseif cond.ballMoved() then
		-- return "exit"
	else
		-- if enemy.situChanged() and 
		if defenceSquence:attackerAmount() <= param.maxPlayer and defenceSquence:attackerAmount() > 0 then
			return "attacker"..defenceSquence:attackerAmount()
		end
	end
end,

["beginning"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.goPassPos("Leader", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zback(),
	Goalie   = task.zgoalie(),
	match    = "[L][ASMD]"
},

["attacker1"] = {
	Leader   = KICKER_TASK(),--,
	Special  = task.defendHead(flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),--task.goPassPos("Leader"),
	Middle   = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zback(),
	Goalie   = task.zgoalie(),
	match    = "[L][DASM]"
},

-- ["attacker2"] = {
-- 	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
-- 	Special  = task.zmarking("Zero", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area, getAttackerNum(0)),
-- 	Middle   = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
-- 	Defender = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
-- 	Assister = task.zback(1,1),
-- 	Goalie   = task.zgoalie(),
-- 	match    = "[L][S][DM][A]"
-- },

["attacker2"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zback(1,1,_,_,getAttackerNum(0)),
	Middle   = task.zmarking("Zero", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area, getAttackerNum(0)),
	Defender = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Goalie   = task.zgoalie(),
	match    = "[L][S][AD][M]"
},


["attacker3"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zback(2,1,_,_,getAttackerNum(0)),
	Middle   = task.zback(2,2,_,_,getAttackerNum(1)),
	Defender = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.goSpeciPos(COR_DEF_POS3, player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Goalie   = task.zgoalie(),
	match    = "[L][SM][DA]"
},

["attacker4"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zback(3,1,_,_,getAttackerNum(0)),
	Middle   = task.zback(3,2,_,_,getAttackerNum(1)),
	Defender = task.zback(3,3,_,_,getAttackerNum(2)),
	Assister = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Goalie   = task.zgoalie(),
	match    = "[L][SMD][A]"
},

["attacker5"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zback(4,1,_,_,getAttackerNum(0)),
	Middle   = task.zback(4,2,_,_,getAttackerNum(1)),
	Defender = task.zback(4,3,_,_,getAttackerNum(2)),
	Assister = task.zback(4,4,_,_,getAttackerNum(3)),
	Goalie   = task.zgoalie(),
	match    = "[L][SDMA]"
},

["attacker6"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zback(4,1,_,_,getAttackerNum(0)),
	Middle   = task.zback(4,2,_,_,getAttackerNum(1)),
	Defender = task.zback(4,3,_,_,getAttackerNum(2)),
	Assister = task.zback(4,4,_,_,getAttackerNum(3)),
	Goalie   = task.zgoalie(),
	match    = "[L][SDMA]"
},

["marking"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zmarking("First", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Middle   = task.zmarking("Second", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zback(1,1),
	Goalie   = task.zgoalie(),
	match    = "[L][SM][D][A]"
},

["norPass"] = {
	Leader   = KICKER_TASK(),--task.defendKick(_,_,_,_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zmarking("First", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Middle   = task.zmarking("Second", flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Defender = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zback(1,1),
	Goalie   = task.zgoalie(),
	match    = "[L][SM][D][A]"
},

["norDef"] = {
	Leader   = KICKER_TASK(),--task.advance(_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Defender = task.goSpeciPos(DEFEND_POS(), player.toBallDir, flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Assister = task.zback(1,1),
	Goalie   = task.zgoalie(),
	match    = "[L][SM][D][A]"
},

["busdefence"] = {
	Leader   = task.zget(_,_,_,flag.dribble + flag.safe + flag.rush),--task.advance(_,flag.allow_dss + flag.avoid_stop_ball_circle + flag.avoid_their_ballplacement_area),
	Special  = task.zback(4,1),
	Middle   = task.zback(4,2),
	Defender = task.zback(4,3),
	Assister = task.zback(4,4),
	Goalie   = task.zgoalie(),
	match    = "[L][SMDA]"
},

name = "Ref_CornerDef_6vs6",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}