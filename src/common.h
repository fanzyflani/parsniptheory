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
#include <SDL_net.h>

#include <assert.h>

#ifndef WIN32
#include <unistd.h>
#include <signal.h>
#endif

// Limits
#define TEAM_MAX 128
#define ABUF_SIZE 8192
#define STEPS_PER_TURN 7
#define STEPS_ATTACK 2
#define TOMATO_SPEED 1
#define PLAYER_HEALTH 100

// Versions
#define MAP_FVERSION 1
#define NET_VERSION 1

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

	OBJ_FOOD_TOMATO,

	OBJ_COUNT
};

enum
{
	FOOD_TOMATO = 0,

	FOOD_COUNT
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
	obj_t *ob;
	uint8_t splatters[FOOD_COUNT];
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
	int please_wait;
	int steps_left;
	int health;
	int tx, ty;
	int vx, vy;
	int *asdir;
	int aslen, asidx;
	int time;

	level_t *level;

	int freeme;

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

enum
{
	NET_INVALID = 0,

	NET_LOCAL,
	NET_SERVER,
	NET_CLIENT,
};

enum
{
	// NOTE: pstr is a u8 followed by that many bytes
	ACT_NOP = 0, // BIDI ()

	ACT_VERSION, // BIDI (u8 version)
	ACT_QUIT, // BIDI (pstr reason)
	ACT_TEXT, // BIDI (pstr data)
	ACT_MAPBEG, // S->C ()
	ACT_MAPDATA, // S->C (u16 len, u8 data[len])
	ACT_MAPEND, // S->C ()

	ACT_LOCK, // S->C ()
	ACT_UNLOCK, // S->C ()

	ACT_NEWTURN, // BIDI (u8 player (ignored C->S), u16 steps_added (ignored C->S))
	ACT_MOVE, // BIDI (s16 sx, s16 sy, s16 dx, s16 dy, u16 steps_used, u16 steps_remain)
	ACT_ATTACK, // BIDI (s16 sx, s16 sy, s16 dx, s16 dy, u16 steps_used, u16 steps_remain)

	ACT_SELECT, // BIDI (s16 sx, s16 sy)
	ACT_DESELECT, // BIDI ()
	ACT_HOVER, // BIDI (s16 mx, s16 my, s16 camx, s16 camy)

	ACT_COUNT
};

enum
{
	STATE_DEAD = 0,

	STATE_GIVEVER,
	STATE_WAITVER,

	STATE_LOCKED,
	STATE_UNLOCKED,
};

typedef struct abuf abuf_t;
struct abuf
{
	TCPsocket sock;
	int state;
	abuf_t *loc_chain;
	void (*f_cont)(abuf_t *ab, void *ud);

	int rsize;
	int wsize;
	uint8_t rdata[ABUF_SIZE];
	uint8_t wdata[ABUF_SIZE];
};

// other includes
#include "obj.h"

// action.c
abuf_t *ab_local;
abuf_t *ab_teams[TEAM_MAX];

void abuf_free(abuf_t *ab);
abuf_t *abuf_new(void);
int abuf_get_rsize(abuf_t *ab);
int abuf_get_rspace(abuf_t *ab);
int abuf_get_wsize(abuf_t *ab);
int abuf_get_wspace(abuf_t *ab);
uint8_t abuf_read_u8(abuf_t *ab);
int8_t abuf_read_s8(abuf_t *ab);
uint16_t abuf_read_u16(abuf_t *ab);
int16_t abuf_read_s16(abuf_t *ab);
void abuf_read_block(void *buf, int len, abuf_t *ab);
void abuf_write_u8(uint8_t v, abuf_t *ab);
void abuf_write_s8(int8_t v, abuf_t *ab);
void abuf_write_u16(uint16_t v, abuf_t *ab);
void abuf_write_s16(int16_t v, abuf_t *ab);
void abuf_write_block(const void *buf, int len, abuf_t *ab);
void abuf_poll_write(abuf_t *ab);
void abuf_poll_read(abuf_t *ab);
void abuf_poll(abuf_t *ab);

// cdefs.c
extern cell_file_t *ce_defaults[];

// cell.c
void cell_reprep(cell_t *ce, int tset, int tidx);

cell_t *layer_cell_ptr(layer_t *ar, int x, int y);
void layer_free(layer_t *ar);
layer_t *layer_new(int x, int y, int w, int h);

void level_free(level_t *lv);
level_t *level_new(int w, int h);
obj_t *level_obj_add(level_t *lv, int otyp, int flags, int cx, int cy, int layer);
int level_obj_free(level_t *lv, obj_t *ob);
obj_t *level_obj_waiting(level_t *lv);
level_t *level_load(const char *fname);
int level_save(level_t *lv, const char *fname);

// clip.c
int clip_d_scox(img_t *dst, img_t *src, int *dx, int *dy, int *sx1, int *sy1, int *sx2, int *sy2);
int clip_d_sd(img_t *dst, img_t *src, int *dx, int *dy, int *sx, int *sy, int *sw, int *sh);

// draw.c
void draw_img_trans_d_sd(img_t *dst, img_t *src, int dx, int dy, int sx, int sy, int sw, int sh, uint8_t tcol);
void draw_img_trans_cmap_d_sd(img_t *dst, img_t *src, int dx, int dy, int sx, int sy, int sw, int sh, uint8_t tcol, uint8_t *cmap);
void draw_rect_d(img_t *dst, int dx, int dy, int dw, int dh, uint8_t col);
void draw_layer(img_t *dst, layer_t *ar, int dx, int dy);
void draw_level(img_t *dst, level_t *lv, int dx, int dy, int ayidx);
void draw_hline_d(img_t *dst, int x, int y, int len, uint8_t c);
void draw_vline_d(img_t *dst, int x, int y, int len, uint8_t c);
void draw_dot_hline_d(img_t *dst, int x, int y, int len, uint8_t c);
void draw_dot_vline_d(img_t *dst, int x, int y, int len, uint8_t c);
void draw_border_d(img_t *dst, int x, int y, int w, int h, uint8_t c);
void draw_num1_printf(img_t *dst, int dx, int dy, uint8_t c, const char *fmt, ...);
void draw_printf(img_t *dst, img_t *font, int fsize, int dx, int dy, uint8_t c, const char *fmt, ...);

// edit.c
int editloop(void);

// game.c
extern int game_camx;
extern int game_camy;
int gameloop(const char *fname, int net_mode, int player_count);

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
int line_layer(layer_t *ar, int *rx, int *ry, int x1, int y1, int x2, int y2);

// shared.c
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
extern img_t *i_food1;
extern img_t *i_icons1;
extern img_t *i_font16;
extern img_t *i_fontnum1;
extern uint8_t *cm_player;
extern uint8_t *cm_tiles1;
extern uint8_t *cm_food1;
extern team_t *teams[TEAM_MAX];

extern uint8_t pal_src[256][4];
extern cmap_t *cmaps;
extern uint8_t pal_main[256][4];
extern uint16_t pal_dither[256][2][2]; // For 16bpp modes

#endif

