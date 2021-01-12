module(..., package.seeall)

--~		Play中统一处理的参数（主要是开射门）
--~		1 ---> task, 2 ---> matchpos, 3---->kick, 4 ---->dir,
--~		5 ---->pre,  6 ---->kp,       7---->cp,   8 ---->flag
------------------------------------- 射门相关的skill ---------------------------------------

-- function test()
-- 	return function(runner)
-- 		if runner == nil then
-- 			matchPos = function()
-- 			  	local x, y = CZGetBallPos()
-- 			  	return CGeoPoint:new_local(x,y)
-- 			end
-- 			return  {_,matchPos}
-- 		else
-- 			return zget()
-- 		end
-- 	end
-- end


function chaseNew()
	local mexe, mpos = ChaseKickV2{dir = dir.chase}
	return {mexe, mpos, _, dir.chase, pre.low, kp.specified(400*10), cp.full, flag.nothing}
end


-- -- p1是截球目标点 默认为当前位置 p2是射门点或传球角色 p是力度 b是截球的缓冲距离 f是是否挑射 
-- function InterTouch(p1, p2, p, b, f)
-- 	local ipos
-- 	local itarget  --add by Wang in 2018/06/18
-- 	local idir
-- 	local ipower
-- 	local ibuffer
-- 	local ikick = f and kick.chip() or kick.flat()
-- 	local iTestMode = false
-- 	if type(p1) == "function" then
-- 		ipos = p1()
-- 	elseif p1 ~= nil then
-- 		ipos = p1
-- 		ipos = nil
-- 	else
-- 	end

-- 	if p2 ~= nil then
-- 		if type(p2) == "userdata" then
-- 			idir = dir.compensate(p2)
-- 			itarget = p2
-- 		elseif type(p2) == "function" then
-- 			idir = dir.compensate(p2)
-- 			itarget = p2
-- 		elseif p2 == false then -- false时为了测试补偿数据
-- 			iTestMode = true
-- 			p2 = CGeoPoint:new_local(param.pitchLength / 2,0)
-- 			idir = dir.nocompensation(CGeoPoint:new_local(param.pitchLength / 2,0))
-- 			itarget = CGeoPoint:new_local(param.pitchLength / 2,0)
-- 		elseif p2 == true then
-- 			idir = dir.shoot()
-- 			itarget = CGeoPoint:new_local(param.pitchLength / 2,0)
-- 		end
-- 	else
-- 		idir = dir.shoot()
-- 		itarget = CGeoPoint:new_local(param.pitchLength/2,param.goalWidth/2 - 10*10)
-- 	end

-- 	if p ~= nil then
-- 		ipower = kp.specified(p)
-- 	else
-- 		if p2 ~= nil then
-- 			ipower = kp.toTarget(p2)
-- 		else
-- 			ipower = kp.full()
-- 		end
-- 	end

-- 	if b ~= nil then
-- 		ibuffer = b
-- 	else
-- 		ibuffer = 9.2*10  -- 接球时默认后退距离
-- 	end

-- 	local mexe, mpos = InterceptTouch{pos = ipos, target = itarget, dir = idir, power = ipower, buffer = ibuffer, kick = ikick,testMode = iTestMode}
-- 	return {mexe, mpos}
-- end

function shoot()
	local ipos = CGeoPoint:new_local(9999*10,9999*10)
	local ipower = kp.full()
	local itarget = CGeoPoint:new_local(param.pitchLength/2.0,0)
    local iflag = flag.kick + flag.safe
	local mexe, mpos = GetBallV4{ pos=itarget, waitpos = ipos, power = ipower, flag = iflag}
	return {mexe, mpos}
end

function penaltyKick2017V1()
	local mexe,mpos = PenaltyKick2017V1{}
	return {mexe, mpos, kick.flat, dir.chase, pre.specified(5), kp.full, cp.full, flag.nothing}
end

function penaltyKick2017V2()
	local mexe,mpos = PenaltyKick2017V2{}
	return {mexe, mpos, kick.flat, dir.chase, pre.specified(5), kp.full, cp.full, flag.nothing}
end

function penaltyTurn(d, k)
	if k == true then
		k = kick.flat
	else
		k = kick.none
	end
	local mexe, mpos = SpeedInRobot{speedW = -11, speedX = 120*10} -- 点球甩飞改 speedW
	return {mexe, mpos, k, d, pre.specified(10), kp.full, cp.full, flag.nothing}
end

function fetchBall(target, power, if_push)
	local itarget
	local ipower
	local iflag = if_push and kickflag.safe or 0
	if target ~= nil then
		if type(target) == "string" then
			itarget = function()
				return player.pos(target)
			end
		else
			itarget = target
		end
	else
		itarget = CGeoPoint:new_local(0,0)
		print("ERROR in fetchBall, need target point")
	end
	if power ~= nil then
		ipower = kp.specified(power)
	elseif target ~= nil then
		ipower = kp.toTarget(itarget)
	else
		ipower = kp.full()
		print("ERROR in fetchBall, need kick power")
	end
	local mexe, mpos =FetchBall{target = itarget, power = ipower, flag = iflag}
	return {mexe, mpos}
