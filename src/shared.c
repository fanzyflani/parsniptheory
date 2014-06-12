/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

// Constants
const int face_dir[4][2] = {
	{ 0, 1},
	{ 1, 0},
	{ 0,-1},
	{-1, 0},
};

// Screen
SDL_Surface *screen_surface = NULL;
img_t *screen = NULL;
int screen_bpp = 32;
int screen_scale = 2;
int screen_ofx = 0;
int screen_ofy = 0;

// Palette
uint8_t pal_src[256][4];
cmap_t *cmaps = NULL;
uint8_t pal_main[256][4];
uint16_t pal_dither[256][2][2]; // For 16bpp modes

// Level
level_t *rootlv = NULL;

// Images
img_t *i_player = NULL;
img_t *i_tiles1 = NULL;
img_t *i_food1 = NULL;
img_t *i_icons1 = NULL;
img_t *i_font16 = NULL;
img_t *i_fontnum1 = NULL;

// Colourmaps
uint8_t *cm_player = NULL;
uint8_t *cm_tiles1 = NULL;
uint8_t *cm_food1 = NULL;

// Teams
team_t *teams[TEAM_MAX];

// Dialogue loop
char *text_dialogue(const char *title, const char *def)
{
	const int tmax = 256;
	char tbuf[256+1];
	int tlen = 0;
	int titlelen = strlen(title);

	if(def == NULL) def = "";
	strncpy(tbuf, def, tmax);
	tbuf[tmax] = '\x00';
	tlen = strlen(tbuf);
	tbuf[tlen] = '\x00';

	input_key_queue_flush();

	for(;;)
	{
		// Draw text
		screen_clear(0);
		draw_printf(screen, i_font16, 16, screen->w/2-8*titlelen, screen->h/2-18, 1, "%s", title);
		draw_printf(screen, i_font16, 16, screen->w/2-8*tlen, screen->h/2+2, 1, "%s", tbuf);

		// Flip
		screen_flip();
		SDL_Delay(20);
		
		// Input
		if(input_poll())
			return NULL;

		while(input_key_queue_peek() != 0)
		{
			int v = input_key_queue_pop();
			if((v & 0x80000000) != 0) continue;

			if(((v>>16)&0x7FFF) == SDLK_RETURN)
			{
				return strdup(tbuf);
			} else if(((v>>16)&0x7FFF) == SDLK_ESCAPE) {
				return NULL;
			} else if(((v>>16)&0x7FFF) == SDLK_BACKSPACE) {
				if(tlen > 0) {
					tlen--;
					tbuf[tlen] = '\x00';
				}
			} else if((v&255) >= 32 && (v&255) <= 126) {
				if(tlen < tmax) {
					tbuf[tlen] = v&255;
					tlen++;
					tbuf[tlen] = '\x00';
				}
			}
		}

	}

}

// Netloop
void netloop(int net_mode)
{
	// TODO!
	for(;;)
	{
		if(input_poll())
			return;

		screen_flip();
		SDL_Delay(20);

	}

}

