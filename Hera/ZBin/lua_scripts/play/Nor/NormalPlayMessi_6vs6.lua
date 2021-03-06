local DSS = bit:_or(flag.allow_dss,flag.dodge_ball)--avoid_ball_and_shoot_line)
USE_ZGET = CGetSettings("Messi/USE_ZGET", "Bool")

--激进单后卫打法开关
local AGRESSIVE = 0

--大巴防守开关
local BUSDEFENSE = 0

local leaderNum = function ()
    return messi:leaderNum()
end
local receiverNum = function ()
    return messi:receiverNum()
end
function getPassPos()
    local rPos = function ()
        return messi:passPos()
    end
    return rPos
end

function goaliePassPos()
    local gPos = function ()
        return messi:goaliePassPos()
    end
    return gPos
end

function getPassVel()
    local vel = function ()
        if messi:needChip() then 
            return messi:passVel() * 0.6
        end

        return messi:passVel() * 0.95
    end
    return vel
end

function getAttackerNum(i)
    return function ()
        return defenceSquence:getAttackNum(i)
    end
end

function getOurNum()
    return vision:getValidNum()
end


function getFlag()
    local flag = function ()
        local f = flag.dribble + flag.safe + flag.rush
        if messi:needChip() then
            f = f + flag.chip
        end
        if messi:needKick() then
            f = f + flag.kick
        end
        return f
    end
    return flag
end

local function ourBallJumpCond()
    local state = messi:nextState()
    local attackerAmount = defenceSquence:attackerAmount()
    if attackerAmount < 0 then 
        attackerAmount = 0
    end

    if BUSDEFENSE == 1 then
        gRoleNum["Leader"] = leaderNum() 
        return "BusDefense"
    end

    if getOurNum() == 3 then
        gRoleNum["Leader"] = leaderNum() 
        return "4car"
    end

    if getOurNum() == 2 then
        gRoleNum["Leader"] = leaderNum()
        return "3cardefend"
    end
    if getOurNum() == 1 then 
        gRoleNum["Leader"] = leaderNum()
        return "2cardefend"
    end
    if state == "Pass" and AGRESSIVE == 1 then 
        if attackerAmount > 1 then
            attackerAmount = 1
        end
        --绕过match直接赋值
        local leader = leaderNum()
        local receiver = receiverNum()
        -- print("valid:", receiver, player.valid(receiver))
        if getOurNum() > 3 then 
            if leader ~= receiver and player.valid(receiver) then
                gRoleNum["Leader"] = leader
                gRoleNum["Receiver"] = receiver
                
            else
                gRoleNum["Leader"] = leader
            end
        end
        return "APass"..attackerAmount
    end

    if state == "Pass" then 
        if attackerAmount > 2 then
            attackerAmount = 2
        end
        --绕过match直接赋值
        local leader = leaderNum()
        local receiver = receiverNum()
        -- print("valid:", receiver, player.valid(receiver))
        if getOurNum() > 3 then 
            if leader ~= receiver and player.valid(receiver) then
                gRoleNum["Leader"] = leader
                gRoleNum["Receiver"] = receiver
                
            else
                gRoleNum["Leader"] = leader
            end
        end
        return "Pass"..attackerAmount

    elseif state == "GetBall" and AGRESSIVE == 1 then
        if attackerAmount > 3 then 
            attackerAmount = 3
        end

        if getOurNum() > 3 then 
            gRoleNum["Leader"] = leaderNum()
        end
        return "AGetBall"..attackerAmount

    elseif state == "GetBall" then
        if attackerAmount > 3 then 
            attackerAmount = 3
        end

        if getOurNum() > 3 then 
            gRoleNum["Leader"] = leaderNum()
        end
        return state..attackerAmount
    elseif state == "fix" then
        gRoleNum["Leader"] = leaderNum()
        return "fix"
    end
end

function ourBallJumpCondTest()
    local leader = leaderNum()
    local receiver = receiverNum()
    local state = messi:nextState()
    if state == "fix" then
        gRoleNum["Leader"] = leader
        return "testFix"
    end
    if leader ~= receiver and player.valid(receiver) then
        gRoleNum["Leader"] = leader
        gRoleNum["Receiver"] = receiver
        return "test4"
    else
        gRoleNum["Leader"] = leader
        return "test3"
    end
end

function leaderTask (t, w, p, f)
    return function()
        if USE_ZGET then
            return task.zget(pos.getPassPos(), _, getPassVel(), getFlag())
        else
            return task.zattack(pos.getPassPos(), _, getPassVel(), getFlag())
        end
    end
end

function receiverTask (p,d)
    return function ()
    -- print("_ is :" , _)
        if getOurNum() <= 3 and messi:nextState() == "GetBall" then 
           return task.zmarking("Zero",_,getAttackerNum(0))
        end

        return task.goCmuRush(pos.getReceivePos(),player.toBallDir,_,DSS)
    end
