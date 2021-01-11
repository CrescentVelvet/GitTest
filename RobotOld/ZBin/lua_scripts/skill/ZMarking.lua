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
		print("Error Priority in ZMarking Skill!!!!!")
	end

	return num
end

function ZMarking(task)
	local mpri
	local mflag = task.flag or 0
	local num
	execute = function(runner)
		if task.pri == nil then
			print("No Priority in ZMarking Skill!!!!!")
		elseif type(task.pri) == "string" then
			mpri = PriToNum(task.pri)
		end

		if task.num == nil then
			num = -1
		elseif type(task.num) == "function" then
			num = task.num(runner)
		else
			num = task.num
		end

		return CZMarking(runner, mpri, mflag, num)
	end

	matchPos = function(runner)
		local x, y = CGetZMarkingPos(runner, mpri, mflag, num)
		return CGeoPoint:new_local(x,y)
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "ZMarking",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}