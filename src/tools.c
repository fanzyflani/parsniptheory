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

void pq_push(struct pq_level *pq, int *pqlen, int pqmax, int ptotal, int pacc, int x, int y)
{
	int pidx, idx;
	struct pq_level ptemp;

	// Allocate a slot
	idx = (*pqlen)++;
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
		if(!(pq[idx].ptotal > pq[pidx].ptotal))
			break;

		// Swap
		memcpy(&ptemp, pq + idx, sizeof(struct pq_level));
		memcpy(pq + idx, pq + pidx, sizeof(struct pq_level));
		memcpy(pq + pidx, &ptemp, sizeof(struct pq_level));

		// Change indices
		idx = pidx;

	}
}

void pq_deque(struct pq_level *pq, int *pqlen)
{
	int idx, cidx;
	struct pq_level ptemp;

	// Decrement length
	(*pqlen)--;
	assert(*pqlen >= 0);

	// Copy end to top
	memcpy(pq, pq + *pqlen, sizeof(struct pq_level));

	// Float down
	idx = 0;

	for(;;)
	{
		// Get first child
		cidx = ((idx+1)<<1)-1;
		if(cidx >= *pqlen) break;

		// Determine largest child
		if(cidx+1 < *pqlen && pq[cidx+1].ptotal < pq[cidx].ptotal)
			cidx = cidx+1;

		// Swap
		memcpy(&ptemp, pq + idx, sizeof(struct pq_level));
		memcpy(pq + idx, pq + cidx, sizeof(struct pq_level));
		memcpy(pq + cidx, &ptemp, sizeof(struct pq_level));

		// Change indices
		idx = cidx;

	}

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
	return pacc + dx + dy;

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
	memset(ar->astar, -1, ar->w * ar->h);

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
		memcpy(&p, pq + 0, sizeof(struct pq_level));
		pq_deque(pq, &pqlen);

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
				dir = ar->astar[x + y*ar->w];
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
			if(ar->astar[x + y*ar->w] >= 0) continue;
			ce = layer_cell_ptr(ar, p.x, p.y);
			if(ce == NULL) continue;

			// Mark spot on table
			ar->astar[x + y*ar->w] = i;

			// Check if we can actually move here
			if(ce->f.ctyp != CELL_FLOOR) continue;

			// Push new entry
			ptotal = pq_ptotal(p.pacc + 1, x, y, x2, y2);
			pq_push(pq, &pqlen, PQMAX, ptotal, p.pacc + 1, x, y);

		}

	}

}

