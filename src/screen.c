/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

float screen_plasma_ang1 = 0.0;
float screen_plasma_ang2 = 0.0;
int screen_plasma_x = 160;
int screen_plasma_y = 100;

#ifdef NO_PLASMA
void screen_plasma(void)
{
	// Easy.
	memset(screen->data, 0, screen->w * screen->h);
}

#else
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
#endif

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

static uint16_t pal_dither_16[256][2][8];
static void screen_flip_16(void)
{
	int x, y, i;
	int r, g, b;
	uint16_t *palp;

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

		// Add to tables
		pal_dither_16[i][0][0] = c00;
		pal_dither_16[i][0][1] = c01;
		pal_dither_16[i][0][2] = c00;
		pal_dither_16[i][0][3] = c01;
		pal_dither_16[i][0][4] = c00;
		pal_dither_16[i][0][5] = c01;
		pal_dither_16[i][0][6] = c00;
		pal_dither_16[i][0][7] = c01;
		pal_dither_16[i][1][0] = c10;
		pal_dither_16[i][1][1] = c11;
		pal_dither_16[i][1][2] = c10;
		pal_dither_16[i][1][3] = c11;
		pal_dither_16[i][1][4] = c10;
		pal_dither_16[i][1][5] = c11;
		pal_dither_16[i][1][6] = c10;
		pal_dither_16[i][1][7] = c11;

	}

	int pitch = screen_surface->pitch;
	int dpitch = pitch - screen_scale * screen->w * 2;
	uint8_t *src = screen->data;
	uint8_t *dst = screen_surface->pixels + pitch * screen_ofy + 2 * screen_ofx;

	switch(screen_scale)
	{
		case 1:
			for(y = 0; y < screen->h; y++, dst += dpitch + 0*pitch)
			for(x = 0; x < screen->w; x++, dst += 2, src++)
			{
				*(uint16_t *)(dst + 2*0 + pitch*0) = pal_dither_16[*src][y&1][x&1];

			}
			break;

		case 2:
			for(y = 0; y < screen->h; y++, dst += dpitch + 1*pitch)
			for(x = 0; x < screen->w; x++, dst += 4, src++)
			{
				palp = pal_dither_16[*src][0];

				*(uint32_t *)(dst + 2*0 + pitch*0) = *(uint32_t *)(palp + 0);
				*(uint32_t *)(dst + 2*0 + pitch*1) = *(uint32_t *)(palp + 8);
			}
			break;

		case 3:
			for(y = 0; y < screen->h; y++, dst += dpitch + 2*pitch)
			for(x = 0; x < screen->w; x++, dst += 6, src++)
			{
				palp = pal_dither_16[*src][0];

				memcpy(dst + 2*0 + pitch*0, palp + ((y<<2)&8) + (x&1), 6);
				memcpy(dst + 2*0 + pitch*1, palp + (((y<<2)&8)^8) + (x&1), 6);
				memcpy(dst + 2*0 + pitch*2, palp + ((y<<2)&8) + (x&1), 6);
			}
			break;

		case 4:
			for(y = 0; y < screen->h; y++, dst += dpitch + 3*pitch)
			for(x = 0; x < screen->w; x++, dst += 8, src++)
			{
				palp = pal_dither_16[*src][0];

				memcpy(dst + 2*0 + pitch*0, palp + 0, 8);
				memcpy(dst + 2*0 + pitch*1, palp + 8, 8);
				memcpy(dst + 2*0 + pitch*2, palp + 0, 8);
				memcpy(dst + 2*0 + pitch*3, palp + 8, 8);
			}
			break;

		case 5:
			for(y = 0; y < screen->h; y++, dst += dpitch + 4*pitch)
			for(x = 0; x < screen->w; x++, dst += 10, src++)
			{
				palp = pal_dither_16[*src][0];

				memcpy(dst + 2*0 + pitch*0, palp + ((y<<3)&8) + (x&1), 10);
				memcpy(dst + 2*0 + pitch*1, palp + (((y<<3)&8)^8) + (x&1), 10);
				memcpy(dst + 2*0 + pitch*2, palp + ((y<<3)&8) + (x&1), 10);
				memcpy(dst + 2*0 + pitch*3, palp + (((y<<3)&8)^8) + (x&1), 10);
				memcpy(dst + 2*0 + pitch*4, palp + ((y<<3)&8) + (x&1), 10);
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
	int i;
#ifdef __EMSCRIPTEN__
	uint8_t t;
#endif

	// Note, assuming B, G, R, A (which is how the palette is stored)
	// This is correct on all platforms EXCEPT FOR OSX. (and HTML5 apparently)

	int pitch = screen_surface->pitch;
	int dpitch = pitch - screen_scale * screen->w * 4;
	uint8_t *src = screen->data;
	uint8_t *dst = screen_surface->pixels + pitch * screen_ofy + 4 * screen_ofx;

	// Clamp palette to mode 13h range
	for(i = 0; i < 256; i++)
	{
		pal_main[i][0] = ((pal_main[i][0]&0xFC)*0x41)>>6;
		pal_main[i][1] = ((pal_main[i][1]&0xFC)*0x41)>>6;
		pal_main[i][2] = ((pal_main[i][2]&0xFC)*0x41)>>6;
	}

#ifdef __EMSCRIPTEN__
	// Swap R/B
	for(i = 0; i < 256; i++) {
		palp = pal_main[i];
		t = palp[0]; palp[0] = palp[2]; palp[2] = t;
	}
#endif

	switch(screen_scale)
	{
		case 1:
			for(y = 0; y < screen->h; y++, dst += dpitch + 0*pitch)
			for(x = 0; x < screen->w; x++, dst += 4, src++)
			{
				palp = pal_main[*src];

				memcpy(dst + 4*0 + pitch*0, palp, 4);

			}
			break;

		case 2:
			for(y = 0; y < screen->h; y++, dst += dpitch + 1*pitch)
			for(x = 0; x < screen->w; x++, dst += 8, src++)
			{
				palp = pal_main[*src];

				memcpy(dst + 4*0 + pitch*0, palp, 4);
				memcpy(dst + 4*1 + pitch*0, palp, 4);
				memcpy(dst + 4*0 + pitch*1, palp, 4);
				memcpy(dst + 4*1 + pitch*1, palp, 4);

			}
			break;

		case 3:
			for(y = 0; y < screen->h; y++, dst += dpitch + 2*pitch)
			for(x = 0; x < screen->w; x++, dst += 12, src++)
			{
				palp = pal_main[*src];

				memcpy(dst + 4*0 + pitch*0, palp, 4);
				memcpy(dst + 4*1 + pitch*0, palp, 4);
				memcpy(dst + 4*2 + pitch*0, palp, 4);
				memcpy(dst + 4*0 + pitch*1, palp, 4);
				memcpy(dst + 4*1 + pitch*1, palp, 4);
				memcpy(dst + 4*2 + pitch*1, palp, 4);
				memcpy(dst + 4*0 + pitch*2, palp, 4);
				memcpy(dst + 4*1 + pitch*2, palp, 4);
				memcpy(dst + 4*2 + pitch*2, palp, 4);

			}
			break;

		case 4:
			for(y = 0; y < screen->h; y++, dst += dpitch + 3*pitch)
			for(x = 0; x < screen->w; x++, dst += 16, src++)
			{
				palp = pal_main[*src];

				memcpy(dst + 4*0 + pitch*0, palp, 4);
				memcpy(dst + 4*1 + pitch*0, palp, 4);
				memcpy(dst + 4*2 + pitch*0, palp, 4);
				memcpy(dst + 4*3 + pitch*0, palp, 4);
				memcpy(dst + 4*0 + pitch*1, palp, 4);
				memcpy(dst + 4*1 + pitch*1, palp, 4);
				memcpy(dst + 4*2 + pitch*1, palp, 4);
				memcpy(dst + 4*3 + pitch*1, palp, 4);
				memcpy(dst + 4*0 + pitch*2, palp, 4);
				memcpy(dst + 4*1 + pitch*2, palp, 4);
				memcpy(dst + 4*2 + pitch*2, palp, 4);
				memcpy(dst + 4*3 + pitch*2, palp, 4);
				memcpy(dst + 4*0 + pitch*3, palp, 4);
				memcpy(dst + 4*1 + pitch*3, palp, 4);
				memcpy(dst + 4*2 + pitch*3, palp, 4);
				memcpy(dst + 4*3 + pitch*3, palp, 4);

			}
			break;

		case 5:
			for(y = 0; y < screen->h; y++, dst += dpitch + 4*pitch)
			for(x = 0; x < screen->w; x++, dst += 20, src++)
			{
				palp = pal_main[*src];

				memcpy(dst + 4*0 + pitch*0, palp, 4);
				memcpy(dst + 4*1 + pitch*0, palp, 4);
				memcpy(dst + 4*2 + pitch*0, palp, 4);
				memcpy(dst + 4*3 + pitch*0, palp, 4);
				memcpy(dst + 4*4 + pitch*0, palp, 4);
				memcpy(dst + 4*0 + pitch*1, palp, 4);
				memcpy(dst + 4*1 + pitch*1, palp, 4);
				memcpy(dst + 4*2 + pitch*1, palp, 4);
				memcpy(dst + 4*3 + pitch*1, palp, 4);
				memcpy(dst + 4*4 + pitch*1, palp, 4);
				memcpy(dst + 4*0 + pitch*2, palp, 4);
				memcpy(dst + 4*1 + pitch*2, palp, 4);
				memcpy(dst + 4*2 + pitch*2, palp, 4);
				memcpy(dst + 4*3 + pitch*2, palp, 4);
				memcpy(dst + 4*4 + pitch*2, palp, 4);
				memcpy(dst + 4*0 + pitch*3, palp, 4);
				memcpy(dst + 4*1 + pitch*3, palp, 4);
				memcpy(dst + 4*2 + pitch*3, palp, 4);
				memcpy(dst + 4*3 + pitch*3, palp, 4);
				memcpy(dst + 4*4 + pitch*3, palp, 4);
				memcpy(dst + 4*0 + pitch*4, palp, 4);
				memcpy(dst + 4*1 + pitch*4, palp, 4);
				memcpy(dst + 4*2 + pitch*4, palp, 4);
				memcpy(dst + 4*3 + pitch*4, palp, 4);
				memcpy(dst + 4*4 + pitch*4, palp, 4);
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

int screen_setup(void)
{
	// Flush input
	input_key_queue_flush();

	// Set palette
	pal_main[0][0] = 170;
	pal_main[0][1] = 0;
	pal_main[0][2] = 0;
	pal_main[1][0] = 255;
	pal_main[1][1] = 255;
	pal_main[1][2] = 255;
	pal_main[2][0] = 0;
	pal_main[2][1] = 0;
	pal_main[2][2] = 0;
	pal_main[3][0] = 85;
	pal_main[3][1] = 85;
	pal_main[3][2] = 85;
	pal_main[4][0] = 170;
	pal_main[4][1] = 170;
	pal_main[4][2] = 170;

	int sel_bpp = screen_bpp;
	int sel_scale = screen_scale;

#define SETUP_BUTTON(x, y, w, h) (mouse_x >= (x) && mouse_y >= (y) && mouse_x < (x)+(w) && mouse_y < (y)+(h))
#define DRAW_SETUP_BUTTON(x, y, w, h, u, s) draw_rect_d(screen, (x), (y), (w), (h), SETUP_BUTTON(x,y,w,h) ? s : u)
	for(;;)
	{
		// Poll for input
		if(input_poll())
			return 0;

		// Check buttons
		if((mouse_ob&~mouse_b))
		{
			if(SETUP_BUTTON(screen->w-16*5, screen->h-8*4, 16*5, 8*4))
			{
				// GO!

				screen_surface = SDL_SetVideoMode(320 * sel_scale, 200 * sel_scale, sel_bpp, SDL_SWSURFACE);
				if(screen_surface == NULL)
				{
					printf("FAILED TO SET VIDEO MODE. Use the test button, please!\n");
					return 0;
				} else {
					screen_bpp = sel_bpp;
					screen_scale = sel_scale;
				}

				return 1;

			} else if(SETUP_BUTTON(0, screen->h-8*4, 16*6, 8*4)) {
				// TEST
				screen_surface = SDL_SetVideoMode(320 * sel_scale, 200 * sel_scale, sel_bpp, SDL_SWSURFACE);
				if(screen_surface == NULL)
				{
					screen_surface = SDL_SetVideoMode(320 * screen_scale, 200 * screen_scale, screen_bpp, SDL_SWSURFACE);
				} else {
					screen_bpp = sel_bpp;
					screen_scale = sel_scale;
				}

			} else if(SETUP_BUTTON(16*1, 32+44*0+18, 16*2+4, 20)) {
				// BPP 16
				sel_bpp = 16;

			} else if(SETUP_BUTTON(16*4, 32+44*0+18, 16*2+4, 20)) {
				// BPP 32 
				sel_bpp = 32;

			} else if(SETUP_BUTTON(16*1, 32+44*1+18, 16*2+4, 20)) {
				// Scale 1x
				sel_scale = 1;

			} else if(SETUP_BUTTON(16*4, 32+44*1+18, 16*2+4, 20)) {
				// Scale 2x 
				sel_scale = 2;

			} else if(SETUP_BUTTON(16*7, 32+44*1+18, 16*2+4, 20)) {
				// Scale 3x 
				sel_scale = 3;

			} else if(SETUP_BUTTON(16*10, 32+44*1+18, 16*2+4, 20)) {
				// Scale 4x 
				sel_scale = 4;

			} else if(SETUP_BUTTON(16*13, 32+44*1+18, 16*2+4, 20)) {
				// Scale 5x 
				sel_scale = 5;

			}
		}
		// Clear screen
		screen_clear(0);

		// Update screen
		draw_printf(screen, i_font16, 16, 1, 0, 0, 1, "STARTUP CONFIG");

		// TODO: refactor this into a proper GUI thing
		// GO! / TEST
		DRAW_SETUP_BUTTON(screen->w-16*5, screen->h-8*4, 16*5, 8*4, 4, 1);
		draw_printf(screen, i_font16, 16, 0, screen->w-8*5, screen->h-8*3, 1, "GO!");
		DRAW_SETUP_BUTTON(0, screen->h-8*4, 16*6, 8*4, 4, 1);
		draw_printf(screen, i_font16, 16, 0, 8*6, screen->h-8*3, 1, "TEST");

		// BPP
		draw_printf(screen, i_font16, 16, 1, 8, 32+44*0, 1, "COLOUR DEPTH");
		DRAW_SETUP_BUTTON(16*1, 32+44*0+18, 16*2+4, 20, (sel_bpp == 16 ? 4 : 3), 1);
		draw_printf(screen, i_font16, 16, 1, 16*1, 32+44*0+20, 1, "16");
		DRAW_SETUP_BUTTON(16*4, 32+44*0+18, 16*2+4, 20, (sel_bpp == 32 ? 4 : 3), 1);
		draw_printf(screen, i_font16, 16, 1, 16*4, 32+44*0+20, 1, "32");

		draw_printf(screen, i_font16, 16, 1, 8, 32+44*1, 1, "SCALE");
		DRAW_SETUP_BUTTON(16*1, 32+44*1+18, 16*2+4, 20, (sel_scale == 1 ? 4 : 3), 1);
		draw_printf(screen, i_font16, 16, 1, 16*1, 32+44*1+20, 1, "1x");
		DRAW_SETUP_BUTTON(16*4, 32+44*1+18, 16*2+4, 20, (sel_scale == 2 ? 4 : 3), 1);
		draw_printf(screen, i_font16, 16, 1, 16*4, 32+44*1+20, 1, "2x");
		DRAW_SETUP_BUTTON(16*7, 32+44*1+18, 16*2+4, 20, (sel_scale == 3 ? 4 : 3), 1);
		draw_printf(screen, i_font16, 16, 1, 16*7, 32+44*1+20, 1, "3x");
		DRAW_SETUP_BUTTON(16*10, 32+44*1+18, 16*2+4, 20, (sel_scale == 4 ? 4 : 3), 1);
		draw_printf(screen, i_font16, 16, 1, 16*10, 32+44*1+20, 1, "4x");
		DRAW_SETUP_BUTTON(16*13, 32+44*1+18, 16*2+4, 20, (sel_scale == 5 ? 4 : 3), 1);
		draw_printf(screen, i_font16, 16, 1, 16*13, 32+44*1+20, 1, "5x");

		// Flip and wait
		screen_flip();
		SDL_Delay(10);
	}
}



