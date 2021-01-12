--
gRoleNum = {
	["Goalie"] = 0,
	["Kicker"] = 0,
	["Assister"] = 0,
	["Special"] = 0,
	["Defender"] = 0,
	["Middle"] = 0,
	["Leader"] = 0,
	["Tier"] = 0,
	["Breaker"] = 0,
	["Fronter"] = 0,
	["Receiver"] = 0,
	["Center"]  = 0,
	["a"] = 0,
	["b"] = 0,
	["c"] = 0,
	["d"] = 0,
	["e"] = 0,
	["f"] = 0,
	["g"] = 0,
	["h"] = 0,
	["i"] = 0,
	["j"] = 0,
	["k"] = 0,
	["l"] = 0,
	["m"] = 0,
	["n"] = 0,
	["o"] = 0,
	["p"] = 0,
	["q"] = 0,
	["r"] = 0,
	["s"] = 0,
	["t"] = 0,
	["u"] = 0,
	["v"] = 0,
	["w"] = 0,
	["x"] = 0,
	["y"] = 0,
	["z"] = 0
}

gLastRoleNum = {

}

gRolePos = {

}

gRolePriority = { "Goalie","Kicker","Tier","Receiver"}
-- gRolePriority = { "Goalie","Kicker", "Tier"} --wangzai test


gOurExistNum = {

}

gRoleLookUpTable = {
-- 角色分为两类
-- 第一类为在gRolePriority和gRoleFixNum中进行真实车号配置的
["G"] = "Goalie",
["K"] = "Kicker",
["T"] = "Tier",
["R"] = "Receiver",

-- 第二类为在脚本中可能会发生匹配变化的角色
["A"] = "Assister",
["S"] = "Special",
["D"] = "Defender",
["M"] = "Middle",
["L"] = "Leader",
["B"] = "Breaker",
["F"] = "Fronter",
["C"] = "Center",
["a"] = "a",
["b"] = "b",
["c"] = "c",
["d"] = "d",
["e"] = "e",
["f"] = "f",
["g"] = "g",
["h"] = "h",
["i"] = "i",
["j"] = "j",
["k"] = "k",
["l"] = "l",
["m"] = "m",
["n"] = "n",
["o"] = "o",
["p"] = "p",
["q"] = "q",
["r"] = "r",
["s"] = "s",
["t"] = "t",
["u"] = "u",
["v"] = "v",
["w"] = "w",
["x"] = "x",
["y"] = "y",
["z"] = "z"
}

function DecodeMatchStr(str)
	local strTable = {}

	local teamCnt = 1
	while true do
		local character = string.sub(str, 1, 1)
		local endChar
		local mode
		if character == '[' then
			endChar = ']'
			mode = "RealTime"
		elseif character == '(' then
			endChar = ')'
			mode = "Once"
		elseif character == '{' then
			endChar = '}'
			mode = "Never"
		else
			break
		end

		local endPos, _ = string.find(str, endChar)
		local teamTable = {}
		for subIndex = 2, endPos-1 do
			table.insert(teamTable, gRoleLookUpTable[string.sub(str, subIndex, subIndex)])
		end
		teamTable["mode"] = mode
		strTable[teamCnt] = teamTable
		teamCnt = teamCnt + 1
		str = string.sub(str,endPos+1)
		if str == "" then break end
	end
	return strTable
end

function GetMatchPotential(num, role)
	local targetPos
	if type(gRolePos[role]) == "function" then
		targetPos = gRolePos[role](num)
	else
		targetPos = gRolePos[role]
	end
	local t = world:timeToTarget(num, targetPos)*10
	local t2 = math.pow(t,2)
	local playerPos = player.pos(num)

	if DEBUG_MATCH then
		debugEngine:gui_debug_line(targetPos,playerPos,4)
		debugEngine:gui_debug_msg(playerPos+(targetPos-playerPos)/3,string.format("%.2f",t),3)
	end

	return t2
end

function RemoveExistNum(num)
	for index, value in ipairs(gOurExistNum) do
		if value == num then
			table.remove(gOurExistNum, index)
		end
	end
end

