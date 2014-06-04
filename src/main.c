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

// Level
level_t *rootlv = NULL;

// Images
img_t *i_player = NULL;
img_t *i_tiles1 = NULL;

// Colourmaps
uint8_t *cm_player = NULL;
uint8_t *cm_tiles1 = NULL;

int sdiv(int n, int d)
{
	if(n >= 0) return n / d;
	else return (n / d) - 1;
}

int smod(int n, int d)
{
	if(n >= 0) return n % d;
	else return d - ((-n) % d);
}

void mainloop_draw(void)
{
	int x, y, i;

	// Clear the screen
	screen_clear(0);

	// TEST: Draw some tiles
	for(y = 0; y < 200; y += 24)
	for(x = 0; x < 320; x += 32)
		draw_img_trans_cmap_d_sd(screen, i_tiles1, x, y, 32*1, 24*0, 32, 24, 0, cm_tiles1);
	
	// Oh, and a back wall might be nice
	for(x = 0; x < 320; x += 32)
		draw_img_trans_cmap_d_sd(screen, i_tiles1, x, 24*0, 32*15, 24*0, 32, 24, 0, cm_tiles1);
	for(x = 0; x < 320; x += 32)
		draw_img_trans_cmap_d_sd(screen, i_tiles1, x, 24*1, 32*4, 24*0, 32, 24, 0, cm_tiles1);
	for(x = 0; x < 320; x += 32)
		draw_img_trans_cmap_d_sd(screen, i_tiles1, x, 24*2, 32*3, 24*0, 32, 24, 0, cm_tiles1);
	for(x = 0; x < 320; x += 32)
		draw_img_trans_cmap_d_sd(screen, i_tiles1, x, 24*3, 32*2, 24*0, 32, 24, 0, cm_tiles1);

	draw_img_trans_cmap_d_sd(screen, i_tiles1, 0, 24*0, 32*13, 24*1, 32, 24, 0, cm_tiles1);
	draw_img_trans_cmap_d_sd(screen, i_tiles1, 0, 24*1, 32*12, 24*3, 32, 24, 0, cm_tiles1);
	draw_img_trans_cmap_d_sd(screen, i_tiles1, 0, 24*2, 32*12, 24*3, 32, 24, 0, cm_tiles1);
	draw_img_trans_cmap_d_sd(screen, i_tiles1, 0, 24*3, 32*12, 24*3, 32, 24, 0, cm_tiles1);
	draw_img_trans_cmap_d_sd(screen, i_tiles1, 0, 24*4, 32*12, 24*2, 32, 24, 0, cm_tiles1);
	draw_img_trans_cmap_d_sd(screen, i_tiles1, 0, 24*5, 32*4, 24*0, 32, 24, 0, cm_tiles1);
	draw_img_trans_cmap_d_sd(screen, i_tiles1, 0, 24*6, 32*3, 24*0, 32, 24, 0, cm_tiles1);
	draw_img_trans_cmap_d_sd(screen, i_tiles1, 0, 24*7, 32*2, 24*0, 32, 24, 0, cm_tiles1);
	
	// TEST: Draw player sprites
	for(i = 0; i <  4; i++)
	{
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i+32*3, 24*5+8, 32*i, 48*6, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i+32*3, 24*5+8, 32*i, 48*5, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i+32*3, 24*5+8, 32*i, 48*4, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i+32*3, 24*5+8, 32*i, 48*3, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i+32*3, 24*5+8, 32*i, 48*2, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i+32*3, 24*5+8, 32*i, 48*1, 32, 48, 0, cm_player);
		draw_img_trans_cmap_d_sd(screen, i_player, 32*i+32*3, 24*5+8, 32*i, 48*0, 32, 48, 0, cm_player);
	}

	// Flip
	screen_flip();
	SDL_Delay(10);

}

void mainloop(void)
{
	for(;;)
	{
		// Draw
		mainloop_draw();

		// Process events
		if(input_poll()) break;
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
	SDL_EnableUNICODE(1);

	// Set up basic video mode
	// TODO: Video mode selector
	SDL_WM_SetCaption("Parsnip Theory - UNRELEASED TESTING version", NULL);
	screen_surface = SDL_SetVideoMode(320 * screen_scale, 200 * screen_scale, screen_bpp, 0);
	screen = img_new(320, 200);

	// Load palette and colourmaps
	load_palette("dat/pal1.pal");
	pal_main[0][0] = 255/5;

	// Load images
	// TODO: png support (and hence the dat/ directory)
	i_player = img_load_tga("tga/player.tga"); 
	i_tiles1 = img_load_tga("tga/tiles1.tga"); 
	cm_player = cmaps[i_player->cmidx].data;
	cm_tiles1 = cmaps[i_tiles1->cmidx].data;

	// TEST: Modify the colourmap
	int i;
	for(i = 0; i < 8; i++)
	{
		cm_player[16+i] = 37+i;
		cm_player[24+i] = 64+i + 8*0;
		cm_player[32+i] = 64+i + 8*5;
		cm_player[40+i] = 64+i + 8*6;
	}

	// Create level
	rootlv = level_new(40, 40);

	// Enter menu loop
	// TODO: Actually have a menu loop.
	// We'll just use one of these two loops.
	editloop();
	//mainloop();

	// Clean up
	
	// That's all folks!
	return 0;
}

