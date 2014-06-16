/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

static void cell_deprep(cell_t *ce)
{
	// Nothing to free yet!

	// But we have to drop the cell association
	ce->ob = NULL;

}

static void cell_prep(cell_t *ce, int tset, int tidx)
{
	int i;

	ce->f.ctyp = ce_defaults[tset][tidx].ctyp;
	ce->f.tset = tset;
	ce->f.tidx = tidx;

	ce->f.p1 = ce_defaults[tset][tidx].p1;

	ce->ob = NULL;

	for(i = 0; i < FOOD_COUNT; i++)
		ce->splatters[i] = 0;
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
	ar->astar = malloc(sizeof(int8_t) * w * h);

	for(i = 0; i < w*h; i++)
		cell_prep(ar->data + i, 0, 0);
		//cell_prep(ar->data + i, 0, 1);

	return ar;
}

void level_free(level_t *lv)
{
	int i;

	if(lv->layers != NULL)
	{
		// Free layers
		for(i = 0; i < lv->lcount; i++)
			layer_free(lv->layers[i]);

		// Free layer list
		free(lv->layers);

	}

	if(lv->objects != NULL)
	{
		// Free objects
		for(i = 0; i < lv->ocount; i++)
			obj_free(lv->objects[i]);

		// Free object list
		free(lv->objects);

	}

	// Free level
	free(lv);

}

level_t *level_new(int w, int h)
{
	level_t *lv = malloc(sizeof(level_t));

	lv->game = NULL;

	// Layers
	if(w == 0 && h == 0)
	{
		lv->layers = NULL;
		lv->lcount = 0;
	} else {
		assert(w >= 1 && h >= 1);
		lv->layers = malloc(sizeof(layer_t *) * 1);
		lv->layers[0] = layer_new(0, 0, w, h);
		lv->lcount = 1;
	}

	// Objects
	lv->ocount = 0;
	lv->objects = NULL;

	return lv;
}

obj_t *level_obj_add(level_t *lv, int otyp, int flags, int cx, int cy, int layer)
{
	// FIXME: Seriously need to refactor this stuff
	// This is actually terrible
	// Not just this function... I really want to see the objects in a linked list

	// Allocate
	obj_t *ob = obj_new(otyp, flags, cx, cy, layer);
	if(ob == NULL) return NULL;
	ob->level = lv;

	// Put in level
	lv->objects = realloc(lv->objects, sizeof(obj_t *) * (lv->ocount+1));
	lv->objects[lv->ocount] = ob;
	lv->ocount++;

	// Return
	return ob;

}

int level_obj_free(level_t *lv, obj_t *ob)
{
	// FIXME: The object list thing needs work...
	int i;
	int ctr;
	cell_t *ce;

	// Initialise instance counter
	// (Just in case some dip sticks it in the objects list twice)
	ctr = 0;

	// Remove from cell
	ce = layer_cell_ptr(lv->layers[ob->f.layer], ob->f.cx, ob->f.cy);
	if(ce != NULL && ce->ob == ob)
		ce->ob = NULL;

	// Remove from objects list
	for(i = 0; i < lv->ocount; i++)
	if(lv->objects[i] == ob)
	{
		memmove(lv->objects + i, lv->objects + (i+1), sizeof(obj_t *) * (lv->ocount-1-i));
		lv->objects = realloc(lv->objects, sizeof(obj_t) * (lv->ocount-1));
		lv->ocount--;
		i--;
		ctr++;
	}

	// Free
	obj_free(ob);

	// Return count
	return ctr;

}

obj_t *level_obj_waiting(level_t *lv)
{
	int i;

	for(i = 0; i < lv->ocount; i++)
		if(lv->objects[i]->please_wait)
			return lv->objects[i];

	return NULL;
}

