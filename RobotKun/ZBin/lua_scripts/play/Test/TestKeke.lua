-- local KICK_SETTING = CGetSettings("ZAutoFit/chipparamtest","Bool")
-- local IS_SIMULATION = CGetIsSimulation()
local kick_power = 0
local kicker_number = 0
local kicked = false
local kicker_side = 1
local kick_target = CGeoPoint:new_local(0,0)
local KICK_FLAG = flag.kick
local max_power = 120.00
local min_power = 30.00
local step_power = 10
local nkick = 0  

-- if KICK_SETTING then
-- 	KICK_FLAG = flag.kick + flag.chip
-- 	max_power = 90.00
-- 	min_power = 30.00
-- else
-- 	KICK_FLAG = flag.kick
-- 	max_power = 120
-- 	min_power = 30
-- end

local get_kicker_num = function()
	return kicker_number
end

local power = function()
    return function()
    	-- if not IS_SIMULATION then 
        	-- if step_power*nkick + min_power > 125 then
         --    	return 120
        	-- else 
         --    	return (step_power*nkick + min_power);
        	-- end

    		return 3000
	end
end

local exist_nums = {}

local append_side = 1
local left_side = {}
local right_side = {}

local left_next_index = 1
local right_next_index = 1


local player_side = {}

local CENTER = CGeoPoint:new_local(0,0)

local OFFSET = CVector:new_local(2000,2000)

local ROBOT_OFFSET = CVector:new_local(200,200)

local FLAG = flag.dodge_ball + flag.our_ball_placement

local get_wait_num = function(side,num)
	return function()
		if side > 0 then
			if num >= table.getn(left_side) then
				return -1
			end
			local index = (num + left_next_index-1) % table.getn(left_side) + 1
			local num = left_side[index]
			if num == kicker_number then return -1 end
			return num
		else
			if num >= table.getn(right_side) then
				return -1
			end
			local index = (num + right_next_index-1) % table.getn(right_side) + 1
			local num = right_side[index]
			if num == kicker_number then return -1 end
			return num
		end
	end
end

local get_wait_pos = function(side,num)
	return function()
		local pos = CENTER+(OFFSET+ROBOT_OFFSET*num)*side
		return pos
	end
end

local get_wait_dir = function(side,num)
	return function()
		-- local pos = CENTER+(OFFSET+ROBOT_OFFSET*num)*side
		local pos = CENTER+(OFFSET)*side
		local d = (CENTER-pos):dir()
		return d
	end
end

local init = function()
	exist_nums = {}
	for i=0,param.maxPlayer-1 do
		if player.valid(i) then
			table.insert(exist_nums,i)
			if append_side > 0 then
				table.insert(left_side,i)
			else
				table.insert(right_side,i)
			end
			append_side = -append_side
		end
	end
	if table.getn(exist_nums) <= 0 then
		print("Error : no robots in field?")
	end
	kicker_number = left_side[table.getn(left_side)]
	kicker_side = 1
	kick_power = 4000
end

local process = function()

end

local STATE = 1
local FETCH_BALL = 1
local WAIT_KICK = 2
local GOTO_POS=3

local set_next_func = function()
	if kicked == true then
		return
	end
	kicked = true
	kicker_side = -kicker_side
	local lens = table.getn(exist_nums)
	local next_kicker = 0
	if kicker_side > 0 then
		next_kicker = left_side[left_next_index]
		left_next_index = left_next_index % table.getn(left_side) + 1
		if left_next_index == 1 then
			nkick = nkick + 1 
		end
	else
		next_kicker = right_side[right_next_index]
		right_next_index = right_next_index % table.getn(right_side) + 1
	end
	kicker_number = next_kicker
end

local KICKER_TASK = function()
	target_pos = CENTER
	wait_pos = CENTER+(OFFSET - ROBOT_OFFSET)*kicker_side

	local goto_dir = (target_pos - wait_pos):dir()

	local dist = player.pos(kicker_number):dist(wait_pos)
	local dir_diff = Utils.Normalize(player.dir(kicker_number)-goto_dir)
	local infraredOn = player.infraredCount(kicker_number)

	if player.kickBall(kicker_number) or infraredOn <= 1 then
		STATE = FETCH_BALL
	end
	if infraredOn > 20 and dist < 200 then
		STATE = GOTO_POS
		if dist < 200 then
			if bufcnt( dir_diff < math.pi/180*8, 20, 900) and player.rotVel(kicker_number) < 0.1 then
				STATE = WAIT_KICK
			end
		end
	end
	if STATE == FETCH_BALL then
		return task.fetchBall(wait_pos,_,true)
	elseif STATE == GOTO_POS then
		return task.goCmuRush(wait_pos,goto_dir,_,flag.dribbling)
	elseif STATE == WAIT_KICK then
		return task.zget(target_pos,_,power(),KICK_FLAG)
	end
