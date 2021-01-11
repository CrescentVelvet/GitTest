function GoAndTurnKickV4(task)
  local mpos = task.pos
  local mdir
  local mpower = task.power
  local mflag = task.flag or 0
  local isChip

  isChip = bit:_and(mflag, flag.chip)

  execute = function(runner)
    if type(task.dir) == "function" then
      mdir = task.dir()
    else
      mdir = task.dir
    end

    if type(task.power) == "function" then
      mpower = task.power(runner)
    end

    if Utils.InTheirPenaltyArea(mpos, 5) then
      mpower = isChip == 0 and kp.full() or cp.full()
    end

    return CGoAndTurnKickV4(runner, mpos:x(), mpos:y(), mflag, mpower, mdir)
  end

  matchPos = function()
    local x, y = CZGetBallPos()
    return CGeoPoint:new_local(x,y)
  end

  return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "GoAndTurnKickV4",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}