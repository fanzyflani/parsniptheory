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

static void menu_draw_player(int x, int y, int team, int face)
{
	int i;

	for(i = 6; i >= 0; i--)
		draw_img_trans_cmap_d_sd(screen, i_player,
			x,
			y,
			face*32, i*48, 32, 48,
			0, teams[team]->cm_player);
}

int menuloop(void)
{
	int i;

	for(;;)
	{
		// Clear the screen
		screen_clear(0);

		// Work out what's selected
		int sel = -1;

		if(mouse_x >= 10 && mouse_x < screen->w/2 - 10)
		{
			if(mouse_y >= 10 && mouse_y < screen->h/2 - 10)
			{
				sel = 0;

			} else if(mouse_y >= screen->h/2 + 10 && mouse_y < screen->h - 10) {
				sel = 2;

			}

		} else if(mouse_x >= screen->w/2 + 10 && mouse_x < screen->w - 10) {
			if(mouse_y >= 10 && mouse_y < screen->h/2 - 10)
			{
				sel = 1;

			} else if(mouse_y >= screen->h/2 + 10 && mouse_y < screen->h - 10) {
				sel = 3;

			}


		}

		// Draw rectangles
		draw_rect_d(screen, 10, 10, screen->w/2-20, screen->h/2-20, 64 + (sel==0 ? 0 : 2) + 8*0);
		draw_rect_d(screen, screen->w/2 + 10, 10, screen->w/2-20, screen->h/2-20, 64 + (sel==1 ? 0 : 2) + 8*1);
		draw_rect_d(screen, 10, screen->h/2 + 10, screen->w/2-20, screen->h/2-20, 64 + (sel==2 ? 0 : 2) + 8*2);
		draw_rect_d(screen, screen->w/2 + 10, screen->h/2 + 10, screen->w/2-20, screen->h/2-20, 64 + (sel==3 ? 0 : 2) + 8*3);

		// Draw backgrounds
		for(i = 0; i < 2; i++)
			menu_draw_player(screen->w/2 - 10 - 32 - 28*1 + 28*i, screen->h/2 - 10 - 44, i, 0);
		for(i = 0; i < 3; i++)
			menu_draw_player(screen->w - 10 - 32 - 28*2 + 28*i, screen->h/2 - 10 - 44, i, 0);
		for(i = 0; i < 4; i++)
			menu_draw_player(screen->w/2 - 10 - 32 - 28*3 + 28*i, screen->h - 10 - 44, i, 0);

		draw_img_trans_cmap_d_sd(screen, i_tiles1,
			screen->w - 10 - 10 - 36*3, screen->h - 10 - 10 - 24,
			32*1, 24*0, 32, 24, 0, cm_tiles1);
		draw_img_trans_cmap_d_sd(screen, i_tiles1,
			screen->w - 10 - 10 - 36*2, screen->h - 10 - 10 - 24,
			32*5, 24*0, 32, 24, 0, cm_tiles1);
		draw_img_trans_cmap_d_sd(screen, i_tiles1,
			screen->w - 10 - 10 - 36*1, screen->h - 10 - 10 - 24,
			32*12, 24*0, 32, 24, 0, cm_tiles1);

		// Draw texts
		draw_printf(screen, i_font16, 16, 15, 20, 1, "2 PLAYER");
		draw_printf(screen, i_font16, 16, 15 + screen->w/2, 20, 1, "3 PLAYER");
		draw_printf(screen, i_font16, 16, 15, 20 + screen->h/2, 1, "4 PLAYER");
		draw_printf(screen, i_font16, 16, 15 + screen->w/2, 20 + screen->h/2, 1, "LVL EDIT");
		draw_printf(screen, i_font16, 16, screen->w/2 - 8*(7+1+6), screen->h/2 - 8, 1,
			"PARSNIP THEORY");

		// Flip
		screen_flip();
		SDL_Delay(20);

		// Check if clicked
		if((mouse_ob & 1) && !(mouse_b & 1))
		switch(sel)
		{
			case 0:
				return 2;
			case 1:
				return 3;
			case 2:
				return 4;
			case 3:
				return 0xED17;

		}

		// Poll input
		if(input_poll()) return -1;

	}

}

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
	SDL_WM_SetCaption("Parsnip Theory - SHAREWARE (alpha 1)", NULL);
	screen_surface = SDL_SetVideoMode(320 * screen_scale, 200 * screen_scale, screen_bpp, 0);
	screen = img_new(320, 200);

	// Load palette and colourmaps
	load_palette("dat/pal1.pal");
	pal_main[0][0] = 255/5;

	// Load images
	// TODO: png support (and hence the dat/ directory)
	i_player = img_load_tga("tga/player.tga"); 
	i_tiles1 = img_load_tga("tga/tiles1.tga"); 
	i_food1 = img_load_tga("tga/food1.tga"); 
	i_icons1 = img_load_tga("tga/icons1.tga"); 
	i_font16 = img_load_tga("tga/font16.tga"); 
	i_fontnum1 = img_load_tga("tga/fontnum1.tga"); 
	cm_player = cmaps[i_player->cmidx].data;
	cm_tiles1 = cmaps[i_tiles1->cmidx].data;
	cm_food1 = cmaps[i_food1->cmidx].data;

	// Prepare teams
	for(i = 0; i < TEAM_MAX; i++)
		teams[i] = team_new(i);

	// Enter menu loop
	for(;;)
	switch(menuloop())
	{
		case 2:
			gameloop("dat/level.psl", 2);
			break;

		case 3:
			gameloop("dat/level.psl", 3);
			break;

		case 4:
			gameloop("dat/level.psl", 4);
			break;

		case 0xED17:
			editloop();
			break;
		
		case -1:
			// Clean up
			
			// That's all folks!
			return 0;
	}
}