end

function zpass(target, p, chip, precision)
	local itarget
	local ipower
	local ikick = chip and kickflag.chip or kickflag.kick
	local iprecision
	if target ~= nil then
		if type(target) == "string" then
			itarget = function()
				return player.pos(target)
			end
		else
			itarget = target
		end
	else
		itarget = CGeoPoint:new_local(param.pitchLength/2.0,0)
	end
	if p ~= nil then
		ipower = chip and cp.specified(p) or kp.specified(p)
	elseif target ~= nil then
		ipower = chip and cp.toTarget(itarget) or kp.toTarget(itarget)
	else
		ipower = chip and cp.full() or kp.full()
	end
	if precision ~= nil then
		iprecision = precision
	else
		iprecision = -1
	end
	local mexe, mpos = ZPass{target = itarget, power = ipower, flag = ikick, precision = iprecision}
	return {mexe, mpos}
end

function zsupport()
	local mexe, mpos = ZSupport()
	return {mexe, mpos}
end

function zbreak(target, p, chip, precision)
	local itarget
	local ipower
	local ikick = chip and kickflag.chip or kickflag.kick
	local iprecision
	if target ~= nil then
		if type(target) == "string" then
			itarget = function()
				return player.pos(target)
			end
		else
			itarget = target
		end
	else
		itarget = CGeoPoint:new_local(param.pitchLength/2.0,0)
	end
	if p ~= nil then
		ipower = chip and cp.specified(p) or kp.specified(p)
	elseif target ~= nil then
		ipower = chip and cp.toTarget(itarget) or kp.toTarget(itarget)
	else
		ipower = chip and cp.full() or kp.full()
	end
	if precision ~= nil then
		iprecision = precision
	else
		iprecision = -1
	end
	local mexe, mpos = ZBreak{target = itarget, power = ipower, flag = ikick, precision = iprecision}
	return {mexe, mpos}
end

function zattack(target, waitpos, p, flag, precision)
	local itarget
	local iwaitpos
	local ipower = p
	local iflag = flag
	local iprecision

	if target ~= nil then
		if type(target) == "string" then
			itarget = function()
				return player.pos(target)
			end
		else
			itarget = target
		end
	else
		itarget = CGeoPoint:new_local(param.pitchLength/2.0,0)
	end

	if type(waitpos) == "function" then
		iwaitpos = waitpos
	elseif type(waitpos) == "userdata" then
		iwaitpos = waitpos
	elseif type(waitpos) == "string" then
		iwaitpos = function()
			return player.pos(waitpos)
		end
	else
		iwaitpos = CGeoPoint:new_local(9999*10,9999*10)
	end

	if precision ~= nil then
		iprecision = precision
	else
		iprecision = -1
	end

	local mexe, mpos = ZAttack{target = itarget, waitpos = iwaitpos, power = ipower, flag = iflag, precision = iprecision}
	return {mexe, mpos}
end

function holdball(t)
	local mexe, mpos = HoldBall{target = t}
	return {mexe, mpos}
end

function zcirclePass(target, p, chip)
	local itarget
	local ipower
	local ikick = chip and kickflag.chip or kickflag.flat
	if target ~= nil then
		if type(target) == "string" then
			itarget = function()
				return player.pos(target)
			end
		else
			itarget = target
		end
	else
		itarget = CGeoPoint:new_local(param.pitchLength/2.0,0)
	end
	if p ~= nil then
		ipower = chip and cp.specified(p) or kp.specified(p)
	elseif target ~= nil then
		ipower = chip and cp.toTarget(itarget) or kp.toTarget(itarget)
	else
		ipower = chip and cp.full() or kp.full()
	end
	local mexe, mpos = ZCirclePass{target = itarget, power = ipower, flag = ikick}
	return {mexe, mpos}
end

function penaltyTurnShoot(d)
	local mexe, mpos = DribbleTurnKick{fDir = d, rotV=3.5, kPower=1200}
	return {mexe, mpos}
end

function penaltyChase(d)
	local mexe, mpos = ChaseKick{dir = d, flag = flag.dribbling+flag.our_ball_placement}
	return {mexe, mpos, kick.flat, d, pre.middle, kp.specified(570), cp.full, flag.force_kick } --penalty如果超速改这里 by.NaN
end


----------------------------------------------------------------------------------------------

