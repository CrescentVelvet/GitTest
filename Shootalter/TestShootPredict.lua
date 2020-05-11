local DSS = bit:_or(flag.allow_dss,flag.dodge_ball)
local LENGTH_X = 3000
local LENGTH_Y = 2500
local PENTY_X = 4700
local PENTY_Y = 1250
local kick_flag = 0
local power_flag = 0
local rotvel_flag = 0
local power_old = 2500
local rotvel_old = 2
local res_pos = CGeoPoint(2000,2000)
local get_pos = CGeoPoint(2000,2000)

local function writeFile(fileName,content)
    local file = assert(io.open(fileName,'a'))
    file:write(content)
    file:close()
end

-- 符号函数，让转速随机不必考虑正负
local function sign_rand()
    local Signrand = function ()
        math.randomseed(tostring(os.time()):reverse():sub(1, 6))
        local signrand = math.sign(math.random()-0.5)
        return signrand
    end
    return Signrand
end

-- 力度在2000到3000之间随机
local function pow_rand1()
    local Powrand1 = function ()
        math.randomseed(tostring(os.time()):reverse():sub(1, 6))
        local powrand1 = math.random()*(3000-2000)+2000
        return powrand1
    end
    return Powrand1
end

local function pow_rand2()
    local Powrand2 = function ()
        local powrand2 = power_old
        return powrand2
    end
    return Powrand2
end

-- 转速在2到5之间随机
local function rot_rand1()
    local Rotrand1 = function ()
        math.randomseed(tostring(os.time()):reverse():sub(1, 6))
        local rotrand1 = math.random()*(5.0-2.0)+2.0
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

-- 位置在(-3000,-2500)到(3000,2500)之间随机
local function pos_rand()
    local Posrand = function ()
        math.randomseed(tostring(os.time()):reverse():sub(1, 6))
        local posrand = CGeoPoint(math.random()*(LENGTH_X+LENGTH_X)-LENGTH_X,math.random()*(LENGTH_Y+LENGTH_Y)-LENGTH_Y)
        return posrand
    end
    return Posrand
end

local function res_posrand()
    local Resposrand = function ()
        local resposrand = res_pos
        return resposrand
    end
    return Resposrand
end

local function get_posrand()
    local Getposrand = function ()
        local getposrand = get_pos
        return getposrand
    end
    return Getposrand
end

gPlayTable.CreatePlay{

firstState = "greb0",
["greb0"] = {-- 跑位拿球
    switch = function()
        -- 球踢出一定距离后保存球数据
        if ball.toPointDist(player.pos("Kicker")) > 500 and kick_flag == 1 then
            writeFile('../shoot_data.txt','ball_pos\n')
            writeFile('../shoot_data.txt','ball_posX'..'\t'..ball.posX()..'\n')
            writeFile('../shoot_data.txt','ball_posY'..'\t'..ball.posY()..'\n')
            writeFile('../shoot_data.txt','000000000'..'\t'..'000000000'..'\n')
            -- print('car_posX  '..'\t'..player.posX("Kicker")..'\n')
            -- print('ball_posX '..'\t'..ball.posX()..'\n')
            print('ball'..'\t'..math.deg(math.atan((ball.posY()-player.posY("Kicker"))/(ball.posX()-player.posX("Kicker"))))..'\n')
            -- print('balloutput'..'\t'..ball.posX()..'\t'..player.posX("Kicker")..'\n')
            -- print('balloutput'..'\t'..ball.posY()..'\t'..player.posY("Kicker")..'\n')
            -- print('ball_posY '..'\t'..ball.posY()..'\n')
            print('000000000'..'\t'..'000000000'..'\n')
            kick_flag = 0
            power_flag = 0
            rotvel_flag = 0
        end
        -- 出界判定
        if ball.posX() > LENGTH_X or ball.posX() < -LENGTH_X
            or ball.posY() > LENGTH_Y or ball.posY() < -LENGTH_Y then
            res_pos = pos_rand()()
            return "rest"..0
        -- 禁区判定
        elseif ball.posX() > PENTY_X and ball.posY() > -PENTY_Y and ball.posY() < PENTY_Y then
            res_pos = pos_rand()()
            return "rest"..0
        end
        if bufcnt(player.infraredOn("Kicker"),3,1000) then
            return "kick"..0
        end
    end,
    Kicker = task.fetchBall(get_posrand(),_,true),
    match = ""
},
["rest0"] = {-- 界外捡球
    switch = function()
        if bufcnt(ball.toPointDist(res_posrand()())<200,3,1000) then
            return "kick"..0
        end
    end,
    Kicker = task.fetchBall(res_posrand(),_,true),
    match = ""
},
["kick0"] = {-- 转圈踢球
    switch = function()
        if rotvel_flag == 0 then
            rotvel_old = rot_rand1()()
            rotvel_flag = 1
        end
        if power_flag == 0 then
            power_old = pow_rand1()()
            power_flag = 1
        end
        -- 球刚刚出手后保存车数据
        if bufcnt(player.kickBall("Kicker"),1,1000) then
            writeFile('../shoot_data.txt','car_pos'..'\n')
            writeFile('../shoot_data.txt','car_dir   '..'\t'..player.dir("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_posX  '..'\t'..player.posX("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_posY  '..'\t'..player.posY("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_velMod'..'\t'..player.velMod("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_velDir'..'\t'..player.velDir("Kicker")..'\n')
            writeFile('../shoot_data.txt','car_power '..'\t'..pow_rand2()()..'\n')
            writeFile('../shoot_data.txt','car_rotVel'..'\t'..player.rawrotVel("Kicker")..'\n')
            print('car_rotVel'..'\t'..player.rawrotVel("Kicker")..'\n')
            -- print('car_posX  '..'\t'..player.posX("Kicker")..'\n')
            -- print('car_rotVel'..'\t'..player.rawrotVel("Kicker")..'\n')
            kick_flag = 1
            return "wait"..0
        -- 丢球判定
        elseif math.sqrt((ball.posX()-player.posX("Kicker"))*(ball.posX()-player.posX("Kicker"))
            +(ball.posY()-player.posY("Kicker"))*(ball.posY()-player.posY("Kicker"))) > 1000 and player.infraredOn("Kicker") == false then
            return "wait"..0
        end
    end,
    Kicker = task.forcespeed(0,0,3,4000),
    -- Kicker = task.forcespeed(0,0,rot_rand2(),pow_rand2()),
    match = ""
},
["wait0"] = {-- 踢后等待
    switch = function()
        if math.sqrt((ball.posX()-player.posX("Kicker"))*(ball.posX()-player.posX("Kicker"))
            +(ball.posY()-player.posY("Kicker"))*(ball.posY()-player.posY("Kicker"))) > 1000 and player.infraredOn("Kicker") == false then
            return "greb"..0
        elseif bufcnt(player.rawrotVel("Kicker") < 1,100,1000) then
            return "greb"..0
        end
    end,
    Kicker = task.speed(0,0,0,0),
    match = ""
},
name = "TestShootPredict",
applicable ={
    exp = "a",
    a = true
},
attribute = "attack",
timeout = 99999
}