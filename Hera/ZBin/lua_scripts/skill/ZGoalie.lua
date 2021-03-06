function ZGoalie(task)
        local mflag = task.flag or 0
        local mpower = task.power or 0
        local mpos = task.pos or CGeoPoint:new_local(param.pitchLength/2-param.penaltyDepth -1,0)
        execute = function(runner)
                if type(task.pos) == "function" then
                        mpos = task.pos(runner)
                end
                if type(task.power) == "function" then
                        mpower = task.power(runner)
                end
                return CZGoalie(runner, mpos:x(), mpos:y(), mpower, mflag)
        end

        matchPos = function()
                return pos.goaliePos()
        end

        return execute, matchPos
end

gSkillTable.CreateSkill{
        name = "ZGoalie",
        execute = function (self)
                print("This is in skill"..self.name)
        end
}
