/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

// Screen
SDL_Surface *screen_surface = NULL;
img_t *screen = NULL;
int screen_bpp = 32;
int screen_scale = 2;
int screen_ofx = 0;
int screen_ofy = 0;

// Palette
uint8_t pal_main[256][4];
uint16_t pal_dither[256][2][2]; // For 16bpp modes

// Images
img_t *i_player = NULL;

int main(int argc, char *argv[])
{
	// General SDL setup
	SDL_Init(SDL_INIT_VIDEO);
#ifndef WIN32
	signal(SIGINT,  SIG_DFL);
	signal(SIGTERM, SIG_DFL);
#endif

	// Set up basic video mode
	// TODO: Video mode selector
	SDL_WM_SetCaption("Parsnip Theory - UNRELEASED TESTING version", NULL);
	screen_surface = SDL_SetVideoMode(320 * screen_scale, 200 * screen_scale, screen_bpp, 0);
	screen = img_new(320, 200);

	// Load images
	i_player = img_load_tga("tga/player.tga"); // TODO: png support (and hence the dat/ directory)
	memcpy(pal_main, i_player->pal, 256*4);

	// Draw an image.
	int i;
	screen_clear(0);
	for(i = 0; i < screen->w * screen->h; i++)
		screen->data[i] = (uint8_t)i;

	// "Flip" the screen.
	screen_flip();

	// TODO!
	SDL_Delay(1000);

	// Clean up
	
	// That's all folks!
	return 0;
}

