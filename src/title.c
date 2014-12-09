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

#include "common.h"

typedef struct timg
{
	int bx, by;
	int face;
} title_char_t;

int title_start_time = 0;

void title_set_time(void)
{
	title_start_time = SDL_GetTicks();
}

int title_get_time(void)
{
	int v = SDL_GetTicks() - title_start_time;
	v = (v * 140 * 4) / (10 * 4);
	v = (v+250)/500;

	return v;
}

void title_draw_char(img_t *dst, title_char_t *ch, int camx, int camy)
{
	int bx, by, face;

	bx = ch->bx - camx;
	by = ch->by - camy;
	face = ch->face & 3;

	if(face & 1)
	{
		// East/west

		// Body
		draw_img_trans_d_sd(dst, i_title[2], bx - 30, by - 100,
			0, 120, 60, 80, 0);

		// Head
		draw_img_trans_d_sd(dst, i_title[2], bx - 30, by - 150,
			(face == DIR_WEST ? 60 : 120), 60, 60, 60, 0);

		// Hair
		draw_img_trans_d_sd(dst, i_title[2], bx - 50, by - 180,
			540, 0, 100, 60, 0);

		// Hands
		// TODO: One in front of the other
		draw_img_trans_d_sd(dst, i_title[2], bx - 45, by - 70,
			120, 120, 20, 20, 0);
		draw_img_trans_d_sd(dst, i_title[2], bx + 25, by - 70,
			120, 120, 20, 20, 0);

		// Feet
		if(face == DIR_WEST)
		{
			draw_img_trans_d_sd(dst, i_title[2], bx - 15 - 30, by - 20,
				140, 140, 40, 20, 0);
			draw_img_trans_d_sd(dst, i_title[2], bx + 15 - 30, by - 20 - 10,
				140, 140, 40, 20, 0);

		} else {
			draw_img_trans_d_sd(dst, i_title[2], bx - 15 - 10, by - 20,
				140, 120, 40, 20, 0);
			draw_img_trans_d_sd(dst, i_title[2], bx + 15 - 10, by - 20 - 10,
				140, 120, 40, 20, 0);

		}
	
	} else {
		// North/south

		// Body (SOUTH)
		if(face == DIR_SOUTH)
			draw_img_trans_d_sd(dst, i_title[2], bx - 30, by - 100,
				0, 120, 60, 80, 0);

		// Head
		draw_img_trans_d_sd(dst, i_title[2], bx - 40, by - 150,
			(face == DIR_NORTH ? 0 : 80), 0, 80, 60, 0);

		// Hair
		draw_img_trans_d_sd(dst, i_title[2], bx - 50, by - 180,
			540, 0, 100, 60, 0);

		// Hands
		draw_img_trans_d_sd(dst, i_title[2], bx - 45, by - 70,
			120, 120, 20, 20, 0);
		draw_img_trans_d_sd(dst, i_title[2], bx + 25, by - 70,
			120, 120, 20, 20, 0);

		// Body (NORTH)
		if(face == DIR_NORTH)
			draw_img_trans_d_sd(dst, i_title[2], bx - 30, by - 100,
				0, 120, 60, 80, 0);

		// Feet
		draw_img_trans_d_sd(dst, i_title[2], bx - 15 - 10, by - 20,
			120, 140, 20, 20, 0);
		draw_img_trans_d_sd(dst, i_title[2], bx + 15 - 10, by - 20 - 10,
			120, 140, 20, 20, 0);
	
	}

}

