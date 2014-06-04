/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

// destination; source; source coordinates, ordered, exclusive
int clip_d_scox(img_t *dst, img_t *src, int *dx, int *dy, int *sx1, int *sy1, int *sx2, int *sy2)
{
	// Clip s[xy][12] to fit its own image
	if(*sx1 < 0) { *dx -= *sx1; *sx1 = 0; }
	if(*sy1 < 0) { *dy -= *sy1; *sy1 = 0; }
	if(*sx2 > src->w) { *sx2 = src->w; }
	if(*sy2 > src->h) { *sy2 = src->h; }

	// Clip s[xy][12] to fit its destination image with respect to d[xy]
	if(*dx < 0) { *sx1 -= *dx; *dx = 0; }
	if(*dy < 0) { *sy1 -= *dy; *dy = 0; }
	if(*sx2 - *sx1 + *dx > dst->w) { *sx2 = *sx1 + dst->w - *dx; }
	if(*sy2 - *sy1 + *dy > dst->h) { *sy2 = *sy1 + dst->h - *dy; }

	// Ensure destination doesn't fall off the edge
	if(*dx >= dst->w) return 0;
	if(*dy >= dst->h) return 0;

	// Ensure source box doesn't close in on itself
	if(*sx1 >= *sx2) return 0;
	if(*sy1 >= *sy2) return 0;

	// Success!
	return 1;
}

// destination; source; source dimensions
int clip_d_sd(img_t *dst, img_t *src, int *dx, int *dy, int *sx, int *sy, int *sw, int *sh)
{
	// Convert
	int sx2 = *sx + *sw;
	int sy2 = *sy + *sh;

	// Clip
	if(!clip_d_scox(dst, src, dx, dy, sx, sy, &sx2, &sy2)) return 0;

	// Deconvert
	*sw = sx2 - *sx;
	*sh = sy2 - *sy;

	// Return
	return 1;
}

