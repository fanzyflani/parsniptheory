/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

int edit_tset = 0;
int edit_tidx = 1;

int edit_camx = 0;
int edit_camy = 0;
int edit_layer = 0;

void editloop_draw(void)
{
	int x, y;
	layer_t *ay;

	// Load pointers
	ay = rootlv->layers[edit_layer];

	// Clear the screen
	screen_clear(0);

	// Draw the level
	draw_level(screen, rootlv, edit_camx, edit_camy, edit_layer);

	// Draw a grid
	if(key_state[SDLK_g])
	{
		for(y = smod(-edit_camy-1, 24); y < 200; y += 24)
			draw_dot_hline_d(screen, 0, y, 320, 2);
		for(x = smod(-edit_camx-1, 32); x < 320; x += 32)
			draw_dot_vline_d(screen, x, 0, 200, 2);
	}

	// Draw the current layer boundaries
	draw_border_d(screen,
		32*ay->x-1-edit_camx, 32*ay->y-1-edit_camy,
		2+32*ay->w, 2+24*ay->h, 1);

	// Flip
	screen_flip();
	SDL_Delay(10);

}

void edit_tselloop_draw(void)
{
	int x, y;
	layer_t *ay;

	// Calculate camera offset
	const int tcamx_spread = 32*16 - 320;
	const int tcamy_spread = 24*16 - 200;
	int tcamx = (mouse_x * tcamx_spread) / 320;
	int tcamy = (mouse_y * tcamy_spread) / 200;

	// Get tile cell
	int cx = (mouse_x + tcamx) / 32;
	int cy = (mouse_y + tcamy) / 24;

	// Load pointers
	ay = rootlv->layers[edit_layer];

	// Clear the screen
	screen_clear(0);

	// Draw all the tiles
	draw_img_trans_cmap_d_sd(screen, i_tiles1,
		-tcamx, -tcamy, 0, 0,
		32*16, 24*16, 0, cm_tiles1);

	// Draw a grid
	for(y = smod(-tcamy-1, 24); y < 200; y += 24)
		draw_dot_hline_d(screen, 0, y, 320, 2);
	for(x = smod(-tcamx-1, 32); x < 320; x += 32)
		draw_dot_vline_d(screen, x, 0, 200, 2);
	
	// Draw a box around the selected tile
	draw_border_d(screen, cx*32-tcamx, cy*24-tcamy, 32, 24, 1);

	// Flip
	screen_flip();
	SDL_Delay(10);

}

int edit_tselloop(void)
{
	input_key_queue_flush();

	for(;;)
	{
		// Draw
		edit_tselloop_draw();

		// Process events
		if(input_poll()) return 1;

		// Process mouse
		if((mouse_ob & 1) && !(mouse_b & 1))
		{
			// Tile select

			// Calculate camera offset
			const int tcamx_spread = 32*16 - 320;
			const int tcamy_spread = 24*16 - 200;
			int tcamx = (mouse_x * tcamx_spread) / 320;
			int tcamy = (mouse_y * tcamy_spread) / 200;

			// Get tile
			int cx = (mouse_x + tcamx) / 32;
			int cy = (mouse_y + tcamy) / 24;

			// Set tile stuff
			edit_tidx = cx + cy*16;

			// Return
			return 0;
		}

		// Process keyboard
		while(input_key_queue_peek())
		{
			// Get key and filter
			uint32_t k = input_key_queue_pop();
			int pressed = (k>>31)&1;
			int sym = (k>>16)&0x7FFF;

			if(pressed) switch(sym)
			{
			} else switch(sym) {
				case SDLK_ESCAPE:
					// Bail out.
					return 0;

			}

		}

	}

}

int editloop(void)
{
	obj_t *ob;

	// Create level
	rootlv = level_new(40, 40);

	// Loop
	for(;;)
	{
		// Draw
		editloop_draw();

		// Process events
		if(input_poll()) return 1;

		// 

		if((mouse_b & 5) == 1 || (mouse_b & 5) == 4)
		{
			// Get cell pointer and check
			// TODO: Make this not spam
			cell_t *ce = layer_cell_ptr(rootlv->layers[edit_layer],
				sdiv((mouse_x + edit_camx),32),
				sdiv((mouse_y + edit_camy),24));

			if(ce != NULL)
			{
				if(mouse_b & 1)
					// Set a tile
					cell_reprep(ce, 0, edit_tidx);
				else
					// Get a tile
					edit_tidx = ce->f.tidx;
			}

		}

		if(mouse_b & 2)
		{
			// Scroll
			edit_camx -= mouse_x - mouse_ox;
			edit_camy -= mouse_y - mouse_oy;
		}

		// Process keyboard
		while(input_key_queue_peek())
		{
			// Get key and filter
			uint32_t k = input_key_queue_pop();
			int pressed = (k>>31)&1;
			int sym = (k>>16)&0x7FFF;

			if(pressed) switch(sym)
			{
				case SDLK_1:
				case SDLK_2:
				case SDLK_3:
				case SDLK_4:
					ob = level_obj_add(rootlv, OBJ_PLAYER, 0,
						sdiv(mouse_x + edit_camx, 32),
						sdiv(mouse_y + edit_camy, 24),
						edit_layer);
					((struct fd_player *)(ob->f.fd))->team = sym - SDLK_1;
					ob->f_init(ob);

					break;

				case SDLK_n:
					if(key_state[SDLK_LCTRL] || key_state[SDLK_RCTRL])
					{
						// Clear objects

						// FIXME: If a NULL shows up, ouch
						printf("Freeing objects...\n");
						while(rootlv->ocount > 0)
							level_obj_free(rootlv, rootlv->objects[0]);

					}

					break;
				case SDLK_l:
					if(key_state[SDLK_LCTRL] || key_state[SDLK_RCTRL])
					{
						// Load

						printf("Loading...\n");
						level_t *tlv = level_load("lvl/level.psl");

						if(tlv != NULL)
						{
							if(rootlv != NULL)
							{
								printf("Cleaning up...\n");
								level_free(rootlv);
							}

							printf("Transferring...\n");
							rootlv = tlv;
						}
					}

					break;

				case SDLK_s:
					if(key_state[SDLK_LCTRL] || key_state[SDLK_RCTRL])
					{
						// Save

						printf("Saving...\n");
						level_save(rootlv, "lvl/level.psl");
					}

					break;

				case SDLK_t:
					if(edit_tselloop()) return 1;
					break;
			}
		}

	}

	return 0;
}


