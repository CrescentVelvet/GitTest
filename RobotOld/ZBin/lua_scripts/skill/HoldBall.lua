function HoldBall(task)
  local mpos
  local mrole   = task.srole or ""

  execute = function(runner)
    if runner >=0 and runner < param.maxPlayer then
      if mrole ~= "" then
        CRegisterRole(runner, mrole)
      end
    else
      print("Error runner in HoldBall", runner)
    end
    if type(task.target) == "function" then
      mpos = task.target(runner)
    else
      mpos = task.target
    end
    return CHoldBall(runner, mpos:x(), mpos:y())
  end

  matchPos = function()
    return ball.pos()
  end

  return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "HoldBall",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}