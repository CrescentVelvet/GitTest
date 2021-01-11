function ChinaTecRun(task)
	local Tpos
	local Spos
	local mflag  
	local msender 
	local mrole   = task.srole or "" 

	matchPos = function()
		if type(task.TargetPos) == "function" then
			Tpos = task.TargetPos()
		else
			Tpos = task.TargetPos
		end
		return Tpos
	end

	execute = function(runner)
		if runner>0 and runner <= param.maxPlayer then
			if mrole ~= "" then
				CRegisterRole(runner, mrole)
			end
		else
			print("Error runner in ChinaTecRun", runner)
		end
		if type(task.TargetPos) == "function" then
			Tpos = task.TargetPos()
		else
			Tpos = task.TargetPos
		end
		
		if type(task.StartPos) == "function" then
			Spos = task.StartPos()
		else
			Spos = task.StartPos
		end

		return CChinaTec(runner, Tpos:x(), Tpos:y(), Spos:x(), Spos:y())
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "ChinaTecRun",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}