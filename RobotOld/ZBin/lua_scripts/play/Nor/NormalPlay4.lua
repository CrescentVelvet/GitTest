local STATIC_POS = {
        CGeoPoint:new_local(-170,-150),
        CGeoPoint:new_local(-170,150)
}
gPlayTable.CreatePlay{

firstState = "attacker1",

switch = function()
        local enemyNum = enemy.attackNum()
        if enemyNum > 3 then
                enemyNum = 3
        elseif enemyNum < 1 then
                enemyNum = 1
        end
        return "attacker"..enemyNum
end,

["attacker1"] = {
        Leader   = IS_YELLOW and task.zgetPass() or task.zget(_,_,_,flag.kick + flag.safe),
        Special  = task.rightBack(),
        Middle   = task.leftBack(),
        Goalie   = task.zgoalie(),
        match    = "[L][SM]"
},
["attacker2"] = {
        Leader   = IS_YELLOW and task.zgetPass() or task.zget(_,_,_,flag.kick + flag.safe),
        Special  = task.zmarking("First"),
        Middle   = task.singleBack(),
        Goalie   = task.zgoalie(),
        match    = "[L][SM]"
},
["attacker3"] = {
        Leader   = IS_YELLOW and task.zgetPass() or task.zget(_,_,_,flag.kick + flag.safe),
        Special  = task.zmarking("First"),
        Middle   = task.zmarking("Second"),
        Goalie   = task.zgoalie(),
        match    = "[L][SM]"
},
name = "NormalPlay4",
applicable ={
        exp = "a",
        a = true
},
attribute = "attack",
timeout = 99999
}