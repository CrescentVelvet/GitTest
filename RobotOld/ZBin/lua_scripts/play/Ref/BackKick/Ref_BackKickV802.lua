-- ImmortalKick 加强版
-- Mark
local posDistance = 250-- 两车距离
local FreeKick_ImmortalStart_Pos = function ()
        local pos
        local ball2goal = CVector:new_local(CGeoPoint:new_local(450, 0) - ball.pos())
        pos = ball.pos() + Utils.Polar2Vector(posDistance, ball2goal:dir()) --距离
        local tempPos = ball.pos()+ Utils.Polar2Vector(posDistance, ball2goal:dir())
        for i = 1, 6 do
                if vision:theirPlayer(i):Valid() then
                        if vision:theirPlayer(i):Pos():dist(tempPos) < 20 then
                                local dir = (CGeoPoint:new_local(450, 30 * ball.antiY()) - vision:theirPlayer(i):Pos()):dir()
                                pos = vision:theirPlayer(i):Pos() + Utils.Polar2Vector(20, dir)
                                break
                        end
                else
                        pos = tempPos
                end
                
        end
        return pos
end

local Field_RobotFoward_Pos = function (role)
        local pos
        local UpdatePos = function ()
                local roleNum = player.num(role)
                if Utils.PlayerNumValid(roleNum) then
                        local carPos = vision:ourPlayer(roleNum):Pos()
                        pos = carPos + Utils.Polar2Vector(2, vision:ourPlayer(roleNum):Dir())
                end
                return pos
        end
        return UpdatePos
end

local FREEKICKPOS
local FIELDROBOTPOS
FREEKICKPOS = FreeKick_ImmortalStart_Pos

PreGoPos = function (role)
        return function ()
                return player.pos(role) + Utils.Polar2Vector(5, player.dir(role))
        end
end

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

local X1 = 60
local X2 = 120
local Y1 = 170
local Y2 = 350
local passposnew = CGeoPoint:new_local(350, Y2)
local ShootPos = ball.refAntiYPos(CGeoPoint:new_local(350, Y2))

local WaitPos = {
        ball.refAntiYPos(CGeoPoint:new_local(X1, Y1)),
        ball.refAntiYPos(CGeoPoint:new_local(X2, -Y2)),
        ball.refAntiYPos(CGeoPoint:new_local(X2, Y2)),
        ball.refAntiYPos(CGeoPoint:new_local(X1, -Y1))
}

