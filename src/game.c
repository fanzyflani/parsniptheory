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

int game_astarx = 0;
int game_astary = 0;

obj_t *game_selobj = NULL;

void gameloop_draw(void)
{
	int x, y, i;
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

	// TEST: A* route
	{
		// Get coordinates
		int asendx = (game_mouse_x + game_camx)/32;
		int asendy = (game_mouse_y + game_camy)/24;

		// Do A* trace
		int dirlist[1024];
		int dirlen = astar_layer(rootlv->layers[0], dirlist, 1024,
			game_astarx, game_astary, asendx, asendy);

		// Trace
		if(dirlen != -1)
		{
			// Get start pos
			x = game_astarx;
			y = game_astary;

			for(i = 0; i < dirlen; i++)
			{
				// Get delta
				int dx = face_dir[dirlist[i]][0];
				int dy = face_dir[dirlist[i]][1];

				// Draw line
				switch(dirlist[i])
				{
					case DIR_SOUTH:
						draw_vline_d(screen,
							x*32+16 - game_camx,
							y*24+12 - game_camy,
							24, 2);
						break;

					case DIR_NORTH:
						draw_vline_d(screen,
							x*32+16 - game_camx,
							y*24-12 - game_camy,
							24, 2);
						break;

					case DIR_EAST:
						draw_hline_d(screen,
							x*32+16 - game_camx,
							y*24+12 - game_camy,
							32, 2);
						break;

					case DIR_WEST:
						draw_hline_d(screen,
							x*32-16 - game_camx,
							y*24+12 - game_camy,
							32, 2);
						break;

				}

				// Move
				x += dx;
				y += dy;

			}
		}
	}


	// Draw HUD
	// TODO!

	// Flip
	screen_flip();
	SDL_Delay(10);

}

int gameloop_tick(void)
{
	int x, y, i;
	obj_t *ob;
	cell_t *ce;

	// TEST: A* route
	if(mouse_b & 1)
	{
		// Get coordinates
		x = (game_mouse_x + game_camx)/32;
		y = (game_mouse_y + game_camy)/24;
		ce = layer_cell_ptr(rootlv->layers[0], x, y);

		if(ce != NULL)
		{
			// Set start point
			game_astarx = x;
			game_astary = y;
		}
	}

	// Tick objects
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

