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
};

#define IMG8(img, x, y)  ((x) +  (uint8_t *)(img->w * (y) + (uint8_t *)(img->data)))

// img.c
void img_free(img_t *img);
img_t *img_new(int w, int h);
img_t *img_load_tga(const char *fname);

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

extern uint8_t pal_main[256][4];
extern uint16_t pal_dither[256][2][2]; // For 16bpp modes

