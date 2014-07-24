--[[
Copyright (C) 2014, fanzyflani. All rights reserved.
See LICENCE.txt for licensing information.
]]

DEBUG = false

local dir = {
	south = 0,
	east = 1,
	north = 2,
	west = 3,
}

local dir_x = { 0, 1, 0,-1}
local dir_y = { 1, 0,-1, 0}
local dir_diag_x = { 0, 1, 0,-1, 1, 1,-1,-1}
local dir_diag_y = { 1, 0,-1, 0, 1,-1,-1, 1}

local ai = {
	wait = 10,
	strat = {},
}

dprint = (DEBUG and print) or function()end

function getdir(gstate, layer, bx, by, skipobj, mask)
	mask = mask or {floor=true}
	local offs = 0 --math.floor(math.random()*4) -- must be predictable!
	local i

	for i=0,4-1 do
		local j = (i + offs) % 4
		local dx = dir_x[j+1]
		local dy = dir_y[j+1]
		local cx = bx + dx -- NO THIS ISN'T X86 ASM
		local cy = by + dy

		local ay = gstate.layers[layer]

		local ce = ay and ay.data[cy] and ay.data[cy][cx]
		--dprint(ce and ce.has_ob, ce and ce.ctyp, ce and mask[ce.ctyp])
		
		if ce and mask[ce.ctyp] and (skipobj or not ce.has_ob) then
			return dx, dy
		end
	end

	return nil, nil
end

function should_crouch(gstate, ob)
	local i

	for i=1,8 do
		local dx = dir_diag_x[i]
		local dy = dir_diag_y[i]
		local cx = ob.cx + dx
		local cy = ob.cy + dy

		local ay = gstate.layers[ob.layer]

		local ce = ay and ay.data[cy] and ay.data[cy][cx]
		
		dprint(ce and ce.ctyp)
		if ce and ce.ctyp == "table" then
			dprint("SHOULD CROUCH")
			return true
		end
	end

	return false
end

function hook_move(gstate)
	local i, j, ob
	local mob, wob, aob
	local wob_dirx, wob_diry
	local wob_best = 10000000
	local aob_best = 10000000

	local teams = {}
	local enemies = {}

	-- Sort players into teams
	for i=1,gstate.ocount do
		ob = gstate.objects[i]
		if not ob then
		elseif ob.otyp == "player" then
			if not teams[ob.fd.team] then teams[ob.fd.team] = {} end
			ob.idx = i

			if ob.fd.team ~= gstate.tid then
				table.insert(enemies, ob)
			end

			if ob.fd.team ~= gstate.tid or ob.steps_left > 0 then
				table.insert(teams[ob.fd.team], ob)
			end
		end
	end

	-- Get our team
	local ourteam = teams[gstate.tid]

	-- If we have no more moves left, NEXT TURN
	if #ourteam == 0 then
		return "newturn"
	end

	-- Pick the first object in our team that can move
	local dirx, diry
	for i=1,#ourteam do
		mob = ourteam[i]

		-- If we are crouching, and it's not our last step, stand
		if mob.steps_left > 2 and mob.flags.crouch then
			return "stand", mob.cx, mob.cy
		end

		dirx, diry = getdir(gstate, mob.layer, mob.cx, mob.cy)
		if dirx then
			-- If we have enough steps,
			-- find the weakest enemy with a line of sight.
			aob = nil
			if mob.steps_left >= 3 then
				for j, ob in pairs(enemies) do
					if mob.layer == ob.layer then
						local los, lx, ly = game.line_layer(mob.layer,
							mob.cx, mob.cy, ob.cx, ob.cy)

						if los then
							local weight = ob.health
							if weight < aob_best then
								aob_best = weight
								aob = ob
							end
						end
					end
				end
			end

			-- If we found something, attack it!
			if aob then
				return "attack", mob.cx, mob.cy, aob.cx, aob.cy
			end

			-- If we aren't about to crouch, find the nearest / weakest enemy.
			wob = nil
			if mob.steps_left > 1 or not should_crouch(gstate, mob) then
				for j, ob in pairs(enemies) do
					local dx, dy = getdir(gstate, ob.layer, ob.cx, ob.cy)
					if dx and mob.layer == ob.layer then
						nx, ny = ob.cx + dx, ob.cy + dy
						local astar = game.astar_layer(mob.layer,
							mob.cx, mob.cy, nx, ny)
						if astar then
							local weight = ob.health * (#astar * #astar)
							if weight < wob_best then
								wob_best = weight
								wob_dirx = dir_x[astar[1]+1]
								wob_diry = dir_y[astar[1]+1]
								wob = ob
							end
						end
					end
				end
			end

			-- If we found a route, run to it!
			if wob then
				return "move", mob.cx, mob.cy, mob.cx + wob_dirx, mob.cy + wob_diry
			end

			-- Crouch if need be.
			if mob.steps_left <= 1 and should_crouch(gstate, mob) then
				return "crouch", mob.cx, mob.cy
			end
		end
	end

	-- Nothing left? Move on.
	return "newturn"
end

do
	local s_hook_move = hook_move
	local function catchcall(...)
		dprint("ret:", ...)
		return ...
	end

	function hook_move(...)
		return catchcall(s_hook_move(...))
	end
end

