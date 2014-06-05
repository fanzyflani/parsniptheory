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

	// Free level
	free(lv);

}

level_t *level_new(int w, int h)
{
	level_t *lv = malloc(sizeof(level_t));

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

	return lv;
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

	// Open file for writing
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
	// TODO: Handle objects
	int objcount = io_get2le(fp);
	assert(objcount == 0); // TODO!

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
	// TODO: Handle objects
	io_put2le(0, fp);

	// Close and return with success
	fclose(fp);
	return 1;

fail:
	printf("level_save failed\n");
	// Close and return with failure
	fclose(fp);
	return 0;

}


