local STATIC_POS = {
        CGeoPoint:new_local(-2700,-3500),
        CGeoPoint:new_local(-2700,3500)
}
gPlayTable.CreatePlay{

firstState = "attacker1",

switch = function()
        local enemyNum = enemy.attackNum()
        if enemyNum > 7 then
                enemyNum = 7
        elseif enemyNum < 1 then
                enemyNum = 1
        end
        return "attacker"..enemyNum
end,

["attacker1"] = {
        Leader   = task.zattack(_,_,_,flag.kick),--task.zget(_,_,_,flag.kick),
        Special  = task.zsupport(),
        Fronter  = task.defendMiddle(),
        Center   = task.leftBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "[L][DSFC]"
},
["attacker2"] = {
        Leader   = task.zattack(_,_,_,flag.kick),--task.zget(_,_,_,flag.kick),
        Special  = task.zmarking("First"),
        Fronter  = task.zsupport(),
        Center   = task.leftBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "[L][DC][SF]"
},
["attacker3"] = {
        Leader   = task.zattack(_,_,_,flag.kick),--task.zget(_,_,_,flag.kick),
        Special  = task.zsupport(),
        Middle   = task.zmarking("First"),
        Assister = task.zmarking("Second"),
        Fronter = task.singleBack(),
        Goalie   = task.zgoalie(),
        match    = "[L][MASF]"
},
["attacker4"] = {
        Leader   = task.zattack(_,_,_,flag.kick),--task.zget(_,_,_,flag.kick),
        Special  = task.zmarking("First"),
        Middle   = task.zmarking("Second"),
        Assister = task.zmarking("Third"),
        Fronter = task.singleBack(),
        Goalie   = task.zgoalie(),
        match    = "[L][SMAF]"
},
["attacker5"] = {
        Leader   = task.zattack(_,_,_,flag.kick),--task.zget(_,_,_,flag.kick),
        Special  = task.zmarking("Third"),
        Middle   = task.zmarking("First"),
        Assister = task.zmarking("Second"),
        Fronter  = task.zmarking("Fourth"),
        Goalie   = task.zgoalie(),
        match    = "[L][SMAF]"
},
["attacker6"] = {
        Leader   = task.zattack(_,_,_,flag.kick),--task.zget(_,_,_,flag.kick),
        Special  = task.zmarking("Third"),
        Middle   = task.zmarking("First"),
        Assister = task.zmarking("Second"),
        Fronter  = task.zmarking("Fourth"),
        Center   = task.zmarking("Fifth"),
        Defender = task.singleBack(),
        Goalie   = task.zgoalie(),
        match    = "[L][D][CSMAF]"
},
["attacker7"] = {
        Leader   = task.zattack(_,_,_,flag.kick),--task.zget(_,_,_,flag.kick),
        Special  = task.zmarking("Third"),
        Middle   = task.zmarking("First"),
        Assister = task.zmarking("Second"),
        Fronter  = task.zmarking("Fourth"),
        Center   = task.zmarking("Fifth"),
        Defender = task.zmarking("Sixth"),
        Goalie   = task.zgoalie(),
        match    = "[L][DCSMAF]"
},
name = "NormalPlayZ6",
applicable ={
        exp = "a",
        a = true
},
attribute = "attack",
timeout = 99999
}