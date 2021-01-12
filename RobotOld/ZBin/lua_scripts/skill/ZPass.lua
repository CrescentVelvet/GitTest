function ZPass(task)
  local mpos
  local mflag   = task.flag or 0
  local mrole   = task.srole or ""
  local mpower = task.power
  local mprecision = task.precision

  matchPos = function()
    local x, y = CZGetBallPos()
    return CGeoPoint:new_local(x,y)
  end
  
  execute = function(runner)
    if not gRoleTask[runner] == nil then
      print("Error : Task Conflict -> ",runner,gRoleTask[runner],"ZPass");
    end
    gRoleTask[runner] = "ZPass"
    if runner >=0 and runner < param.maxPlayer then
      if mrole ~= "" then
        CRegisterRole(runner, mrole)
      end
    else
      print("Error runner in ZPass", runner)
    end
    if type(task.target) == "function" then
      mpos = task.target(runner)
    else
      mpos = task.target
    end
    if type(task.power) == "function" then
      mpower = task.power(runner)
    end
    if type(task.precision) == "function" then
      mprecision = task.precision(runner)
    end
    -- print("precision: ", mprecision)
    if type(task.flag) == "function" then
      mflag = task.flag()
    end
    return CZPass(runner, mpos:x(), mpos:y(), mpower, mflag, mprecision)
  end

  return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "ZPass",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}