/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

SDL_mutex *mutLmain;
lua_State *Lmain;

int lint_checkargs(lua_State *L, int args)
{
	int top = lua_gettop(L);

	if(top < args)
		luaL_error(L, "expected %d arguments, got %d\n", args, top);

	return top;
}

int lint_f_draw_border_d(lua_State *L)
{
	int top = lint_checkargs(L, 6);

	img_t *dst = (img_t *)lua_touserdata(L, 1);
	int x = lua_tointeger(L, 2);
	int y = lua_tointeger(L, 3);
	int w = lua_tointeger(L, 4);
	int h = lua_tointeger(L, 5);
	int c = lua_tointeger(L, 6);

	draw_border_d(screen, x, y, w, h, c);

	return 0;
}

int lint_f_draw_rect_d(lua_State *L)
{
	int top = lint_checkargs(L, 6);

	img_t *dst = (img_t *)lua_touserdata(L, 1);
	int x = lua_tointeger(L, 2);
	int y = lua_tointeger(L, 3);
	int w = lua_tointeger(L, 4);
	int h = lua_tointeger(L, 5);
	int c = lua_tointeger(L, 6);

	draw_rect_d(screen, x, y, w, h, c);

	return 0;
}

int lint_f_draw_16_printf(lua_State *L)
{
	int top = lint_checkargs(L, 6);

	img_t *dst = (img_t *)lua_touserdata(L, 1);
	int align = lua_tointeger(L, 2);
	int x = lua_tointeger(L, 3);
	int y = lua_tointeger(L, 4);
	int c = lua_tointeger(L, 5);
	const char *s = lua_tostring(L, 6);

	draw_printf(dst, i_font16, 16, align, x, y, c, "%s", s);

	return 0;
}

int lint_f_draw_57_printf(lua_State *L)
{
	int top = lint_checkargs(L, 6);

	img_t *dst = (img_t *)lua_touserdata(L, 1);
	int align = lua_tointeger(L, 2);
	int x = lua_tointeger(L, 3);
	int y = lua_tointeger(L, 4);
	int c = lua_tointeger(L, 5);
	const char *s = lua_tostring(L, 6);

	draw_57_printf(dst, align, x, y, c, "%s", s);

	return 0;
}

int lint_f_pal_get(lua_State *L)
{
	int top = lint_checkargs(L, 1);

	int idx = lua_tointeger(L, 1) & 255;

	lua_pushinteger(L, pal_main[idx][2]);
	lua_pushinteger(L, pal_main[idx][1]);
	lua_pushinteger(L, pal_main[idx][0]);

	return 3;
}

int lint_f_pal_set(lua_State *L)
{
	int top = lint_checkargs(L, 4);

	int idx = lua_tointeger(L, 1) & 255;
	int r = lua_tointeger(L, 2);
	int g = lua_tointeger(L, 3);
	int b = lua_tointeger(L, 4);

	pal_main[idx][2] = r;
	pal_main[idx][1] = g;
	pal_main[idx][0] = b;

	return 0;
}

int lint_f_input_key_queue_flush(lua_State *L)
{
	int top = lint_checkargs(L, 0);

	input_key_queue_flush();

	return 0;
}

int lint_f_input_key_queue_pop(lua_State *L)
{
	int top = lint_checkargs(L, 0);

	int v = input_key_queue_pop();

	lua_pushinteger(L, (v>>16)&0x7FFF);
	lua_pushinteger(L, v&0xFFFF);
	lua_pushboolean(L, !(v>>31));

	return 3;
}

int lint_f_input_key_queue_peek(lua_State *L)
{
	int top = lint_checkargs(L, 0);

	int v = input_key_queue_peek();

	lua_pushinteger(L, (v>>16)&0x7FFF);
	lua_pushinteger(L, v&0xFFFF);
	lua_pushboolean(L, !(v>>31));

	return 3;
}

int lint_f_input_mouse(lua_State *L)
{
	int top = lint_checkargs(L, 0);

	lua_pushinteger(L, mouse_x);
	lua_pushinteger(L, mouse_y);
	lua_pushinteger(L, mouse_b);
	lua_pushinteger(L, mouse_ox);
	lua_pushinteger(L, mouse_oy);
	lua_pushinteger(L, mouse_ob);

	return 6;
}

int lint_f_input_key(lua_State *L)
{
	int top = lint_checkargs(L, 1);

	int kidx = lua_tointeger(L, 1);
	if(kidx < 0 || kidx >= SDLK_LAST)
		return luaL_error(L, "key index %d out of range", kidx);
	
	lua_pushboolean(L, key_state[kidx]);

	return 1;
}

