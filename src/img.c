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

// TODO: Relocate
uint16_t io_get2le(FILE *fp)
{
	int v0 = fgetc(fp);
	int v1 = fgetc(fp);

	return (v1<<8)|v0;
}

void io_put2le(int v, FILE *fp)
{
	int v0 = v&255;
	int v1 = (v>>8)&255;

	fputc(v0, fp);
	fputc(v1, fp);
}

uint32_t io_get4le(FILE *fp)
{
	int v0 = io_get2le(fp);
	int v1 = io_get2le(fp);

	return (v1<<16)|v0;
}

void io_put4le(int v, FILE *fp)
{
	int v0 = v&255;
	int v1 = (v>>16)&255;

	io_put2le(v0, fp);
	io_put2le(v1, fp);
}

uint16_t io_get2be(FILE *fp)
{
	int v1 = fgetc(fp);
	int v0 = fgetc(fp);

	return (v1<<8)|v0;
}

void io_put2be(int v, FILE *fp)
{
	int v1 = v&255;
	int v0 = (v>>8)&255;

	fputc(v1, fp);
	fputc(v0, fp);
}

uint32_t io_get4be(FILE *fp)
{
	int v1 = io_get2be(fp);
	int v0 = io_get2be(fp);

	return (v1<<16)|v0;
}

void io_put4be(int v, FILE *fp)
{
	int v0 = v&255;
	int v1 = (v>>16)&255;

	io_put2be(v1, fp);
	io_put2be(v0, fp);
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
	img->cmidx = -1;

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

	// Close
	fclose(fp);
	
	// Find colourmap
	for(i = 0; cmaps[i].fname != NULL; i++)
	{
		if(!strcasecmp(cmaps[i].fname, fname))
		{
			img->cmidx = i;
			break;
		}
	}

	// Did we find one?
	if(cmaps[i].fname == NULL)
	{
		// Use a dummy
		img->cmidx = -1;
	}

	// Return
	return img;
}

