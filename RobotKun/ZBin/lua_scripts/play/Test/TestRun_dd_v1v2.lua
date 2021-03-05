local testPos  = {
    CGeoPoint:new_local(-2500, 2500),   --发球点
    CGeoPoint:new_local( 2500,-2500),    --目标点
    CGeoPoint:new_local(-4000,-3000),
    CGeoPoint:new_local(-500,-3000)
}
local vel = CVector:new_local(0, 0)
local maxvel=0
local time = 120
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)


--set test robot number
local Robot_number = player.num()

--get flat max&min power
local max_power = CGetKSettings("Robot"..Robot_number.."/FLAT_MAX", "Double")
local min_power = CGetKSettings("Robot"..Robot_number.."/FLAT_MIN", "Double")
-- print("get settings",max_power,min_power)

local cnt = 10  --测试总次数
local nkick = 1       --踢球计数器
local kcycle = 0
local step_power = (max_power - min_power)/cnt
local power = step_power*nkick + min_power
local fmaxvel = 0
local smaxvel = 0
local tmaxvel = 0
--

local GetBallMaxVel = function()

    local v = 0
    local vball = ball.velMod()

    if vball > tmaxvel and vball < smaxvel then
        tmaxvel = vball

    elseif vball > smaxvel and vball < fmaxvel then
        tmaxvel = smaxvel
        smaxvel = vball

    elseif vball > fmaxvel then
        tmaxvel = smaxvel
        smaxvel = fmaxvel
        fmaxvel = vball

    end

    if vision:getCycle() - kcycle == 30 then 
        v = (tmaxvel + smaxvel + fmaxvel )/3
        tmaxvel = 0
        smaxvel = 0
        fmaxvel = 0

    end

    return v

end

local azhe = function(v)
    if v ~= 0 then
        print("link start!")
        local a = CLeastSqure(v/10, step_power*nkick + min_power, Robot_number)

        nkick = nkick + 1
        if nkick > cnt then
            nkick = 0
        end
    end

    return a 
end

local power = function(v)
    return function()
        return step_power*nkick + min_power;
    end
end

gPlayTable.CreatePlay{

firstState = "run1",


["run1"] = {
    switch = function()
        if player.kickBall("Tier")then

            return "run"..3
        end
    end,
    Tier = task.zget(testPos[1], _, power(), flag.kick),
    Kicker = task.goCmuRush(testPos[1], player.toPlayerDir("Tier"),_, DSS_FLAG),
    match = ""
},

["run2"] = {
    switch = function()
        if vision:getCycle() - kcycle <= 30 then
            azhe(GetBallMaxVel())
            return "run"..2
        else
            return "run"..1
        end
    end,
    Tier = task.goCmuRush(testPos[2], player.toPlayerDir("Kicker"),_, DSS_FLAG),
    Kicker = task.goCmuRush(testPos[1], player.toPlayerDir("Tier"),_, DSS_FLAG),
    match = ""
},

["run3"] = {
    switch = function()
        if player.kickBall("Kicker")then
            kcycle = vision:getCycle()
            print("run3",kcycle,nkick)
            return "run"..2
        end
    end,
    Kicker = task.zget(testPos[2], _, power(), flag.kick),
    Tier = task.goCmuRush(testPos[2], player.toPlayerDir("Kicker"),_, DSS_FLAG),
    match = ""    

},

name = "TestRun_dd_v1v2",
applicable ={
    exp = "a",
    a = true
},
attribute = "attack",
timeout = 99999
}