gPlayTable.CreatePlay{

firstState = "startball",

["startball"] = {
        switch = function ()
                if bufcnt(player.toTargetDist("Special") < 30 or
                          player.toTargetDist("Defender") < 30, 70, 300)then
                        return "changepos"
                end
        end,
        Assister = task.staticGetBall(CGeoPoint:new_local(450, 0)),
        Leader   = task.leftBack(),
        Middle   = task.rightBack(),
        Center   = task.goCmuRush(WaitPos[3], _, _, DSS_FLAG),
        Fronter  = task.goCmuRush(WaitPos[1], _, _, DSS_FLAG),
        Special  = task.goCmuRush(WaitPos[2], _, _, DSS_FLAG),
        Defender = task.goCmuRush(WaitPos[4], _, _, DSS_FLAG),
        Goalie   = task.zgoalie(),
        match    = "(AL)[MSDCF]"
},

["changepos"] = {
        switch = function ()
                if bufcnt(player.toTargetDist("Leader") < 30, 30, 120)then
                        return "getball"
                end
        end,
        Assister = task.slowGetBall(CGeoPoint:new_local(450, 0)),       
        Middle   = task.singleBack(),
        Center   = task.goCmuRush(WaitPos[3], _, _, DSS_FLAG),
        Fronter  = task.goCmuRush(WaitPos[1], _, _, DSS_FLAG),
        Special  = task.goCmuRush(WaitPos[2], _, _, DSS_FLAG),
        Defender = task.goCmuRush(WaitPos[4], _, _, DSS_FLAG),
        Leader   = task.goSpeciPos(FREEKICKPOS),
        Goalie   = task.zgoalie(),
        match    = "{ALM}[SDCF]"
},

["getball"] = {
        switch = function ()
                if bufcnt(player.InfoControlled("Assister") and player.toTargetDist("Leader") < 30, 30, 120) then
                        return "chipball"
                end
        end,
        Assister = task.slowGetBall(CGeoPoint:new_local(450, 0)),
        Center   = task.goCmuRush(WaitPos[3], _, _, DSS_FLAG),
        Fronter  = task.goCmuRush(WaitPos[1], _, _, DSS_FLAG),
        Special  = task.goCmuRush(WaitPos[2], _, _, DSS_FLAG),
        Defender = task.goCmuRush(WaitPos[4], _, _, DSS_FLAG),   
        Leader   = task.goSpeciPos(Field_RobotFoward_Pos("Leader")),
        Middle   = task.singleBack(),
        Goalie   = task.zgoalie(),
        match    = "{AL}{S}[DMCF]"
},      

["chipball"] = {
    switch = function ()
                if bufcnt(player.kickBall("Assister") or
                                  player.toBallDist("Assister") > 20, "fast") then
                        return "waitball"
                elseif  bufcnt(true,60) then
                        return "exit"
                end
        end,
        Assister = task.zget(_,_,400,flag.kick+flag.chip),--task.chipPass(CGeoPoint:new_local(450, 20), 110),--chip的力度
        Leader   = task.goSpeciPos(Field_RobotFoward_Pos("Leader")),
        Center   = task.goCmuRush(WaitPos[3], _, _, DSS_FLAG),
        Fronter  = task.goCmuRush(WaitPos[1], _, _, DSS_FLAG),
        Special  = task.goCmuRush(WaitPos[2], _, _, DSS_FLAG),
        Defender = task.goCmuRush(WaitPos[4], _, _, DSS_FLAG),
        Middle   = task.singleBack(),
        Goalie   = task.zgoalie(),
        match    = "{AL}{S}[DMCF]"
},

["waitball"] = {
    switch = function ()
                if bufcnt( math.abs(Utils.Normalize((player.dir("Leader") - player.toBallDir("Leader"))))< math.pi / 4, "normal") then
                        return "shootball"
                elseif  bufcnt(true, 40) then
                        return "exit"
                end
        end,
        Leader   = task.goSpeciPos(PreGoPos("Leader")),
        Assister = task.rightBack(),
        Center   = task.goCmuRush(WaitPos[3], _, _, DSS_FLAG),
        Fronter  = task.goCmuRush(WaitPos[1], _, _, DSS_FLAG),
        Special  = task.goCmuRush(WaitPos[2], _, _, DSS_FLAG),
        Defender = task.goCmuRush(WaitPos[4], _, _, DSS_FLAG),	
        Middle   = task.leftBack(),
        Goalie   = task.zgoalie(),
        match    = "{AL}{S}[DMCF]"
},

["shootball"] = {
    switch = function ()
                if bufcnt(false, 1, 60) then
                        return "exit"
                end
        end,
        Assister = task.rightBack(),
        Leader   = task.chaseNew(),
        Center   = task.goCmuRush(WaitPos[3], _, _, DSS_FLAG),
        Fronter  = task.goCmuRush(WaitPos[1], _, _, DSS_FLAG),
        Special  = task.goCmuRush(WaitPos[2], _, _, DSS_FLAG),
        Defender = task.goCmuRush(WaitPos[4], _, _, DSS_FLAG),  
        Middle   = task.leftBack(),
        Goalie   = task.zgoalie(),
        match    = "{LS}[ADMCF]"
},

name = "Ref_BackKickV802",
applicable ={
        exp = "a",
        a   = true
},
attribute = "attack",
timeout = 99999
}
