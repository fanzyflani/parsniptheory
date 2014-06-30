/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
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