------------------------------------ 传球相关的skill ---------------------------------------
function receivePass(p,c)
	local idir
	if type(p) == "string" then
		idir = player.toPlayerHeadDir(p)
	elseif type(p) == "function" then
		idir = player.toPointDir(p)
	else
		idir = player.toPointDir(p)
	end

	local ipower
	if c == nil then
		ipower = kp.toTarget(p)
	else
		ipower = kp.specified(c)
	end
	local mexe, mpos = ReceivePass{dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.high, ipower, cp.full, flag.nothing}
end

-- f为false代表p为函数,并且返回值为userdata,p不能有参数
-- f为nil是为了兼容以前的情况
function chipPass(p, c, f, anti)
	local idir
	local ipower
	if type(p) == "string" then
		idir = player.toPlayerDir(p)
	elseif type(p) == "function" then
		if f == nil or f == true then
			idir = p
		elseif anti == false then
			idir = function (role)
				return (p() - player.pos(role)):dir()
			end
		else
			idir = ball.antiYDir(p)
		end
	else
		if anti == false then
			idir = function (role)
				if type(p) == "userdata" then
					return (p - player.pos(role)):dir()
				end
			end
		else
			idir = player.antiYDir(p)
		end
	end

	if c == nil then
		if type(p) == "string" then
			ipower = cp.toPlayer(p)
		elseif type(p) == "userdata" then
			ipower = cp.toTarget(ball.antiYPos(p))
		elseif type(p) == "function" then
			if f == false or f == nil then
				if anti == true then
					ipower = cp.toTarget(ball.antiYPos(p))
				else
					ipower = cp.toTarget(p())
				end
			end
		end
	elseif type(c) == "number" then
		ipower = cp.specified(c)
	else
		ipower = c
	end
	local mexe, mpos = ChaseKick{pos = ball.pos,dir = idir}
	return {mexe, mpos, kick.chip, idir, pre.middle, kp.specified(0), ipower, flag.nothing}
end

--添加接口 hzy
function flatPass(roleorp, power)
	local idir
	local ipower

	if type(roleorp) == "string" then
		idir = ball.toPlayerHeadDir(roleorp)
	elseif type(roleorp) == "function" then
		idir = player.toPointDir(roleorp())
	else
		idir = player.toPointDir(roleorp)
	end

	if power == nil then
		ipower = kp.toTarget(roleorp)
	elseif type(power) == "string" then
		-- ipower = getluadata:getKickParamPower()
		-- print("is function")
		return function ()
			local ppower = getluadata:getKickParamPower()
			ipower = kp.specified( ppower )
			local mexe, mpos = ChaseKickV2{ pos = ball.pos, dir = idir}
			return {mexe, mpos, kick.flat, idir, pre.high, ipower, cp.full, flag.nothing}
		end
	else
		ipower = kp.specified(power)
	end

	local mexe, mpos = ChaseKickV2{ pos = ball.pos, dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.high, ipower, cp.full, flag.nothing}
end

function passToPos(p,c)
	local idir=player.toPointDir(p)
	local ipower
	if type(c)== "number" and c ~= nil then
		ipower=kp.specified(c)
	else
		ipower=kp.toTarget(p)
	end
	local mexe, mpos = ChaseKickV2{ pos = ball.pos, dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.middle, ipower, cp.full, flag.nothing}
end

-- role 为接球车/接球点
function goAndTurnKick(role, power, icircle, a, isPos) -- 2014-03-28 added by yys, 增加转动圈数参数,并且第一个参数可以直接传入弧度
	local idir
	local ipower
	if type(role) == "number" then  -- 这里idir必须为函数，因为play.lua里面函数调用
		idir = function()
		  return role
		end
	elseif type(role) == "string" then
		idir = ball.toPlayerHeadDir(role)
	elseif type(role) == "function" then
		if isPos == true then
			idir = ball.toPointDir(role)
		else
			idir = ball.toFuncDir(role)
		end
	elseif type(role) == "userdata" then
		idir = player.antiYDir(role)
	end

	if power == nil then
		ipower = kp.toTarget(role)
	else
		ipower = kp.specified(power)
	end

	if icircle == nil then
		icircle = 0
	end

	local mexe, mpos = GoAndTurnKickV3{ pos = ball.pos, dir = idir, pre = pre.high, circle = icircle, acc = a}
	return {mexe, mpos, kick.flat, idir, pre.specified(4), ipower, cp.full, flag.nothing}
end

