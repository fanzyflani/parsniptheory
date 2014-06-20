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

void draw_rect_d(img_t *dst, int dx, int dy, int dw, int dh, uint8_t col)
{
	int x, y;
	uint8_t *dp;
	int dpitch;

	// Clip
	if(dx < 0) { dw += dx; dx = 0; }
	if(dy < 0) { dh += dy; dy = 0; }
	if(dx + dw > dst->w) { dw = dst->w - dx; }
	if(dy + dh > dst->h) { dh = dst->h - dy; }

	// Get pitches and pointers
	dpitch = dst->w - dw;
	dp = IMG8(dst, dx, dy);

	// Draw
	for(y = 0; y < dh; y++, dp += dpitch)
	for(x = 0; x < dw; x++, dp++)
		*dp = col;
	
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
	h--; w--;
	draw_hline_d(dst, x,   y,   w+1, c);
	draw_hline_d(dst, x,   y+h, w+1, c);
	draw_vline_d(dst, x,   y+1, h-1, c);
	draw_vline_d(dst, x+w, y+1, h-1, c);
}

void draw_57_printf(img_t *dst, int dx, int dy, uint8_t c, const char *fmt, ...)
{
	va_list va;
	char buf[1024];
	img_t *font = i_font57;
	const int fwidth = 5;
	const int fheight = 7;
	const int fwstep = 4;

	// Firstly, make sure we actually *have* a font.
	if(font == NULL) return;

	// Secondly, make sure dx, dy are in range.
	//if(dx + fsize > screen->w || dy + fsize > screen->h || dy < 0) return;

	// Now format the string.
	va_start(va, fmt);
	vsnprintf(buf, 1023, fmt, va);
	buf[1023] = '\x00';

	// Finally, start drawing it.
	// TODO: Support multiple colours! (i.e. use cmap)
	uint8_t *cp = (uint8_t *)buf;
	for(; *cp != '\x00' /*&& dx + fsize <= screen->w*/; cp++, dx += fwstep)
	{
		// Check if X in range
		//if(dx < 0) continue;

		// Get source char position
		int sx = 0;
		int sy = 0;

		sx = ((*cp)   )&15;
		sy = ((*cp)>>4)&15;

		sx *= fwidth;
		sy *= fheight;

		// Draw
		draw_img_trans_d_sd(dst, font, dx, dy, sx, sy, fwidth, fheight, 0);
	}
}


void draw_printf(img_t *dst, img_t *font, int fsize, int dx, int dy, uint8_t c, const char *fmt, ...)
{
	va_list va;
	char buf[1024];

	// Firstly, make sure we actually *have* a font.
	if(font == NULL) return;

	// Secondly, make sure dx, dy are in range.
	//if(dx + fsize > screen->w || dy + fsize > screen->h || dy < 0) return;

	// Now format the string.
	va_start(va, fmt);
	vsnprintf(buf, 1023, fmt, va);
	buf[1023] = '\x00';

	// Finally, start drawing it.
	// TODO: Support multiple colours! (i.e. use cmap)
	uint8_t *cp = (uint8_t *)buf;
	for(; *cp != '\x00' /*&& dx + fsize <= screen->w*/; cp++, dx += fsize)
	{
		// Check if X in range
		//if(dx < 0) continue;

		// Get source char position
		int sx = (*cp)&15;
		int sy = (*cp)>>4;
		sx *= fsize;
		sy *= fsize;

		// Draw
		draw_img_trans_d_sd(dst, font, dx, dy, sx, sy, fsize, fsize, 0);
	}
}

void draw_layer(img_t *dst, layer_t *ar, int dx, int dy)
{
	int cx1, cy1, cx2, cy2;
	int x, y;
	int i, j;
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
	{
		// Draw tile
		draw_img_trans_cmap_d_sd(dst, i_tiles1,
			x*32-dx, y*24-dy,
			(ce->f.tidx&15)*32,
			(ce->f.tidx>>4)*24,
			32, 24, 0, cm_tiles1);

		// Draw food splatters
		// Interleave between foods
		for(i = 0; i < 4; i++)
		for(j = 0; j < FOOD_COUNT; j++)
		if(ce->splatters[j] & (1<<i))
		{
			draw_img_trans_cmap_d_sd(dst, i_food1,
				x*32-dx, y*24-dy-4,
				i*32 + 128,
				j*64,
				32, 32, 0, cm_food1);
		}

	}

}

void draw_level(img_t *dst, level_t *lv, int dx, int dy, int ayidx)
{
	int i;

	// TODO: Show linked layers
	draw_layer(dst, lv->layers[ayidx], dx, dy);

	// Also draw objects
	for(i = 0; i < lv->ocount; i++)
	{
		obj_t *ob = lv->objects[i];

		if(ob->f.layer == ayidx && ob->f_draw != NULL)
			ob->f_draw(ob, dst, -dx, -dy);
	}

}

