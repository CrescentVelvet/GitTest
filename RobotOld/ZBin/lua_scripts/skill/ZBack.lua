function ZBack(task)
    local mflag = task.flag or 0
    local mguardNum = task.guardNum or 0
    local mindex = task.index or 0
    local mpower = task.power
    execute = function(runner)
        return CZBack(runner, mguardNum, mindex, mpower, mflag)
    end

    matchPos = function()
        return guardpos:backPos(mguardNum, mindex)
    end

    return execute, matchPos
end

gSkillTable.CreateSkill{
    name = "ZBack",
    execute = function (self)
        print("This is in skill"..self.name)
    end
}