function goAndTurnKickV4(p, pow, d, f)
	local mdir
	local mpos
	local mpower
	--朝向点和方向
	if p == nil then--不发点则射对方门
		mpos = CGeoPoint:new_local(600*10,0)
	else
		if type(p) == "string" then
			mpos = player.pos(p)
		elseif type(p) == "function" then
			mpos = p
		else
			mpos = p
		end
	end

	if d == nil then
		mdir = 9999
	else
		if type(d) == "function" then
			mdir = d
		else
			mdir = d
		end
	end

	--射门力度
	if pow == nil and f ~= nil then
		if bit:_and(f, flag.chip) ~= 0 then
			mpower = cp.toTarget(mpos)
		elseif bit:_and(f, flag.kick) ~= 0 then
			mpower = kp.toTarget(mpos)
		end
	elseif pow == nil and f == nil then
		mpower = kp.toTarget(mpos)
  elseif type(pow) == "function" then
		mpower = pow()
	else
		mpower = pow
	end

	local mexe, mpos = GoAndTurnKickV4{ pos=mpos, power = mpower, dir = mdir, flag = f}
	return {mexe, mpos}
end

------------------------------------ 跑位相关的skill ---------------------------------------
--~ p为要走的点,d默认为射门朝向
function goSpeciPos(p, d, f, a) -- 2014-03-26 增加a(加速度参数)
	local idir
	local iflag
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end

	if f ~= nil then
		iflag = f
	else
		iflag = 0
	end

	local mexe, mpos = SmartGoto{pos = p, dir = idir, flag = iflag, acc = a}
	return {mexe, mpos}
end

function goSimplePos(p, d, f)
	local idir
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end

	if f ~= nil then
		iflag = f
	else
		iflag = 0
	end

	local mexe, mpos = SimpleGoto{pos = p, dir = idir, flag = iflag}
	return {mexe, mpos}
end

-- role为传球车
function goPassPos(role, f)
	local mexe, mpos = SmartGoto{ pos = messi:receiverPos(), dir = player.toShootOrRobot(role), flag = f}
	return {mexe, mpos}
end

-- function goFirstPassPos(role)
-- 	local mexe, mpos = GoAvoidShootLine{ pos = ball.firstPassPos(), dir = player.toShootOrRobot(role),sender=role}
-- 	return {mexe, mpos}
-- end

-- function goSecondPassPos(role)
-- 	local mexe, mpos = GoAvoidShootLine{ pos = ball.secondPassPos(), dir = player.toShootOrRobot(role),sender=role}
-- 	return {mexe, mpos}
-- end

function runMultiPos(p, c, d, idir, a)
	if c == nil then
		c = false
	end

	if d == nil then
		d = 20
	end

	if idir == nil then
		idir = dir.shoot()
	end

	local mexe, mpos = RunMultiPos{ pos = p, close = c, dir = idir, flag = flag.not_avoid_our_vehicle, dist = d, acc = a}
	return {mexe, mpos}
end

--~ p为要走的点,d默认为射门朝向
function goCmuRush(p, d, a, f, r, v)
	local idir
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end
	local mexe, mpos = GoCmuRush{pos = p, dir = idir, acc = a, flag = f,rec = r,vel = v}
	return {mexe, mpos}
end

function goBezierRush(p, d, a, f, r)
	local idir
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end

	local mexe, mpos = BezierRush{pos = p, dir = idir, acc = a, flag = f, rec = r}
	return {mexe, mpos}
end

--immortal rush
function noneZeroRush(p, d, a, f, r, v)
	local idir
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end
	local mexe, mpos = NoneZeroRush{pos = p, dir = idir, acc = a, flag = f, rec = r, vel = v}
	return {mexe, mpos}
end

------------------------------------ 防守相关的skill ---------------------------------------

function defendDefault(p)
	local mexe, mpos = GotoMatchPos{ pos = pos.defaultDefPos(p), dir = player.toBallDir, acc = 1200*10,flag = flag.avoid_stop_ball_circle}
	return {mexe, mpos}
end

function defendHead(f)
	local mflag = f or 0
	local mexe, mpos = GotoMatchPos{ pos = pos.defendHeadPos, dir = player.toBallDir, acc = 800*10,flag = mflag}
	return {mexe, mpos}
end

function defendKick(p1,p2,p3,p4,f)
	local mflag = f or 0
	local mexe, mpos = SmartGoto{ acc = 800*10, pos = pos.defendKickPos(p1,p2,p3,p4), dir = player.toBallDir, flag = mflag}
	return {mexe, mpos}
end

function defendKickDir(p1,p2,p3,p4)
	local mexe, mpos = SmartGoto{ acc = 800*10, pos = pos.defendKickPos(p1,p2,p3,p4), dir = player.toTheirGoal}
	return {mexe, mpos}
end

function leftBack(p)
	local ipower
	if p == nil then
		ipower = 2700
	else
		ipower = p
	end
	local mexe, mpos = GotoMatchPos{ method = 4, acc = 500*10, pos = pos.leftBackPos, dir = dir.backSmartGotoDir, srole = "leftBack", flag = bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle, flag.allow_dss)}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(ipower),cp.specified(ipower), bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
end

