/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

void ai_free(ai_t *ai)
{
	// Free AI
	free(ai);
}

void ai_normal_do_move(ai_t *ai)
{
	obj_t *ob, *tob;
	int i, j;
	int asx, asy;
	obj_t *best_tob;
	int best_tob_dist;
	int dirbuf[512];

	// Get stuff
	game_t *game = ai->game;

	// Block input if waiting for object
	if(level_obj_waiting(game->lv) != NULL)
		return;

	// Scan for objects that belong to us

	for(i = 0; i < game->lv->ocount; i++)
	{
		// Get object
		ob = game->lv->objects[i];

		// Check constraints
		if(ob == NULL) continue;
		struct fd_player *fde = (struct fd_player *)ob->f.fd;
		assert(fde != NULL);
		if(fde->team != ai->tid) continue;
		if(ob->steps_left <= 0) continue;

		// Find a player to harrass
		best_tob_dist = 0x10000000;
		best_tob = NULL;
		for(j = 0; j < game->lv->ocount; j++)
		{
			// Get object
			tob = game->lv->objects[j];

			// Check constraints
			if(tob == NULL) continue;
			struct fd_player *fde = (struct fd_player *)tob->f.fd;
			assert(fde != NULL);
			if(fde->team == ai->tid) continue;

			// Find nearby tile we can touch
			if(!find_free_neighbour_layer(game->lv->layers[ob->f.layer],
				tob->f.cx, tob->f.cy, &asx, &asy)) continue;
			if(1 > astar_layer(game->lv->layers[ob->f.layer], dirbuf, 1024,
				ob->f.cx, ob->f.cy, asx, asy)) continue;

			// Get distance to object
			int dx = tob->f.cx - ob->f.cx;
			int dy = tob->f.cy - ob->f.cy;
			int dist = dx*dx + dy*dy;

			if(best_tob == NULL || dist < best_tob_dist)
			{
				best_tob_dist = dist;
				best_tob = tob;

			}
		}

		// If we don't have a "best object" to harass, continue
		if(best_tob == NULL) continue;
		tob = best_tob;

		//printf("found enemy %i\n", best_tob_dist);

		// Check if we have a line of sight toward this object
		// (as well as enough turns to throw stuff)
		int rx, ry;
		if(ob->steps_left >= STEPS_ATTACK+1)
		if(line_layer(game->lv->layers[ob->f.layer], &rx, &ry,
			ob->f.cx, ob->f.cy, asx, asy))
		{
			// Throw!

			printf("enemy LoS\n");

			game_push_hover(game, game->ab_local, 160, 100,
				tob->f.cx*32+16 - 160, tob->f.cy*24+12 - 100);

			abuf_write_u8(ACT_SELECT, game->ab_local);
			abuf_write_s16(ob->f.cx, game->ab_local);
			abuf_write_s16(ob->f.cy, game->ab_local);

			printf("attack %i %i -> %i %i\n", ob->f.cx, ob->f.cy, tob->f.cx, tob->f.cy);
			abuf_write_u8(ACT_ATTACK, game->ab_local);
			abuf_write_s16(ob->f.cx, game->ab_local);
			abuf_write_s16(ob->f.cy, game->ab_local);
			abuf_write_s16(tob->f.cx, game->ab_local);
			abuf_write_s16(tob->f.cy, game->ab_local);
			abuf_write_s16(STEPS_ATTACK, game->ab_local);
			abuf_write_s16(ob->steps_left - STEPS_ATTACK, game->ab_local);

			return;
		}
		
		// Check if we can move to this object
		if(!find_free_neighbour_layer(game->lv->layers[ob->f.layer],
			tob->f.cx, tob->f.cy, &asx, &asy)) continue;
		int asl = astar_layer(game->lv->layers[ob->f.layer], dirbuf, 512,
			ob->f.cx, ob->f.cy, asx, asy);

		printf("A* trace: %i\n", asl);

		if(asl >= 1)
		{
			game_push_hover(game, game->ab_local, 160, 100,
				ob->f.cx*32+16 - 160, ob->f.cy*24+12 - 100);

			if(asl > ob->steps_left)
				asl = ob->steps_left;

			abuf_write_u8(ACT_SELECT, game->ab_local);
			abuf_write_s16(ob->f.cx, game->ab_local);
			abuf_write_s16(ob->f.cy, game->ab_local);

			int tx = ob->f.cx;
			int ty = ob->f.cy;

			for(j = 0; j < asl; j++)
			switch(dirbuf[j])
			{
				case DIR_SOUTH: ty++; break;
				case DIR_EAST:  tx++; break;
				case DIR_NORTH: ty--; break;
				case DIR_WEST:  tx--; break;
			}

			printf("move %i %i -> %i %i\n", ob->f.cx, ob->f.cy, tx, ty);
			abuf_write_u8(ACT_MOVE, game->ab_local);
			abuf_write_s16(ob->f.cx, game->ab_local);
			abuf_write_s16(ob->f.cy, game->ab_local);
			abuf_write_s16(tx, game->ab_local);
			abuf_write_s16(ty, game->ab_local);
			abuf_write_s16(asl, game->ab_local);
			abuf_write_s16(ob->steps_left - asl, game->ab_local);

			return;

		}

	}

	// All done! Force a change.
	game_push_newturn(game, game->ab_local, -1, ai->tid);
}

ai_t *ai_new(game_t *game, int tid)
{
	// Allocate AI
	ai_t *ai = malloc(sizeof(ai_t));

	// Drop in arguments
	ai->game = game;
	ai->tid = tid;

	// Fill in the defaults

	// Fill in function pointers
	ai->f_do_move = ai_normal_do_move;

	// Return
	return ai;
}