end

local printDebug = function()
	local left_debug = "left : "
	for i=1,table.getn(left_side) do
		left_debug = left_debug .. left_side[i] .. " "
	end
	local right_debug = "right : "
	for i=1,table.getn(right_side) do
		right_debug = right_debug .. right_side[i] .. " "
	end
	-- print(left_debug)
	-- print(right_debug)
	-- print("index : ",left_next_index,right_next_index)
	print("kicker : ",kicker_number,"side : ",kicker_side, "kick power",(power())())
	local left_stand = "lstan : "
	for i=0,7 do
		left_stand = left_stand .. get_wait_num(1,i)() .. " "
	end
	local right_stand = "rstan : "
	for i=0,7 do
		right_stand = right_stand .. get_wait_num(-1,i)() .. " "
	end
	-- print(left_stand)
	-- print(right_stand)
	-- print("-----------------------")
end

gPlayTable.CreatePlay{

firstState = "init",

["init"] = {
	switch = function()
		init()
		printDebug()
		return "test"
	end,
	match = ""
},

["run"] = {
	switch = function()
		-------------- use for debugging
		if bufcnt(true,60) then
			printDebug()
			set_next_func()
			return "run"
		else
			kicked = false
		end
	end,
	-- [0] = KICKER_TASK,
	match = ""
},

["test"] = {
	switch = function()
		if bufcnt(player.kickBall(kicker_number), 1, 1300) then
			set_next_func()
			printDebug()
			return "test"
		else
			debugEngine:gui_debug_msg(kick_target,string.format("%d",(power())()),3)
			kicked = false
		end
	end,
	[get_kicker_num] = KICKER_TASK,
	[get_wait_num(-1,0)] = task.goCmuRush(get_wait_pos(-1,0),get_wait_dir(-1,0),_,FLAG),
	[get_wait_num(-1,1)] = task.goCmuRush(get_wait_pos(-1,1),get_wait_dir(-1,1),_,FLAG),
	[get_wait_num(-1,2)] = task.goCmuRush(get_wait_pos(-1,2),get_wait_dir(-1,2),_,FLAG),
	[get_wait_num(-1,3)] = task.goCmuRush(get_wait_pos(-1,3),get_wait_dir(-1,3),_,FLAG),
	[get_wait_num(-1,4)] = task.goCmuRush(get_wait_pos(-1,4),get_wait_dir(-1,4),_,FLAG),
	[get_wait_num(-1,5)] = task.goCmuRush(get_wait_pos(-1,5),get_wait_dir(-1,5),_,FLAG),
	[get_wait_num(-1,6)] = task.goCmuRush(get_wait_pos(-1,6),get_wait_dir(-1,6),_,FLAG),
	[get_wait_num( 1,0)] = task.goCmuRush(get_wait_pos( 1,0),get_wait_dir( 1,0),_,FLAG),
	[get_wait_num( 1,1)] = task.goCmuRush(get_wait_pos( 1,1),get_wait_dir( 1,1),_,FLAG),
	[get_wait_num( 1,2)] = task.goCmuRush(get_wait_pos( 1,2),get_wait_dir( 1,2),_,FLAG),
	[get_wait_num( 1,3)] = task.goCmuRush(get_wait_pos( 1,3),get_wait_dir( 1,3),_,FLAG),
	[get_wait_num( 1,4)] = task.goCmuRush(get_wait_pos( 1,4),get_wait_dir( 1,4),_,FLAG),
	[get_wait_num( 1,5)] = task.goCmuRush(get_wait_pos( 1,5),get_wait_dir( 1,5),_,FLAG),
	[get_wait_num( 1,6)] = task.goCmuRush(get_wait_pos( 1,6),get_wait_dir( 1,6),_,FLAG),
	[get_wait_num( 1,7)] = task.goCmuRush(get_wait_pos( 1,7),get_wait_dir( 1,7),_,FLAG),
	match = ""
},

name = "TestKeke",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
