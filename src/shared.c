/*
Parsnip Theory
Copyright (c) 2014, fanzyflani

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
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
int screen_fullscreen = 0;
#ifdef SCREEN_BPP
int screen_bpp = SCREEN_BPP;
#else
int screen_bpp = 32;
#endif
#ifdef SCREEN_SCALE
int screen_scale = SCREEN_SCALE;
#else
int screen_scale = 2;
#endif
#if defined(SCREEN_WIDTH) || defined(SCREEN_HEIGHT)
int screen_width = SCREEN_WIDTH;
int screen_height = SCREEN_HEIGHT;
#else
int screen_width = 320;
int screen_height = 200;
#endif
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
img_t *i_font57 = NULL;
img_t *i_titleff1 = NULL;
img_t *i_title[TITLE_IMAGES];

// Colourmaps
uint8_t *cm_player = NULL;
uint8_t *cm_tiles1 = NULL;
uint8_t *cm_food1 = NULL;

// Teams
team_t *teams[TEAM_MAX];

int load_graphics(void)
{
	char buf[128];
	int i;

#ifdef NO_ZLIB
	i_player = img_load_tga("tga/player.tga"); 
	i_tiles1 = img_load_tga("tga/tiles1.tga"); 
	i_food1 = img_load_tga("tga/food1.tga"); 
	i_icons1 = img_load_tga("tga/icons1.tga"); 
	i_font16 = img_load_tga("tga/font16.tga"); 
	i_font57 = img_load_tga("tga/font57.tga"); 
	i_titleff1 = img_load_tga("tga/titleff1.tga"); 

	for(i = 0; i < TITLE_IMAGES; i++)
	{
		sprintf(buf, "tga/title%i.tga", i+1);
		i_title[i] = img_load_tga(buf);
	}

#else
	i_player = img_load_png("dat/player.img"); 
	i_tiles1 = img_load_png("dat/tiles1.img"); 
	i_food1 = img_load_png("dat/food1.img"); 
	i_icons1 = img_load_png("dat/icons1.img"); 
	i_font16 = img_load_png("dat/font16.img"); 
	i_font57 = img_load_png("dat/font57.img"); 
	i_titleff1 = img_load_png("dat/titleff1.img"); 

	for(i = 0; i < TITLE_IMAGES; i++)
	{
		sprintf(buf, "dat/title%i.img", i+1);
		i_title[i] = img_load_png(buf);
	}

#endif
	cm_player = cmaps[i_player->cmidx].data;
	cm_tiles1 = cmaps[i_tiles1->cmidx].data;
	cm_food1 = cmaps[i_food1->cmidx].data;

	return 1;
}