int title_presents(void)
{
	int i, j;

	// Change palette
	memset(pal_main, 0, sizeof(pal_main));

	// Play titleff1
	music_play(mod_titleff1);

	// Flush input
	input_key_queue_flush();

	// Clear screen
	memset(screen->data, 0, screen->w*screen->h);

	// Draw title
	draw_img_trans_d_sd(screen, i_titleff1,
		(screen->w-i_titleff1->w)/2,
		(screen->h-i_titleff1->h)/2,
		0, 0, i_titleff1->w, i_titleff1->h, 0);

	// Loop
	title_set_time();
	while(sackit == NULL || sackit->process_order == 65535 || sackit->process_order == 0)
	{
		// Deal to palette
		int row = title_get_time();

		if(sackit == NULL || sackit->process_order == 0)
		{
			if(row == 65534)
			{
				// do nothing
				memset(pal_main, 0, sizeof(pal_main));

			} else if(row < 16) {
				memset(pal_main, 0, sizeof(pal_main));
			} else if(row < 48) {
				for(i = 0; i < 8; i++)
				for(j = 0; j < 4; j++)
					pal_main[i][j] = (((int)i_titleff1->pal[i][j])*(row-16))/32;
				for(i = 8; i < 256; i++)
				for(j = 0; j < 4; j++)
					pal_main[i][j] = (((int)i_titleff1->pal[0][j])*(row-16))/32;
	 
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
		}

		// Flip and wait
		screen_flip();
		SDL_Delay(10);

		// Check input
		if(input_poll())
			return 1;

		// Break on key / mouse
		if((mouse_ob&~mouse_b))
			break;

		//if(input_key_queue_pop() & 0x80000000)
		//	break;

	}

	return 0;

}

#if 0
int title_main(void)
{
	int i, j;
	title_char_t chtest[3];
	title_char_t *ch;

	// Change palette
	memset(pal_main, 0, sizeof(pal_main));

	// Play trk2
	music_play(mod_trk2);

	// Flush input
	input_key_queue_flush();

	// TEST: Draw character
	ch = &chtest[0];
	ch->bx = screen->w/2;
	ch->by = screen->h - 10;
	ch->face = DIR_SOUTH;

	ch = &chtest[1];
	ch->bx = screen->w/4;
	ch->by = screen->h - 30;
	ch->face = DIR_WEST;

	ch = &chtest[2];
	ch->bx = 3*screen->w/4;
	ch->by = screen->h - 30;
	ch->face = DIR_EAST;

	// Loop
	title_set_time();
	while(sackit == NULL || sackit->process_order == 65535 || sackit->process_order <= 39 || sackit->current_row < 48)
	{
		// Deal to palette
		int row = title_get_time();

		if(row == 65534)
		{
			// do nothing
			memset(pal_main, 0, sizeof(pal_main));

		} else if(row < 16) {
			memset(pal_main, 0, sizeof(pal_main));
		} else if(row < 48) {
			for(i = 128; i < 256; i++)
			for(j = 0; j < 4; j++)
				pal_main[i][j] = (((int)i_title[2]->pal[i][j])*(row-16))/32;
			for(i = 0; i < 128; i++)
			for(j = 0; j < 4; j++)
				pal_main[i][j] = (((int)i_title[1]->pal[i][j])*(row-16))/32;
		} else if(row < 136) {
			memcpy(pal_main, i_title[2]->pal, sizeof(pal_main));
			memcpy(pal_main, i_title[1]->pal, 4*128);
		} else if(row < 168) {
			for(i = 128; i < 256; i++)
			for(j = 0; j < 4; j++)
				pal_main[i][j] = (((int)i_title[2]->pal[i][j])*(168-row))/32;
			for(i = 0; i < 128; i++)
			for(j = 0; j < 4; j++)
				pal_main[i][j] = (((int)i_title[1]->pal[i][j])*(168-row))/32;
		} else {
			memset(pal_main, 0, 4*128);
		}
		//printf("%i\n", row);

		// Clear screen
		memset(screen->data, 0, screen->w*screen->h);

		// Draw stuff
		if(row < 168)
		{
			// Shot 1: Front of school
			draw_img_trans_d_sd(screen, i_title[1],
				0, 0, 0 + row-16, 200, 320, 200, 0);

			title_draw_char(screen, chtest + 2, row + 150, -50);
		}

		// Draw character
		/*
		title_draw_char(screen, chtest + 0, row, 0);
		title_draw_char(screen, chtest + 1, row, 0);
		title_draw_char(screen, chtest + 2, row, 0);
		*/

		// Flip and wait
		screen_flip();
		SDL_Delay(10);

		// Check input
		if(input_poll())
			return 1;

		// Break on key / mouse
		if((mouse_ob&~mouse_b))
			break;

		//if(input_key_queue_pop() & 0x80000000)
		//	break;

	}

	return 0;


}
#endif

int titleloop(void)
{
#ifdef NO_TITLE
	return 0;
#endif
	// PRESENTS
	if(title_presents())
		return 1;

	// Clear screen
	memset(screen->data, 0, screen->w*screen->h);

#if 0
	// Intro sequence
	if(title_main())
		return 1;

	// Clear screen
	memset(screen->data, 0, screen->w*screen->h);
#endif

	// Switch to game palette
	memcpy(pal_main, pal_src, sizeof(pal_main));
	pal_main[0][0] = 255/5;
	pal_main[0][1] = 0;
	pal_main[0][2] = 0;

	// Flip
	screen_flip();

	// Stop music
	music_play(NULL);

	return 0;
}