int lint_f_astar_layer(lua_State *L)
{
	int i;
	int dirbuf[2048];

	int top = lint_checkargs(L, 5);

	game_t *game = lua_touserdata(L, lua_upvalueindex(1));
	int lidx = lua_tointeger(L, 1)-1;
	int x1 = lua_tointeger(L, 2);
	int y1 = lua_tointeger(L, 3);
	int x2 = lua_tointeger(L, 4);
	int y2 = lua_tointeger(L, 5);

	if(game == NULL)
		return luaL_error(L, "upvalue 1 must be a userdata of type 'game'");
	if(lidx < 0 || lidx >= game->lv->lcount)
		return luaL_error(L, "argument 1 (layer) out of range");
	
	layer_t *ay = game->lv->layers[lidx];

	int ret = astar_layer(ay, dirbuf, 2048, x1, y1, x2, y2);

	if(ret < 0)
	{
		lua_pushnil(L);
	} else {
		lua_newtable(L);

		for(i = 0; i < ret; i++)
		{
			lua_pushinteger(L, i+1);
			lua_pushinteger(L, dirbuf[i]);
			lua_settable(L, -3);
		}
	}

	return 1;
}

int lint_f_line_layer(lua_State *L)
{
	int i;

	int top = lint_checkargs(L, 5);

	game_t *game = lua_touserdata(L, lua_upvalueindex(1));
	int lidx = lua_tointeger(L, 1)-1;
	int x1 = lua_tointeger(L, 2);
	int y1 = lua_tointeger(L, 3);
	int x2 = lua_tointeger(L, 4);
	int y2 = lua_tointeger(L, 5);

	if(game == NULL)
		return luaL_error(L, "argument 1 must be a userdata of type 'game'");
	if(lidx < 0 || lidx >= game->lv->lcount)
		return luaL_error(L, "argument 2 (layer) out of range");
	
	layer_t *ay = game->lv->layers[lidx];

	int ret = line_layer(ay, &x1, &y1, x1, y1, x2, y2);

	lua_pushboolean(L, ret);
	lua_pushinteger(L, x1);
	lua_pushinteger(L, y1);

	return 3;
}

void lint_lock(void)
{
	if(SDL_mutexP(mutLmain) != 0)
	{
		printf("FATAL: Lua mutex lock failed\n");
		fflush(stdout);
		abort();
	}
}

void lint_unlock(void)
{
	if(SDL_mutexV(mutLmain) != 0)
	{
		printf("FATAL: Lua mutex unlock failed\n");
		fflush(stdout);
		abort();
	}
}

int lint_init(void)
{
	mutLmain = SDL_CreateMutex();
	Lmain = luaL_newstate();
	printf("Lua init stuff: %p\n", Lmain);

	luaL_openlibs(Lmain);

#if 0
	lua_newtable(Lmain);

	lua_pushcclosure(Lmain, lint_f_draw_border_d, 0);
	lua_setfield(Lmain, -2, "draw_border_d");
	lua_pushcclosure(Lmain, lint_f_draw_rect_d, 0);
	lua_setfield(Lmain, -2, "draw_rect_d");
	lua_pushcclosure(Lmain, lint_f_draw_16_printf, 0);
	lua_setfield(Lmain, -2, "draw_16_printf");
	lua_pushcclosure(Lmain, lint_f_draw_57_printf, 0);
	lua_setfield(Lmain, -2, "draw_57_printf");

	lua_pushcclosure(Lmain, lint_f_pal_get, 0);
	lua_setfield(Lmain, -2, "pal_get");
	lua_pushcclosure(Lmain, lint_f_pal_set, 0);
	lua_setfield(Lmain, -2, "pal_set");

	lua_pushcclosure(Lmain, lint_f_input_key_queue_flush, 0);
	lua_setfield(Lmain, -2, "input_key_queue_flush");
	lua_pushcclosure(Lmain, lint_f_input_key_queue_pop, 0);
	lua_setfield(Lmain, -2, "input_key_queue_pop");
	lua_pushcclosure(Lmain, lint_f_input_key_queue_peek, 0);
	lua_setfield(Lmain, -2, "input_key_queue_peek");
	lua_pushcclosure(Lmain, lint_f_input_mouse, 0);
	lua_setfield(Lmain, -2, "input_mouse");
	lua_pushcclosure(Lmain, lint_f_input_key, 0);
	lua_setfield(Lmain, -2, "input_key");

	lua_setglobal(Lmain, "game");

	luaL_dofile(Lmain, "lua/main.lua");
#endif

	return 1;
}



