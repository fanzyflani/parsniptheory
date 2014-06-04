/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

void screen_clear(uint8_t col)
{
	// Easy.
	memset(screen->data, col, screen->w * screen->h);
}

static void screen_flip_32(void)
{
	int x, y;
	uint8_t *palp;

	// Note, assuming B, G, R, A (which is how the palette is stored)
	// This is correct on all platforms EXCEPT FOR OSX.

	int pitch = screen_surface->pitch;
	int dpitch = pitch - screen_scale * screen->w * 4;
	uint8_t *src = screen->data;
	uint8_t *dst = screen_surface->pixels + pitch * screen_ofy + 4 * screen_ofx;

	switch(screen_scale)
	{
		case 2:
			for(y = 0; y < screen->h; y++, dst += dpitch + pitch)
			for(x = 0; x < screen->w; x++, dst += 8, src++)
			{
				palp = pal_main[*src];
				memcpy(dst + 4*0 + pitch*0, palp, 4);
				memcpy(dst + 4*1 + pitch*0, palp, 4);
				memcpy(dst + 4*0 + pitch*1, palp, 4);
				memcpy(dst + 4*1 + pitch*1, palp, 4);
			}
			break;

		default:
			printf("FATAL ERROR: %ix scale for 32bpp not handled yet!\n", screen_scale);
			abort();
			break;
	}
}

void screen_flip(void)
{
	switch(screen_bpp)
	{
		case 32:
			screen_flip_32();
			break;

		default:
			printf("FATAL ERROR: bpp of %i not handled yet!\n", screen_bpp);
			abort();
			break;

	}

	SDL_Flip(screen_surface);
}


