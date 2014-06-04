/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <math.h>

#include <zlib.h>
#include <SDL.h>

#include <assert.h>

#ifndef WIN32
#include <unistd.h>
#include <signal.h>
#endif

// This is a bit simpler than pixra's structure.
// It's probably because this isn't an image editor.
typedef struct img img_t;
struct img
{
	int w, h;
	uint8_t pal[256][4];
	uint8_t *data;
	int cmidx;
};

typedef struct player
{
	//
	int x, y;
} player_t;

typedef struct team
{
	//
} team_t;

enum
{
	CELL_WALK = 0,
	CELL_BLOCK,
};

typedef struct cell_file
{
	uint8_t ctyp;
	uint8_t tset, tidx;
	uint8_t p1;
} cell_file_t;

typedef struct cell
{
	cell_file_t f;
} cell_t;

typedef struct layer
{
	int w, h;
	int x, y;
	cell_t *data;
} layer_t;

typedef struct level
{
	int lcount;
	layer_t *layers;
} level_t;

typedef struct cmap
{
	char *fname;
	uint8_t data[256];
} cmap_t;

#define IMG8(img, x, y)  ((x) +  (uint8_t *)(img->w * (y) + (uint8_t *)(img->data)))

// clip.c
int clip_d_scox(img_t *dst, img_t *src, int *dx, int *dy, int *sx1, int *sy1, int *sx2, int *sy2);
int clip_d_sd(img_t *dst, img_t *src, int *dx, int *dy, int *sx, int *sy, int *sw, int *sh);

// draw.c
void draw_img_trans_d_sd(img_t *dst, img_t *src, int dx, int dy, int sx, int sy, int sw, int sh, uint8_t tcol);
void draw_img_trans_cmap_d_sd(img_t *dst, img_t *src, int dx, int dy, int sx, int sy, int sw, int sh, uint8_t tcol, uint8_t *cmap);

// img.c
void img_free(img_t *img);
img_t *img_new(int w, int h);
img_t *img_load_tga(const char *fname);
void load_palette(const char *fname);

// screen.c
void screen_clear(uint8_t col);
void screen_flip(void);

// main.c
extern SDL_Surface *screen_surface;
extern img_t *screen;
extern int screen_bpp;
extern int screen_scale;
extern int screen_ofx;
extern int screen_ofy;

extern uint8_t pal_src[256][4];
extern cmap_t *cmaps;
extern uint8_t pal_main[256][4];
extern uint16_t pal_dither[256][2][2]; // For 16bpp modes

