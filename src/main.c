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

// Colourmaps
uint8_t *cm_player = NULL;
uint8_t *cm_tiles1 = NULL;

// Teams
team_t *teams[TEAM_MAX];

int main(int argc, char *argv[])
{
	int i;

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

	// Prepare teams
	for(i = 0; i < TEAM_MAX; i++)
		teams[i] = team_new(i);

	// Enter menu loop
	// TODO: Actually have a menu loop.
	// We'll just use one of these two loops.
	//editloop();
	gameloop();

	// Clean up
	
	// That's all folks!
	return 0;
}