function DoRoleMatchReset(str)
	-- 循环获得需要重新匹配的角色
	while true do
		local character = string.sub(str, 1, 1)
		
		if character ~= '[' then
			break
		end

		local endPos, _ = string.find(str, ']')
		local roleTable = {}
		local numTable = {}
		for subIndex = 2, endPos-1 do
			local roleNameStr = gRoleLookUpTable[string.sub(str, subIndex, subIndex)]
			table.insert(roleTable, roleNameStr)
			table.insert(numTable, gRoleNum[roleNameStr])
		end

		-- 开始对每个分组进行重新的匹配
		local nrows = table.getn(roleTable)
		local ncols = table.getn(numTable)
		local matrix = Matrix_double_:new_local(nrows, ncols)
		for row = 1, nrows do
			for col = 1, ncols do
				matrix:setValue(row-1, col-1, GetMatchPotential(numTable[col], roleTable[row]))
			end
		end

		local m = Munkres:new_local()
		m:solve(matrix)

		for row = 1, nrows do
			for col = 1, ncols do
				if matrix:getValue(row-1, col-1) == 0 then
					gRoleNum[roleTable[row]] = numTable[col]				
					break
				end
			end
		end

		str = string.sub(str,endPos+1)
		if str == "" then break end
	end
end

function DoMunkresMatch(rolePos)
	local nrows = table.getn(rolePos)
	local ncols = table.getn(gOurExistNum)
	local matrix = Matrix_double_:new_local(nrows, ncols)
	for row = 1, nrows do
		for col = 1, ncols do
			matrix:setValue(row-1, col-1, GetMatchPotential(gOurExistNum[col], rolePos[row]))
		end
	end

	local m = Munkres:new_local()
	m:solve(matrix)

	local eraseList = {}
	for row = 1, nrows do	
		for col = 1, ncols do
			if matrix:getValue(row-1, col-1) == 0 then
--~ 				print(rolePos[row], gOurExistNum[col])
				gRoleNum[rolePos[row]] = gOurExistNum[col]
				table.insert(eraseList, gOurExistNum[col])				
				break
			end
		end
	end

	for _, value in ipairs(eraseList) do
		RemoveExistNum(value)
	end
end


function DoFixNumMatch(fixNums)
	for _, fixNum in ipairs(fixNums) do
		for index, carNum in ipairs(gOurExistNum) do
			if carNum == fixNum then  --CGetRealNum(carNum) //test by wz
				-- print("Wzdebug: rolematch", "carNum", carNum, "C++ num" , CGetRealNum(carNum))
				table.remove(gOurExistNum, index)
				return carNum
			end
		end
	end
	return 0
end

function SetNoMatchRoleZero()
	for _, rolename in pairs(gRoleLookUpTable) do
		if gRoleNum[rolename] == nil then
			gRoleNum[rolename] = 0
		end
	end	
end

function UpdateRole(matchTactic, isPlaySwitched, isStateSwitched)
	if isPlaySwitched then
		gRoleNum = {}
	end
	--print("---------------------------------")
	local ourExistNum = {}

	for i = 1, param.maxPlayer do
		if player.valid(i) then
			table.insert(ourExistNum, i)
		end
	end

	gOurExistNum = ourExistNum
		
	for _, rolename in pairs(gRolePriority) do
		for existname, _ in pairs(gRolePos) do
			if rolename == existname and
				type(gRoleFixNum[rolename]) == "table" then
				gRoleNum[rolename] = DoFixNumMatch(gRoleFixNum[rolename])
				if rolename == "Goalie" then
					CRegisterRole(gRoleNum[rolename], "goalie")
				end
			end
		end
	end
	
	local matchList = {}
	for tactic, value in pairs(matchTactic) do
		if value.mode == "RealTime" or isPlaySwitched or
			(isStateSwitched and value.mode == "Once") then
			table.insert(matchList, value)
		else
			for _, rolename in ipairs(value) do
				gRoleNum[rolename] = gLastRoleNum[rolename]
				RemoveExistNum(gRoleNum[rolename])
			end
		end
	end
	
	for _, value in ipairs(matchList) do
		local role = {}
		for index = 1, table.getn(value) do
			local haveMatchRobot = false
			for rolename, pos in pairs(gRolePos) do
				if value[index] == rolename and
					table.getn(gOurExistNum) >= index then
					table.insert(role, rolename)
					haveMatchRobot = true
					break
				end
			end
			if not haveMatchRobot then
				gRoleNum[value[index]] = 0
			end
		end
		DoMunkresMatch(role)
	end
	
	SetNoMatchRoleZero()
	gLastRoleNum = gRoleNum	
end
