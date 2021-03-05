local pos = {
    CGeoPoint:new_local(2000, 2000),
    CGeoPoint:new_local(2000, -2000),
    CGeoPoint:new_local(-2000, -2000),
    CGeoPoint:new_local(-2000, 2000),
    CGeoPoint:new_local(0, 0)
}

local _flag = flag.kick + flag.chip + flag.dribble
local power = 2400
local chip_predict_pos

local getChipPredictPos = function()
    return ball.chipPredictPos()
end

local getChipState = function()
    return ball.isChip()
end

gPlayTable.CreatePlay{

firstState = "init",

["init"] = {
    switch = function()
        if player.toPointDist("a", pos[5]) < 500 then
            return "state0"
        end
    end,
    a = task.fetchBall(pos[5], 0, true),
    match = "{a}"
},

["state0"] = {
    switch = function()
        chip_predict_pos = getChipPredictPos()
        if getChipState() == true then
            debugEngine:gui_debug_arc(chip_predict_pos, 100, 0, 360)
            debugEngine:gui_debug_arc(chip_predict_pos, 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","true"),3)
        else
            debugEngine:gui_debug_arc(pos[5], 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","false"),3)
        end

        if player.kickBall("a") then
            return "state1"
        end
    end,
    a = task.zget(pos[1], _, power, _flag),
    match = "{a}"
},

["state1"] = {
    switch = function()
        chip_predict_pos = getChipPredictPos()
        if getChipState() == true then
            debugEngine:gui_debug_arc(chip_predict_pos, 100, 0, 360)
            debugEngine:gui_debug_arc(chip_predict_pos, 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","true"),3)
        else
            debugEngine:gui_debug_arc(pos[5], 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","false"),3)
        end
        if player.toPointDist("a", pos[1]) < 500 then
            return "state2"
        end
    end,
    a = task.fetchBall(pos[1], 0, true),
    match = "{a}"
},

["state2"] = {
    switch = function()
        chip_predict_pos = getChipPredictPos()
        if getChipState() == true then
            debugEngine:gui_debug_arc(chip_predict_pos, 100, 0, 360)
            debugEngine:gui_debug_arc(chip_predict_pos, 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","true"),3)
        else
            debugEngine:gui_debug_arc(pos[5], 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","false"),3)
        end
        if player.kickBall("a") then
            return "state3"
        end
    end,
    a = task.zget(pos[3], _, power, _flag),
    match = "{a}"
},

["state3"] = {
    switch = function()
        chip_predict_pos = getChipPredictPos()
        if getChipState() == true then
            debugEngine:gui_debug_arc(chip_predict_pos, 100, 0, 360)
            debugEngine:gui_debug_arc(chip_predict_pos, 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","true"),3)
        else
            debugEngine:gui_debug_arc(pos[5], 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","false"),3)
        end
        if player.toPointDist("a", pos[3]) < 500 then
            return "state4"
        end
    end,
    a = task.fetchBall(pos[3], 0, true),
    match = "{a}"
},

["state4"] = {
    switch = function()
        chip_predict_pos = getChipPredictPos()
        if getChipState() == true then
            debugEngine:gui_debug_arc(chip_predict_pos, 100, 0, 360)
            debugEngine:gui_debug_arc(chip_predict_pos, 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","true"),3)
        else
            debugEngine:gui_debug_arc(pos[5], 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","false"),3)
        end
        if player.kickBall("a") then
            return "state5"
        end
    end,
    a = task.zget(pos[4], _, power, _flag),
    match = "{a}"
},

["state5"] = {
    switch = function()
        chip_predict_pos = getChipPredictPos()
        if getChipState() == true then
            debugEngine:gui_debug_arc(chip_predict_pos, 100, 0, 360)
            debugEngine:gui_debug_arc(chip_predict_pos, 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","true"),3)
        else
            debugEngine:gui_debug_arc(pos[5], 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","false"),3)
        end
        if player.toPointDist("a", pos[4]) < 500 then
            return "state6"
        end
    end,
    a = task.fetchBall(pos[4], 0, true),
    match = "{a}"
},

["state6"] = {
    switch = function()
        chip_predict_pos = getChipPredictPos()
        if getChipState() == true then
            debugEngine:gui_debug_arc(chip_predict_pos, 100, 0, 360)
            debugEngine:gui_debug_arc(chip_predict_pos, 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","true"),3)
        else
            debugEngine:gui_debug_arc(pos[5], 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","false"),3)
        end
        if player.kickBall("a") then
            return "state7"
        end
    end,
    a = task.zget(pos[2], _, power, _flag),
    match = "{a}"
},

["state7"] = {
    switch = function()
        chip_predict_pos = getChipPredictPos()
        if getChipState() == true then
            debugEngine:gui_debug_arc(chip_predict_pos, 100, 0, 360)
            debugEngine:gui_debug_arc(chip_predict_pos, 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","true"),3)
        else
            debugEngine:gui_debug_arc(pos[5], 150, 0, 360)
            debugEngine:gui_debug_msg(pos[5],string.format("%s","false"),3)
        end
        if player.toPointDist("a", pos[2]) < 500 then
            return "state0"
        end
    end,
    a = task.fetchBall(pos[2], 0, true),
    match = "{a}"
},



name = "TestForChip",
applicable = {
    exp = "a",
    a = true
},

attribute = "attack",
timeout = 99999
}