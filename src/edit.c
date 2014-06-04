/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

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
	for(y = smod(-edit_camy-1, 24); y < 200; y += 24)
		draw_dot_hline_d(screen, 0, y, 320, 2);
	for(x = smod(-edit_camx-1, 32); x < 320; x += 32)
		draw_dot_vline_d(screen, x, 0, 200, 2);

	// Draw the current layer boundaries)
		draw_border_d(screen,
			32*ay->x-1-edit_camx, 32*ay->y-1-edit_camy,
			2+32*ay->w, 2+24*ay->h, 1);


	// Flip
	screen_flip();
	SDL_Delay(10);

}

void editloop(void)
{
	for(;;)
	{
		// Draw
		editloop_draw();

		// Process events
		if(input_poll()) break;

		// 

		if((mouse_b & 5) == 1 || (mouse_b & 5) == 4)
		{
			// Nice floor, or blank?
			// Get cell pointer and check
			// TODO: Make this not spam
			cell_t *ce = layer_cell_ptr(rootlv->layers[edit_layer],
				sdiv((mouse_x + edit_camx),32),
				sdiv((mouse_y + edit_camy),24));

			if(ce != NULL)
			{
				// Select.
				if(mouse_b & 1)
					cell_reprep(ce, 0, 1);
				else
					cell_reprep(ce, 0, 0);
			}

		}

		if(mouse_b & 2)
		{
			edit_camx -= mouse_x - mouse_ox;
			edit_camy -= mouse_y - mouse_oy;
		}
	}
}


