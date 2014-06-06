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

obj_t *game_selob = NULL;

int game_player_count = 0;
int game_curplayer = 0;

static void gameloop_start_turn(void)
{
	int i;
	obj_t *ob;
	struct fd_player *fde;

	// Grant steps to all players of this team
	for(i = 0; i < rootlv->ocount; i++)
	{
		// Get object
		ob = rootlv->objects[i];
		fde = (struct fd_player *)(ob->f.fd);
		
		// Do the compare
		if(fde->team == game_curplayer)
			ob->steps_left = STEPS_PER_TURN;
	}
}

static void gameloop_next_turn(void)
{
	// Deselect object
	game_selob = NULL;

	// Move to the next player
	game_curplayer++;
	if(game_curplayer >= game_player_count)
		game_curplayer = 0;
	
	// Start their turn
	gameloop_start_turn();
}

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
		if(ob == game_selob)
		draw_border_d(screen,
			ob->bx + ob->f.ox + ob->f.cx*32 - game_camx,
			ob->by + ob->f.oy + ob->f.cy*24 - game_camy,
			ob->bw,
			ob->bh,
			2);
	}

	// TEST: Mark walkable paths
	/*
	for(y = 0; y < rootlv->layers[0]->w; y++)
	for(x = 0; x < rootlv->layers[0]->h; x++)
	{
		cell_t *ce = layer_cell_ptr(rootlv->layers[0], x, y);

		if(ce != NULL && ce->f.ctyp == CELL_FLOOR)
		{
			draw_border_d(screen,
				32*x + 8 - game_camx,
				24*y + 6 - game_camy,
				16,
				12,
				ce->ob == NULL ? 1 : 2);
		}

	}
	*/

	// Draw A* route for selected object
	if(game_selob != NULL)
	{
		// Get coordinates
		int asendx = (game_mouse_x + game_camx)/32;
		int asendy = (game_mouse_y + game_camy)/24;

		// Do A* trace
		int dirlist[1024];
		int dirlen = astar_layer(rootlv->layers[0], dirlist, 1024,
			game_selob->f.cx, game_selob->f.cy, asendx, asendy);

		// Trace
		if(dirlen != -1)
		{
			// Get start pos
			x = game_selob->f.cx;
			y = game_selob->f.cy;

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
							24, (i < game_selob->steps_left ? 2 : 1));
						break;

					case DIR_NORTH:
						draw_vline_d(screen,
							x*32+16 - game_camx,
							y*24-12 - game_camy,
							24, (i < game_selob->steps_left ? 2 : 1));
						break;

					case DIR_EAST:
						draw_hline_d(screen,
							x*32+16 - game_camx,
							y*24+12 - game_camy,
							32, (i < game_selob->steps_left ? 2 : 1));
						break;

					case DIR_WEST:
						draw_hline_d(screen,
							x*32-16 - game_camx,
							y*24+12 - game_camy,
							32, (i < game_selob->steps_left ? 2 : 1));
						break;

				}

				// Move
				x += dx;
				y += dy;

			}
		}
	}

	// Draw HUD
	for(i = 6; i >= 0; i--)
		draw_img_trans_cmap_d_sd(screen, ob->img,
			0,
			0,
			0*32, i*48, 32, 48,
			0, teams[game_curplayer]->cm_player);
		
	draw_printf(screen, i_font16, 16, 32, 0, 1, "PLAYER %i TURN", game_curplayer+1);

	// TODO!

	// Flip
	screen_flip();
	SDL_Delay(10);

}

int gameloop_tick(void)
{
	int i;
	int mx, my;
	obj_t *ob;
	cell_t *ce;

	// Get coordinates
	mx = (game_mouse_x + game_camx)/32;
	my = (game_mouse_y + game_camy)/24;
	ce = layer_cell_ptr(rootlv->layers[0], mx, my);

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

	// Block input if waiting for object
	if(game_selob != NULL && game_selob->please_wait)
		return 0;

	// INPUT STARTS HERE

	// Scan keys
	while(input_key_queue_peek() != 0)
	switch(input_key_queue_pop()>>16)
	{
		case SDLK_RETURN | 0x8000:
			// Next turn!
			gameloop_next_turn();
			break;
	}

	// Object select
	if((mouse_b & ~mouse_ob) & 1)
	{

		if(ce != NULL && ce->ob != NULL && ce->ob->f.otyp == OBJ_PLAYER
			&& ((struct fd_player *)(ce->ob->f.fd))->team == game_curplayer)
		{
			// Select object
			game_selob = ce->ob;

		} else {
			// Deselect objects
			game_selob = NULL;

		}
	}

	// Object move
	if((mouse_b & ~mouse_ob) & 4)
	{
		// Check if we have an object selected

		if(game_selob != NULL)
		{
			// Move it
			game_selob->tx = mx;
			game_selob->ty = my;

			// Destroy the old list
			if(game_selob->asdir != NULL)
			{
				free(game_selob->asdir);
				game_selob->asdir = NULL;
			}

			// Mark it as "please wait"
			game_selob->please_wait = 1;

		}

	}

	return 0;
}

int gameloop(const char *fname, int player_count)
{
	// Initialise
	game_camx = 0;
	game_camy = 0;
	game_player_count = player_count;
	game_curplayer = 0;

	// Load level
	rootlv = level_load(fname);
	assert(rootlv != NULL); // TODO: Be more graceful

	// Start turn
	gameloop_start_turn();
	
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

