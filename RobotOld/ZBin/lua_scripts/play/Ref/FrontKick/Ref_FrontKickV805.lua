-- try zshoot
local DSS_FLAG = flag.allow_dss + flag.dodge_ball

local PRE_PASS_POS = ball.refAntiYPos(CGeoPoint:new_local(200,80))
local PASS_POS = CGeoPoint:new_local(180, -100)

local WAIT_POS = ball.refAntiYPos(PASS_POS)
local FAKE_POS = ball.refSyntYPos(CGeoPoint:new_local(120, 100))
local FrontPosX = 100
local FrontPosY = -150
local Dist = 25
local CHIP_BUF
local BALL_ORIGIN_POS
local PRE_POS = {
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*0, FrontPosY+Dist*5)),
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*1, FrontPosY+Dist*4)),
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*2, FrontPosY+Dist*3)),
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*3, FrontPosY+Dist*2)),
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*4, FrontPosY+Dist*1)),
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*5, FrontPosY+Dist*0)),
}

local POS = {
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*0, FrontPosY+Dist*0)),
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*1, FrontPosY+Dist*1)),
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*2, FrontPosY+Dist*2)),
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*3, FrontPosY+Dist*3)),
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*4, FrontPosY+Dist*4)),
  ball.refSyntYPos(CGeoPoint:new_local(FrontPosX+Dist*5, FrontPosY+Dist*5)),
}

gPlayTable.CreatePlay{
firstState = "start",

["start"] = {
	switch = function()
    	        if  bufcnt(player.toTargetDist("Leader") < 20, 200) then
                        CHIP_BUF = cp.getFixBuf(WAIT_POS)
                        BALL_ORIGIN_POS = ball.pos()
                        return "pass"
                end
	end,
	Assister = task.staticGetBall(PASS_POS,false),
	Leader   = task.goCmuRush(PRE_PASS_POS,player.toPlayerDir("Assister"),_, DSS_FLAG+flag.free_kick),
    Special  = task.goCmuRush(PRE_POS[2],_,_, DSS_FLAG),
    Middle   = task.goCmuRush(PRE_POS[3],_,_, DSS_FLAG),
    Fronter  = task.goCmuRush(PRE_POS[4],_,_, DSS_FLAG),
    Center   = task.goCmuRush(PRE_POS[5],_,_, DSS_FLAG),
    Defender = task.goCmuRush(PRE_POS[6],_,_, DSS_FLAG),
    Goalie   = task.zgoalie(),
    match    = "(A)[L][SMFCD]"
},
["pass"] = {
        switch = function()
                if  bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 30, 1) then
                        return "fix"
                end
        end,
        Assister = task.zget(WAIT_POS,_,_,flag.kick+flag.chip,true),
        Leader   = task.goCmuRush(WAIT_POS,player.toPlayerDir("Assister")),
        Special  = task.goCmuRush(POS[2],_,_, DSS_FLAG),
        Middle   = task.goCmuRush(POS[3],_,_, DSS_FLAG),
        Fronter  = task.goCmuRush(POS[4],_,_, DSS_FLAG),
        Center   = task.goCmuRush(POS[5],_,_, DSS_FLAG),
        Defender = task.goCmuRush(POS[6],_,_, DSS_FLAG),
        Goalie   = task.zgoalie(),
        match    = "{AL}{SMFCD}"
},
["fix"] = {
        switch = function()
                if  bufcnt(true, CHIP_BUF) then
                        return "get"
                end
        end,
	Assister = task.goCmuRush(FAKE_POS,_,_, DSS_FLAG),
        Leader   = task.goCmuRush(WAIT_POS,player.toPlayerDir("Assister")),
        Special  = task.goCmuRush(POS[2],_,_, DSS_FLAG),
        Middle   = task.goCmuRush(POS[3],_,_, DSS_FLAG),
        Fronter  = task.goCmuRush(POS[4],_,_, DSS_FLAG),
        Center   = task.goCmuRush(POS[5],_,_, DSS_FLAG),
        Defender = task.goCmuRush(POS[6],_,_, DSS_FLAG),
        Goalie   = task.zgoalie(),
        match    = "{AL}{SMFCD}"
},
["get"] = {
	switch = function()
        	if bufcnt(player.infraredOn("Leader"),3,210) then
        		return "zshoot"
            elseif bufcnt(true,200) then
                return "exit"
            end
        end,
        Assister = task.goCmuRush(FAKE_POS,_,_, DSS_FLAG),
        Leader   = task.receivePass("Assister",0),--task.zget(BALL_ORIGIN_POS,WAIT_POS,0,flag.dribble),
        Special  = task.goCmuRush(POS[2],_,_, DSS_FLAG),
        Middle   = task.goCmuRush(POS[3],_,_, DSS_FLAG),
        Fronter  = task.defendHead(),
        Center   = task.leftBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "{AL}[SMFCD]"
},
["zshoot"] = {
	switch = function()
		if bufcnt(player.kickBall("Leader"),1,260) then
			return "exit"
		end
        end,
        Assister = task.goCmuRush(FAKE_POS,_,_, DSS_FLAG),
        Leader   = task.zpass(),
        Special  = task.goCmuRush(POS[2],_,_, DSS_FLAG),
        Middle   = task.goCmuRush(POS[3],_,_, DSS_FLAG),
        Fronter  = task.defendHead(),
        Center   = task.leftBack(),
        Defender = task.rightBack(),
        Goalie   = task.zgoalie(),
        match    = "{AL}[SMFCD]"
},

name = "Ref_FrontKickV805",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
