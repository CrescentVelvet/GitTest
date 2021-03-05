local SHOOT_POS = CGeoPoint:new_local(param.pitchLength/2,0)
local CHIP_POS = CGeoPoint:new_local(0,0)
local FLAT_POS = CGeoPoint:new_local(500,0)
local OFFSET = 600

local CHIP_BUF
local random_x = 0.5
local random_y = 0.5
local X_S = 0
local X_E = -1500
local Y_S = -2700
local Y_E = 2700

local random_chip_pos = function()
    local x = X_S + (X_E-X_S)*random_x
    local y = Y_S + (Y_E-Y_S)*random_y
    print("chip_pos", x, y)
    return CGeoPoint:new_local(x,y)
end

local random_flat_pos = function()

    local vec = (SHOOT_POS - random_chip_pos())
    local pos = random_chip_pos() + vec / vec:mod() * OFFSET

    -- local x = X_S + (X_E-X_S)*random_x + math.cos(dir*180/math.pi)*OFFSET
    -- local y = Y_S + (Y_E-Y_S)*random_y + math.sin(dir*180/math.pi)*OFFSET
    print("flat_pos",x, y)
    return pos
end

-- local normal_start = function()
--     return function()
--         return FLAT_POS + OFFSET
--     end
-- end

-- local TURN_DIR = function()
--     return function()
--         p = flat_pos + OFFSET  --normal_start
--         return player.toTargetDir(p, "Kicker")
--     end
-- end

-- debugEngine:gui_debug_msg(kick_target,string.format("%d",(power())()),3)

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

gPlayTable.CreatePlay{

firstState = "init",
["init"] = {
    switch = function()
                random_x = math.random()
                random_y = math.random()
        return "fetchBall"
    end,
    Leader = task.stop(),
    Assister = task.stop(),

    match = "{LA}"
},
["fetchBall"] = {
    switch = function() 
        if bufcnt(player.infraredOn("Leader"), 20, 9999) then
            return "turn"
        end
    end,
    Leader = task.fetchBall(random_chip_pos,_,true),
    Assister = task.stop(),
    -- Assister = task.goCmuRush(FLAT_POS, player.toPointDir(SHOOT_POS), _, DSS_FLAG),
    match = "{LA}"
},
["turn"] = {
    switch = function()
        if player.infraredOn("Leader") then
            if bufcnt(player.toTargetDist("Leader") < 50 and player.toTargetDist("Assister") < 50, 20 ,9999) then
                CHIP_BUF = cp.getFixBuf(random_chip_pos)
                return "chip"
            end
        else 
            return "fetchBall"
        end
    end,
    Leader = task.goCmuRush(random_chip_pos, _, _, flag.dribbling),
    Assister = task.goCmuRush(random_flat_pos,player.toPointDir(SHOOT_POS), _, DSS_FLAG),
    match = "{LA}"
},
["chip"] = {
    switch = function()
        if bufcnt(player.kickBall("Leader"), 1 ,9999) then
            return "fix"
        end
    end,
    Leader = task.zget(random_flat_pos, _, 800, flag.kick + flag.chip),
    Assister = task.goCmuRush(random_flat_pos, player.toPointDir(SHOOT_POS), _, DSS_FLAG),
    match = "{LA}"
}, 
["fix"] = {
    switch = function()
        if  bufcnt(true, CHIP_BUF) then
            return "flat"
        end
    end,
    Leader = task.goCmuRush(random_chip_pos, _, _, DSS_FLAG),
    Assister = task.goCmuRush(random_flat_pos, player.toPointDir(SHOOT_POS), _, DSS_FLAG),
    match = "{LA}"
}, 
["flat"] = {
    switch = function()
        if bufcnt(player.kickBall("Assister"), 1 ,300) then
            return "init"
        end
    end,
    Leader = task.stop(),
    -- Assister = task.zget(_, _, _, flag.kick, _,10),
    Assister = task.chaseNew(),
    match = "{LA}"
}, 

name = "ImmortalKick",
applicable ={
    exp = "a",
    a = true
},
attribute = "attack",
timeout = 99999
}
