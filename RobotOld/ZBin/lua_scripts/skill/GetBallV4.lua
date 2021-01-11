-- function GetBallV4(task)
-- 	local mflag = task.flag
-- 	local mpos = task.pos
-- 	local waitpos = task.waitpos
-- 	local mpower = task.power

-- 	execute = function(runner)
-- 		if type(task.pos) == "function" then
-- 			mpos = task.pos(runner)
-- 		end
-- 		if type(task.waitpos) == "function" then
-- 			waitpos = task.waitpos(runner)
-- 		end
-- 		if type(task.power) == "function" then
-- 			mpower = task.power(runner)
-- 		end
-- 		if type(task.flag) == "function" then
-- 			mflag = task.flag()
-- 		else
-- 			mflag = task.flag or 0
-- 		end
-- 		return CGetBallV4(runner, mpos:x(), mpos:y(), waitpos:x(), waitpos:y(), mpower, mflag)
-- 	end

-- 	matchPos = function()
-- 		local x, y = CZGetBallPos()
--                 return CGeoPoint:new_local(x,y)
-- 	end

-- 	return execute, matchPos
-- end

gSkillTable.CreateSkill{
	name = "GetBallV4",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

function GetBallV4(task)
	local mflag = task.flag or 0
	local mpos = task.pos
	local waitpos = task.waitpos
	local mpower = task.power
	local mprecision = task.precision
	local isChip

	execute = function(runner)
		if not gRoleTask[runner] == nil then
			print("Error : Task Conflict -> ",runner,gRoleTask[runner],"GetBallV4");
		end
		gRoleTask[runner] = "GetBallV4"

		if type(task.pos) == "function" then
			mpos = task.pos(runner)
		end
		if type(task.waitpos) == "function" then
			waitpos = task.waitpos(runner)
		end
		if type(task.precision) == "function" then
			mprecision = task.precision(runner)
		end
		-- print("precision: ", mprecision)
		if type(task.flag) == "function" then
			mflag = task.flag()
		else
			mflag = task.flag or 0
		end
		isChip = bit:_and(mflag,flag.chip)

		-- print("task power: ", type(task.power))
		if task.power ~= nil then
			if type(task.power) == "function" then
				mpower = task.power(runner)
				-- print("mpower", mpower)
			end
		elseif mpos ~= nil then
			if task.freekickflag == true then
				mpower = isChip == 0 and kp.toTarget(mpos) or cp.toTargetFreeKick(mpos)
			else
				mpower = isChip == 0 and kp.toTarget(mpos) or cp.toTarget(mpos)
			end
		else
			mpower = isChip == 0 and kp.full() or cp.full()
		end
		if type(mpower) == "function" then
			mpower = mpower(runner)
		end
		-- print("final mpower", mpower)

		return CGetBallV4(runner, mpos:x(), mpos:y(), waitpos:x(), waitpos:y(), mpower, mflag, mprecision)
	end

	matchPos = function(runner)
		local x, y
		if runner == nil then
			x, y = CZGetBallPos()
		else
			x, y = CZGetBallPos(runner)
		end
        return CGeoPoint:new_local(x,y)
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "GetBallV4",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}