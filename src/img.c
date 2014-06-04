/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

static uint16_t io_get2le(FILE *fp)
{
	int v0 = fgetc(fp);
	int v1 = fgetc(fp);

	return (v1<<8)|v0;
}

void img_free(img_t *img)
{
	if(img == NULL) return;

	free(img->data);
	free(img);
}

img_t *img_new(int w, int h)
{
	// Allocate the image.
	img_t *img = malloc(sizeof(img_t));
	img->data = malloc(w*h*1);
	img->w = w;
	img->h = h;

	// We really don't care about the palette.

	// Return image.
	return img;
}

img_t *img_load_tga(const char *fname)
{
	FILE *fp;
	int x, y, i;
	uint8_t t;

	// Open file
	fp = fopen(fname, "rb");
	if(fp == NULL)
	{
		perror("img_load_tga");
		return NULL;
	}

	// We'll just shove assert()s everywhere.
	// If you drop in a TGA file this can't handle,
	// it's your fault for being bad at modding.

	// Read TGA header
	uint8_t idlen = fgetc(fp);
	uint8_t cmaptyp = fgetc(fp);
	uint8_t datatyp = fgetc(fp);
	uint16_t cmapbeg = io_get2le(fp);
	uint16_t cmaplen = io_get2le(fp);
	uint8_t cmapbpp = fgetc(fp);
	int16_t ix = io_get2le(fp);
	int16_t iy = io_get2le(fp);
	uint16_t iw = io_get2le(fp);
	uint16_t ih = io_get2le(fp);
	uint8_t ibpp = fgetc(fp);
	uint8_t idesc = fgetc(fp);

	// Shut the compiler up
	(void)ix;
	(void)iy;
	(void)cmapbeg;

	// Check if this image is supported
	assert(datatyp == 1);
	assert(cmaptyp == 1);
	assert(ibpp == 8);
	assert(cmapbpp == 24);
	assert((idesc & ~0x20) == 0x00);
	assert(iw >= 1 && ih >= 1);
	// XXX: This could do with a few more checks.
	// Palettes with a nonzero origin, images with a weird offset, etc.

	// Skip comment
	while(idlen-- > 0) fgetc(fp);

	// Create image
	img_t *img = img_new(iw, ih);

	// Load palette
	for(i = 0; i < cmaplen; i++)
	{
		img->pal[i][0] = fgetc(fp);
		img->pal[i][1] = fgetc(fp);
		img->pal[i][2] = fgetc(fp);
		img->pal[i][3] = 0;
	}

	// Clear remainder of palette
	for(; i < 256; i++)
	{
		img->pal[i][0] = 0;
		img->pal[i][1] = 0;
		img->pal[i][2] = 0;
		img->pal[i][3] = 0;
	}

	// Load image
	fread(img->data, img->w, img->h, fp);

	// Flip if origin on bottom
	if((idesc & 0x20) == 0)
	{
		for(y = 0; y < (ih>>1); y++)
		for(x = 0; x < iw; x++)
		{
			t = *IMG8(img, x, y);
			*IMG8(img, x, y) = *IMG8(img, x, ih-1-y);
			*IMG8(img, x, ih-1-y) = t;
		}
	}

	// Close + return
	fclose(fp);
	return img;
}

