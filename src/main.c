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
uint8_t pal_src[256][4];
cmap_t *cmaps = NULL;
uint8_t pal_main[256][4];
uint16_t pal_dither[256][2][2]; // For 16bpp modes

// Images
img_t *i_player = NULL;
img_t *i_tiles1 = NULL;

// Colourmaps
uint8_t *cm_player = NULL;

void mainloop_draw(void)
{
	// Clear the screen
	screen_clear(0);

	// Draw an image
	int i;
	for(i = 0; i <  4; i++)
	{
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i, 0, 32*i, 48*6, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i, 0, 32*i, 48*5, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i, 0, 32*i, 48*4, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i, 0, 32*i, 48*3, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i, 0, 32*i, 48*2, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i, 0, 32*i, 48*1, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i, 0, 32*i, 48*0, 32, 48, 0, cm_player);
	}

	// Flip
	screen_flip();
	SDL_Delay(10);

}

void mainloop(void)
{
	SDL_Event ev;

	for(;;)
	{
		// Draw
		mainloop_draw();

		// Process events
		while(SDL_PollEvent(&ev))
		switch(ev.type)
		{
			case SDL_QUIT:
				return;

		}
	}
}

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

	// Load palette and colourmaps
	load_palette("dat/pal1.pal");
	pal_main[0][0] = 170;

	// Load images
	// TODO: png support (and hence the dat/ directory)
	i_player = img_load_tga("tga/player.tga"); 
	i_tiles1 = img_load_tga("tga/tiles1.tga"); 
	cm_player = cmaps[i_player->cmidx].data;

	// TEST: Modify the colourmap
	int i;
	for(i = 0; i < 8; i++)
	{
		cm_player[16+i] = 37+i;
		cm_player[24+i] = 64+i + 8*0;
		cm_player[32+i] = 64+i + 8*5;
		cm_player[40+i] = 64+i + 8*6;
	}

	// Enter main loop
	mainloop();

	// Clean up
	
	// That's all folks!
	return 0;
}

