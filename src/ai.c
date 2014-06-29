/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

//define DEBUG_AI

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
	obj_t *best_sob;
	int best_tob_dist;
	int best_sob_dist;
	int dirbuf[512];

	// Wait
	if(--ai->wait > 0) return;

	// Get stuff
	game_t *game = ai->game;

	// Block input if waiting for object
	if(level_obj_waiting(game->lv) != NULL)
	{
		return;
	}

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
		best_sob_dist = 0x10000000;
		best_sob = NULL;
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
			int tob_candidate = 0;
			if(find_free_neighbour_layer(game->lv->layers[ob->f.layer],
				tob->f.cx, tob->f.cy, &asx, &asy))
			if(astar_layer(game->lv->layers[ob->f.layer], dirbuf, 1024,
				ob->f.cx, ob->f.cy, asx, asy) >= 1) tob_candidate = 1;

			// Find if there's a line of sight
			int sob_candidate = 0;
			int rx, ry;
			if(line_layer(game->lv->layers[ob->f.layer], &rx, &ry,
				ob->f.cx, ob->f.cy, tob->f.cx, tob->f.cy))
					sob_candidate = 1;

			// Get distance to object
			int dx = tob->f.cx - ob->f.cx;
			int dy = tob->f.cy - ob->f.cy;
			int dist = dx*dx + dy*dy;
			dist *= dist * tob->health;

			if(tob_candidate)
			if(best_tob == NULL || dist < best_tob_dist)
			{
				best_tob_dist = dist;
				best_tob = tob;
			}

			dist = tob->health;
			if(sob_candidate)
			if(best_sob == NULL || dist < best_sob_dist)
			{
				best_sob_dist = dist;
				best_sob = tob;
			}
		}

		//printf("found enemy %i\n", best_tob_dist);

		// Check if we have a line of sight toward this object
		// (as well as enough turns to throw stuff)
		// Also check if we are going for the guy we have a LoS for
		int rx, ry;
		if(best_sob != NULL && line_layer(game->lv->layers[ob->f.layer], &rx, &ry,
			ob->f.cx, ob->f.cy, best_sob->f.cx, best_sob->f.cy))
				tob = best_sob;
		else
			tob = best_tob;
		// If we don't have a "best object" to harass, continue
		if(tob == NULL) continue;


		if(ob->steps_left >= STEPS_ATTACK+1 && tob->health > 0)
		if(line_layer(game->lv->layers[ob->f.layer], &rx, &ry,
			ob->f.cx, ob->f.cy, tob->f.cx, tob->f.cy))
		{
			// Throw!

#ifdef DEBUG_AI
			printf("enemy LoS\n");
#endif

			game->camx = tob->f.cx*32+16 - 160;
			game->camy = tob->f.cy*24+12 - 100;
			if(game->camx < 0) game->camx = 0;
			if(game->camy < 0) game->camy = 0;
			// TODO: not hardcode this
			if(game->camx > 32*40 - 320) game->camx = 32*40 - 320;
			if(game->camy > 32*40 - 320) game->camy = 32*40 - 320;
			game->mx = tob->f.cx*32+16 - game->camx;
			game->my = tob->f.cy*24+12 - game->camy;
			game_push_hover(game, ai->ab, game->mx, game->my, game->camx, game->camy);

			abuf_write_u8(ACT_SELECT, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);

#ifdef DEBUG_AI
			printf("attack %i %i -> %i %i\n", ob->f.cx, ob->f.cy, tob->f.cx, tob->f.cy);
#endif
			abuf_write_u8(ACT_ATTACK, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);
			abuf_write_s16(tob->f.cx, ai->ab);
			abuf_write_s16(tob->f.cy, ai->ab);
			abuf_write_s16(STEPS_ATTACK, ai->ab);
			abuf_write_s16(ob->steps_left - STEPS_ATTACK, ai->ab);

			ai->wait = 15;

			return;
		}
		
		// Check if we can move to this object
		tob = best_tob;
		if(tob == NULL) continue;
		if(!find_free_neighbour_layer(game->lv->layers[ob->f.layer],
			tob->f.cx, tob->f.cy, &asx, &asy)) continue;
		int asl = astar_layer(game->lv->layers[ob->f.layer], dirbuf, 512,
			ob->f.cx, ob->f.cy, asx, asy);

#ifdef DEBUG_AI
		printf("A* trace: %i\n", asl);
#endif

		if(asl >= 1)
		{
			game->camx = ob->f.cx*32+16 - 160;
			game->camy = ob->f.cy*24+12 - 100;
			if(game->camx < 0) game->camx = 0;
			if(game->camy < 0) game->camy = 0;
			// TODO: not hardcode this
			if(game->camx > 32*40 - 320) game->camx = 32*40 - 320;
			if(game->camy > 32*40 - 320) game->camy = 32*40 - 320;

			asl = 1;
			if(asl > ob->steps_left)
				asl = ob->steps_left;

			abuf_write_u8(ACT_SELECT, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);

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

			game->mx = tx*32+16 - game->camx;
			game->my = ty*24+12 - game->camy;
			game_push_hover(game, ai->ab, game->mx, game->my, game->camx, game->camy);

#ifdef DEBUG_AI
			printf("move %i %i -> %i %i\n", ob->f.cx, ob->f.cy, tx, ty);
#endif
			abuf_write_u8(ACT_MOVE, ai->ab);
			abuf_write_s16(ob->f.cx, ai->ab);
			abuf_write_s16(ob->f.cy, ai->ab);
			abuf_write_s16(tx, ai->ab);
			abuf_write_s16(ty, ai->ab);
			abuf_write_s16(asl, ai->ab);
			abuf_write_s16(ob->steps_left - asl, ai->ab);

			return;

		}

	}

	// All done! Force a change.
	game_push_newturn(game, ai->ab, -1, ai->tid);
}

ai_t *ai_new(game_t *game, abuf_t *ab, int tid)
{
	// Allocate AI
	ai_t *ai = malloc(sizeof(ai_t));

	// Drop in arguments
	ai->game = game;
	ai->tid = tid;
	ai->ab = ab;

	// Fill in the defaults
	ai->wait = 0;

	// Fill in function pointers
	ai->f_do_move = ai_normal_do_move;

	// Return
	return ai;
}

