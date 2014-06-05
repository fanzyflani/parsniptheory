/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

int game_camx = 0;
int game_camy = 0;
int game_mouse_x = 0;
int game_mouse_y = 0;
int game_mouse_ox = 0;
int game_mouse_oy = 0;

obj_t *game_selobj = NULL;

void gameloop_draw(void)
{
	int i;
	obj_t *ob;

	// Clear the screen
	screen_clear(0);

	// Draw level
	draw_level(screen, rootlv, game_camx, game_camy, 0);

	// Draw overlays
	for(i = 0; i < rootlv->ocount; i++)
	{
		ob = rootlv->objects[i];

		if(ob == NULL) continue;

		// Cell occupancy
		draw_border_d(screen,
			ob->f.cx*32 - game_camx,
			ob->f.cy*24 - game_camy,
			32,
			24,
			1);

		// Bounding box
		draw_border_d(screen,
			ob->bx + ob->f.ox + ob->f.cx*32 - game_camx,
			ob->by + ob->f.oy + ob->f.cy*24 - game_camy,
			ob->bw,
			ob->bh,
			2);
	}

	// Draw HUD
	// TODO!

	// Flip
	screen_flip();
	SDL_Delay(10);

}

int gameloop_tick(void)
{
	int i;
	obj_t *ob;

	for(i = 0; i < rootlv->ocount; i++)
	{
		ob = rootlv->objects[i];

		if(ob == NULL) continue;

		if(ob->f_tick != NULL) ob->f_tick(ob);
	}

	// Update UI
	if(game_mouse_x < (screen->w>>2)) game_camx -= 4;
	if(game_mouse_y < (screen->h>>2)) game_camy -= 4;
	if(game_mouse_x >= ((screen->w*3)>>2)) game_camx += 4;
	if(game_mouse_y >= ((screen->h*3)>>2)) game_camy += 4;

	return 0;
}

int gameloop(void)
{
	// Initialise
	game_camx = 0;
	game_camy = 0;

	// Load level
	rootlv = level_load("dat/genesis.psl");
	assert(rootlv != NULL); // TODO: Be more graceful
	
	for(;;)
	{
		// Draw
		gameloop_draw();

		// Process events
		if(input_poll()) return 1;

		game_mouse_x = mouse_x;
		game_mouse_y = mouse_y;
		game_mouse_ox = mouse_ox;
		game_mouse_oy = mouse_oy;

		// Process tick
		// TODO: Time this nicely
		if(gameloop_tick()) return 0;
	}

	return 0;
}

