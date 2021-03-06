/*
Parsnip Theory
Copyright (c) 2014, fanzyflani

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "common.h"
#ifdef WIN32
#include <windows.h>
#endif

int sdiv(int n, int d)
{
	if(n >= 0) return n / d;
	else return (n / d) - 1;
}

int smod(int n, int d)
{
	if(n >= 0) return n % d;
	else return d - ((-n) % d);
}

struct pq_level
{
	int ptotal;
	int pacc;
	int x, y;
};

void pq_check_heap(struct pq_level *pq, int *pqlen)
{
	// Enable this if the heap is being stupid.
#if 0
	int i;
	int ci1, ci2, pidx;

	fflush(stdout);

	for(i = 0; i < *pqlen; i++)
	{
		pidx = ((i+1)>>1)-1;
		ci1 = ((i+1)<<1)-1;
		ci2 = ci1+1;

		if(pidx >= 0)
			assert(pq[pidx].ptotal <= pq[i].ptotal);

		if(ci1 >= *pqlen) continue;
		assert(pq[i].ptotal <= pq[ci1].ptotal);

		if(ci2 >= *pqlen) continue;
		assert(pq[i].ptotal <= pq[ci2].ptotal);
	}
#endif
}

void pq_push(struct pq_level *pq, int *pqlen, int pqmax, int ptotal, int pacc, int x, int y)
{
	int pidx, idx;
	struct pq_level ptemp;

	// Allocate a slot
	idx = *pqlen;
	(*pqlen) += 1;
	assert((*pqlen) <= pqmax);

	// Fill in end entry
	pq[idx].x = x;
	pq[idx].y = y;
	pq[idx].pacc = pacc;
	pq[idx].ptotal = ptotal;

	// Float it up
	while(idx > 0)
	{
		// Get parent
		pidx = ((idx+1)>>1)-1;

		// Compare ptotal
		if(pq[pidx].ptotal <= pq[idx].ptotal)
			break;

		// Swap
		ptemp = pq[idx];
		pq[idx] = pq[pidx];
		pq[pidx] = ptemp;

		// Change indices
		idx = pidx;

	}

	// Check
	pq_check_heap(pq, pqlen);
}

void pq_deque(struct pq_level *pq, int *pqlen)
{
	int idx, cidx;
	struct pq_level ptemp;

	// Decrement length
	(*pqlen)--;
	assert(*pqlen >= 0);

	// Copy end to top
	pq[0] = pq[*pqlen];

	// Float down
	idx = 0;

	for(;;)
	{
		// Get first child
		cidx = ((idx+1)<<1)-1;
		if(cidx >= *pqlen) break;

		// Check if there's a second child
		if(cidx+1 < *pqlen)
		{
			// There is

			// Determine route to take
			if(pq[idx].ptotal <= pq[cidx].ptotal && pq[idx].ptotal <= pq[cidx+1].ptotal)
				// Tree in order
				break;
			else if(pq[cidx+1].ptotal < pq[cidx].ptotal)
				// Swapping with cidx+1
				cidx = cidx+1;
			else
				// Swapping with cidx
				;//cidx = cidx;

		} else {
			// There isn't

			// Do a check
			if(pq[idx].ptotal <= pq[cidx].ptotal)
				// Tree in order
				break;
			else
				// Swapping with cidx
				;//cidx = cidx;

		}

		// Swap
		ptemp = pq[idx];
		pq[idx] = pq[cidx];
		pq[cidx] = ptemp;

		assert(pq[idx].ptotal <= pq[cidx].ptotal);

		// Change indices
		idx = cidx;

	}

	// Check
	pq_check_heap(pq, pqlen);

}

int pq_ptotal(int pacc, int x1, int y1, int x2, int y2)
{
	// Get deltas
	int dx = x1 - x2;
	int dy = y1 - y2;

	// Abs
	if(dx < 0) dx = -dx;
	if(dy < 0) dy = -dy;

	// Finally...
	//return pacc + dx + dy;
	return pacc;

}

#define PQMAX 1024
int astar_layer(layer_t *ar, int *dirbuf, int dirbuflen, int x1, int y1, int x2, int y2)
{
	int x, y, i;
	int dir;
	int dx, dy;
	int ptotal;
	cell_t *ce;

	struct pq_level p;
	struct pq_level pq[PQMAX];
	int pqlen = 0;

	// Very quick test
	if(x1 == x2 && y1 == y2) return 0;

	// Quick test
	ce = layer_cell_ptr(ar, x1, y1);
	if(ce == NULL) return -1;
	if(ce->f.ctyp != CELL_FLOOR) return -1;

	// Clear layer A* table
	memset(ar->astar, 0, ar->w * ar->h);

	// Push
	ptotal = pq_ptotal(0, x1, y1, x2, y2);
	pq_push(pq, &pqlen, PQMAX, ptotal, 0, x1, y1);

	// Mark spot on table
	ar->astar[x1 + y1*ar->w] = 0; // Doesn't matter as long as it's valid + marked

	// Start tracing
	for(;;)
	{
		// Make sure we have something in the queue
		// Otherwise, we couldn't find anything so return -1
		if(pqlen <= 0) return -1;

		// We MUST pop pq[0], otherwise we get race conditions
		// Copy and pop
		p = pq[0];
		pq_deque(pq, &pqlen);

		// Mark cell as visited
		ar->astar[p.x + p.y*ar->w] |= 4;

		//printf("%i: [%i] %i,%i -> %i\n", pqlen, p.pacc, p.x, p.y, ar->astar[p.x + p.y*ar->w]);

		// Are we there yet?
		if(p.x == x2 && p.y == y2)
		{
			// Yep! Let's spew the chain!
			// Check if it will fit
			if(p.pacc > dirbuflen) return -1;
			
			// Get end position
			x = p.x;
			y = p.y;

			// Follow
			for(i = 0; i < p.pacc; i++)
			{
				//printf("%i: %i %i\n", i, x, y);
				// Get direction
				assert(x >= 0 && y >= 0 && x < ar->w && y < ar->h);
				dir = ar->astar[x + y*ar->w] & 3;
				dirbuf[p.pacc - 1 - i] = dir;

				// Get delta
				dx = face_dir[dir][0];
				dy = face_dir[dir][1];

				// Proceed
				x -= dx;
				y -= dy;
			}

			// Return!
			return p.pacc;
		}

		// Loop through neighbours
		for(i = 0; i < 4; i++)
		{
			// Get delta
			dx = face_dir[i][0];
			dy = face_dir[i][1];
			
			// Get position
			x = p.x + dx;
			y = p.y + dy;

			// Check if cell valid
			ce = layer_cell_ptr(ar, x, y);
			if(ce == NULL) continue;
			if(ce->ob != NULL) continue;
			if(ar->astar[x + y*ar->w] >= 4) continue;

			// Mark spot on table
			ar->astar[x + y*ar->w] = i | 4;

			// Check if we can actually move here
			if(ce->f.ctyp != CELL_FLOOR) continue;

			// Push new entry
			ptotal = pq_ptotal(p.pacc + 1, x, y, x2, y2);
			pq_push(pq, &pqlen, PQMAX, ptotal, p.pacc + 1, x, y);

		}

	}

}

static int has_tables(layer_t *ar, int cx, int cy, int gx, int gy)
{
	cell_t *ce;

	// If it's not a floor, return false (we can hit tables / back walls).
	ce = layer_cell_ptr(ar, cx, cy);
	if(ce != NULL && ce->f.ctyp != CELL_FLOOR) return 0;

	// If it's got a standing player, return false (we can hit standing players).
	if(ce != NULL && ce->ob != NULL && ce->ob->f.otyp == OBJ_PLAYER
		&& !(ce->ob->f.flags & OF_CROUCH))
			return 0;

	// Check neighbours, return true if any of them are tables
	ce = layer_cell_ptr(ar, cx-gx, cy   ); if(ce != NULL && ce->f.ctyp == CELL_TABLE) return 1;
	ce = layer_cell_ptr(ar, cx,    cy-gy); if(ce != NULL && ce->f.ctyp == CELL_TABLE) return 1;
	ce = layer_cell_ptr(ar, cx-gx, cy-gy); if(ce != NULL && ce->f.ctyp == CELL_TABLE) return 1;

	// No tables. Return false.
	return 0;
}

int line_layer(layer_t *ar, int *rx, int *ry, int x1, int y1, int x2, int y2)
{
	int trx, try;
	int vx, vy;
	int gx, gy;
	int dx, dy;
	int cx, cy;
	int lcx, lcy;
	int hasnt_hit_end;
	cell_t *ce;

	// If same cell, return false
	if(x1 == x2 && y1 == y2)
	{
		*rx = x1;
		*ry = y1;
		return 0;
	}

	// Shove some dummies in
	if(rx == NULL) rx = &trx;
	if(ry == NULL) ry = &try;

	// Get velocity + direction
	gx = (x2 < x1 ? -1 : x2 != x1 ? 1 : 0);
	gy = (y2 < y1 ? -1 : y2 != y1 ? 1 : 0);
	vx = (x2 < x1 ? x1-x2 : x2-x1);
	vy = (y2 < y1 ? y1-y2 : y2-y1);

	// Get distance to edge
	dx = 1<<8;
	dy = 1<<8;
	lcx = cx = x1;
	lcy = cy = y1;

	// Trace
	hasnt_hit_end = (cx != x2 || cy != y2);
	while(hasnt_hit_end || has_tables(ar, cx, cy, gx, gy))
	{
		// Get cell
		ce = layer_cell_ptr(ar, cx, cy);

		// Check
		if(ce == NULL) break;
		//if(ce->ob != NULL) break;
		if(ce->f.ctyp == CELL_OOB) break;
		if(ce->f.ctyp == CELL_SOLID) break;
		//if(ce->f.ctyp == CELL_BACKWALL) break;

		// Store last
		lcx = cx;
		lcy = cy;

		// Advance
		//if(dx/vx < dy/vy) // What's actually happening.
		//if(time_to_x < time_to_y) // What I'm trying to actually calculate.
		if((hasnt_hit_end && cy == y2) || ((cx != x2 || !hasnt_hit_end)
			&& (vy == 0 || (vx != 0 && dx*vy < dy*vx))))
		{
			// Advance X
			dy -= (dx*vy)/vx;
			cx += gx;
			dx = 2<<8;

		} else {
			// Advance Y
			dx -= (dy*vx)/vy;
			cy += gy;
			dy = 2<<8;

		}

		// Check
		hasnt_hit_end = hasnt_hit_end && (cx != x2 || cy != y2);

		// Get next cell
		ce = layer_cell_ptr(ar, cx, cy);
	}

	// Return
	int ret = (!hasnt_hit_end);
	int cret = (ret || has_tables(ar, cx, cy, gx, gy));
	*rx = (cret ? cx : lcx);
	*ry = (cret ? cy : lcy);

	return (cx == x2 && cy == y2);

}

int find_free_neighbour_layer(layer_t *ar, int sx, int sy, int *rx, int *ry)
{
	cell_t *c;
	int dx, dy;
	int i, t;

	dx = 0; dy = 1;
	for(i = 0; i < 4; i++)
	{
		c = layer_cell_ptr(ar, sx+dx, sy+dy);

		if(c != NULL && c->f.ctyp == CELL_FLOOR && c->ob == NULL)
		{
			*rx = sx + dx;
			*ry = sy + dy;
			return 1;
		}

		t = dx; dx = dy; dy = -t;
	}

	return 0;
}

// Error message loop
void errorloop(const char *error)
{
	input_key_queue_flush();

#ifdef __EMSCRIPTEN__
	if(1) {
		return;
	}
#endif

	for(;;)
	{
		// Draw text
		screen_plasma();
		draw_printf(screen, i_font16, 16, 1, screen->w/2-8*6, screen->h/2-24, 1, "ERROR:");
		draw_printf(screen, i_font16, 16, 1, screen->w/2-8*strlen(error), screen->h/2-8, 1, "%s", error);
		draw_printf(screen, i_font16, 16, 1, screen->w/2-8*19, screen->h/2+8, 1,
			"Click / Press ENTER");

		// Flip
		screen_flip();
		SDL_Delay(20);
		
		// Input
		if(input_poll())
			return;

		while(input_key_queue_peek() != 0)
		{
			int v = input_key_queue_pop();
			if((v & 0x80000000) != 0)
			{
				if(((v>>16)&0x7FFF) == SDLK_RETURN)
				{
					return;
				}

				continue;
			}
		}

		if((mouse_ob & ~mouse_b) != 0)
			return;
	}


}

// Dialogue loop
int options_dialogue(const char *title, const char *opt1, const char *opt2)
{
#ifdef __EMSCRIPTEN__
	if(1) {
		return -2;
	}
#endif
	int titlelen = strlen(title);
	int opt1len = strlen(opt1);
	int opt2len = strlen(opt2);
	int hl1 = 2;
	int hl2 = 2;

	input_key_queue_flush();

	for(;;)
	{
		// Determine selection
		hl1 = (mouse_y >= screen->h/2-4-1 && mouse_y < screen->h/2-4-1+18 ? 0 : 1);
		hl2 = (mouse_y >= screen->h/2+16-1 && mouse_y < screen->h/2+16-1+18 ? 0 : 1);

		// Draw text
		screen_plasma();
		draw_rect_d(screen, screen->w/2-8*opt1len-1, screen->h/2-4-1, opt1len*16+2, 18, 64+4*2+hl1);
		draw_rect_d(screen, screen->w/2-8*opt2len-1, screen->h/2+16-1, opt2len*16+2, 18, 64+4*2+hl2);
		draw_printf(screen, i_font16, 16, 1, screen->w/2-8*titlelen, screen->h/2-30, 1, "%s", title);
		draw_printf(screen, i_font16, 16, 1, screen->w/2-8*opt1len, screen->h/2-4, 1, "%s", opt1);
		draw_printf(screen, i_font16, 16, 1, screen->w/2-8*opt2len, screen->h/2+16, 1, "%s", opt2);

		// Flip
		screen_flip();
		SDL_Delay(20);
		
		// Input
		if(input_poll())
			return -1;

		while(input_key_queue_peek() != 0)
		{
			int v = input_key_queue_pop();
			if((v & 0x80000000) != 0)
			{
				continue;
			}

			if(((v>>16)&0x7FFF) == SDLK_ESCAPE) {
				return -1;
			} 
		}

		if(((mouse_ob & ~mouse_b) & 1) && (hl1 == 0 || hl2 == 0))
		{
			// Eat clicks and return
			if(input_poll()) return -1;
			if(hl1 == 0) return 0;
			if(hl2 == 0) return 1;
		}

	}

}

// Dialogue loop
char *text_dialogue(const char *title, const char *def)
{
#ifdef __EMSCRIPTEN__
	if(1) {
		return NULL;
	}
#endif

	const int tmax = 256;
	char tbuf[256+1];
	int tlen = 0;
	int titlelen = strlen(title);

	if(def == NULL) def = "";
	strncpy(tbuf, def, tmax);
	tbuf[tmax] = '\x00';
	tlen = strlen(tbuf);
	tbuf[tlen] = '\x00';

	input_key_queue_flush();

	for(;;)
	{
		// Draw text
		screen_plasma();
		draw_printf(screen, i_font16, 16, 1, screen->w/2-8*titlelen, screen->h/2-18, 1, "%s", title);
		draw_printf(screen, i_font16, 16, 1, screen->w/2-8*tlen, screen->h/2+2, 1, "%s", tbuf);

		// Flip
		screen_flip();
		SDL_Delay(20);
		
		// Input
		if(input_poll())
			return NULL;

		while(input_key_queue_peek() != 0)
		{
			int v = input_key_queue_pop();
			if((v & 0x80000000) != 0)
			{
				if(((v>>16)&0x7FFF) == SDLK_RETURN)
				{
					return strdup(tbuf);
				}

				continue;
			}

			if(((v>>16)&0x7FFF) == SDLK_ESCAPE) {
				return NULL;
			} else if(((v>>16)&0x7FFF) == SDLK_BACKSPACE) {
				if(tlen > 0) {
					tlen--;
					tbuf[tlen] = '\x00';
				}
			} else if((v&255) >= 32 && (v&255) <= 126) {
				if(tlen < tmax) {
					tbuf[tlen] = v&255;
					tlen++;
					tbuf[tlen] = '\x00';
				}
			}
		}

	}

}

// Netloop
void netloop(int net_mode)
{
	// TODO!
	for(;;)
	{
		if(input_poll())
			return;

		screen_flip();
		SDL_Delay(20);

	}

}

// chdir to exe
void chdir_to_exe(const char *farg)
{
#ifdef __EMSCRIPTEN__
	return;
#else
	char fnbuf[2048] = "";
#ifdef WIN32
	GetModuleFileName(NULL, fnbuf, 2047);
	fnbuf[2047] = '\x00';

	// No, I'm not adding shlwapi.
	char *fol = fnbuf + strlen(fnbuf);
	for(; fol >= fnbuf; fol--)
	if(*fol == '\\')
	{
		// Truncate
		*fol = '\x00';
		break;
	}

	chdir(fnbuf);

#else
	strncpy(fnbuf, farg, 2048);
	fnbuf[2047] = '\x00';

	// There are two possibilities here.
	// 1. It was launched globally. Thus, we... OK, we just go with the current directory.
	// 2. It was launched from some directory. Thus, we follow along.

	// Why do we use this method? Because, well...
	// - Linux has /proc/self/exe.
	// - FreeBSD has /proc/curproc/file, but only if procfs is mounted.
	// - NetBSD I think has /proc/self/exe but only if something's mounted.
	//   I don't quite remember what I needed to do, it's been a while.

	// Seeing as I'm devving on FreeBSD and Raspbian Linux,
	// I don't want too much platform-specific crap *between* the two.
	// But hey, Windows insists on making people do platform-specific crap.

	// *sigh* I hate Windows.

	char *fol = fnbuf + strlen(fnbuf);
	for(; fol >= fnbuf; fol--)
	if(*fol == '/')
	{
		// Truncate
		*fol = '\x00';
		break;
	}

	if(fol >= fnbuf)
		chdir(fnbuf);
	else
		printf("WARNING: Launched globally - running from current dir!\n");

#endif
#endif
}

