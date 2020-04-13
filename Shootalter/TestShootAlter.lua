local DSS = bit:_or(flag.allow_dss,flag.dodge_ball)
local DIST_PENALTY = 91
local LENGTH_X = 6000
local LENGTH_Y = 4500
local FIELD_X1 = 3500
local FIELD_X2 = -3500
local FIELD_Y1 = 2500
local FIELD_Y2 = -2500
local CENTER_X = 500
local CENTER_Y = 500
local PENTY_X = 4700
local PENTY_Y = 1250
local GATE_X = 6200
local GATE_Y = 600
local rotvel_old = 2
local timerun = 10
local timeget = 10000
local shoot_flag = 0
local runres_flag = -1
local rotrand_flag = 0
local target_point = CGeoPoint(0,0)

local function writeFile(fileName,content)
    local file = assert(io.open(fileName,'a'))
    file:write(content)
    file:close()
end

local function rot_rand1()
    local Rotrand1 = function ()
        -- local rotrand1 = pos.getOtherPos(1)():x()%200/100+3
        math.randomseed(tostring(os.time()):reverse():sub(1, 6))
        local rotrand1 = math.random()*(5.0-1.0)+1.0
        return rotrand1
    end
    return Rotrand1
end

local function rot_rand2()
    local Rotrand2 = function ()
        local rotrand2 = rotvel_old
        return rotrand2
    end
    return Rotrand2
end

local function velx_rand()
    local Velxrand = function ()
        math.randomseed(tostring(os.time()):reverse():sub(1, 6)+1)
        local velxrand = math.random()*(800+800)-800
        return velxrand
    end
    return Velxrand
end

local function vely_rand()
    local Velyrand = function ()
        math.randomseed(tostring(os.time()):reverse():sub(1, 6)+2)
        local velyrand = math.random()*(800+800)-800
        return velyrand
    end
    return Velyrand
end

