/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

void cell_prep(cell_t *ce, int tset, int tidx)
{
	ce->f.ctyp = ce_defaults[tset][tidx].ctyp;
	ce->f.tset = tset;
	ce->f.tidx = tidx;

	ce->f.p1 = ce_defaults[tset][tidx].p1;
}

layer_t *layer_new(int x, int y, int w, int h)
{
	int i;

	layer_t *ay = malloc(sizeof(layer_t));

	ay->x = x;
	ay->y = y;
	ay->w = w;
	ay->h = h;

	ay->data = malloc(sizeof(cell_t) * w * h);

	for(i = 0; i < w*h; i++)
		cell_prep(ay->data + i, 0, 0);
		//cell_prep(ay->data + i, 0, 1);

	return ay;
}

level_t *level_new(int w, int h)
{
	level_t *lv = malloc(sizeof(level_t));

	lv->lcount = 0;
	lv->layers = malloc(sizeof(layer_t *) * 1);
	lv->layers[0] = layer_new(0, 0, w, h);

	return lv;
}


