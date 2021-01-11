function ZAttack(task)
    local mpos
    local mwaitpos
    local mflag   = task.flag or 0
    local mrole   = task.srole or ""
    local mpower = task.power
    local mprecision = task.precision
    local isChip

    matchPos = function(runner)
        local x, y
        if runner == nil then
            x, y = CZGetBallPos()
        else
            x, y = CZGetBallPos(runner)
        end
        return CGeoPoint:new_local(x,y)
    end
  
    execute = function(runner)
        if not gRoleTask[runner] == nil then
            print("Error : Task Conflict -> ",runner,gRoleTask[runner],"ZAttack");
        end
        gRoleTask[runner] = "ZAttack"
        if runner>0 and runner <= param.maxPlayer then
            if mrole ~= "" then
                CRegisterRole(runner, mrole)
            end
        else
            print("Error runner in ZAttack", runner)
        end

        if type(task.target) == "function" then
            mpos = task.target(runner)
        else
            mpos = task.target
        end

        if type(task.waitpos) == "function" then
            mwaitpos = task.waitpos(runner)
        else
            mwaitpos = task.waitpos
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

        if task.power ~= nil then
            if type(task.power) == "function" then
                mpower = task.power(runner)
            else
                mpower = task.power
            end
        elseif mpos ~= nil then
            mpower = isChip == 0 and kp.toTarget(mpos) or cp.toTarget(mpos)
        else
            mpower = isChip == 0 and kp.full() or cp.full()
        end
        if type(mpower) == "function" then
            mpower = mpower(runner)
        end
        -- print(mpos:x(), mpos:y(), mwaitpos:x(), mwaitpos:y(), mpower) 
        return CZAttack(runner, mpos:x(), mpos:y(), mwaitpos:x(), mwaitpos:y(), mpower, mflag, mprecision)
    end

    return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "ZAttack",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}