function rightBack(p)
	local ipower
	if p == nil then
		ipower = 2700
	else
		ipower = p
	end
	local mexe, mpos = GotoMatchPos{ method = 4, acc = 500*10, pos = pos.rightBackPos, dir = dir.backSmartGotoDir, srole = "rightBack", flag = bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle, flag.allow_dss)}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(ipower),cp.specified(ipower), bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
end

function multiBack(guardNum, index, p)
	local ipower
	if p == nil then
		ipower = 2700
	else
		ipower = p
	end
	local mexe, mpos = GotoMatchPos{ method = 4, acc = 450, pos = pos.multiBackPos(guardNum, index), dir = dir.backSmartGotoDir, flag = bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(ipower),cp.specified(ipower), bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
end

function zback(guardNum, index, p, f)
	local ipower
	local iflag = f or (flag.kick + flag.chip + flag.not_avoid_our_vehicle + flag.not_avoid_their_vehicle)
	if p == nil then
		ipower = 2700
	else
		ipower = p
	end
	local mexe, mpos = ZBack{ guardNum = guardNum, index = index, power = ipower, flag = iflag }
	return {mexe, mpos}
end

function leftBack4Stop()
	local STOP_FLAG = flag.dodge_ball
	local STOP_NO_DODGE_SELF = bit:_or(STOP_FLAG, flag.not_avoid_our_vehicle)
	local STOP_DSS = bit:_or(STOP_NO_DODGE_SELF, flag.allow_dss)
	local mexe, mpos = GotoMatchPos{ pos = pos.leftBackPos, dir = dir.backSmartGotoDir, flag = STOP_DSS}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(400), STOP_DSS}
end

function rightBack4Stop()
	local STOP_FLAG = flag.dodge_ball
	local STOP_NO_DODGE_SELF = bit:_or(STOP_FLAG, flag.not_avoid_our_vehicle)
	local STOP_DSS = bit:_or(STOP_NO_DODGE_SELF, flag.allow_dss)
	local mexe, mpos = GotoMatchPos{ pos = pos.rightBackPos, dir = dir.backSmartGotoDir, flag = STOP_DSS}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(400), STOP_DSS}
end

function singleBack4Stop()
	local STOP_FLAG = flag.dodge_ball
	local STOP_DSS = bit:_or(STOP_FLAG, flag.allow_dss)
	local mexe, mpos = GotoMatchPos{ pos = pos.singleBackPos, dir = dir.backSmartGotoDir, flag = STOP_DSS}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(400), STOP_DSS}
end

-- 此处的车为射门的车，如果不传则不避射门线
function defendMiddle(role, f)
	local mflag = f or 0
	if role == nil then
		mflag = 0--flag.not_avoid_our_vehicle
	else
		mflag = mflag--bit:_or(flag.avoid_shoot_line, flag.not_avoid_our_vehicle)
	end
	mflag=bit:_or(flag.allow_dss,mflag)
	local mexe, mpos = GotoMatchPos{ pos = pos.defendMiddlePos,dir = dir.backSmartGotoDir , srole = "defendMiddle", flag = mflag, sender = role}
	return {mexe, mpos, kick.flat, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(400), mflag}
end

function defendMiddle4Stop(role)
	local mflag
	if role == nil then
		mflag = 0--flag.not_avoid_our_vehicle
	else
		mflag = mflag--bit:_or(flag.avoid_shoot_line, flag.not_avoid_our_vehicle)
	end
	local STOP_FLAG = bit:_or(flag.dodge_ball)
	local STOP_DSS = bit:_or(STOP_FLAG, flag.allow_dss)
	mflag=bit:_or(STOP_DSS,mflag)
	local mexe, mpos = GotoMatchPos{ pos = pos.defendMiddlePos,dir = dir.backSmartGotoDir , srole = "defendMiddle", flag = mflag, sender = role}
	return {mexe, mpos, kick.flat, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(400), mflag}
end

-- function singleBack()
-- 	local mexe, mpos = GotoMatchPos{ method = 1, acc = 1000, pos = pos.singleBackPos, dir = dir.backSmartGotoDir, srole = "singleBack",bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
-- 	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(400), bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
-- end

function singleBack()
		local mexe, mpos = GotoMatchPos{ method = 1, acc = 1000*10, pos = pos.singleBackPos, dir = dir.backSmartGotoDir, srole = "singleBack",bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle, flag.allow_dss)}
		return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(300),cp.specified(300), bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle, flag.allow_dss)}
	end

function sideBack()
	local mexe, mpos = GotoMatchPos{ pos = pos.sideBackPos, dir = dir.sideBackDir, srole = "sideBack",bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle, flag.allow_dss)}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(400), bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle, flag.allow_dss)}
end

