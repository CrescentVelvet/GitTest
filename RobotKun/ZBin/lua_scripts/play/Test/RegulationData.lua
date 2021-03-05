local PLACE_POS = CGeoPoint:new_local(0,0)
local SHOOT_POS = CGeoPoint:new_local(4500,0)
local TURN_DIR = -math.pi

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)
local rot_speed = 0
local rot_limit = 0

function rotspeed()
    return function()
        return rot_speed
    end
end
function linedir()
    return function()
        return player.toPointDir(SHOOT_POS, "Leader")
    end
end
function runpos()
    return function()
        return PLACE_POS
    end
end

local process = function(power)
    local recordfile = io.open("data/regulation.txt","a")
    omega = rot_limit
    debugEngine:gui_debug_msg(ball.pos(),"giao!!")
    delta_dir = (SHOOT_POS - PLACE_POS):dir() - (ball.pos() - PLACE_POS):dir()
    print(delta_dir)
    need_reg = power * math.tan(delta_dir)
    recordfile:write(omega," ",need_reg,"\n")
end

gPlayTable.CreatePlay{

firstState = "fetchBall",
["fetchBall"] = {
    switch = function()
        if bufcnt(player.toTargetDist("Leader")< 100, 20, 9999) then
            return "turn1"
        end
        if bufcnt(not ball.valid(),60,9999) then
            return "waitforball"
        end
    end,
    Leader = task.fetchBall(runpos(),0,true),
    match = "{L}"
},
["turn1"] = {
    switch = function()
        rot_speed = 0
        if bufcnt(not player.infraredOn("Leader"),30,9999) then
            return "fetchBall"
        else
            if bufcnt(player.toTargetDist("Leader")< 100, 20, 9999) then
                if ball.toPointDist(PLACE_POS) < 50 or player.infraredOn("Leader") then
                    rot_limit = math.random() * 6.0 + 1.0
                    return "turn2"
                else
                    return "fetchBall"
                end
            end
        end
    end,
    Leader = task.goCmuRush(runpos(), TURN_DIR, _, flag.dribbling),
    match = "{L}"
},
["turn2"] = {
    switch = function()
        if rot_speed > rot_limit - 0.08 then
            rot_speed = rot_limit
        else
            rot_speed = rot_speed + 0.08
        end
        -- if math.abs(player.dir("Leader") - player.toPointDir(SHOOT_POS, "Leader")) < 10 / 180 * math.pi then
        --     return "kick"
        -- end
        -- print(rot_speed)
        if bufcnt(player.kickBall("Leader"), 1 ,999999) then
            -- PLACE_POS = CGeoPoint:new_local((math.random() - 0.5) * 1000, (math.random() - 0.5) * 5000)
            return "record"
        end
    end,
    Leader = task.openSpeed(0, 0, rotspeed, SHOOT_POS, linedir(), flag.dribbling),
    match = "{L}"
},
["record"] = {
    switch = function()
        if ball.toPointDist(PLACE_POS) > 2000 then
            process((SHOOT_POS - PLACE_POS):mod() + 1000)
            -- PLACE_POS = CGeoPoint:new_local((math.random() - 0.5) * 1000, (math.random() - 0.5) * 5000)
            return "fetchBall"
        end
    end,
    match = ""
},
-- ["kick"] = {
--     switch = function()
--         if bufcnt(player.kickBall("Leader"), 1 ,300) then
--             return "fetchBall"
--         end
--     end,
--     Leader = task.zget(SHOOT_POS, _, _, flag.kick),
--     -- Leader = task.zbreak(SHOOT_POS, _, _, _),
--     match = "{L}"
-- },

["waitforball"] = {
    switch = function()
        if bufcnt(ball.valid(),20,9999) then
            return "fetchBall"
        end
    end,
    Leader = task.goCmuRush(runpos()),
    match = "{L}"

},

name = "RegulationData",
applicable ={
    exp = "a",
    a = true
},
attribute = "attack",
timeout = 99999
}
