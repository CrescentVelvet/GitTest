function StaticGetBall(task)
	local mpos
	local mdir
	local mflag = task.flag or flag.allow_dss

	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end

		return CStaticGetBall(runner, mdir, mflag)
	end

	matchPos = function()
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		ballpos = CGeoPoint:new_local(ball.posX(),ball.posY())
		mpos = ballpos + Utils.Polar2Vector(-20,mdir)
		return mpos
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "StaticGetBall",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