function sideBack4Stop()
	local STOP_FLAG = flag.dodge_ball
	local STOP_DSS = bit:_or(STOP_FLAG, flag.allow_dss)
	local mexe, mpos = GotoMatchPos{ pos = pos.sideBackPos, dir = dir.sideBackDir, srole = "sideBack",flag = STOP_DSS}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(400), STOP_DSS}
end

function zgoalie(target,p,f)
    local ipower
    local itarget = target or CGeoPoint:new_local(param.pitchLength/2.0,0)
    local iflag = f or (flag.kick + flag.chip + flag.dribble)
    local isChip = bit:_and(iflag,flag.chip)
    if p ~= nil then
        ipower = isChip == 0 and kp.specified(p) or cp.specified(p)
    elseif target ~= nil then
        ipower = isChip == 0 and kp.toTarget(itarget) or cp.toTarget(itarget)
    else
        ipower = isChip == 0 and 300*10 or 80*10
    end
    local mexe, mpos = ZGoalie{pos = itarget, power = ipower, flag = iflag}
    return {mexe, mpos}
end

function penaltyGoalieV2()
	local mexe, mpos = PenaltyGoalieV2{}
	return {mexe, mpos}
end

function penaltyGoalie2017V1()
	local mexe, mpos = PenaltyGoalie2017V1{}
	return {mexe, mpos}
end

-- 用来盯人的skill,其中p为优先级
function marking(p, f)
	local mexe, mpos = Marking{pri = p, flag = f}
	return {mexe, mpos}
end

function zmarking(p, f, n)
	local mexe, mpos = ZMarking{pri = p, flag = f, num = n}
	return {mexe, mpos}
end

function zblocking(p, defPos)
	local mexe, mpos = ZBlocking{pri = p, pos = defPos}
	return {mexe, mpos}
end

function zdrag(p, targetPos)
	local ipos
	local itarget
	if type(p) == "function" or type(targetPos) == "userdata" then
		ipos = p
	elseif type(p) == "string" then
		ipos = function()
			return player.pos(p)
		end
	else
		ipos = CGeoPoint:new_local(9999*10,9999*10)
	end

	if type(targetPos) == "function" or type(targetPos) == "userdata" then
		itarget = targetPos
	elseif type(targetPos) == "string" then
		itarget = function()
			return player.pos(targetPos)
		end
	else
		itarget = CGeoPoint:new_local(9999*10,9999*10)
	end
	local mexe, mpos = ZDrag{pos = p, target = itarget}
	return {mexe, mpos}
end

function  markingDir(p,p0)
	local mexe, mpos = Marking{pri = p,dir = p0}
	return {mexe, mpos}
end

-- 强行绕前盯人
-- by zhyaic 2014.7.4
function markingFront(p)
	local mexe, mpos = Marking{pri = p, front = true, dir = dir.shoot()}
	return {mexe, mpos}
end

----------------------------------------- 其他动作 --------------------------------------------
function advance(role, f)
	local itandem = nil
	local mflag = f or 0
	if type(role) == "string" then
		itandem = role
	elseif type(role) == "function" then
		itandem = role()
	end
	local mexe, mpos = AdvanceBall{pos = pos.advance, srole = "advancer", tandem = itandem, flag = mflag}
	return {mexe, mpos}
end

function getBallV4(p, waitpos, pow, f)
	local mdir
	local ipos1
	local ipos2
	local ipower
	--朝向点和方向
	if p == nil then--不发点则射对方门
		--mdir = dir.chase
		ipos1 = CGeoPoint:new_local(600*10,0)
		mdir = dir.compensate(ipos1)
	else
		if type(p) == "string" then
			mdir = dir.compensate(p)
			ipos1 = player.pos(p)
		elseif type(p) == "function" then
			mdir = dir.compensate(p())
			ipos1 = p()
		elseif type(p) == "userdata" then
			mdir = dir.compensate(p)
			ipos1 = p
		else
			mdir = dir.compensate(p)
			ipos1 = p
		end
	end
	--等待点
	if waitpos == nil then
		ipos2 = CGeoPoint:new_local(9999*10,9999*10)--发一个不在场的点则认为无效
	elseif type(waitpos) == "string" then
		ipos2 = player.pos(waitpos)
	elseif type(waitpos) == "function" then
		ipos2 = waitpos()
	else
		ipos2 = waitpos
	end
	--射门力度
	if pow == nil and f ~= nil then
		if bit:_and(f, flag.chip) ~= 0 then
			ipower = cp.toTarget(ipos1)
		else
			ipower = kp.toTarget(ipos1)
		end
	elseif type(pow) == "function" then
		ipower = pow()
	else
		ipower = pow
	end
	--]]
	local mexe, mpos = GetBallV4{ pos=ipos1, waitpos = ipos2, dir = mdir, power = ipower, flag = f}
	return {mexe, mpos}
end

-- function zget(target, waitpos, p, f,freekickflag)
-- 	local ipos--touch的点
-- 	local ipower
-- 	local itarget--目标点
--     local iflag = f
--     local isChip

