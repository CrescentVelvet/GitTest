function ZDrag(task)
  local mpos
  local mtarget

  matchPos = function()
    if type(task.pos) == "function" then 
      mpos = task.pos()
    else
      mpos = task.pos
    end
    return mpos
  end
  
  execute = function(runner)
    if type(task.pos) == "function" then 
      mpos = task.pos()
    else
      mpos = task.pos
    end
    if type(task.target) == "function" then 
      mtarget = task.target()
    else
      mtarget = task.target
    end
    return CZDrag(runner, mpos:x(), mpos:y(), mtarget:x(), mtarget:y())
  end

  return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "ZDrag",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}