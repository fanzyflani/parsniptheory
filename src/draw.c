/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

// destination; source; source dimensions
void draw_img_trans_d_sd(img_t *dst, img_t *src, int dx, int dy, int sx, int sy, int sw, int sh, uint8_t tcol)
{
	int x, y;
	uint8_t *sp, *dp;
	int spitch, dpitch;

	// Clip
	if(!clip_d_sd(dst, src, &dx, &dy, &sx, &sy, &sw, &sh)) return;

	// Get pitches and pointers
	spitch = src->w - sw;
	dpitch = dst->w - sw;
	sp = IMG8(src, sx, sy);
	dp = IMG8(dst, dx, dy);

	// Draw
	for(y = 0; y < sh; y++, sp += spitch, dp += dpitch)
	for(x = 0; x < sw; x++, sp++ /*****/, dp++)
		if(*sp != tcol)
			*dp = *sp;
	
	//

}

// destination; source; source dimensions
void draw_img_trans_cmap_d_sd(img_t *dst, img_t *src, int dx, int dy, int sx, int sy, int sw, int sh, uint8_t tcol, uint8_t *cmap)
{
	int x, y;
	uint8_t *sp, *dp;
	int spitch, dpitch;

	// Clip
	if(!clip_d_sd(dst, src, &dx, &dy, &sx, &sy, &sw, &sh)) return;

	// Get pitches and pointers
	spitch = src->w - sw;
	dpitch = dst->w - sw;
	sp = IMG8(src, sx, sy);
	dp = IMG8(dst, dx, dy);

	// Draw
	for(y = 0; y < sh; y++, sp += spitch, dp += dpitch)
	for(x = 0; x < sw; x++, sp++ /*****/, dp++)
		if(*sp != tcol)
			*dp = cmap[*sp];
	
	//

}