-- 	isChip = bit:_and(iflag,flag.chip)

-- 	if type(waitpos) == "function" then
-- 		ipos = waitpos()
-- 	elseif type(waitpos) == "userdata" then
-- 		ipos = waitpos
-- 	elseif type(waitpos) == "string" then
-- 		ipos = function()
-- 			return player.pos(waitpos)
-- 		end
-- 	else
-- 		ipos = CGeoPoint:new_local(9999,9999)
-- 	end

-- 	if target ~= nil then
-- 		if type(target) == "string" then
-- 			itarget = function()
-- 				return player.pos(target)
-- 			end
-- 		else
-- 			itarget = target
-- 		end
-- 	else
-- 		itarget = CGeoPoint:new_local(param.pitchLength/2.0,0)
-- 	end

-- 	if p ~= nil then
-- 		if type(p) == "function" then
-- 			ipower = p
-- 		else
-- 			ipower = isChip == 0 and kp.specified(p) or cp.specified(p)
-- 		end
-- 	elseif target ~= nil then
-- 		if freekickflag == true then
-- 			ipower = isChip == 0 and kp.toTarget(itarget) or cp.toTargetFreeKick(itarget)
-- 		else
-- 			ipower = isChip == 0 and kp.toTarget(itarget) or cp.toTarget(itarget)
-- 		end
-- 	else
-- 		ipower = isChip == 0 and kp.full() or cp.full()
-- 	end
-- 	local mexe, mpos = GetBallV4{ pos=itarget, waitpos = ipos, power = ipower, flag = iflag}
-- 	return {mexe, mpos}
-- end

function zget(target, waitpos, p, f,freekickflag, precision)
	local ipos--touch的点
	local ipower
	local itarget--目标点
    local iflag
    local ifreekickflag
    local iprecision

	if type(waitpos) == "function" then
		ipos = waitpos
	elseif type(waitpos) == "userdata" then
		ipos = waitpos
	elseif type(waitpos) == "string" then
		ipos = function()
			return player.pos(waitpos)
		end
	else
		ipos = CGeoPoint:new_local(9999*10,9999*10)
	end

	if target ~= nil then
		if type(target) == "string" then
			itarget = function()
				return player.pos(target)
			end
		else
			itarget = target
		end
	else
		itarget = CGeoPoint:new_local(param.pitchLength/2.0,0)
	end

	if precision ~= nil then
		iprecision = precision
	else
		iprecision = -1
	end

	ipower = p

	iflag = f

	ifreekickflag = freekickflag

	local mexe, mpos = GetBallV4{ pos = itarget, waitpos = ipos, power = ipower, flag = iflag, freekickflag = ifreekickflag, precision = iprecision}
	return {mexe, mpos}
end
-- 注意，此处的点是用来做匹配的
-- 最终的StaticGetBall中调用的是拿球，离球4cm
-- anti若为false,则不将点进行反向
-- f为传入的flag
function staticGetBall(p, anti, f)
	-- local mexe, mpos = StaticGetBall{ pos = pos.backBall(p), dir = dir.backBall(p)}
	local ipos
	local idir
	if p == nil then
		ipos = pos.specified(CGeoPoint:new_local(600*10,0))
	elseif type(p) == "string" then
		ipos = player.pos(p)
	elseif type(p) == "userdata" then
		ipos = ball.backPos(p, _, _, anti)
	elseif type(p) =="function" then --add
		ipos = p()
	end

	idir = ball.backDir(p, anti)

	local mexe, mpos = StaticGetBall{ pos = ipos, dir = idir, flag = f}
	return {mexe, mpos}
end

-- p为朝向，如果p传的是pos的话，不需要根据ball.antiY()进行反算
function goBackBall(p, d)
	local mexe, mpos = GoCmuRush{ pos = ball.backPos(p, d, 0), dir = ball.backDir(p), flag = flag.dodge_ball}
	return {mexe, mpos}
end

-- 带避车和避球
function goBackBallV2(p, d)
	local mexe, mpos = GoCmuRush{ pos = ball.backPos(p, d, 0), dir = ball.backDir(p), flag = bit:_or(flag.allow_dss,flag.dodge_ball)}
	return {mexe, mpos}
end

function goSupportPos(role)
	local mexe, mpos = ZSupport()
	return {mexe, mpos}
end

function stop()
	local mexe, mpos = Stop{}
	return {mexe, mpos}
end

