gOppoConfig = {
	CornerKick  = {801,802,803,804,805},      --{"TestDynamicKickPickVersion"},
	FrontKick   = {801,802,803,804,805},
	MiddleKick  = {"Ref_FrontKickV1901"}, --{"Ref_FrontKickV804"},
	BackKick    = {801, 802},
	CenterKick  = function()
	    if ball.refPosX() > 0 * param.pitchLength then
	        return {"Ref_FrontKickV802"}
	    else
	        return {"Ref_FrontKickV1901"}
	    end
	end,
	KickOff     = "Ref_KickOffV801",    --V1挑门

	CornerDef   = "Ref_CornerDefV1",   -- V1防头球 V2全盯人
	BackDef     = "Ref_BackDefV1",
	MiddleDef   = "Ref_MiddleDefV1",
	FrontDef    = "Ref_FrontDefV1",
	KickOffDef  = "Ref_KickOffDefV1",

	PenaltyKick = "Ref_PenaltyKickV3", --"Ref_PenaltyKickV3",   --Ref_PenaltyKick2017V2
	PenaltyDef  = "Ref_PenaltyDefV1",    --V2
	
	NorPlay     = "NormalPlayMessi",
	IfHalfField = false,
	USE_ZPASS = false
}