end
gPlayTable.CreatePlay{
firstState = "initState",
["stop"] = {
    switch = function ()

    end,
    Leader   = task.stop(),
    Receiver = task.stop(),
    Center   = task.stop(),
    Assister = task.stop(),
    Defender = task.stop(),
    Goalie   = task.stop(),
    match    = "[LRCAD]"
},

["test3"] = {
    switch = function ()
        return ourBallJumpCondTest()
    end,
    Leader   = leaderTask(),
    Receiver = receiverTask(), 
    Center   = task.zdrag(pos.getOtherPos(2)),
    Goalie   = task.zgoalie(),
    match    = "{L}[R][C]"
},
["test4"] = {
    switch = function ()
        return ourBallJumpCondTest()
    end,
    Leader   = leaderTask(),--task.zattack(pos.getPassPos(), _, getPassVel(), getFlag()),
    Receiver = receiverTask(), --task.goCmuRush(pos.getReceivePos(),player.toBallDir,_,DSS),
    Center  = task.zdrag(pos.getOtherPos(2)),
    match    = "{LR}[C]"
},
["testFix"] = {
    switch = function ()
        local nextState = ourBallJumpCondTest()
        if nextState ~= "testFix" then
            return nextState
        end
    end,
    Leader   = task.goCmuRush(pos.getLeaderWaitPos(),player.toBallDir,_,flag.allow_dss),
    Receiver = receiverTask(),
    Center  = task.zdrag(pos.getOtherPos(2)),
    match    = "{L}[R][C]"
},
["initState"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = task.zget(),
    Special  = task.sideBack(),
    Middle   = task.defendMiddle("Leader"),
    -- Defender = task.leftBack(),
    -- Assister = task.rightBack(),
    Defender = task.zback(2,1),
    Assister = task.zback(2,2),
    Goalie   = task.zgoalie(),
    match    = "[L][ADMS]"
},

["Pass0"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = receiverTask(),
    Center   = task.zdrag(pos.getOtherPos(2)),
    -- Defender = task.leftBack(),
    -- Assister = task.rightBack(),
    Defender = task.zback(2,1),
    Assister = task.zback(2,2),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = function ()
        local leader = leaderNum()
        local receiver = receiverNum()

        if getOurNum() <= 3 then
            return "(DA)[L][R][C]"
        elseif leader ~= receiver and player.valid(receiver) then 
            return  "{LR}[DA][C]"
        else 
            return "{L}[DA][R][C]"
        end
    end,--"{LR}[DA][M][F][C]"
},

["Pass1"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = receiverTask(),
    Center   = task.zmarking("Zero",_,getAttackerNum(0)),
    -- Center   = task.zsupport(),
    -- Defender = task.leftBack(),
    -- Assister = task.rightBack(),
    Defender = task.zback(2,1),
    Assister = task.zback(2,2),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = function ()
        local leader = leaderNum()
        local receiver = receiverNum()

        if getOurNum() <= 3 then
            return "(DA)[L][R][C]"
        elseif leader ~= receiver and player.valid(receiver) then 
            return  "{LR}[DA][C]"
        else 
            return "{L}[DA][R][C]"
        end
    end,
},

["Pass2"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = receiverTask(),
    Center   = task.zmarking("Zero",_,getAttackerNum(0)),
    -- Defender = task.leftBack(),
    -- Assister = task.rightBack(),
    Defender = task.zback(2,1),
    Assister = task.zback(2,2),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = function ()
        local leader = leaderNum()
        local receiver = receiverNum()
        
        if getOurNum() <= 3 then
            return "(DA)[L][R][C]"
        elseif leader ~= receiver and player.valid(receiver) then 
            return  "{LR}[DA][C]"
        else 
            return "{L}[DA][R][C]"
        end
    end,
},

["GetBall0"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = receiverTask(),
    -- Center   = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
    Center   = task.zsupport(),
    -- Defender = task.leftBack(),
    -- Assister = task.rightBack(),
    Defender = task.zback(2,1),
    Assister = task.zback(2,2),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "{L}[DA][R][C]"
},
["GetBall1"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    -- Leader   = task.zget(pos.getPassPos(), _, getPassVel(), getFlag()),
    Receiver = receiverTask(),
    Center   = task.zmarking("Zero",_,getAttackerNum(0)),
    -- Defender = task.leftBack(),
    -- Assister = task.rightBack(),
    Defender = task.zback(2,1),
    Assister = task.zback(2,2),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "{L}[DA][C][R]"
},
["GetBall2"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    -- Leader   = task.zget(pos.getPassPos(), _, getPassVel(), getFlag()),
    Receiver = task.zmarking("Zero",_,getAttackerNum(0)),
    Center   = task.zmarking("First",_,getAttackerNum(1)),
    -- Defender = task.leftBack(),
    -- Assister = task.rightBack(),
    Defender = task.zback(2,1),
    Assister = task.zback(2,2),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "{L}[DA][RC]"
},
["GetBall3"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    -- Leader   = task.zget(pos.getPassPos(), _, getPassVel(), getFlag()),
    Receiver = task.zmarking("Zero",_,getAttackerNum(0)),
    Center   = task.zmarking("First",_,getAttackerNum(1)),
    -- Defender = task.leftBack(),
    -- Assister = task.rightBack(),
    Defender = task.zback(2,1),
    Assister = task.zback(2,2),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "{L}[DA][RC]"
},
["GetBall4"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = task.zmarking("Zero",_,getAttackerNum(0)),
    Center   = task.zmarking("First",_,getAttackerNum(1)),
    Defender = task.zmarking("Second",_,getAttackerNum(2)),
    -- Assister = task.singleBack(),
    Assister = task.zback(1,1),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "{L}[A][RCD]"
},
["GetBall5"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = task.zmarking("Zero",_,getAttackerNum(0)),
    Center   = task.zmarking("First",_,getAttackerNum(1)),
    Defender = task.zmarking("Second",_,getAttackerNum(2)),
    Assister = task.zmarking("Third",_,getAttackerNum(3)),
    -- Assister = task.zmarking("Fourth",_,getAttackerNum(4)),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "{L}[RCDA]"
},
["GetBall6"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = task.zmarking("Zero",_,getAttackerNum(0)),
    Center   = task.zmarking("First",_,getAttackerNum(1)),
    Defender = task.zmarking("Second",_,getAttackerNum(2)),
    Assister = task.zmarking("Third",_,getAttackerNum(3)),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "{L}[RCDA]"
},

["3cardefend"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = receiverTask(),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "[L][R]" 
},

["2cardefend"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "[L]" 
},

["4car"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = receiverTask(),
    -- Center   = task.singleBack(),
    Center   = task.zback(1,1),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "{L}(C)[R]" 
},


["fix"] = {
    switch = function ()
        local nextState = ourBallJumpCond()
        if nextState ~= "fix" then
            return nextState
        end
    end,
    Leader   = task.goCmuRush(pos.getLeaderWaitPos(),player.toBallDir,_,flag.allow_dss),
    Receiver = receiverTask(),
    Center   = task.continue(),
    Defender = task.continue(),
    Assister = task.continue(),
    Goalie   = task.continue(),
    match    = "{L}[R][DA][C]"
},

["BusDefense"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = task.zback(6,1),
    Receiver = task.zback(6,2),
    Center   = task.zback(6,3),
    Defender = task.zback(6,4),
    Assister = task.zback(6,5),
    Goalie   = task.zgoalie(),
    match    = "[LRCDA]"
},

["APass0"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = receiverTask(),
    Center   = task.zdrag(pos.getOtherPos(2)),
    Assister = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
    Defender = task.zback(1,1),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = function ()
        local leader = leaderNum()
        local receiver = receiverNum()

        if getOurNum() <= 3 then
            return "(D)[L][R][CA]"
        elseif leader ~= receiver and player.valid(receiver) then 
            return  "{LR}[D][CA]"
        else 
            return "{L}[D][R][CA]"
        end
    end,--"{LR}[DA][M][F][C]"
},

["APass1"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = receiverTask(),
    Center   = task.zmarking("Zero",_,getAttackerNum(0)),
    Assister = task.zdrag(pos.getOtherPos(2)),
    Defender = task.zback(1,1),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = function ()
        local leader = leaderNum()
        local receiver = receiverNum()

        if getOurNum() <= 3 then
            return "(D)[L][R][C][A]"
        elseif leader ~= receiver and player.valid(receiver) then 
            return  "{LR}[D][C][A]"
        else 
            return "{L}[D][R][C][A]"
        end
    end,
},

["AGetBall0"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = receiverTask(),
    -- Center   = task.zmarking("Zero",_,getAttackerNum(0)),
    Center   = task.goCmuRush(pos.getBackPos(),player.toBallDir,_,DSS),
    Assister = task.zdrag(pos.getOtherPos(2)),
    Defender = task.zback(1,1),
    Goalie   = task.zgoalie(goaliePassPos()),
    -- match    = "{L}(D)[AC][R]"
    match    = "{L}(D)[R][A][C]"

},
["AGetBall1"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = receiverTask(),
    Center   = task.zmarking("Zero",_,getAttackerNum(0)),
    Assister = task.zdrag(pos.getOtherPos(2)),
    Defender = task.zback(1,1),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "{L}(D)[C][R][A]"
},
["AGetBall2"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = task.zmarking("Zero",_,getAttackerNum(0)),
    Center   = task.zmarking("First",_,getAttackerNum(1)),
    Assister = task.zdrag(pos.getOtherPos(2)),
    Defender = task.zback(1,1),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "{L}(D)[RC][A]"
},
["AGetBall3"] = {
    switch = function ()
        return ourBallJumpCond()
    end,
    Leader   = leaderTask(),
    Receiver = task.zmarking("Zero",_,getAttackerNum(0)),
    Center   = task.zmarking("First",_,getAttackerNum(1)),
    Assister = task.zmarking("Second",_,getAttackerNum(2)),
    Defender = task.zback(1,1),
    Goalie   = task.zgoalie(goaliePassPos()),
    match    = "{L}(D)[RCA]"
},

name = "NormalPlayMessi_6vs6",
applicable ={
    exp = "a",
    a = true
},
attribute = "attack",
timeout = 99999
}