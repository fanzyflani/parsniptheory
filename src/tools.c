/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

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

int line_layer(layer_t *ar, int *rx, int *ry, int x1, int y1, int x2, int y2)
{
	int trx, try;
	int vx, vy;
	int gx, gy;
	int dx, dy;
	int cx, cy;
	cell_t *ce;

	// Shove some dummies in
	if(rx == NULL) rx = &trx;
	if(ry == NULL) ry = &try;

	// Get velocity + direction
	gx = (x2 < x1 ? -1 : 1);
	gy = (y2 < y1 ? -1 : 1);
	vx = (x2 < x1 ? x1-x2 : x2-x1);
	vy = (y2 < y1 ? y1-y2 : y2-y1);

	// Get distance to edge
	dx = 1<<8;
	dy = 1<<8;
	cx = x1;
	cy = y1;

	// Trace
	while(cx != x2 || cy != y2)
	{
		// Get cell
		ce = layer_cell_ptr(ar, cx, cy);

		// Check
		if(ce == NULL) break;
		//if(ce->ob != NULL) break;
		if(ce->f.ctyp == CELL_OOB) break;
		if(ce->f.ctyp == CELL_SOLID) break;
		if(ce->f.ctyp == CELL_BACKWALL) break;

		// Advance
		//if(dx/vx < dy/vy) // What's actually happening.
		//if(time_to_x < time_to_y) // What I'm trying to actually calculate.
		if(cy == y2 || (cx != x2 && dx*vy < dy*vx))
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

	}

	// Return
	*rx = cx;
	*ry = cy;

	return (cx == x2 && cy == y2);

}

