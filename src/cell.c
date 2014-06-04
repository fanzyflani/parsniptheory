/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

static void cell_deprep(cell_t *ce)
{
	// Nothing to free yet!

}

static void cell_prep(cell_t *ce, int tset, int tidx)
{
	ce->f.ctyp = ce_defaults[tset][tidx].ctyp;
	ce->f.tset = tset;
	ce->f.tidx = tidx;

	ce->f.p1 = ce_defaults[tset][tidx].p1;
}

void cell_reprep(cell_t *ce, int tset, int tidx)
{
	cell_deprep(ce);
	cell_prep(ce, tset, tidx);

}

cell_t *layer_cell_ptr(layer_t *ar, int x, int y)
{
	// Bounds check
	if(x - ar->x < 0) return NULL;
	if(y - ar->y < 0) return NULL;
	if(x - ar->x >= ar->w) return NULL;
	if(y - ar->y >= ar->h) return NULL;

	// Return!
	return ar->data + x + y*ar->w;
}

void layer_free(layer_t *ar)
{
	int i;

	// Deprep cells
	for(i = 0; i < ar->w * ar->h; i++)
		cell_deprep(ar->data + i);

	// Free cell array
	free(ar->data);

	// Free layer
	free(ar);

}

layer_t *layer_new(int x, int y, int w, int h)
{
	int i;

	layer_t *ar = malloc(sizeof(layer_t));

	ar->x = x;
	ar->y = y;
	ar->w = w;
	ar->h = h;

	ar->data = malloc(sizeof(cell_t) * w * h);

	for(i = 0; i < w*h; i++)
		cell_prep(ar->data + i, 0, 0);
		//cell_prep(ar->data + i, 0, 1);

	return ar;
}

level_t *level_new(int w, int h)
{
	level_t *lv = malloc(sizeof(level_t));

	lv->lcount = 0;
	lv->layers = malloc(sizeof(layer_t *) * 1);
	lv->layers[0] = layer_new(0, 0, w, h);

	return lv;
}


