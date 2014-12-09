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

team_t *team_new(int idx)
{
	int i;

	// Allocate
	team_t *tm = malloc(sizeof(team_t));

	// Set things
	tm->idx = idx;

	// Copy colourmap
	memcpy(tm->cm_player, cm_player, 256);

	// Modify colourmap
	int skintone = 32+((rand()>>12)&7)*4;
	//int skintone = 32+((idx)&7)*4;
	for(i = 0; i < 4; i++)
	{
		tm->cm_player[16+i] = skintone+i; // This will be varied per-player.
		tm->cm_player[24+i] = 64+i + 4*((idx + 1 + (idx>>3))&7);
		tm->cm_player[32+i] = 64+i + 4*((idx)&7);
		tm->cm_player[40+i] = 64+i + 4*((idx + 3 + 2*(idx>>3) + (idx>>6))&7);
	}

	// Return team
	return tm;
}