-- f为false代表p为函数,并且返回值为userdata,p不能有参数
-- f为nil是为了兼容以前的情况
function slowGetBall(p, f, anti)
	local idir
	if type(p) == "string" then
		idir = player.toPlayerDir(p)
	elseif type(p) == "function" then
		if f == nil or f == true then
			idir = p
		elseif anti == false then
			idir = function (role)
				return (p() - player.pos(role)):dir()
			end
		else
			idir = ball.antiYDir(p)
		end
	else
		if anti == false then
			idir = function (role)
				if type(p) == "userdata" then
					return (p - player.pos(role)):dir()
				end
			end
		else
			idir = ball.antiYDir(p)
		end
	end

	local mexe, mpos = SlowGetBall{ dir = idir}
	return {mexe, mpos}
end

function tandem(role)
	local mrole = "Leader"
	if role==nil then
		mrole ="Leader"
	elseif type(role) == "string" then
		mrole =role
	end
	local mexe, mpos = GoAvoidShootLine{ pos = pos.getTandemPos(mrole), dir = dir.getTandemDir(mrole),sender=mrole}
	return {mexe, mpos}
end

function continue()
	return {["name"] = "continue"}
end

------------------------------------ 测试相关的skill ---------------------------------------

function openSpeed(vx, vy, vdir)
	local spdX = function()
		return vx
	end

	local spdY = function()
		return vy
	end

	local spdW = function()
		return vdir
	end

	local mexe, mpos = OpenSpeed{speedX = spdX, speedY = spdY, speedW = spdW}
	return {mexe, mpos}
end

function speed(vx, vy, vdir, vtarget_point)
	-- local spdX = function()
	-- 	return vx
	-- end

	-- local spdY = function()
	-- 	return vy
	-- end

	-- local spdW = function()
		-- return vdir
	-- end
	local spdX = vx
	local spdY = vy
	local spdW = vdir
	local itarget_point = vtarget_point

	if itarget_point ~= nil then
        ipower = kp.toTarget(itarget_point)
    else
        ipower = 3000
    end

	local mexe, mpos = Speed{speedX = spdX, speedY = spdY, speedW = spdW, kick_power = ipower}
	return {mexe, mpos}
end

------------------------------------ RL skill ---------------------------------------
function goChinaTecRush(p, d)
	local mexe, mpos = ChinaTecRun{TargetPos = p, StartPos = d}
	return {mexe, mpos}
end

function  zgetPass(t, w, p, f)
	local infraredCnt = 0
	local zpassBuf = 0
	return function (runner)
		if runner == nil then
			matchPos = function()
			  	local x, y = CZGetBallPos()
			  	return CGeoPoint:new_local(x,y)
			end
			return  {_,matchPos}
		else
			local target = t
			if target == nil then
				target = CGeoPoint:new_local(param.pitchLength/2.0,0)
			else
				if type(t) == "function" then
					target = t()
				else
					target = t
				end
			end

			if player.infraredOn(runner) then
				infraredCnt = infraredCnt < 100 and infraredCnt + 1 or 100
			else
				infraredCnt = 0
			end

			if (Utils.InTheirPenaltyArea(target, 10*10) and player.toPointDist(runner, CGeoPoint:new_local(param.pitchLength/2.0,0)) < 250*10) or zpassBuf > 0 then
				-- print("zgetPass", world:canShootOnBallPos(vision:getCycle(),runner), (infraredCnt <= 3))
				-- if world:canShootOnBallPos(vision:getCycle(),runner) and infraredCnt <= 3 then
				-- 	return shoot()
				-- else
					if zpassBuf == 0 then
						zpassBuf = 120*10
					else
						zpassBuf = zpassBuf - 1*10
					end
					return zpass()
				-- end
			else
				return zget(t,w,p,f)
			end
		end
	end
end

function dirrble4ballplace(p)
	local dribblepos
	local DRIBBLE_BALL_POS = function()
 	 return ball.pos() + Utils.Polar2Vector(-5,dir.playerToBall("Special"))
	end
	local TargetPos = function ()
  		return CGeoPoint:new_local(ball.placementPos():x(), ball.placementPos():y())
	end
	return function ()
		if type(p) == "function" then
		    dribblepos = p()
		  else
		    dribblepos = p
		end
		if not player.infraredOn("Special") then
			return goCmuRush(DRIBBLE_BALL_POS,player.toBallDir,50,flag.our_ball_placement+flag.dribbling+flag.allow_dss)
		--无红外，吸球
		elseif player.posX("Special")>-575*10 and player.posX("Special")<575*10 and player.posY("Special")>-425*10 and player.posY("Special")<425*10 then
		--球入场
			if not player.faceball2target("Special",TargetPos()) then
				return task.goCmuRush(player.pos("Special"),player.toPointDir(TargetPos),50,flag.our_ball_placement+flag.dribbling)
			--球入场后，吸球转身朝向target
			else
				return task.stop()
			end
		else
		--球还未入场，吸球后退
			return task.goCmuRush(dribblepos,player.toBallDir,50,flag.our_ball_placement+flag.dribbling)
		end
	end
end