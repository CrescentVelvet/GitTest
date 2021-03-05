local testPos  = {
    CGeoPoint:new_local(-2500, 2000),   --发球点
    CGeoPoint:new_local(-2500,-2000),    --目标点
    CGeoPoint:new_local(-4000,-3000),
    CGeoPoint:new_local(-500,-3000)
}
local vel = CVector:new_local(0, 0)
local maxvel=0
local time = 120
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)


--set test robot number
local Robot_number1 = player.num("a")
local Robot_number2 = player.num("b")
--get flat max&min power
-- local max_power1 = CGetKSettings("Robot"..Robot_number1.."/FLAT_MAX", "Double")
-- local min_power1 = CGetKSettings("Robot"..Robot_number1.."/FLAT_MIN", "Double")
-- local max_power2 = CGetKSettings("Robot"..Robot_number2.."/FLAT_MAX", "Double")
-- local min_power2 = CGetKSettings("Robot"..Robot_number2.."/FLAT_MIN", "Double")
local max_power1 = 127.00
local min_power1 = 40.00
local max_power2 = 127.00
local min_power2 = 40.00

-- print("get settings",max_power,min_power)
local last_a = nil
local cnt = 10  --测试总次数
local nkick1 = 0       --踢球计数器
local nkick2 = 0
local step_power1 = (max_power1 - min_power1)/cnt
local step_power2 = (max_power2 - min_power2)/cnt
--

local power1 = function()
    return function()
        if step_power1*nkick1 + min_power1 > 127 then
            nkick1 = 5
            return 125
        else 
            return step_power1*nkick1 + min_power1;
        -- return 127
        end
    end
    -- return 5000
end

local power2 = function()
    return function()
        if step_power2*nkick2 + min_power2 > 127 then
            nkick2 = 5
            return 127
        else 
            return step_power2*nkick2 + min_power2;
        -- return 127
        end
    end
    -- return 5000
end

-- local power1 = function()
--     return function()
--         return 5000
--     end
-- end

-- local power2 = function()
--     return function()
--         return 5000
--     end
-- end

gPlayTable.CreatePlay{

firstState = "fetchBall1",

["fetchBall1"] = {
    switch = function()
        if bufcnt(player.infraredOn("b"), 20, 9999) then
            return "turn"..1
        end
    end,
    b= task.zget(ball.pos(), _, _, flag.our_ball_placement),
    a = task.goCmuRush(testPos[1], player.toPlayerDir("b"),_, DSS_FLAG),
    match = "{ab}"
},

["turn1"] = {
    switch = function()
        if bufcnt(player.toTargetDist("b") < 50, 20 ,300) then
            return "kick1"
        end
    end,
    b = task.goCmuRush(testPos[2], player.toPlayerDir("a"), _, flag.dribbling),
    a = task.goCmuRush(testPos[1], player.toPlayerDir("b"),_, DSS_FLAG),
    match = "{ab}"
},

["kick1"] = {
    switch = function()
        if player.kickBall("b")then
            nkick1 = nkick1 + 1
            last_a = "b"
            return "reset"
        end
    end,
    b = task.zget(testPos[1], _, power1(), flag.kick),
    a = task.goCmuRush(testPos[1], player.toPlayerDir("b"),_, DSS_FLAG),
    match = "{ab}"
},

["reset"] = {
    switch = function()
        if bufcnt(player.toTargetDist("b") < 50 and player.toTargetDist("a"), 5, 99999)then
            if nkick1 == cnt and nkick2 == cnt then 
                return "fetchBall1"
            end 
            if last_a == "b" then
                return "fetchBall"..2
            else
                return "fetchBall"..1
            end
        end
    end,
    b = task.goCmuRush(testPos[2], player.toPlayerDir("a"),_, DSS_FLAG),
    a = task.goCmuRush(testPos[1], player.toPlayerDir("b"),_, DSS_FLAG),
    match = "{ab}"
},

["fetchBall2"] = {
    switch = function()
        if bufcnt(player.infraredOn("a"), 20, 9999) then
            return "turn"..2
        end
    end,
    a = task.zget(ball.pos(), _, _, flag.our_ball_placement),
    b = task.goCmuRush(testPos[2], player.toPlayerDir("a"),_, DSS_FLAG),
    match = "{ab}"
},

["turn2"] = {
    switch = function()
        if bufcnt(player.toTargetDist("a") < 50, 20 ,300) then
            return "kick2"
        end
    end,
    a = task.goCmuRush(testPos[1], player.toPlayerDir("b"), _, flag.dribbling),
    b = task.goCmuRush(testPos[2], player.toPlayerDir("a"),_, DSS_FLAG),
    match = "{ab}"
},

["kick2"] = {
    switch = function()
        if player.kickBall("a")then
            last_a = "a"
            nkick2 = nkick2+1
            return "reset"
        end
    end,
    a = task.zget(testPos[2], _, power2(), flag.kick),
    b = task.goCmuRush(testPos[2], player.toPlayerDir("a"),_, DSS_FLAG),
    match = "{ab}"    
},

["run_end"] = {
    switch = function()

    end,
    b = task.goCmuRush(testPos[2], player.toPlayerDir("a"),_, DSS_FLAG),
    a = task.goCmuRush(testPos[1], player.toPlayerDir("b"),_, DSS_FLAG),
    match = "{ab}"
},



name = "TestRun_autoshootfit",
applicable ={
    exp = "a",
    a = true
},
attribute = "attack",
timeout = 99999
}
