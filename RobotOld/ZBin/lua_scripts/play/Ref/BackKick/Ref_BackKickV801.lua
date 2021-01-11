-- 直接挑前场进入normalplay
-- TestDynamicBackKick
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)


local passposnew = CGeoPoint:new_local(350, 200)
local ShootPos = ball.refAntiYPos(CGeoPoint:new_local(350, 200))

local WaitPos = {
        ball.refAntiYPos(CGeoPoint:new_local(100,240)),
        ball.refAntiYPos(CGeoPoint:new_local(100,80)),
        ball.refAntiYPos(CGeoPoint:new_local(100,-80)),
        ball.refAntiYPos(CGeoPoint:new_local(100,-240))
}

local WaitPos_backup = {
        ball.refAntiYPos(CGeoPoint:new_local(-30,-200)),
        ball.refAntiYPos(CGeoPoint:new_local(0  ,-200)),
        ball.refAntiYPos(CGeoPoint:new_local(30 ,-200)),
        ball.refAntiYPos(CGeoPoint:new_local(60 ,-200))
}


local RushPos = {
        ball.refAntiYPos(CGeoPoint:new_local(255, 100)),
        ball.refAntiYPos(CGeoPoint:new_local(215, 0)),
        ball.refAntiYPos(CGeoPoint:new_local(300, 0))
}

local FixPos = ball.refAntiYPos(CGeoPoint:new_local(225, 300))
--local fixBuf = getFixBuf(ShootPos)

gPlayTable.CreatePlay{
firstState = "Ready",
["Ready"] = {
        switch = function()
                updateFlag = true
                if bufcnt(player.toTargetDist("Special") < 20 and player.toTargetDist("Leader") < 20 ,30 ,180) then
                        return "Wait"
                end
        end,
        Assister = task.staticGetBall(),
        Leader   = task.goCmuRush(WaitPos[3], _, _, DSS_FLAG),
        Special  = task.goCmuRush(WaitPos[1], _, _, DSS_FLAG),
        Middle   = task.goCmuRush(WaitPos[2], _, _, DSS_FLAG),
        Center   = task.goCmuRush(WaitPos[4], _, _, DSS_FLAG),
        Fronter  = task.sideBack(),
        Defender = task.singleBack(),
        Goalie   = task.zgoalie(),
        match    = "{A}(LSMDFC)"
},

["Wait"] = {
        switch = function()
                if bufcnt(true,20) then 
                        return "exit"
                end
        end,
        Assister = task.staticGetBall(passposnew),
        Leader   = task.goCmuRush(WaitPos[3], _, 500,DSS_FLAG),
        Special  = task.goCmuRush(WaitPos[1], _, 500,DSS_FLAG),
        Middle   = task.goCmuRush(WaitPos[2], _, 500,DSS_FLAG),
        Center   = task.goCmuRush(WaitPos[4], _, 500, DSS_FLAG),
        Fronter  = task.sideBack(),
        Defender = task.leftBack(),
        Goalie   = task.zgoalie(),
        match    = "{A}{LSMDFC}"
},

["Dribble"] = {
        switch = function()
                if bufcnt(true,100) then 
                        return "ChipPass"
                end
        end,
        Assister = task.slowGetBall(passposnew),
        Leader   = task.goCmuRush(WaitPos[3]),
        Special  = task.goCmuRush(WaitPos[1]),
        Middle   = task.goCmuRush(WaitPos[2]),
        Center   = task.goCmuRush(WaitPos[4], _, 500, DSS_FLAG),
        Fronter  = task.sideBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "{A}{LSMDFC}"
},

["Rush"] = {
        switch = function()
                if bufcnt(player.toTargetDist("Leader") < 150 ,15 ,180) then
                        return "ChipPass"
                end
        end,
        Assister = task.slowGetBall(passposnew),
        Leader   = task.goCmuRush(ShootPos,player.toBallDir, 500,DSS_FLAG),
        Special  = task.goCmuRush(WaitPos[1],player.toBallDir, 500,DSS_FLAG),
        Center   = task.goCmuRush(WaitPos[4], _, 500, DSS_FLAG),
        Fronter  = task.sideBack(),
        Middle   = task.leftBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "{A}[CLS][FDM]"
},

["ChipPass"] = {
        switch = function()
        --ShootPosCalculate()
                if bufcnt(player.kickBall("Assister") ,1, 50) then
                        return "exit"
                end
        end,
        Assister = task.zget(_,_,_,flag.kick+flag.chip),--task.chipPass(passposnew),
        Leader   = task.goCmuRush(ShootPos, player.toBallDir),
        Special  = task.goCmuRush(WaitPos[1],player.toBallDir),
        Center   = task.goCmuRush(WaitPos[4], _, 500, DSS_FLAG),
        Fronter  = task.sideBack(),
        Middle   = task.leftBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "{A}{FCLSMD}"
},

["fixposition"] = {
        switch = function()
                if bufcnt(true,cp.getFixBuf(ShootPos)) then
                        return "receiveBall"
                end
        end,
        Assister = task.defendMiddle(),
        Leader   = task.goCmuRush(ShootPos,player.toBallDir),
        Special  = task.goCmuRush(WaitPos[1],player.toBallDir),
        Center   = task.goCmuRush(WaitPos[4], _, 500, DSS_FLAG),
        Fronter  = task.sideBack(),
        Middle   = task.leftBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "{A}{FCLSMD}"
},

["receiveBall"] = {
        switch = function()
                if bufcnt(ball.velMod() < 50,2,120) then
                        return "exit"
                end
        end,
        Assister = task.defendMiddle(),
        Leader   = task.receivePass("Special"),
        Special  = task.zsupport(),
        Center   = task.goCmuRush(WaitPos[4], _, 500, DSS_FLAG),
        Fronter  = task.sideBack(),
        Middle   = task.leftBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "{A}{LSMDFC}"
},

name = "Ref_BackKickV801",
applicable ={
        exp = "a",
        a = true
},
attribute = "attack",
timeout   = 99999
}