function ZCirclePass(task)
  local mpos
  local mflag   = task.flag or 0
  local mrole   = task.srole or ""
  local mpower = task.power

  matchPos = function()
    return ball.pos()
  end
  
  execute = function(runner)
    if runner >=0 and runner < param.maxPlayer then
      if mrole ~= "" then
        CRegisterRole(runner, mrole)
      end
    else
      print("Error runner in ZCirclePass", runner)
    end
    if type(task.target) == "function" then
      mpos = task.target(runner)
    else
      mpos = task.target
    end
    if type(task.power) == "function" then
      mpower = task.power(runner)
    end
    if type(task.flag) == "function" then
      mflag = task.flag()
    end
    return CZCirclePass(runner, mpos:x(), mpos:y(), mpower, mflag)
  end

  return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "ZPass",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}