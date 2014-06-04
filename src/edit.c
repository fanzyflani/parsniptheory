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

	// Clear the screen
	screen_clear(0);

	// Draw the level
	draw_level(screen, rootlv, edit_camx, edit_camy, edit_layer);

	// Draw a grid too
	for(y = smod(edit_camy-1, 24); y < 200; y += 24)
		draw_dot_hline_d(screen, 0, y, 320, 1);
	for(x = smod(edit_camx-1, 32); x < 320; x += 32)
		draw_dot_vline_d(screen, x, 0, 200, 1);

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
	}
}


