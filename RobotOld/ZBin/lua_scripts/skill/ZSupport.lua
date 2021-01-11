function ZSupport(task)
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
      print("Error : Task Conflict -> ",runner,gRoleTask[runner],"ZSupport");
    end
    gRoleTask[runner] = "ZSupport"
    return CZSupport(runner)
  end

  return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "ZSupport",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}