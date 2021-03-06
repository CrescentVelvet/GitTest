function FetchBall(task)
  local mpos
  local mflag  = task.flag
  local mpower = task.power

  matchPos = function()
    local x, y = ball.posX(),ball.posY()
    return CGeoPoint:new_local(x,y)
  end
  
  execute = function(runner)
    if not gRoleTask[runner] == nil then
      print("Error : Task Conflict -> ",runner,gRoleTask[runner],"FetchBall");
    end

    if type(task.target) == "function" then
      mpos = task.target(runner)
    else
      mpos = task.target
    end

    if type(task.power) == "function" then
      mpower = task.power(runner)
    end
    
    return CFetchBall(runner, mpos:x(), mpos:y(), mpower, mflag)
  end

  return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "FetchBall",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}