img_t *img_load_png(const char *fname)
{
	FILE *fp;
	int x, y, i;
	int res;
	int iw, ih, bits, ctyp, cmpm, filtm, intm;
	int len;
	uint8_t header[8];
	uint8_t tag[4];

	int valid_ihdr = 0;
	uint8_t *plte = NULL;
	uint8_t *idat_cmp = NULL;
	uint8_t *idat = NULL;
	int plte_len = 0;
	int plte_entries = 0;
	int idat_cmp_len = 0;
	uLongf idat_len = 0;

	// Open file
	fp = fopen(fname, "rb");
	if(fp == NULL)
	{
		perror("img_load_png");
		return NULL;
	}

	// We'll just shove assert()s everywhere.
	// If you drop in a PNG file this can't handle,
	// it's your fault for being bad at modding.

	// Read PNG header
	header[7] = '\x00';
	fread(header, 8, 1, fp);
	if(memcmp(header, "\x89PNG\x0D\x0A\x1A\x0A", 8))
	{
		printf("img_load_png: not a PNG file\n");
		fclose(fp);
		return NULL;
	}

	// Read all the stuff
	for(;;)
	{
		// Shove in a dummy, and read
		len = io_get4be(fp);
		memcpy(tag, "EOF!", 4);
		fread(tag, 4, 1, fp);

		if(!memcmp(tag, "IEND", 4))
		{
			// All good!
			break;

		} else if(!memcmp(tag, "IHDR", 4)) {
			// Load header
			assert(!valid_ihdr);
			assert(len == 13);

			// Load everything
			iw = io_get4be(fp);
			ih = io_get4be(fp);
			bits = fgetc(fp);
			ctyp = fgetc(fp);
			cmpm = fgetc(fp);
			filtm = fgetc(fp);
			intm = fgetc(fp);

			// Check things
			assert(iw != 0);
			assert(ih != 0);
			assert(iw < 16384); // mostly for sanity
			assert(ih < 16384); // mostly for sanity
			assert(bits == 1 || bits == 2 || bits == 4 || bits == 8);
			assert(bits == 8); // OK, we don't support the other things yet
			assert(ctyp == 3);
			assert(cmpm == 0);
			assert(filtm == 0);
			assert(intm == 0);

			// We have a header now!
			valid_ihdr = 1;

		} else if(!memcmp(tag, "PLTE", 4)) {
			// Load palette
			assert(len <= (1<<bits)*3);
			assert(len % 3 == 0);
			plte = malloc(len);
			plte_len = len;
			plte_entries = plte_len / 3;
			fread(plte, len, 1, fp);

		} else if(!memcmp(tag, "IDAT", 4)) {
			// Add this to the compressed IDAT stream
			idat_cmp = realloc(idat_cmp, idat_cmp_len + len);
			fread(idat_cmp + idat_cmp_len, 1, len, fp);
			idat_cmp_len += len;

		} else if((tag[0] & 0x20) == 0) {
			printf("img_load_png: critical chunk %c%c%c%c not supported\n",
				tag[0], tag[1], tag[2], tag[3]);
			fflush(fp);
			abort();

		} else {
			// Skip this chunk
			fseek(fp, len, SEEK_CUR);
		}

		// Skip CRC
		io_get4be(fp);

	}

	// Do some checks
	assert(valid_ihdr);
	assert(idat_cmp != NULL);
	assert(plte != NULL);

	// Decompress image
	idat_len = ih*(1+iw*1);
	idat = malloc(idat_len);
	res = uncompress((Bytef *)idat, &idat_len, (Bytef *)idat_cmp, idat_cmp_len);
	assert(res == Z_OK);

	// Create image
	img_t *img = img_new(iw, ih);

	// Load palette
	for(i = 0; i < plte_entries; i++)
	{
		img->pal[i][0] = plte[i*3+2];
		img->pal[i][1] = plte[i*3+1];
		img->pal[i][2] = plte[i*3+0];
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

	// Read data
	//printf("idat len %i %s\n", (int)idat_len, fname);
	for(y = 0; y < ih; y++)
	{
		int fmode = idat[y*(1+iw*1)];
		assert(fmode == 0); // TODO: Support other modes

		for(x = 0; x < iw; x++)
		{
			*IMG8(img, x, y) = idat[y*(1+iw*1)+1 + x];
		}
	}

	// Close
	fclose(fp);
	
	// Find colourmap
	for(i = 0; cmaps[i].fname != NULL; i++)
	{
		if(!strcasecmp(cmaps[i].fname, fname))
		{
			img->cmidx = i;
			break;
		}
	}

	// Did we find one?
	if(cmaps[i].fname == NULL)
	{
		// Use a dummy
		img->cmidx = -1;
	}

	// Free extra crap
	if(plte != NULL) free(plte);
	if(idat_cmp != NULL) free(idat_cmp);
	if(idat != NULL) free(idat);

	// Return
	return img;
}

void load_palette(const char *fname)
{
	int i, j;
	cmap_t *cm;
	char *fndata;
	int fnsize;
	int cmcount;

	// FIXME: This isn't very error-safe.
	// Then again, we should only have to do this once.
	// So, assertiontime!

	// Clear all the colourmap info
	if(cmaps != NULL)
	{
		for(cm = cmaps; cm->fname != NULL; cm++)
			free(cm->fname);

		free(cmaps);

		cmaps = NULL;
	}

	// Load our file
	FILE *fp = fopen("dat/pal1.pal", "rb");
	if(fp == NULL) perror("load_palette");
	assert(fp != NULL);

	// Load the palette data
	for(i = 0; i < 256; i++)
	for(j = 2; j >= 0; j--)
		pal_src[i][j] = pal_main[i][j] = fgetc(fp);
	
	// Get the colourmap count
	cmcount = fgetc(fp);
	assert(cmcount >= 0);
	
	// Create colourmaps
	cmaps = malloc((cmcount+1)*sizeof(cmap_t));

	// Load filenames
	fnsize = io_get2le(fp);
	fndata = malloc(fnsize);
	fread(fndata, fnsize, 1, fp);
	assert(fndata[fnsize-1] == '\x00');

	// Copy filenames to colourmaps
	cmaps[cmcount].fname = NULL;
	for(i = 0; i < cmcount; i++)
		cmaps[i].fname = strdup(fndata + (int)io_get2le(fp));

	// Free filename data
	free(fndata);

	// Load colourmaps
	for(i = 0; i < cmcount; i++)
		fread(cmaps[i].data, 256, 1, fp);

	// Close and return
	fclose(fp);

}

