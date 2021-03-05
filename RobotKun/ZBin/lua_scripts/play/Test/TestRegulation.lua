local PLACE_POS = CGeoPoint:new_local(4000,-600)
local TURN_DIR = -math.pi

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)
gPlayTable.CreatePlay{

firstState = "fetchBall",
["fetchBall"] = {
    switch = function()
        if bufcnt(player.infraredOn("Leader"), 20, 9999) then
            return "turn"
        end
    end,
    Leader = task.zget(_, _, _, flag.our_ball_placement),
    match = "[L]"
},
["turn"] = {
    switch = function()
        if bufcnt(player.toTargetDist("Leader") < 50, 20 ,300) then
            return "kick"
        end
    end,
    Leader = task.goCmuRush(PLACE_POS, TURN_DIR, _, flag.dribbling),
    match = "[L]"
},
["kick"] = {
    switch = function()
        if bufcnt(player.kickBall("Leader"), 1 ,300) then
            return "fetchBall"
        end
    end,
    Leader = task.zget(_, _, _, flag.kick),
    match = "[L]"
},

name = "TestRegulation",
applicable ={
    exp = "a",
    a = true
},
attribute = "attack",
timeout = 99999
}
