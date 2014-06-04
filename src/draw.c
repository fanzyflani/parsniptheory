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

void draw_hline_d(img_t *dst, int x, int y, int len, uint8_t c)
{
	if(y < 0 || y >= dst->h) return;
	if(x < 0) { len += x; x = 0; }
	if(x + len > dst->w) { len = dst->w - x; }

	uint8_t *dp = dst->data + x + y*dst->w;

	for(; len > 0; len--, dp++)
		*dp = c;
}

void draw_vline_d(img_t *dst, int x, int y, int len, uint8_t c)
{
	if(x < 0 || x >= dst->w) return;
	if(y < 0) { len += y; y = 0; }
	if(y + len > dst->h) { len = dst->h - y; }

	uint8_t *dp = dst->data + x + y*dst->w;

	for(; len > 0; len--, dp += dst->w)
		*dp = c;
}

void draw_dot_hline_d(img_t *dst, int x, int y, int len, uint8_t c)
{
	if(y < 0 || y >= dst->h) return;
	if(x < 0) { len += x; x = 0; }
	if(x + len > dst->w) { len = dst->w - x; }
	if(((x^y)&1) != 0) { x++; len--; }
	len >>= 1;

	uint8_t *dp = dst->data + x + y*dst->w;

	for(; len > 0; len--, dp += 2)
		*dp = c;
}

void draw_dot_vline_d(img_t *dst, int x, int y, int len, uint8_t c)
{
	if(x < 0 || x >= dst->w) return;
	if(y < 0) { len += y; y = 0; }
	if(y + len > dst->h) { len = dst->h - y; }
	if(((x^y)&1) != 0) { y++; len--; }
	len >>= 1;

	uint8_t *dp = dst->data + x + y*dst->w;

	for(; len > 0; len--, dp += dst->w << 1)
		*dp = c;
}

void draw_border_d(img_t *dst, int x, int y, int w, int h, uint8_t c)
{
	draw_hline_d(dst, x,   y,   x+w,   c);
	draw_hline_d(dst, x,   y+h, x+w,   c);
	draw_vline_d(dst, x,   y+1, y+h-2, c);
	draw_vline_d(dst, x+w, y+1, y+h-2, c);
}

void draw_layer(img_t *dst, layer_t *ar, int dx, int dy)
{
	int cx1, cy1, cx2, cy2;
	int x, y;
	cell_t *ce;
	int cestep;

	// Get cell boundaries
	cx1 = dx / 32;
	cy1 = dy / 24;
	cx2 = (dx + 320 -1) / 32;
	cy2 = (dy + 200 -1) / 24;

	// Clamp to layer boundary
	if(cx1 < ar->x) cx1 = ar->x;
	if(cy1 < ar->y) cy1 = ar->y;
	if(cx2 >= ar->x + ar->w) cx2 = ar->x + ar->w - 1;
	if(cy2 >= ar->y + ar->h) cy2 = ar->y + ar->h - 1;

	// Bail out if collapsed
	if(cx1 > cx2) return;
	if(cy1 > cy2) return;

	// Draw cells
	// TODO: support tilesets other than 0 (tiles1.tga)
	cestep = ar->w - (cx2-cx1+1);
	ce = ar->data + cx1 + cy1*ar->w;
	for(y = cy1; y <= cy2; y++, ce += cestep)
	for(x = cx1; x <= cx2; x++, ce++)
		draw_img_trans_cmap_d_sd(dst, i_tiles1,
			x*32-dx, y*24-dy,
			(ce->f.tidx&15)*32,
			(ce->f.tidx>>4)*24,
			32, 24, 0, cm_tiles1);

}

void draw_level(img_t *dst, level_t *lv, int dx, int dy, int ayidx)
{
	// TODO: Show linked layers
	draw_layer(dst, lv->layers[ayidx], dx, dy);
}

