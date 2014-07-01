/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

int titleloop(void)
{
	int i, j;

	// Change palette
	//memcpy(pal_main, i_titleff1->pal, sizeof(pal_main));
	memset(pal_main, 0, sizeof(pal_main));

	// Play titleff1 jingle
	music_play(mod_titleff1);

	// Flush input
	input_key_queue_flush();

	// Loop
	while(sackit == NULL || sackit->process_order == 65535 || sackit->process_order == 0)
	{
		// Clear screen
		memset(screen->data, 0, screen->w*screen->h);

		// Deal to palette
		int row = sackit->current_row;

		if(row == 65534)
		{
			// do nothing

		} else if(row < 48) {
			for(i = 0; i < 8; i++)
			for(j = 0; j < 4; j++)
				pal_main[i][j] = (((int)i_titleff1->pal[i][j])*row)/48;
			for(i = 8; i < 256; i++)
			for(j = 0; j < 4; j++)
				pal_main[i][j] = (((int)i_titleff1->pal[0][j])*row)/48;

		} else if(row < 64) {
			memcpy(pal_main, i_titleff1->pal, 4*8);
			for(i = 8; i < 256; i++)
			for(j = 0; j < 4; j++)
				pal_main[i][j] = (
					((int)i_titleff1->pal[i][j] - (int)i_titleff1->pal[0][j])
					*(row-48)
					+((int)i_titleff1->pal[0][j])*16
				)/16;

		} else if(row < 88) {
			memcpy(pal_main, i_titleff1->pal, 4*256);

		} else if(row < 108)  {
			for(i = 0; i < 256; i++)
			for(j = 0; j < 4; j++)
				pal_main[i][j] = (((int)i_titleff1->pal[i][j])*(108-row))/20;

		} else {
			memset(pal_main, 0, sizeof(pal_main));
		}

		// Draw title
		draw_img_trans_d_sd(screen, i_titleff1,
			(screen->w-i_titleff1->w)/2,
			(screen->h-i_titleff1->h)/2,
			0, 0, i_titleff1->w, i_titleff1->h, 0);

		// Flip and wait
		screen_flip();
		SDL_Delay(10);

		// Check input
		if(input_poll())
			return 1;

		// Break on key / mouse
		if((mouse_ob&~mouse_b))
			break;

		if(input_key_queue_pop() & 0x80000000)
			break;

	}

	// Clear screen
	memset(screen->data, 0, screen->w*screen->h);

	// Switch to game palette
	memcpy(pal_main, pal_src, sizeof(pal_main));

	return 0;
}

