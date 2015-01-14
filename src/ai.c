/*
Parsnip Theory
Copyright (c) 2014, fanzyflani

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//define DEBUG_AI

#include "common.h"

void ai_free(ai_t *ai)
{
	// Free AI
	free(ai);
}

void ai_normal_do_move(ai_t *ai)
{
	obj_t *ob, *tob;
	int i, j;
	int asx, asy;
	obj_t *best_tob;
	obj_t *best_sob;
	int best_tob_dist;
	int best_sob_dist;
	int dirbuf[512];

	// Wait
	if(--ai->wait > 0) return;

	// Get stuff
	game_t *game = ai->game;

	// Block input if waiting for object
	if(level_obj_waiting(game->lv) != NULL)
	{
		return;
	}

	// Scan for objects that belong to us

	for(i = 0; i < game->lv->ocount; i++)
	{
		// Get object
		ob = game->lv->objects[i];

		// Check constraints
		if(ob == NULL) continue;
		struct fd_player *fde = (struct fd_player *)ob->f.fd;
		assert(fde != NULL);
		if(fde->team != ai->tid) continue;
		if(ob->steps_left <= 0) continue;

		// Find a player to harrass
		best_tob_dist = 0x10000000;
		best_tob = NULL;
		best_sob_dist = 0x10000000;
		best_sob = NULL;
		for(j = 0; j < game->lv->ocount; j++)
		{
			// Get object
			tob = game->lv->objects[j];

			// Check constraints
			if(tob == NULL) continue;
			struct fd_player *fde = (struct fd_player *)tob->f.fd;
			assert(fde != NULL);
			if(fde->team == ai->tid) continue;

			// Find nearby tile we can touch
			int tob_candidate = 0;
			if(find_free_neighbour_layer(game->lv->layers[ob->f.layer],
				tob->f.cx, tob->f.cy, &asx, &asy))
			if(astar_layer(game->lv->layers[ob->f.layer], dirbuf, 1024,
				ob->f.cx, ob->f.cy, asx, asy) >= 1) tob_candidate = 1;

			// Find if there's a line of sight
			int sob_candidate = 0;
			int rx, ry;
			if(line_layer(game->lv->layers[ob->f.layer], &rx, &ry,
				ob->f.cx, ob->f.cy, tob->f.cx, tob->f.cy))
					sob_candidate = 1;

			// Get distance to object
			int dx = tob->f.cx - ob->f.cx;
			int dy = tob->f.cy - ob->f.cy;
			int dist = dx*dx + dy*dy;
			dist *= dist * tob->health;

			if(tob_candidate)
			if(best_tob == NULL || dist < best_tob_dist)
			{
				best_tob_dist = dist;
				best_tob = tob;
			}

			dist = tob->health;
			if(sob_candidate)
			if(best_sob == NULL || dist < best_sob_dist)
			{
				best_sob_dist = dist;
				best_sob = tob;
			}
		}

		//printf("found enemy %i\n", best_tob_dist);

		// Check if we have a line of sight toward this object
		// (as well as enough turns to throw stuff)
		// Also check if we are going for the guy we have a LoS for
		int rx, ry;
		if(best_sob != NULL && line_layer(game->lv->layers[ob->f.layer], &rx, &ry,
			ob->f.cx, ob->f.cy, best_sob->f.cx, best_sob->f.cy))
				tob = best_sob;
		else
			tob = best_tob;
		// If we don't have a "best object" to harass, continue
		if(tob == NULL) continue;


		if(ob->steps_left >= STEPS_ATTACK+1 && tob->health > 0)
		if(line_layer(game->lv->layers[ob->f.layer], &rx, &ry,
			ob->f.cx, ob->f.cy, tob->f.cx, tob->f.cy))
		{
			// Throw!

#ifdef DEBUG_AI
			printf("enemy LoS\n");
#endif

			game->camx = tob->f.cx*32+16 - screen_width/2;
			game->camy = tob->f.cy*24+12 - screen_height/2;
			if(game->camx < 0) game->camx = 0;
			if(game->camy < 0) game->camy = 0;
			// TODO: not hardcode this
			if(game->camx > 32*40 - screen_width) game->camx = 32*40 - screen_width;
			if(game->camy > 32*40 - screen_width) game->camy = 32*40 - screen_width;
			game->mx = tob->f.cx*32+16 - game->camx;
			game->my = tob->f.cy*24+12 - game->camy;
			game_push_hover(game, ai->ab, game->mx, game->my, game->camx, game->camy);

			abuf_write_u8(ACT_SELECT, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);

#ifdef DEBUG_AI
			printf("attack %i %i -> %i %i\n", ob->f.cx, ob->f.cy, tob->f.cx, tob->f.cy);
#endif
			abuf_write_u8(ACT_ATTACK, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);
			abuf_write_s16(tob->f.cx, ai->ab);
			abuf_write_s16(tob->f.cy, ai->ab);
			abuf_write_s16(STEPS_ATTACK, ai->ab);
			abuf_write_s16(ob->steps_left - STEPS_ATTACK, ai->ab);

			ai->wait = 15;

			return;
		}
		
		// Check if we can move to this object
		tob = best_tob;
		if(tob == NULL) continue;
		if(!find_free_neighbour_layer(game->lv->layers[ob->f.layer],
			tob->f.cx, tob->f.cy, &asx, &asy)) continue;
		int asl = astar_layer(game->lv->layers[ob->f.layer], dirbuf, 512,
			ob->f.cx, ob->f.cy, asx, asy);

#ifdef DEBUG_AI
		printf("A* trace: %i\n", asl);
#endif

		if(asl >= 1)
		{
			game->camx = ob->f.cx*32+16 - screen_width/2;
			game->camy = ob->f.cy*24+12 - screen_height/2;
			if(game->camx < 0) game->camx = 0;
			if(game->camy < 0) game->camy = 0;
			// TODO: not hardcode this
			if(game->camx > 32*40 - screen_width) game->camx = 32*40 - screen_width;
			if(game->camy > 32*40 - screen_width) game->camy = 32*40 - screen_width;

			asl = 1;
			if(asl > ob->steps_left)
				asl = ob->steps_left;

			abuf_write_u8(ACT_SELECT, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);

			int tx = ob->f.cx;
			int ty = ob->f.cy;

			for(j = 0; j < asl; j++)
			switch(dirbuf[j])
			{
				case DIR_SOUTH: ty++; break;
				case DIR_EAST:  tx++; break;
				case DIR_NORTH: ty--; break;
				case DIR_WEST:  tx--; break;
			}

			game->mx = tx*32+16 - game->camx;
			game->my = ty*24+12 - game->camy;
			game_push_hover(game, ai->ab, game->mx, game->my, game->camx, game->camy);

#ifdef DEBUG_AI
			printf("move %i %i -> %i %i\n", ob->f.cx, ob->f.cy, tx, ty);
#endif
			abuf_write_u8(ACT_MOVE, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);
			abuf_write_s16(tx, ai->ab);
			abuf_write_s16(ty, ai->ab);
			abuf_write_s16(asl, ai->ab);
			abuf_write_s16(ob->steps_left - asl, ai->ab);

			return;

		}

	}

	// All done! Force a change.
	game_push_newturn(game, ai->ab, -1, ai->tid);
}

void ai_lua_do_move(ai_t *ai)
{
	int x, y, i;
	int tx, ty;
	obj_t *ob;

	// Initialise Lua if need be
	lua_State *L = ai->L;

	if(L == NULL)
	{
		L = ai->L = luaL_newstate();
		luaL_openlibs(L);

		lua_newtable(L);
		lua_pushlightuserdata(L, ai->game);
		lua_pushcclosure(L, lint_f_line_layer, 1);
		lua_setfield(L, -2, "line_layer");
		lua_pushlightuserdata(L, ai->game);
		lua_pushcclosure(L, lint_f_astar_layer, 1);
		lua_setfield(L, -2, "astar_layer");
		lua_setglobal(L, "game");

		if(luaL_dofile(L, "lua/ai/main.lua"))
		{
			printf("ERROR: AI failed to load!\n");
			printf("Lua says: %s\n", lua_tostring(L, -1));
			printf("Reverting to C behaviour.\n");
			ai->f_do_move = ai_normal_do_move;
			return ai_normal_do_move(ai);
		}
	}

	// Get stuff
	game_t *game = ai->game;

	// Block input if waiting for object
	if(level_obj_waiting(game->lv) != NULL)
	{
		return;
	}

	// Wait
	if(--ai->wait > 0) return;

	// Push function
	lua_getglobal(L, "hook_move");

	// Construct a Lua table
	lua_newtable(L);

	// Set some stuff
	lua_pushinteger(L, ai->tid);
	lua_setfield(L, -2, "tid");
	lua_pushlightuserdata(L, game);
	lua_setfield(L, -2, "ghandle");

	// TABLE: Layers
	lua_newtable(L);

	for(i = 0; i < game->lv->lcount; i++)
	{
		layer_t *ay = game->lv->layers[i];

		lua_pushinteger(L, i+1);
		lua_newtable(L);
		lua_pushinteger(L, ay->w);
		lua_setfield(L, -2, "w");
		lua_pushinteger(L, ay->h);
		lua_setfield(L, -2, "h");
		lua_pushinteger(L, ay->x);
		lua_setfield(L, -2, "x");
		lua_pushinteger(L, ay->y);
		lua_setfield(L, -2, "y");

		lua_newtable(L);
		for(y = 0; y < ay->h; y++)
		{
			lua_pushinteger(L, y + ay->y);
			lua_newtable(L);
			for(x = 0; x < ay->w; x++)
			{
				cell_t *ce = &ay->data[y*ay->w + x];

				lua_pushinteger(L, x + ay->x);
				lua_newtable(L);

				// Cell data START
				switch(ce->f.ctyp)
				{
					case CELL_OOB: lua_pushstring(L, "oob"); break;
					case CELL_FLOOR: lua_pushstring(L, "floor"); break;
					case CELL_SOLID: lua_pushstring(L, "solid"); break;
					case CELL_LAYER: lua_pushstring(L, "layer"); break;
					case CELL_BACKWALL: lua_pushstring(L, "backwall"); break;
					case CELL_TABLE: lua_pushstring(L, "table"); break;

					default:
						printf("EDOOFUS: Cell type %i not catered for!\n", ce->f.ctyp);
						fflush(stdout);
						abort();
						break;
				}
				lua_setfield(L, -2, "ctyp");

				lua_pushinteger(L, ce->f.tset);
				lua_setfield(L, -2, "tset");
				lua_pushinteger(L, ce->f.tidx);
				lua_setfield(L, -2, "tidx");
				lua_pushinteger(L, ce->f.p1);
				lua_setfield(L, -2, "p1");

				lua_pushboolean(L, ce->ob != NULL);
				lua_setfield(L, -2, "has_ob");

				// Cell data END
				lua_settable(L, -3);
			}
			lua_settable(L, -3);
		}
		lua_setfield(L, -2, "data");
		lua_settable(L, -3);
	}

	lua_setfield(L, -2, "layers");

	// TABLE: Objects
	lua_newtable(L);

	for(i = 0; i < game->lv->ocount; i++)
	{
		ob = game->lv->objects[i];

		lua_pushinteger(L, i+1);
		lua_newtable(L);

		// Object data START
		switch(ob->f.otyp)
		{
			case OBJ_PLAYER: lua_pushstring(L, "player"); break;
			case OBJ_FOOD_TOMATO: lua_pushstring(L, "food_tomato"); break;

			default:
				printf("EDOOFUS: Object type %i not catered for!\n", ob->f.otyp);
				fflush(stdout);
				abort();
				break;
		}
		lua_setfield(L, -2, "otyp");

		lua_pushinteger(L, ob->f.cx);
		lua_setfield(L, -2, "cx");
		lua_pushinteger(L, ob->f.cy);
		lua_setfield(L, -2, "cy");
		lua_pushinteger(L, ob->f.ox);
		lua_setfield(L, -2, "ox");
		lua_pushinteger(L, ob->f.oy);
		lua_setfield(L, -2, "oy");
		lua_pushinteger(L, ob->f.layer+1);
		lua_setfield(L, -2, "layer");

		lua_pushinteger(L, ob->steps_left);
		lua_setfield(L, -2, "steps_left");
		lua_pushinteger(L, ob->health);
		lua_setfield(L, -2, "health");

		// flags
		lua_newtable(L);
		lua_pushboolean(L, ob->f.flags & OF_CROUCH);
		lua_setfield(L, -2, "crouch");
		lua_setfield(L, -2, "flags");

		// fd block
		lua_newtable(L);
		switch(ob->f.otyp)
		{
			case OBJ_PLAYER:
				lua_pushinteger(L, ((struct fd_player *)(ob->f.fd))->team);
				lua_setfield(L, -2, "team");
				lua_pushinteger(L, ((struct fd_player *)(ob->f.fd))->face);
				lua_setfield(L, -2, "face");
				break;

			case OBJ_FOOD_TOMATO:
				lua_pushinteger(L, ((struct fd_food *)(ob->f.fd))->team);
				lua_setfield(L, -2, "team");
				lua_pushinteger(L, ((struct fd_food *)(ob->f.fd))->face);
				lua_setfield(L, -2, "face");
				break;

			default:
				printf("EDOOFUS: Object type %i not catered for!\n", ob->f.otyp);
				fflush(stdout);
				abort();
				break;
		}
		lua_setfield(L, -2, "fd");

		// Object data END

		lua_settable(L, -3);
	}

	lua_setfield(L, -2, "objects");
	lua_pushinteger(L, game->lv->ocount);
	lua_setfield(L, -2, "ocount");

	// Call
	if(lua_pcall(L, 1, 8, 0))
	{
		printf("ERROR: AI broke!\n");
		printf("Lua says: %s\n", lua_tostring(L, -1));
		printf("Reverting to C behaviour.\n");
		ai->f_do_move = ai_normal_do_move;
		return;
	}

	// Check return type
	const char *typ = lua_tostring(L, 1);
	if(!strcmp(typ, "idle"))
	{
		// DO NOTHING
	} else if(!strcmp(typ, "newturn")) {
		ai->wait = 10;
		game_push_newturn(game, ai->ab, -1, ai->tid);
	} else if(!strcmp(typ, "attack")) {
		ai->wait = 15;
		x = lua_tointeger(L, 2);
		y = lua_tointeger(L, 3);
		tx = lua_tointeger(L, 4);
		ty = lua_tointeger(L, 5);

		// Check if we have an object
		cell_t *ce  = layer_cell_ptr(game->lv->layers[ob->f.layer], x, y);
		ob = (ce == NULL ? NULL : ce->ob);

		if(ob == NULL)
		{
			printf("ERROR: AI tried to attack from a cell with nothing in it!\n");
			printf("Reverting to C behaviour.\n");
			ai->f_do_move = ai_normal_do_move;
		} else if(ob->f.otyp != OBJ_PLAYER) {
			printf("ERROR: AI tried to attack from a non-player object!\n");
			printf("Reverting to C behaviour.\n");
			ai->f_do_move = ai_normal_do_move;
		} else if(((struct fd_player *)(ob->f.fd))->team != ai->tid) {
			printf("ERROR: AI tried to attack from a player belonging to another team!\n");
			printf("Reverting to C behaviour.\n");
			ai->f_do_move = ai_normal_do_move;
		} else {
			game->camx = tx*32+16 - screen_width/2;
			game->camy = ty*24+12 - screen_height/2;
			if(game->camx < 0) game->camx = 0;
			if(game->camy < 0) game->camy = 0;
			// TODO: not hardcode this
			if(game->camx > 32*40 - screen_width) game->camx = 32*40 - screen_width;
			if(game->camy > 32*40 - screen_width) game->camy = 32*40 - screen_width;
			game->mx = tx*32+16 - game->camx;
			game->my = ty*24+12 - game->camy;
			game_push_hover(game, ai->ab, game->mx, game->my, game->camx, game->camy);

			abuf_write_u8(ACT_SELECT, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);

			abuf_write_u8(ACT_ATTACK, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);
			abuf_write_s16(tx, ai->ab);
			abuf_write_s16(ty, ai->ab);
			abuf_write_s16(STEPS_ATTACK, ai->ab);
			abuf_write_s16(ob->steps_left - STEPS_ATTACK, ai->ab);
		}

	} else if((!strcmp(typ, "crouch")) || (!strcmp(typ, "stand"))) {
		ai->wait = 10;
		x = lua_tointeger(L, 2);
		y = lua_tointeger(L, 3);
		
		// Check if we have an object
		cell_t *ce = layer_cell_ptr(game->lv->layers[ob->f.layer], x, y);
		ob = (ce == NULL ? NULL : ce->ob);

		if(ob == NULL)
		{
			printf("ERROR: AI tried to crouch/stand a cell with nothing in it!\n");
			printf("Reverting to C behaviour.\n");
			ai->f_do_move = ai_normal_do_move;
		} else if(ob->f.otyp != OBJ_PLAYER) {
			printf("ERROR: AI tried to crouch/stand a non-player object!\n");
			printf("Reverting to C behaviour.\n");
			ai->f_do_move = ai_normal_do_move;
		} else if(((struct fd_player *)(ob->f.fd))->team != ai->tid) {
			printf("ERROR: AI tried to crouch/stand a player belonging to another team!\n");
			printf("Reverting to C behaviour.\n");
			ai->f_do_move = ai_normal_do_move;
		} else {
			game->camx = ob->f.cx*32+16 - screen_width/2;
			game->camy = ob->f.cy*24+12 - screen_height/2;
			if(game->camx < 0) game->camx = 0;
			if(game->camy < 0) game->camy = 0;
			// TODO: not hardcode this
			if(game->camx > 32*40 - screen_width) game->camx = 32*40 - screen_width;
			if(game->camy > 32*40 - screen_width) game->camy = 32*40 - screen_width;
			game->mx = ob->f.cx*32+16 - game->camx;
			game->my = ob->f.cy*24+12 - game->camy;
			game_push_hover(game, ai->ab, game->mx, game->my, game->camx, game->camy);

			abuf_write_u8(ACT_SELECT, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);

			abuf_write_u8(typ[0] == 's' ? ACT_STAND : ACT_CROUCH, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);
		}

	} else if(!strcmp(typ, "move")) {
		ai->wait = 0;
		x = lua_tointeger(L, 2);
		y = lua_tointeger(L, 3);
		tx = lua_tointeger(L, 4);
		ty = lua_tointeger(L, 5);

		// Check if we have an object
		cell_t *ce = layer_cell_ptr(game->lv->layers[ob->f.layer], x, y);
		ob = (ce == NULL ? NULL : ce->ob);

		if(ob == NULL)
		{
			printf("ERROR: AI tried to move a cell with nothing in it!\n");
			printf("Reverting to C behaviour.\n");
			ai->f_do_move = ai_normal_do_move;
		} else if(ob->f.otyp != OBJ_PLAYER) {
			printf("ERROR: AI tried to move a non-player object!\n");
			printf("Reverting to C behaviour.\n");
			ai->f_do_move = ai_normal_do_move;
		} else if(((struct fd_player *)(ob->f.fd))->team != ai->tid) {
			printf("ERROR: AI tried to move a player belonging to another team!\n");
			printf("Reverting to C behaviour.\n");
			ai->f_do_move = ai_normal_do_move;
		} else {
			// TODO: Actually do A*
			int asl = 1;

			game->camx = ob->f.cx*32+16 - screen_width/2;
			game->camy = ob->f.cy*24+12 - screen_height/2;
			if(game->camx < 0) game->camx = 0;
			if(game->camy < 0) game->camy = 0;
			// TODO: not hardcode this
			if(game->camx > 32*40 - screen_width) game->camx = 32*40 - screen_width;
			if(game->camy > 32*40 - screen_width) game->camy = 32*40 - screen_width;
			game->mx = tx*32+16 - game->camx;
			game->my = ty*24+12 - game->camy;
			game_push_hover(game, ai->ab, game->mx, game->my, game->camx, game->camy);

			abuf_write_u8(ACT_SELECT, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);

			abuf_write_u8(ACT_MOVE, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);
			abuf_write_s16(tx, ai->ab);
			abuf_write_s16(ty, ai->ab);
			abuf_write_s16(asl, ai->ab);
			abuf_write_s16(ob->steps_left - asl, ai->ab);
		}
	} else {
		printf("ERROR: AI performed an invalid move: \"%s\"\n", typ);
		printf("Reverting to C behaviour.\n");
		ai->f_do_move = ai_normal_do_move;
	}

	// Pop
	lua_pop(L, 8);

}

ai_t *ai_new(game_t *game, abuf_t *ab, int tid)
{
	// Allocate AI
	ai_t *ai = malloc(sizeof(ai_t));

	// Drop in arguments
	ai->game = game;
	ai->tid = tid;
	ai->ab = ab;

	// Fill in the defaults
	ai->wait = 0;
	ai->L = NULL;

	// Fill in function pointers
	//ai->f_do_move = ai_normal_do_move;
	ai->f_do_move = ai_lua_do_move;

	// Return
	return ai;
}

