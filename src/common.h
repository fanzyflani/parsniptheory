/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#ifndef _PARSNIP_COMMON_H_
#define _PARSNIP_COMMON_H_
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

// Limits
#define TEAM_MAX 128

enum
{
	DIR_SOUTH = 0,
	DIR_EAST  = 1,
	DIR_NORTH = 2,
	DIR_WEST  = 3,
};

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
	int idx;
	uint8_t cm_player[256];

} team_t;

// Here's the level stuff.
enum
{
	CELL_OOB = 0,
	CELL_FLOOR,
	CELL_SOLID,
	CELL_LAYER,

	CELL_BACKWALL,
	CELL_TABLE,

	CELL_COUNT
};

enum
{
	OBJ_FREE = 0,
	OBJ_PLAYER,

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
	void *fd;
	int16_t cx, cy; // measured in  CELLS
	int16_t ox, oy; // measured in PIXELS offset from x,y
	uint16_t fdlen;
	uint16_t flags;
	uint8_t otyp;
	uint8_t layer;
} obj_file_t;

struct obj
{
	// Links
	obj_t *next, *prev;
	obj_t *parent;

	// Function pointers
	int (*f_init)(obj_t *ob);
	int (*f_init_fd)(obj_t *ob);
	int (*f_load_fd)(obj_t *ob, FILE *fp);
	int (*f_save_fd)(obj_t *ob, FILE *fp);
	void (*f_reset)(obj_t *ob);
	void (*f_tick)(obj_t *ob);
	void (*f_draw)(obj_t *ob, img_t *dst, int camx, int camy);
	void (*f_free)(obj_t *ob);

	// Level state
	obj_file_t f;

	// Visible state
	img_t *img;
	uint8_t *cmap;
	int bx, by, bw, bh; // Click box, relative to f.[oc][xy]

	// Extra state for anything to use
	// THIS IS TOTALLY A GOOD IDEA
	int tx, ty;
	void *v1;
	int i1, i2;

};

struct layer
{
	int w, h;
	int x, y;
	cell_t *data;
	int8_t *astar;
};

struct level
{
	int lcount;
	int ocount;
	layer_t **layers;
	obj_t **objects;
};

#define IMG8(img, x, y)  ((x) +  (uint8_t *)(img->w * (y) + (uint8_t *)(img->data)))

// other includes
#include "obj.h"

// cdefs.c
extern cell_file_t *ce_defaults[];

// cell.c
#define MAP_FVERSION 1

void cell_reprep(cell_t *ce, int tset, int tidx);

cell_t *layer_cell_ptr(layer_t *ar, int x, int y);
void layer_free(layer_t *ar);
layer_t *layer_new(int x, int y, int w, int h);

void level_free(level_t *lv);
level_t *level_new(int w, int h);
obj_t *level_obj_add(level_t *lv, int otyp, int flags, int cx, int cy, int layer);
level_t *level_load(const char *fname);
int level_save(level_t *lv, const char *fname);

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

// game.c
extern int game_camx;
extern int game_camy;
int gameloop(void);

// img.c
uint16_t io_get2le(FILE *fp);
void io_put2le(int v, FILE *fp);

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

// obj.c
void obj_free(obj_t *ob);
obj_t *obj_new(int otyp, int flags, int cx, int cy, int layer);
obj_t *obj_load(FILE *fp);
int obj_save(FILE *fp, obj_t *ob);

// screen.c
void screen_clear(uint8_t col);
void screen_flip(void);

// team.c
team_t *team_new(int idx);

// tools.c
int sdiv(int n, int d);
int smod(int n, int d);
int astar_layer(layer_t *ar, int *dirbuf, int dirbuflen, int x1, int y1, int x2, int y2);

// main.c
extern const int face_dir[4][2];

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
extern team_t *teams[TEAM_MAX];

extern uint8_t pal_src[256][4];
extern cmap_t *cmaps;
extern uint8_t pal_main[256][4];
extern uint16_t pal_dither[256][2][2]; // For 16bpp modes

#endif

