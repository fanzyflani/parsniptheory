/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#ifndef _PARSNIP_COMMON_H_
#define _PARSNIP_COMMON_H_
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <math.h>

#include <zlib.h>
#include <SDL.h>
#ifndef NO_NET
#include <SDL_net.h>
#else
typedef void *TCPsocket;
typedef void *SDLNet_SocketSet;
typedef void *IPaddress;
#define SDLNet_Init(...) 0
#define SDLNet_TCP_Send(...) 0
#define SDLNet_TCP_Recv(...) 0
#define SDLNet_TCP_Accept(...) 0
#define SDLNet_TCP_Open(...) 0
#define SDLNet_TCP_Close(...) 0
#define SDLNet_CheckSockets(...) 0
#define SDLNet_SocketReady(...) 0
#define SDLNet_AllocSocketSet(...) NULL
#define SDLNet_AddSocket(...) 0
#define SDLNet_ResolveHost(...) -1
#endif

#include <sackit.h>

#include <assert.h>

#ifndef WIN32
#include <unistd.h>
#include <signal.h>
#endif

#ifdef __EMSCRIPTEN__
#define SDL_CreateMutex() ((void *)1)
#define SDL_mutexP(x) (0)
#define SDL_mutexV(x) (0)
#endif

// Limits
#define TEAM_MAX 128
#define TEAM_PRACTICAL_MAX 16
// 512KB for abuf just because we have to send maps
#define ABUF_SIZE (1<<19)
#define STEPS_PER_TURN 7
#define STEPS_ATTACK 2
#define TOMATO_SPEED 1
#define PLAYER_HEALTH 100
#define TIME_STEP_MS 20
#define TIME_HOVER_MS 200
#define MAP_BUFFER_SIZE 2048
#define MAP_BUFFER_SEND 2048
#define SND_SPLAT_COUNT 3
#define SND_STEP_COUNT 4
#define ACHN_COUNT 32

// Versions
#define MAP_FVERSION 1
#define NET_VERSION 5

// Other things
#define NET_PORT 2014

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

#define IMG8(img, x, y)  ((x) +  (uint8_t *)(img->w * (y) + (uint8_t *)(img->data)))

typedef struct cmap
{
	char *fname;
	uint8_t data[256];
} cmap_t;

// Some sound stuff.
typedef struct snd
{
	int freq;
	int len;
	int16_t *data;
	int16_t *ldata, *rdata;
} snd_t;

typedef struct achn
{
	int age;
	int nokill;
	int freq;
	int offs, suboffs;
	int vol; // 0x100 = 1.0
	int sx, sy;
	int use_world;

	snd_t *snd;
} achn_t;

// Here's the GUI stuff.
typedef struct widget widget_t;
struct widget
{
	widget_t *parent, *fchild, *lchild;
	widget_t *psib, *nsib;
	int sx, sy;
	int w, h;

	void (*f_free)(widget_t *g);
	void (*f_draw)(widget_t *g, int sx, int sy);
	int (*f_mouse_b)(widget_t *g, int mx, int my, int mb, int db, int ds);
	int (*f_mouse_m)(widget_t *g, int mx, int my, int mb, int dx, int dy);
	int (*f_mouse_f)(widget_t *g, int mx, int my, int enter, int leave);

	// This may resemble the Allegro 4 GUI.
	// Hey, at least it works.
	int i1;
	void *v1;
};

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

typedef struct ai ai_t;
typedef struct cell cell_t;
typedef struct obj obj_t;
typedef struct layer layer_t;
typedef struct level level_t;
typedef struct game game_t;
typedef struct game_settings game_settings_t;

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
	uint8_t splatpos[FOOD_COUNT][4];
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

#define OF_CROUCH 0x0001

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
	int crouch_trans;
	int skintone;
	int please_wait;
	int steps_left;
	int health;
	int tx, ty, tmode;
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
	game_t *game;
	layer_t **layers;
	obj_t **objects;
};


enum
{
	NET_INVALID = 0,

	NET_LOCAL,
	NET_SERVER,
	NET_CLIENT,

	NET_C2S,
	NET_S2C,
};

enum
{
	// NOTE: pstr is a u8 followed by that many bytes
	// DO NOT MOVE THIS THING EVER
	ACT_NOP = 0, // BIDI ()

	// DO NOT MOVE THESE THREE THINGS EVER
	ACT_VERSION, // BIDI (u8 version)
	ACT_QUIT, // BIDI (pstr reason)
	ACT_TEXT, // BIDI (pstr data)

