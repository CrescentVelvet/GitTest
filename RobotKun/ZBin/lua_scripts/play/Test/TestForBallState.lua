math.randomseed(os.time())

newPos = function()
    local posX = math.random(-3000, 3000)
    local posY = math.random(-2500, 2500)
    local pos = CGeoPoint:new_local(posX, posY)
    -- print("(" .. posX .. ",~ " .. posY .. ")")
    return pos
end  

newPower = function()
    local power = math.random(2000, 4000)
    return power
end

local temp_pos = CGeoPoint:new_local(1000, 1000)
local temp_power = 500
local _flag_flat = flag.kick + flag.dribble
local _flag_chip = flag.kick + flag.chip + flag.dribble
local _flag = _flag_chip
-- local chip = true

getTempPos = function()
    return temp_pos
end

getTempPower = function()
    return temp_power
end

-- getTempFlag = function()
--     if chip == true then
--         return _flag_chip
--     else
--         return _flag_flat
--     end
-- end

gPlayTable.CreatePlay{

firstState = "init",

["init"] = {
    switch = function()
        if bufcnt(true, 100) then
            temp_pos = newPos()
            temp_power = newPower()
            return "state1"
        end
    end,
    a = task.goCmuRush(CGeoPoint:new_local(0, 0)),
    match = "[a]"
},

["state1"] = {
    switch = function()
        print("power:"..temp_power)
        if player.kickBall("a") then
            temp_pos = newPos()
            temp_power = newPower()
            return "state1"
        end
    end,
    a = task.zget(getTempPos, _, getTempPower, _flag),
    match = "[a]"
},


name = "TestForBallState",
applicable = {
    exp = "a",
    a = true
},

attribute = "attack",
timeout = 99999
}