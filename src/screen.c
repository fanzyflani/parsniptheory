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

void screen_dim_halftone(void)
{
	int x, y;
	uint8_t *dst;

	for(y = 0; y < screen->h; y++)
	for(dst = screen->data + screen->w*y + (y&1), x = (y&1); x < screen->w; x += 2, dst += 2)
		*dst = 0;

}

static void screen_flip_32(void)
{
	int x, y;
	uint8_t *palp;
#ifdef __EMSCRIPTEN__
	int i;
	uint8_t t;
#endif

	// Note, assuming B, G, R, A (which is how the palette is stored)
	// This is correct on all platforms EXCEPT FOR OSX.

	int pitch = screen_surface->pitch;
	int dpitch = pitch - screen_scale * screen->w * 4;
	uint8_t *src = screen->data;
	uint8_t *dst = screen_surface->pixels + pitch * screen_ofy + 4 * screen_ofx;

#ifdef __EMSCRIPTEN__
	// Swap R/B
	for(i = 0; i < 256; i++) {
		palp = pal_main[i];
		t = palp[0]; palp[0] = palp[2]; palp[2] = t;
	}
#endif

	switch(screen_scale)
	{
		case 2:
			for(y = 0; y < screen->h; y++, dst += dpitch + pitch)
			for(x = 0; x < screen->w; x++, dst += 8, src++)
			{
				palp = pal_main[*src];


				//pal_main[*src][3] = 0x00;
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

#ifdef __EMSCRIPTEN__
	// Swap R/B back
	for(i = 0; i < 256; i++) {
		palp = pal_main[i];
		t = palp[0]; palp[0] = palp[2]; palp[2] = t;
	}
#endif

}

void screen_flip(void)
{
	SDL_LockSurface(screen_surface);

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

	SDL_UnlockSurface(screen_surface);
	SDL_Flip(screen_surface);
	//SDL_UpdateRect(screen_surface, 0, 0, screen_surface->w, screen_surface->h);
}