	// BUT YOU CAN MOVE EVERYTHING ELSE LIKE THIS
	ACT_NETID, // S->C (u8 netid)
	ACT_SETTINGS, // see network.c's game_push_settings for more info.
	ACT_CLAIM, // BIDI (u8 netid, u8 team (0xFF == actually claim admin status))
	ACT_UNCLAIM, // BIDI (u8 netid, u8 team (0xFF == actually unclaim admin status))
	ACT_STARTBUTTON, // BIDI ()

	ACT_MAPBEG, // S->C (u16 len_lo, u8 len_hi (len = len_lo + (len_hi<<16)))
	ACT_MAPDATA, // S->C (u16 len, u8 data[len])
	ACT_MAPEND, // S->C ()

	ACT_LOCK, // S->C ()
	ACT_UNLOCK, // S->C ()

	ACT_NEWTURN, // BIDI (u8 player (ignored C->S, 0xFF = game over)), u16 steps_added (ignored C->S))
	ACT_MOVE, // BIDI (s16 sx, s16 sy, s16 dx, s16 dy, u16 steps_used, u16 steps_left)
	ACT_ATTACK, // BIDI (s16 sx, s16 sy, s16 dx, s16 dy, u16 steps_used, u16 steps_left)

	ACT_SELECT, // BIDI (s16 sx, s16 sy)
	ACT_DESELECT, // BIDI ()
	ACT_HOVER, // BIDI (s16 mx, s16 my, s16 camx, s16 camy)

	ACT_CROUCH, // BIDI (s16 sx, s16 sy)
	ACT_STAND, // BIDI (s16 sx, s16 sy)

	ACT_COUNT
};

enum
{
	CLIENT_DEAD = 0,

	CLIENT_PORTBIND,

	CLIENT_WAITVER,
	CLIENT_WAITVERREPLY,

	CLIENT_SETUP,

	CLIENT_SENDMAP,

	CLIENT_LOCKED,
	CLIENT_UNLOCKED,
};

typedef struct abuf abuf_t;
struct abuf
{
	// Connection stuff
	TCPsocket sock;
	SDLNet_SocketSet sset;
	abuf_t *loc_chain;

	// Buffer stuff
	int rsize;
	int wsize;
	uint8_t rdata[ABUF_SIZE];
	uint8_t wdata[ABUF_SIZE];

	// Game control stuff
	int netid;
	int state;
};

enum
{
	GAME_LOGIN0,

	GAME_SETUP,
	GAME_LOADING,
	GAME_WAIT_PLAY,
	GAME_PLAYING,

	GAME_OVER,
};

struct game_settings
{
	int player_count;
	char map_name[256];
};

struct game
{
	int camx, camy;
	int mx, my;
	int cmx, cmy;

	game_settings_t settings;

	int curplayer;
	int main_state;
	int net_mode;
	int curtick;

	int time_now;
	int time_next;
	int time_next_hover;
	int tick_next_pulse;
	int turn_change_cooldown;

	abuf_t *ab_local;
	abuf_t *ab_teams[TEAM_MAX];
	ai_t *ai_teams[TEAM_MAX];

	// 0xFF == unclaimed, 0xFE == ab_local
	uint8_t claim_team[TEAM_MAX];
	uint8_t claim_admin;
	uint8_t netid;

	obj_t *selob;
	level_t *lv;
	FILE *mapfp;
};

struct ai
{
	game_t *game; // Tie this to the model.
	abuf_t *ab;
	int tid;

	int wait;

	void (*f_do_move)(ai_t *ai);
};


// other includes
#include "obj.h"

// action.c
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
void abuf_bc_u8(uint8_t v, game_t *game);
void abuf_bc_s8(int8_t v, game_t *game);
void abuf_bc_u16(uint16_t v, game_t *game);
void abuf_bc_s16(int16_t v, game_t *game);
void abuf_bc_block(const void *buf, int len, game_t *game);
int abuf_bc_get_wspace(game_t *game);

// ai.c
void ai_free(ai_t *ai);
ai_t *ai_new(game_t *game, abuf_t *ab, int tid);

// audio.c
extern volatile sackit_playback_t *sackit;

extern snd_t *snd_splat[];
extern snd_t *snd_step[];
extern it_module_t *mod_titleff1;
extern it_module_t *mod_trk1;
extern it_module_t *mod_trk2;

