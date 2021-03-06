function ZBack(task)
    local mflag = task.flag or 0
    local mguardNum = task.guardNum or 0
    local mindex = task.index or 0
    local mpower = task.power
    local mdefendNum = 0

    execute = function(runner)
        -- print(type(mdefendNum))
        if task.defendNum == nil then
            mdefendNum = -1
        elseif type(task.defendNum) == "function" then
            mdefendNum = task.defendNum()
            -- mdefendNum = task.defendNum()
        else
            mdefendNum = task.defendNum
        end
        -- print("defendNumfunction: ",mdefendNum)
        -- print("defendNum: ",mdefendNum)
        return CZBack(runner, mguardNum, mindex, mpower, mflag, mdefendNum)
    end

    matchPos = function(runner)
        return guardpos:backPos(mguardNum, mindex, runner, mdefendNum)
    end
    return execute, matchPos
end

gSkillTable.CreateSkill{
    name = "ZBack",
    execute = function (self)
        print("This is in skill"..self.name)
    end
}