static layer_t *level_load_layer(FILE *fp)
{
	int i;
	int w,h,x,y;
	layer_t *ay;

	// Read w,h,x,y
	w = (int16_t)io_get2le(fp);
	h = (int16_t)io_get2le(fp);
	x = (int16_t)io_get2le(fp);
	y = (int16_t)io_get2le(fp);
	if(!(w >= 1 && h >= 1))
	{
		printf("level_load_layer: invalid dimensions\n");
		return NULL;
	}

	// Allocate layer
	ay = layer_new(x, y, w, h);

	// Read cells
	for(i = 0; i < ay->w * ay->h; i++)
	{
		cell_t *ce = ay->data + i;

		ce->f.ctyp = fgetc(fp);
		ce->f.tset = fgetc(fp);
		ce->f.tidx = fgetc(fp);
		ce->f.p1 = fgetc(fp);

		// TODO: replace this stupid hack
		ce->f.ctyp = ce_defaults[ce->f.tset][ce->f.tidx].ctyp;

	}

	// Return
	return ay;
}

level_t *level_load(const char *fname)
{
	FILE *fp;
	char buf[8];
	level_t *lv;
	int i;

	// Open file for reading
	fp = fopen(fname, "rb");
	if(fp == NULL)
	{
		perror("level_load(fopen)");
		return NULL;

	}

	// Check magic number
	buf[7] = '\x00';
	fread(buf, 8, 1, fp);
	if(memcmp(buf, "PsnThMap", 8))
	{
		printf("level_load: not a Parsnip Theory map\n");
		return 0;
	}

	// Check version
	uint8_t fversion = fgetc(fp);
	if(fversion != MAP_FVERSION)
	{
		printf("level_load: map version not supported! got %i, expected %i\n"
			, fversion, MAP_FVERSION);
		goto fail;
	}

	// Create level object
	lv = level_new(0, 0);

	// Get layer count
	lv->lcount = fgetc(fp);
	assert(lv->lcount >= 0);

	// Prep layer array
	lv->layers = malloc(sizeof(layer_t *) * lv->lcount);
	for(i = 0; i < lv->lcount; i++)
		lv->layers[i] = NULL;

	// Get layers
	for(i = 0; i < lv->lcount; i++)
	{
		lv->layers[i] = level_load_layer(fp);
		if(lv->layers[i] == NULL)
			goto fail_delevel;
	}
	
	// Read object count
	lv->ocount = io_get2le(fp);

	// Prepare object pointer array
	lv->objects = malloc(sizeof(obj_t *) * lv->ocount);
	for(i = 0; i < lv->lcount; i++)
		lv->objects[i] = NULL;

	// Load objects
	for(i = 0; i < lv->ocount; i++)
	{
		lv->objects[i] = obj_load(fp);
		if(lv->objects[i] == NULL)
			goto fail_delevel;

		lv->objects[i]->level = lv;
	}

	// Return with success
	fclose(fp);
	return lv;

	// Failures
fail_delevel:
	level_free(lv);
fail:
	fclose(fp);
	return NULL;

}

static int level_save_layer(FILE *fp, layer_t *ay)
{
	int i;

	// Write w,h,x,y
	io_put2le(ay->w, fp);
	io_put2le(ay->h, fp);
	io_put2le(ay->x, fp);
	io_put2le(ay->y, fp);

	printf("layer %i %i\n", ay->w, ay->h);

	// Write cells
	for(i = 0; i < ay->w * ay->h; i++)
	{
		cell_t *ce = ay->data + i;

		fputc(ce->f.ctyp, fp);
		fputc(ce->f.tset, fp);
		fputc(ce->f.tidx, fp);
		fputc(ce->f.p1, fp);
	}

	// Return
	return 1;
}

int level_save(level_t *lv, const char *fname)
{
	FILE *fp;
	int i;

	// Open file for writing
	fp = fopen(fname, "wb");
	if(fp == NULL)
	{
		perror("level_save(fopen)");
		return 0;

	}

	// Write magic number
	fwrite("PsnThMap", 8, 1, fp);

	// Write version
	fputc(MAP_FVERSION, fp);

	// Write layer count
	fputc(lv->lcount, fp);

	// Write layers
	for(i = 0; i < lv->lcount; i++)
		if(!level_save_layer(fp, lv->layers[i]))
			goto fail;
	
	// Write object count
	io_put2le(lv->ocount, fp);

	// Write objects
	for(i = 0; i < lv->ocount; i++)
		if(!obj_save(fp, lv->objects[i]))
			goto fail;

	// Close and return with success
	fclose(fp);
	return 1;

fail:
	printf("level_save failed\n");
	// Close and return with failure
	fclose(fp);
	return 0;

}


