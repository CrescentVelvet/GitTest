local function PriToNum(str)
	local num
	if str == "Zero" then
		num = 0
	elseif str == "First" then
		num = 1
	elseif str == "Second" then
		num = 2
	elseif str == "Third" then
		num = 3
	elseif str == "Fourth" then
		num = 4
	elseif str == "Fifth" then
		num = 5
	elseif str == "Sixth" then
		num = 6
	elseif str == "Seventh" then
		num = 7
	else
		print("Error Priority in ZBlocking Skill!!!!!")
	end

	return num
end

function ZBlocking(task)
	local mflag = task.flag or 0
	local mfront = task.front or false
	local defPos = task.pos
	local mpri

	execute = function(runner)
		if task.pri == nil then
			print("No Priority in ZBlocking Skill!!!!!")
		elseif type(task.pri) == "string" then
			mpri = PriToNum(task.pri)
		end

		local idefPos = nil
		if type(defPos) == "function" then
			idefPos = defPos()
		else
			idefPos = defPos
		end

		return CZBlocking(runner, mpri, idefPos:x(), idefPos:y())
	end

	matchPos = function()
		local idefPos = nil
		if type(defPos) == "function" then
			idefPos = defPos()
		else
			idefPos = defPos
		end
		local x, y = CGetZBlockingPos(mpri, idefPos:x(), idefPos:y())--改成要走的点
		return CGeoPoint:new_local(x,y)
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "ZBlocking", 
	execute = function (self)
		print("This is in skill"..self.name)
	end
}