gPlayTable.CreatePlay{

firstState = "run0",
["run0"] = {--随机跑位
    switch = function()
    --球到达目标，保存数据
        if ball.toPointDist(target_point) < 1000 and shoot_flag == 1 then
            writeFile('../shoot_data.txt','ball_pos\n')
            writeFile('../shoot_data.txt','ball_posX'..'\t'..ball.posX()..'\n')
            writeFile('../shoot_data.txt','ball_posY'..'\t'..ball.posY()..'\n')
            writeFile('../shoot_data.txt','000000000'..'\t'..'000000000'..'\n')
            print('ball_posX'..'\t'..ball.posX()..'\n')
            print('ball_posY'..'\t'..ball.posY()..'\n')
            print('000000000'..'\t'..'000000000'..'\n')
            shoot_flag = 0
            rotrand_flag = 0
        end
        if bufcnt(player.toTargetDist("Kicker")<200,timerun) then
            return "get"..0
        end
    end,
    --跑位随机点（+-3500，+-2500）减去（+-1000，+-1000）
    Kicker = task.goCmuRush(pos.getOtherPos(1),0,_,DSS),
    match = ""
},
["run1"] = {--绕过球门后
    switch = function ()
        if ball.posY() > 0 then
            runres_flag = 1
        else
            runres_flag = -1
        end
        if bufcnt(player.toTargetDist("Kicker")<200,timerun) then
            return "run"..2
        end
    end,
    Kicker = task.goCmuRush(CGeoPoint(4000,2000*runres_flag),0,_,DSS),
    match = ""
},
["run2"] = {
    switch = function ()
        if bufcnt(player.toTargetDist("Kicker")<200,timerun) then
            return "res"..0
        end
    end,
    Kicker = task.goCmuRush(CGeoPoint(6400,4000*runres_flag),0,_,DSS),
    match = ""
},
["get0"] = {--拿球
    switch = function()
        -- 出界判定
        if ball.posX() > LENGTH_X or ball.posX() < -LENGTH_X then
            return "res"..0
        elseif ball.posY() > LENGTH_Y or ball.posY() < -LENGTH_Y then
            return "res"..0
        -- 禁区判定
        elseif ball.posX() > PENTY_X and ball.posY() > -PENTY_Y and ball.posY() < PENTY_Y then
            return "res"..0
        end
        if bufcnt(player.infraredOn("Kicker"),3,timeget) then
            return "get"..1
        end
    end,
    -- Kicker = task.zget(target_point,_,_,flag.kick+flag.dribble),
    Kicker = task.fetchBall(pos.getOtherPos(1),_,true),
    match = ""
},
["get1"] = {--有初速射球
    switch = function()
        -- print(player.rawrotVel("Kicker"))
        -- print(rot_rand()()*(3.0-0.5)+0.5)
        -- 门后判定
        if ball.posX() > GATE_X
            and ball.posY() > -GATE_Y and ball.posY() < GATE_Y then
            return "run"..1
        -- 白圈判定
        elseif ball.posX() > -CENTER_X and ball.posX() < CENTER_X
            and ball.posY() > -CENTER_Y and ball.posY() < CENTER_Y then
            return "res"..0
        -- 丢球判定
        elseif math.sqrt((ball.posX()-player.posX("Kicker"))*(ball.posX()-player.posX("Kicker"))
            +(ball.posY()-player.posY("Kicker"))*(ball.posY()-player.posY("Kicker"))) > 1000 and player.infraredOn("Kicker") == false then
            print(1)
            return "get"..0
        -- 球出手，保存数据
        elseif bufcnt(player.kickBall("Kicker"),1,timeget) then
            writeFile('../shoot_data.txt','car_pos'..'\n')
            writeFile('../shoot_data.txt','car_dir   '..'\t'..player.dir("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_posX  '..'\t'..player.posX("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_posY  '..'\t'..player.posY("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_velMod'..'\t'..player.velMod("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_velDir'..'\t'..player.velDir("Kicker")..'\n')
            -- writeFile('../shoot_data.txt','car_rotVel'..'\t'..player.rotVel("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_rotVel'..'\t'..player.rawrotVel("Kicker")..'\n')
            print('car_vel  '..'\t'..player.velMod("Kicker")..'\n')
            print('car_rot  '..'\t'..player.rawrotVel("Kicker")..'\n')
            shoot_flag = 1
            -- return "res"..0
            return "run"..0
        -- 出界判定
        elseif ball.posX() > LENGTH_X or ball.posX() < -LENGTH_X then
            return "res"..0
        elseif ball.posY() > LENGTH_Y or ball.posY() < -LENGTH_Y then
            return "res"..0
        -- 禁区判定
        elseif ball.posX() > PENTY_X and ball.posY() > -PENTY_Y and ball.posY() < PENTY_Y then
            return "res"..0
        end
        if rotrand_flag == 0 then
            rotvel_old = rot_rand1()()
            rotrand_flag = 1
        end
    end,
    -- Kicker = task.zget(target_point,_,_,flag.kick),
    -- 转速在-3和3之间随机
    -- Kicker = task.speed(0,0,3,target_point),
    Kicker = task.speed(0,0,rot_rand2(),target_point),
    -- Kicker = task.speed(-500,500,1,target_point),
    -- Kicker = task.speed(velx_rand(),vely_rand(),rot_rand(),target_point),
    match = ""
},
["res0"] = {--界外捡球
    switch = function()
        if player.posY("Kicker") > -GATE_Y and player.posY("Kicker") < GATE_Y
            and player.posX("Kicker") < GATE_X and ball.posX() > GATE_X
            and ball.posY() > -PENTY_Y and ball.posY() < PENTY_Y then
            return "run"..1
        -- elseif ball.posX() > FIELD_X2 and ball.posX() < FIELD_X1
        --     and ball.posY() > FIELD_Y2 and ball.posY() < FIELD_Y1 then
            -- if not (ball.posX() > -CENTER_X and ball.posX() < CENTER_X
            --     and ball.posY() > -CENTER_Y and ball.posY() < CENTER_Y) then
            --     -- return "run"..0
            --     return "get"..0
            -- end
        -- end
        elseif bufcnt(ball.toPointDist(pos.getOtherPos(1))<200,timerun) then
            return "get"..0
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