achn_t *snd_play(snd_t *snd, int vol, int use_world, int sx, int sy, int fmul, int offs, int lockme);
void snd_play_splat(int use_world, int sx, int sy);
void snd_play_step(int use_world, int stepcls, int sx, int sy);
void music_free(it_module_t *mod);
it_module_t *music_load_it(const char *fname);
void music_play(it_module_t *mod);
int audio_init(void);

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
void draw_57_printf(img_t *dst, int dx, int dy, uint8_t c, const char *fmt, ...);
void draw_printf(img_t *dst, img_t *font, int fsize, int align, int dx, int dy, uint8_t c, const char *fmt, ...);

// edit.c
int editloop(void);

// game.c
extern int game_1button;
extern game_t *game_m;
extern game_t *game_v;
void game_free(game_t *game);
int gameloop_player_can_play(game_t *game, int tid);
void gameloop_start_turn(game_t *game, int steps_added);
int gameloop_next_turn(game_t *game, int tid, int steps_added);
int game_tick_playing(game_t *game);
int game_tick(game_t *game);
int gameloop(int net_mode, TCPsocket sock);

// gui.c
int gui_bag_init(widget_t *g, void *ud);
int gui_label_init(widget_t *g, void *ud);
int gui_label57_init(widget_t *g, void *ud);

void gui_reparent(widget_t *gp, widget_t *gc);
void gui_free(widget_t *g);
widget_t *gui_new(int (*f_init)(widget_t *g, void *ud), widget_t *parent, int w, int h, void *ud);
void gui_draw(widget_t *g, int sx, int sy);
void gui_draw_children(widget_t *g, int sx, int sy);
int gui_mouse_b(widget_t *g, int mx, int my, int mb, int db, int ds);
int gui_mouse_m(widget_t *g, int mx, int my, int mb, int dx, int dy);
void gui_mouse_auto(widget_t *g, int sx, int sy);

// img.c
uint16_t io_get2le(FILE *fp);
void io_put2le(int v, FILE *fp);
uint32_t io_get4le(FILE *fp);
void io_put4le(int v, FILE *fp);
uint16_t io_get2be(FILE *fp);
void io_put2be(int v, FILE *fp);
uint32_t io_get4be(FILE *fp);
void io_put4be(int v, FILE *fp);

void img_free(img_t *img);
img_t *img_new(int w, int h);
img_t *img_load_tga(const char *fname);
img_t *img_load_png(const char *fname);
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

// network.c
void game_push_version(game_t *game, abuf_t *ab, int version);
void game_push_settings(game_t *game, abuf_t *ab, game_settings_t *settings);
void game_push_claim(game_t *game, abuf_t *ab, int netid, int tid);
void game_push_unclaim(game_t *game, abuf_t *ab, int netid, int tid);
void game_push_startbutton(game_t *game, abuf_t *ab);
void game_push_hover(game_t *game, abuf_t *ab, int mx, int my, int camx, int camy);
void game_push_newturn(game_t *game, abuf_t *ab, int tid, int steps_added);
void game_push_click(game_t *game, abuf_t *ab, int rmx, int rmy, int camx, int camy, int button);
int game_parse_actions(game_t *game, abuf_t *ab, int typ);

// obj.c
void obj_free(obj_t *ob);
obj_t *obj_new(level_t *lv, int otyp, int flags, int cx, int cy, int layer);
obj_t *obj_load(level_t *lv, FILE *fp);
int obj_save(FILE *fp, obj_t *ob);

// screen.c
void screen_plasma(void);
void screen_clear(uint8_t col);
void screen_dim_halftone(void);
void screen_flip(void);
int screen_setup(void);

// team.c
team_t *team_new(int idx);

// title.c
int titleloop(void);

// tools.c
int sdiv(int n, int d);
int smod(int n, int d);
int astar_layer(layer_t *ar, int *dirbuf, int dirbuflen, int x1, int y1, int x2, int y2);
int line_layer(layer_t *ar, int *rx, int *ry, int x1, int y1, int x2, int y2);
int find_free_neighbour_layer(layer_t *ar, int sx, int sy, int *rx, int *ry);
void errorloop(const char *error);
int options_dialogue(const char *title, const char *opt1, const char *opt2);
char *text_dialogue(const char *title, const char *def);
void chdir_to_exe(const char *farg);

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
extern img_t *i_font57;
#define TITLE_IMAGES 3
extern img_t *i_title[TITLE_IMAGES];
extern img_t *i_titleff1;
extern uint8_t *cm_player;
extern uint8_t *cm_tiles1;
extern uint8_t *cm_food1;
extern team_t *teams[TEAM_MAX];

extern uint8_t pal_src[256][4];
extern cmap_t *cmaps;
extern uint8_t pal_main[256][4];

int load_graphics(void);

#endif

