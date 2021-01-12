local DSS = bit:_or(flag.allow_dss,flag.dodge_ball)
local DIST_PENALTY = 91
local LENGTH_X = 6000
local LENGTH_Y = 4475
local FIELD_X1 = 4000
local FIELD_X2 = 2000
local FIELD_Y1 = 3000
local FIELD_Y2 = -3000
local PENTY_X = 4700
local PENTY_Y = 1250
local GATE_X = 6200
local GATE_Y = 600
local timerun = 10
local timeget = 1000
local shoot_flag = 0
local runres_flag = -1

local function writeFile(fileName,content)
    local f = assert(io.open(fileName,'a'))
    f:write(content)
    f:close()
end

gPlayTable.CreatePlay{

firstState = "run0",
["run0"] = {
    switch = function()
        if bufcnt(player.toTargetDist("Kicker")<600,timerun) then
            return "get"..0
        end
    end,
    Kicker = task.goCmuRush(pos.getOtherPos(1),0,_,DSS),
    match = ""
},
["run1"] = {
    switch = function ()
        if ball.posY() > 0 then
            runres_flag = 1
        else
            runres_flag = -1
        end
        if bufcnt(player.toTargetDist("Kicker")<600,timerun) then
            return "run"..2
        end
    end,
    Kicker = task.goCmuRush(CGeoPoint(4000,2000*runres_flag),0,_,DSS),
    match = ""
},
["run2"] = {
    switch = function ()
        if bufcnt(player.toTargetDist("Kicker")<400,timerun) then
            return "res"..0
        end
    end,
    Kicker = task.goCmuRush(CGeoPoint(6400,4000*runres_flag),0,_,DSS),
    match = ""
},
["get0"] = {
    switch = function()
        if ball.posX() > GATE_X
            and ball.posY() > -GATE_Y and ball.posY() < GATE_Y then
            return "run"..1
        elseif bufcnt(player.kickBall("Kicker"),1,timeget) then
            writeFile('../shoot_data.txt','car_pos\n')
            writeFile('../shoot_data.txt','car_posX  '..'\t'..player.posX("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_posY  '..'\t'..player.posY("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_velMod'..'\t'..player.velMod("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_velDir'..'\t'..player.velDir("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_rotVel'..'\t'..player.rotVel("Kicker")..'\n')
            shoot_flag = 1
            return "res"..0
        elseif ball.posX() > LENGTH_X or ball.posX() < -LENGTH_X then
            return "res"..0
        elseif ball.posY() > LENGTH_Y or ball.posY() < -LENGTH_Y then
            return "res"..0
        elseif ball.posX() > PENTY_X and ball.posY() > -PENTY_Y and ball.posY() < PENTY_Y then
            return "res"..0
        end
    end,
    Kicker = task.zget(CGeoPoint(6000,0),_,_,flag.kick),
    match = ""
},
["res0"] = {
    switch = function()
        if ball.posX() > LENGTH_X-100 and ball.posX() < LENGTH_X+100
            and shoot_flag == 1 then
            writeFile('../shoot_data.txt','ball_pos\n')
            writeFile('../shoot_data.txt','ball_posX'..'\t'..ball.posX()..'\n')
            writeFile('../shoot_data.txt','ball_posY'..'\t'..ball.posY()..'\n')
            writeFile('../shoot_data.txt','000000000'..'\t'..'000000000'..'\n')
            shoot_flag = 0
        end
        if player.posY("Kicker") > -GATE_Y and player.posY("Kicker") < GATE_Y
            and player.posX("Kicker") < GATE_X and ball.posX() > GATE_X
            and ball.posY() > -PENTY_Y and ball.posY() < PENTY_Y then
            return "run"..1
        elseif ball.posX() > FIELD_X2 and ball.posX() < FIELD_X1
            and ball.posY() > FIELD_Y2 and ball.posY() < FIELD_Y1 then
            return "run"..0
        end
    end,
    Kicker = task.fetchBall(pos.getOtherPos(1),_,true),
    match = ""
},

name = "TestShootAlter",
applicable ={
    exp = "a",
    a = true
},
attribute = "attack",
timeout = 99999
}