local vel_table = {} 
local x_table = {}
local y_table = {}
local friction_table = {}
local RATE = 71
local start_time = 50    
local end_time = 150
local gap = 20 
local friction = 1500 
local num_threshold = 50
local average_number = 5
local average_friction_table = {}
local average_friction = friction
local key

for i = 1,average_number do 
	table.insert(average_friction_table, friction)
end

function getAverage(tab)
	local num = 0
	local sum = 0
	for key, i in ipairs(tab) do
		sum = sum + i
		num = num + 1
	end

	return sum/num
end

function getNum(tab)
	local num = 0
	for key, i in ipairs(tab) do
		num = num + 1
	end

	return num
end


gPlayTable.CreatePlay{

firstState = "testState1",
["testState1"] = { 
	switch   = function()
		vel_table = {}
		friction_table = {}
		x_table = {}
		y_table = {}
		debugEngine:gui_debug_msg(CGeoPoint:new_local(-1000,1000), "friction:" .. friction)
		debugEngine:gui_debug_msg(CGeoPoint:new_local(-1000,1200), "average friction in the latest " .. average_number .. " times: " .. average_friction)
		if bufcnt(ball.valid(),15) and ball.velMod() ~= 0 then
			return "testState2"
		end
	end,
	match = ""
},

["testState2"] = { 

	switch   = function()
		table.insert(vel_table, ball.velMod())
		table.insert(x_table, ball.posX())
		table.insert(y_table, ball.posY())
		local dist1 = ball.velMod()*ball.velMod() / friction
		local dist2 = ball.velMod()*ball.velMod() / 1520

		--152simulation
		debugEngine:gui_debug_x(ball.pos() + Utils.Polar2Vector(dist1, ball.velDir()))
		debugEngine:gui_debug_msg(CGeoPoint:new_local(-1000,1000), "friction:" .. friction)
		debugEngine:gui_debug_msg(CGeoPoint:new_local(-1000,1200), "average friction in the latest " .. average_number .. " times: " .. average_friction)
		-- debugEngine:gui_debug_x(ball.pos() + Utils.Polar2Vector(dist2, ball.velDir()))

		if bufcnt(not ball.valid(),10) then 
			return "testState1"
		end
		if ball.velMod() == 0 then 
			return "testState3"
		end

	end,
	match    = ""
},

["testState3"] = { 
	switch   = function()
		if getNum(vel_table) < end_time+gap then 
			debugEngine:gui_debug_msg(CGeoPoint:new_local(-1000,1000), "Fricition Test error because of sudden interruption or the path is too short!!!")
			-- print("Fricition Test error because of sudden interruption or the path is too short!!!")
			return "testState1"
		end

		--use basic physics formula: v^2-v0^2=2ax
		--use successional difference gap to reduce the error
		for i = start_time, end_time do
			delta_x = math.sqrt(math.pow(x_table[i]-x_table[i+gap],2) + math.pow(y_table[i]-y_table[i+gap],2))
			acc = (math.pow(vel_table[i],2) - math.pow(vel_table[i+gap],2)) / (2*delta_x)
			table.insert(friction_table, acc)

		end

		friction = 2 * getAverage(friction_table)

		--calculate average friction of certain times to reduce error
		table.remove(average_friction_table, 1)
		table.insert(average_friction_table, friction)
		average_friction = getAverage(average_friction_table)

		-- print("friction:", friction)
		debugEngine:gui_debug_msg(CGeoPoint:new_local(-1000,1000), "friction:" .. friction)
		debugEngine:gui_debug_msg(CGeoPoint:new_local(-1000,1200), "average friction in the latest " .. average_number .. " times: " .. average_friction)
		-- print("average friction in the latest" .. average_number .. "times: ", average_friction)
		return "testState1"
	end,
	match = ""
},

name = "TestForFrictionAuto",
applicable ={
        exp = "a",
        a = true
},
attribute = "attack",
timeout   = 99999
}
