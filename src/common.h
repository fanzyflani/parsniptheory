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

typedef struct cmap
{
	char *fname;
	uint8_t data[256];
} cmap_t;

// Here's the player/team stuff.
typedef struct player
{
	//
	int x, y;
} player_t;

typedef struct team
{
	//
} team_t;

// Here's the level stuff.
enum
{
	CELL_OOB = 0,
	CELL_FLOOR,
	CELL_SOLID,
	CELL_LAYER,

	CELL_COUNT
};

enum
{
	OBJ_FREE = 0,
	OBJ_DUMMY, // Useful for visible parts of e.g. a player

	OBJ_COUNT
};

typedef struct cell cell_t;
typedef struct obj obj_t;
typedef struct layer layer_t;
typedef struct level level_t;

typedef struct cell_file
{
	uint8_t ctyp;
	uint8_t tset, tidx;
	uint8_t p1;
} cell_file_t;

struct cell
{
	cell_file_t f;
};

typedef struct obj_file
{
	uint8_t typ;
	uint8_t layer;
	uint8_t p1, p2;
	int16_t cx, cy; // measured in  CELLS
	int16_t ox, oy; // measured in PIXELS offset from x,y
} obj_file_t;

struct obj
{
	// Links
	obj_t *next, *prev;
	obj_t *parent;

	// Level state
	obj_file_t f;
	int cx, cy; // measured in  CELLS
	int ox, oy; // measured in PIXELS offset from x,y

	// Visible state
	int sx, sy, sw, sh;
	img_t *img;
	uint8_t *cmap;

};

struct layer
{
	int w, h;
	int x, y;
	cell_t *data;
};

struct level
{
	int lcount;
	layer_t **layers;
};

#define IMG8(img, x, y)  ((x) +  (uint8_t *)(img->w * (y) + (uint8_t *)(img->data)))

// cdefs.c
extern cell_file_t *ce_defaults[];

// cell.c
void cell_reprep(cell_t *ce, int tset, int tidx);
cell_t *layer_cell_ptr(layer_t *ar, int x, int y);
void layer_free(layer_t *ar);
layer_t *layer_new(int x, int y, int w, int h);
level_t *level_new(int w, int h);

// clip.c
int clip_d_scox(img_t *dst, img_t *src, int *dx, int *dy, int *sx1, int *sy1, int *sx2, int *sy2);
int clip_d_sd(img_t *dst, img_t *src, int *dx, int *dy, int *sx, int *sy, int *sw, int *sh);

// draw.c
void draw_img_trans_d_sd(img_t *dst, img_t *src, int dx, int dy, int sx, int sy, int sw, int sh, uint8_t tcol);
void draw_img_trans_cmap_d_sd(img_t *dst, img_t *src, int dx, int dy, int sx, int sy, int sw, int sh, uint8_t tcol, uint8_t *cmap);
void draw_layer(img_t *dst, layer_t *ar, int dx, int dy);
void draw_level(img_t *dst, level_t *lv, int dx, int dy, int ayidx);
void draw_hline_d(img_t *dst, int x, int y, int len, uint8_t c);
void draw_vline_d(img_t *dst, int x, int y, int len, uint8_t c);
void draw_dot_hline_d(img_t *dst, int x, int y, int len, uint8_t c);
void draw_dot_vline_d(img_t *dst, int x, int y, int len, uint8_t c);
void draw_border_d(img_t *dst, int x, int y, int w, int h, uint8_t c);

// edit.c
int editloop(void);

// img.c
void img_free(img_t *img);
img_t *img_new(int w, int h);
img_t *img_load_tga(const char *fname);
void load_palette(const char *fname);

// input.c
extern int mouse_x, mouse_y, mouse_b;
extern int mouse_ox, mouse_oy, mouse_ob;
extern uint8_t key_state[SDLK_LAST];

void input_key_queue_flush(void);
void input_key_queue_push(uint32_t key);
uint32_t input_key_queue_peek(void);
uint32_t input_key_queue_pop(void);
int input_poll(void);

// screen.c
void screen_clear(uint8_t col);
void screen_flip(void);

// main.c
int sdiv(int n, int d);
int smod(int n, int d);

extern SDL_Surface *screen_surface;
extern img_t *screen;
extern int screen_bpp;
extern int screen_scale;
extern int screen_ofx;
extern int screen_ofy;

extern level_t *rootlv;
extern img_t *i_player;
extern img_t *i_tiles1;
extern uint8_t *cm_player;
extern uint8_t *cm_tiles1;

extern uint8_t pal_src[256][4];
extern cmap_t *cmaps;
extern uint8_t pal_main[256][4];
extern uint16_t pal_dither[256][2][2]; // For 16bpp modes

