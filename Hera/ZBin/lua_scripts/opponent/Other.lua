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
	KickOff     = "Ref_KickOffV601",    --V1挑门

	CornerDef   = "Ref_CornerDef_6vs6",   -- V1防头球 V2全盯人
	BackDef     = "Ref_BackDefV1_6vs6",
	MiddleDef   = "Ref_MiddleDef_6vs6",
	FrontDef    = "Ref_FrontDef_6vs6",
	KickOffDef  = "Ref_KickOffDef_6vs6",

	PenaltyKick = "Ref_PenaltyKickV3_6V6", --"Ref_PenaltyKickV3",   --Ref_PenaltyKick2017V2
	PenaltyDef  = "Ref_PenaltyDefV1_6V6",    --V2
	
	NorPlay     = "NormalPlayMessi_6vs6",
	IfHalfField = false,
	USE_ZPASS = false
}