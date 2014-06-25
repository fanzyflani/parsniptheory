/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

float screen_plasma_ang1 = 0.0;
float screen_plasma_ang2 = 0.0;
int screen_plasma_x = 160;
int screen_plasma_y = 100;

void screen_plasma(void)
{
	int x, y, i, v;
	int vx, vy;
	for(i = 0; i < 16; i++)
	{
		pal_main[i+16][0] = 120+120*sin(screen_plasma_ang1 + M_PI*i/24.0 + M_PI*0.0/3.0);
		pal_main[i+16][1] = 120+120*sin(screen_plasma_ang2 + M_PI*i/24.0 + M_PI*2.0/3.0);
		pal_main[i+16][2] = 120+120*sin(screen_plasma_ang1 + screen_plasma_ang2 + M_PI*i/24.0 + M_PI*4.0/3.0);
	}

	screen_plasma_ang1 += 0.07;
	screen_plasma_ang2 += 0.057;
	screen_plasma_x = screen->w/2 + (screen->h/4)*sin(screen_plasma_ang1);
	screen_plasma_y = screen->h/2 + (screen->h/4)*cos(screen_plasma_ang2);

	int yoffstab[256];
	for(x = 0; x < 256; x++)
		yoffstab[x] = screen->h/4*cos(x*M_PI/128 + screen_plasma_ang2 + sin(screen_plasma_ang1 + x/512));

	for(y = 0; y < screen->h; y++)
	{
		int xoffs = screen->h/4*sin(y*M_PI*2.0/screen->h + screen_plasma_ang1 + sin(screen_plasma_ang2 + y/(screen->h*4)));

		for(x = 0; x < screen->w; x++)
		{
			vx = (x+xoffs-screen_plasma_x);
			vy = (y+yoffstab[(x-xoffs)&255]-screen_plasma_y);
			v = vx*vx + vy*vy;
			v >>= 8;
			v += (x^y)&1;
			v >>= 1;
			v ^= ((v>>4)&1)*15;
			v &= 15;
			*IMG8(screen, x, y) = v+16;
		}
	}
}

void screen_clear(uint8_t col)
{
#ifdef NO_PLASMA
	// Easy.
	memset(screen->data, col, screen->w * screen->h);
#else
	screen_plasma();
#endif
}

void screen_dim_halftone(void)
{
	int x, y;
	uint8_t *dst;

	for(y = 0; y < screen->h; y++)
	for(dst = screen->data + screen->w*y + (y&1), x = (y&1); x < screen->w; x += 2, dst += 2)
		*dst = 0;

}

static void screen_flip_16(void)
{
	int x, y, i;
	int r, g, b;
	uint8_t *palp;

	uint32_t pal_dither[256][2];
	// It's no big deal if the endianness is big or little,
	// it's still sufficient for positional dithering

	// Generate palette
	for(i = 0; i < 256; i++)
	{
		r = pal_main[i][2];
		g = pal_main[i][1];
		b = pal_main[i][0];

		// Assuming the compiler doesn't suck
		uint16_t c01, c10, c11;
		uint16_t c00 = 0
			|((r>>3)<<11)
			|((g>>2)<<5)
			|((b>>3)<<0);
		
		c01 = c00;
		c10 = c00;
		c11 = c00;

		// Apply dither
		if(r < 0xF8 && (r&7) >= 6) c11 += (1<<11);
		if(g < 0xFC && (g&3) >= 3) c11 += (1<<5);
		if(b < 0xF8 && (b&7) >= 6) c11 += (1<<0);

		if(r < 0xF8 && (r&7) >= 4) c01 += (1<<11);
		if(g < 0xFC && (g&3) >= 2) c01 += (1<<5);
		if(b < 0xF8 && (b&7) >= 4) c01 += (1<<0);

		if(r < 0xF8 && (r&7) >= 2) c10 += (1<<11);
		if(g < 0xFC && (g&3) >= 1) c10 += (1<<5);
		if(b < 0xF8 && (b&7) >= 2) c10 += (1<<0);

		// Add to table
		pal_dither[i][0] = (c01<<16)|c00;
		pal_dither[i][1] = (c11<<16)|c10;

	}

	int pitch = screen_surface->pitch;
	int dpitch = pitch - screen_scale * screen->w * 2;
	uint8_t *src = screen->data;
	uint8_t *dst = screen_surface->pixels + pitch * screen_ofy + 2 * screen_ofx;

	switch(screen_scale)
	{
		case 2:
			for(y = 0; y < screen->h; y++, dst += dpitch + pitch)
			for(x = 0; x < screen->w; x++, dst += 4, src++)
			{
				palp = pal_main[*src];
				*(uint32_t *)(dst + 2*0 + pitch*0) = pal_dither[*src][0];
				*(uint32_t *)(dst + 2*0 + pitch*1) = pal_dither[*src][1];

			}
			break;

		default:
			printf("FATAL ERROR: %ix scale for 16bpp not handled yet!\n", screen_scale);
			abort();
			break;
	}

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
		case 16:
			screen_flip_16();
			break;

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


