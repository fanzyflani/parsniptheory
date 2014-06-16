/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

game_t *rootgame = NULL;

void game_free(game_t *game)
{
	int i;

	// Free structures
	if(game->lv != NULL) free(game->lv);

	// Free action buffers
	if(game->ab_local == NULL)
		free(game->ab_local);
	for(i = 0; i < TEAM_MAX; i++)
		if(game->ab_teams[i] != NULL)
			free(game->ab_teams[i]);

	// Free
	free(game);
}

game_t *game_new(int net_mode)
{
	// Allocate
	game_t *game = malloc(sizeof(game_t));

	// Initialise
	game->camx = 0;
	game->camy = 0;
	game->mx = 161;
	game->my = 121;
	game->cmx = 161/32;
	game->cmy = 121/24;
	game->player_count = 0;
	game->curplayer = 0;
	game->main_state = GAME_SETUP;
	game->net_mode = net_mode;
	game->curtick = 0;
	game->locked = 1;

	game->time_now = game->time_next = SDL_GetTicks();

	game->selob = NULL;
	game->lv = NULL;

	// OK!
	return game;

}

static void gameloop_start_turn(game_t *game)
{
	int i;
	obj_t *ob;
	struct fd_player *fde;
	int obcount = 0;

	// Grant steps to all players of this team
	for(i = 0; i < game->lv->ocount; i++)
	{
		// Get object
		ob = game->lv->objects[i];
		fde = (struct fd_player *)(ob->f.fd);
		
		// Do the compare
		if(fde->team == game->curplayer)
		{
			obcount++;
			ob->steps_left = STEPS_PER_TURN;
			game->camx = ob->f.cx*32+16 - screen->w/2;
			game->camy = ob->f.cy*24+12 - screen->h/2;

		} else {
			ob->steps_left = -1;

		}
	}

	// Move to the next player if this fails
	if(obcount == 0)
	{
		game->curplayer++;
		if(game->curplayer >= game->player_count)
			game->curplayer = 0;

		gameloop_start_turn(game);
	}
}

static int gameloop_next_turn(game_t *game)
{
	// Deselect object
	game->selob = NULL;

	// Take note of the current player
	int oldcp = game->curplayer;

	// Move to the next player
	game->curplayer++;
	if(game->curplayer >= game->player_count)
		game->curplayer = 0;
	
	// Start their turn
	gameloop_start_turn(game);

	// If it's the same player, they've won
	return game->curplayer != oldcp;
}

void game_handle_version(game_t *game, abuf_t *ab, int typ, int ver)
{
	// TODO
}

void game_handle_quit(game_t *game, abuf_t *ab, int typ)
{
	// TODO
}

void game_handle_text(game_t *game, abuf_t *ab, int typ, int len, char *buf)
{
	// TODO
}

void game_handle_lock(game_t *game, abuf_t *ab, int typ)
{
	// TODO
}

void game_handle_unlock(game_t *game, abuf_t *ab, int typ)
{
	// TODO
}

void game_handle_newturn(game_t *game, abuf_t *ab, int typ, int tid, int steps_added)
{
	int i;

	assert(typ == NET_C2S || typ == NET_S2C);

	if(typ == NET_C2S)// && ab == game->ab_teams[game->curplayer])
	{

	} else if(typ == NET_S2C) {

	} else return;

	// Next player
	if(!gameloop_next_turn(game))
	{
		game->main_state = GAME_OVER;
	}

	// Broadcast
	if(typ == NET_C2S)
	for(i = 0; i < TEAM_MAX; i++)
	if(game->ab_teams[i] != NULL)
	{
		abuf_write_u8(ACT_NEWTURN, game->ab_teams[i]);
		abuf_write_u8(tid, game->ab_teams[i]);
		abuf_write_s16(steps_added, game->ab_teams[i]);
	}

}

void game_handle_move(game_t *game, abuf_t *ab, int typ, int sx, int sy, int dx, int dy, int steps_used, int steps_left)
{
	cell_t *ce, *dce;
	int i;

	assert(typ == NET_C2S || typ == NET_S2C);

	ce = layer_cell_ptr(game->lv->layers[0], sx, sy);
	dce = layer_cell_ptr(game->lv->layers[0], dx, dy);

	if(typ == NET_C2S)// && ab == game->ab_teams[game->curplayer])
	{
		if(!(ce != NULL)) return;
		if(!(ce->ob != NULL)) return;
		if(!(dce != NULL)) return;
		if(!(dce->ob == NULL)) return;

	} else if(typ == NET_S2C) {
		assert(ce != NULL);
		assert(ce->ob != NULL);
		assert(dce != NULL);
		assert(dce->ob == NULL);

	} else return;

	// Move it
	ce->ob->tx = dx;
	ce->ob->ty = dy;

	// Destroy the old list
	if(ce->ob->asdir != NULL)
	{
		free(ce->ob->asdir);
		ce->ob->asdir = NULL;
	}

	// Mark it as "please wait"
	ce->ob->please_wait = 1;

	// Broadcast
	if(typ == NET_C2S)
	for(i = 0; i < TEAM_MAX; i++)
	if(game->ab_teams[i] != NULL)
	{
		abuf_write_u8(ACT_MOVE, game->ab_teams[i]);
		abuf_write_s16(sx, game->ab_teams[i]);
		abuf_write_s16(sy, game->ab_teams[i]);
		abuf_write_s16(dx, game->ab_teams[i]);
		abuf_write_s16(dy, game->ab_teams[i]);
		abuf_write_u16(steps_used, game->ab_teams[i]);
		abuf_write_u16(steps_left, game->ab_teams[i]);
	}

	// TODO: Lock this
}

void game_handle_attack(game_t *game, abuf_t *ab, int typ, int sx, int sy, int dx, int dy, int steps_used, int steps_left)
{
	int i;
	cell_t *ce, *dce;

	assert(typ == NET_C2S || typ == NET_S2C);

	ce = layer_cell_ptr(game->lv->layers[0], sx, sy);
	dce = layer_cell_ptr(game->lv->layers[0], dx, dy);

	if(typ == NET_C2S)// && ab == game->ab_teams[game->curplayer])
	{
		if(!(ce != NULL)) return;
		if(!(ce->ob != NULL)) return;
		if(!(dce != NULL)) return;

	} else if(typ == NET_S2C) {
		assert(ce != NULL);
		assert(ce->ob != NULL);
		assert(dce != NULL);

	} else return;

	// Fire an attack
	ce->ob->tx = dx;
	ce->ob->ty = dy;

	// Destroy the old list
	if(ce->ob->asdir != NULL)
	{
		free(ce->ob->asdir);
		ce->ob->asdir = NULL;
	}

	// Mark it as "please wait"
	ce->ob->please_wait = 1;

	// Broadcast
	if(typ == NET_C2S)
	for(i = 0; i < TEAM_MAX; i++)
	if(game->ab_teams[i] != NULL)
	{
		abuf_write_u8(ACT_ATTACK, game->ab_teams[i]);
		abuf_write_s16(sx, game->ab_teams[i]);
		abuf_write_s16(sy, game->ab_teams[i]);
		abuf_write_s16(dx, game->ab_teams[i]);
		abuf_write_s16(dy, game->ab_teams[i]);
		abuf_write_u16(steps_used, game->ab_teams[i]);
		abuf_write_u16(steps_left, game->ab_teams[i]);
	}

	// TODO: Lock this
}

void game_handle_select(game_t *game, abuf_t *ab, int typ, int cx, int cy)
{
	int i;

	cell_t *ce;

	assert(typ == NET_C2S || typ == NET_S2C);

	ce = layer_cell_ptr(game->lv->layers[0], cx, cy);

	if(typ == NET_C2S)// && ab == game->ab_teams[game->curplayer])
	{
		if(!(ce != NULL)) return;
		if(!(ce->ob != NULL)) return;

	} else if(typ == NET_S2C) {
		assert(ce != NULL);
		assert(ce->ob != NULL);

	} else return;

	// Select
	game->selob = ce->ob;

	// Broadcast
	if(typ == NET_C2S)
	for(i = 0; i < TEAM_MAX; i++)
	if(game->ab_teams[i] != NULL)
	{
		abuf_write_u8(ACT_SELECT, game->ab_teams[i]);
		abuf_write_s16(cx, game->ab_teams[i]);
		abuf_write_s16(cy, game->ab_teams[i]);
	}

}

void game_handle_deselect(game_t *game, abuf_t *ab, int typ)
{
	int i;

	assert(typ == NET_C2S || typ == NET_S2C);

	if(typ == NET_C2S)// && ab == game->ab_teams[game->curplayer])
	{
		//

	} else if(typ == NET_S2C) {
		//

	} else return;

	// Deselect
	game->selob = NULL;

	// Broadcast
	if(typ == NET_C2S)
	for(i = 0; i < TEAM_MAX; i++)
	if(game->ab_teams[i] != NULL)
	{
		abuf_write_u8(ACT_DESELECT, game->ab_teams[i]);
	}
}

void game_handle_hover(game_t *game, abuf_t *ab, int typ, int mx, int my, int camx, int camy)
{
	// TODO
}

void game_push_end_turn(game_t *game, abuf_t *ab)
{
	// TODO
}

void game_push_hover(game_t *game, abuf_t *ab, int mx, int my, int camx, int camy)
{
	// TODO!
}

void game_push_newturn(game_t *game, abuf_t *ab, int tid, int steps_added)
{
	// Next turn!
	abuf_write_u8(ACT_NEWTURN, ab);
	abuf_write_u8(tid, ab);
	abuf_write_u16(steps_added, ab);

}

void game_push_click(game_t *game, abuf_t *ab, int rmx, int rmy, int camx, int camy, int button)
{
	cell_t *ce;
	cell_t *dce;
	int mx, my;

	// Get coordinates
	mx = (rmx + camx)/32;
	my = (rmy + camy)/24;
	ce = layer_cell_ptr(game->lv->layers[0], mx, my);

	// Object select
	if(button == 0)
	{
		if(ce != NULL && ce->ob != NULL && ce->ob->f.otyp == OBJ_PLAYER
			&& ((struct fd_player *)(ce->ob->f.fd))->team == game->curplayer)
		{
			// Select object
			abuf_write_u8(ACT_SELECT, ab);
			abuf_write_s16(mx, ab);
			abuf_write_s16(my, ab);

		} else {
			// Deselect objects
			// TODO: Send
			abuf_write_u8(ACT_DESELECT, ab);
			//game_handle_deselect(game, ab, NET_S2C);

		}
	}

	// Object move / attack
	else if(button == 2)
	{
		// Check if we have an object selected

		if(game->selob != NULL)
		{
			// Check destination
			dce = layer_cell_ptr(game->lv->layers[0], mx, my);

			if(dce != NULL && dce->ob == NULL)
			{
				// Move it
				abuf_write_u8(ACT_MOVE, ab);
				abuf_write_s16(game->selob->f.cx, ab);
				abuf_write_s16(game->selob->f.cy, ab);
				abuf_write_s16(mx, ab);
				abuf_write_s16(my, ab);
				abuf_write_u16(0, ab);
				abuf_write_u16(game->selob->steps_left, ab);
				
				//game_handle_move(game, ab, NET_S2C, game->selob->f.cx, game->selob->f.cy, mx, my,
				//	0, game->selob->steps_left);
			} else if(dce != NULL) {
				// Attack it
				abuf_write_u8(ACT_ATTACK, ab);
				abuf_write_s16(game->selob->f.cx, ab);
				abuf_write_s16(game->selob->f.cy, ab);
				abuf_write_s16(mx, ab);
				abuf_write_s16(my, ab);
				abuf_write_u16(0, ab);
				abuf_write_u16(game->selob->steps_left, ab);

				//game_handle_attack(game, ab, NET_S2C, game->selob->f.cx, game->selob->f.cy, mx, my,
				//	0, game->selob->steps_left);

			}

		}

	}
}

int game_parse_actions(game_t *game, abuf_t *ab, int typ)
{
	// Make sure we have a byte
	char buf[257];
	int ver, len, tid;
	int steps_added;
	int steps_used;
	int steps_left;
	int sx, sy, dx, dy;
	int cx, cy, mx, my, camx, camy;

	int bsiz = abuf_get_rsize(ab);
	if(bsiz < 1) return 0;
	//printf("msg %02X\n", ab->rdata[0]);
	bsiz--;

	switch(ab->rdata[0])
	{
		case ACT_NOP:
			if(bsiz < 0) return 0;
			abuf_read_u8(ab);
			return 1;

		case ACT_VERSION:
			if(bsiz < 1) return 0;
			abuf_read_u8(ab);
			ver = abuf_read_u8(ab);
			game_handle_version(game, ab, typ, ver);
			return 1;

		case ACT_QUIT:
			abuf_read_u8(ab);
			len = (bsiz < 1 ? 0 : abuf_read_u8(ab));
			abuf_read_block(buf, (bsiz-1 < len ? bsiz-1 : len), ab);
			buf[bsiz] = '\x00';
			printf("QUIT received: \"%s\"\n", buf);
			game_handle_quit(game, ab, typ);
			return 1;

		case ACT_TEXT:
			if(bsiz < 1) return 0;
			if(bsiz < 1+(int)(ab->rdata[1])) return 0;
			abuf_read_u8(ab);
			len = abuf_read_u8(ab);
			abuf_read_block(buf, len, ab);
			buf[bsiz] = '\x00';
			printf("text received: \"%s\"\n", buf);
			game_handle_text(game, ab, typ, len, buf);
			return 1;

		// ACT_MAPBEG
		// ACT_MAPDATA
		// ACT_MAPEND

		case ACT_LOCK:
			if(bsiz < 0) return 0;
			abuf_read_u8(ab);
			game_handle_lock(game, ab, typ);
			return 1;

		case ACT_UNLOCK:
			if(bsiz < 0) return 0;
			abuf_read_u8(ab);
			game_handle_unlock(game, ab, typ);
			return 1;

		case ACT_NEWTURN:
			if(bsiz < 3) return 0;
			abuf_read_u8(ab);
			tid = abuf_read_u8(ab);
			steps_added = abuf_read_u16(ab);
			game_handle_newturn(game, ab, typ, tid, steps_added);
			return 1;

		case ACT_MOVE:
			if(bsiz < 12) return 0;
			abuf_read_u8(ab);
			sx = abuf_read_s16(ab);
			sy = abuf_read_s16(ab);
			dx = abuf_read_s16(ab);
			dy = abuf_read_s16(ab);
			steps_used = abuf_read_u16(ab);
			steps_left = abuf_read_u16(ab);
			game_handle_move(game, ab, typ, sx, sy, dx, dy, steps_used, steps_left);
			return 1;

		case ACT_ATTACK:
			if(bsiz < 12) return 0;
			abuf_read_u8(ab);
			sx = abuf_read_s16(ab);
			sy = abuf_read_s16(ab);
			dx = abuf_read_s16(ab);
			dy = abuf_read_s16(ab);
			steps_used = abuf_read_u16(ab);
			steps_left = abuf_read_u16(ab);
			game_handle_attack(game, ab, typ, sx, sy, dx, dy, steps_used, steps_left);
			return 1;

		case ACT_SELECT:
			if(bsiz < 4) return 0;
			abuf_read_u8(ab);
			cx = abuf_read_s16(ab);
			cy = abuf_read_s16(ab);
			game_handle_select(game, ab, typ, cx, cy);
			return 1;

		case ACT_DESELECT:
			if(bsiz < 0) return 0;
			abuf_read_u8(ab);
			game_handle_deselect(game, ab, typ);
			return 1;

		case ACT_HOVER:
			if(bsiz < 8) return 0;
			abuf_read_u8(ab);
			mx = abuf_read_s16(ab);
			my = abuf_read_s16(ab);
			camx = abuf_read_s16(ab);
			camy = abuf_read_s16(ab);
			game_handle_hover(game, ab, typ, mx, my, camx, camy);
			return 1;

		default:
			printf("FIXME: handle invalid packet %i\n", ab->rdata[0]);
			fflush(stdout);
			abort();
			break;

	}

}

void gameloop_draw(game_t *game)
{
	int x, y, i;
	obj_t *ob;
	cell_t *ce;

	// Clear the screen
	screen_clear(0);

	// Draw level
	draw_level(screen, game->lv, game->camx, game->camy, 0);

	// Draw overlays
	for(i = 0; i < game->lv->ocount; i++)
	{
		ob = game->lv->objects[i];

		if(ob == NULL) continue;

		// Cell occupancy
		draw_border_d(screen,
			ob->f.cx*32 - game->camx,
			ob->f.cy*24 - game->camy,
			32,
			24,
			1);

		// Bounding box
		if(ob == game->selob)
		draw_border_d(screen,
			ob->bx + ob->f.ox + ob->f.cx*32 - game->camx,
			ob->by + ob->f.oy + ob->f.cy*24 - game->camy,
			ob->bw,
			ob->bh,
			2);
	}

	// TEST: Mark walkable paths
	/*
	for(y = 0; y < game->lv->layers[0]->w; y++)
	for(x = 0; x < game->lv->layers[0]->h; x++)
	{
		cell_t *ce = layer_cell_ptr(game->lv->layers[0], x, y);

		if(ce != NULL && ce->f.ctyp == CELL_FLOOR)
		{
			draw_border_d(screen,
				32*x + 8 - game->camx,
				24*y + 6 - game->camy,
				16,
				12,
				ce->ob == NULL ? 1 : 2);
		}

	}
	*/

	// Draw A* route for selected object
	if(game->selob != NULL)
	{
		// Get coordinates
		int asendx = (game->mx + game->camx)/32;
		int asendy = (game->my + game->camy)/24;

		// Line trace
		int canattack = line_layer(game->lv->layers[0], &x, &y,
			game->selob->f.cx, game->selob->f.cy, asendx, asendy);
		draw_border_d(screen,
			x*32 - game->camx + 1,
			y*24 - game->camy + 1,
			30,
			22,
			2);

		// Draw attack icon if that would make sense
		if(canattack)
		{
			ce = layer_cell_ptr(game->lv->layers[0], x, y);
			if(ce != NULL && ce->ob != NULL && ce->ob->f.otyp == OBJ_PLAYER
				&& ((struct fd_player *)(ce->ob->f.fd))->team != game->curplayer)
			{
				if(game->selob->steps_left >= STEPS_ATTACK)
				{
					draw_img_trans_d_sd(screen, i_icons1,
						x*32 - game->camx,
						y*24 - game->camy,
						32*1, 24*1, 32, 24, 0);
				} else {
					draw_img_trans_d_sd(screen, i_icons1,
						x*32 - game->camx,
						y*24 - game->camy,
						32*4, 24*0, 32, 24, 0);

				}
			}
		}

		// Do A* trace
		int dirlist[1024];
		int dirlen = astar_layer(game->lv->layers[0], dirlist, 1024,
			game->selob->f.cx, game->selob->f.cy, asendx, asendy);

		// Trace
		if(dirlen >= 1)
		{
			// Get start pos
			x = game->selob->f.cx;
			y = game->selob->f.cy;

			for(i = 0; i < dirlen; i++)
			{
				// Get delta
				int dx = face_dir[dirlist[i]][0];
				int dy = face_dir[dirlist[i]][1];

				// Draw line
				if(i == game->selob->steps_left)
					draw_img_trans_d_sd(screen, i_icons1,
						x*32 - game->camx,
						y*24 - game->camy,
						32*0, 24*1, 32, 24, 0);
				else if(i < game->selob->steps_left)
					draw_img_trans_d_sd(screen, i_icons1,
						x*32 - game->camx,
						y*24 - game->camy,
						32*dirlist[i], 24*0, 32, 24, 0);
				else
					draw_img_trans_d_sd(screen, i_icons1,
						x*32 - game->camx,
						y*24 - game->camy,
						32*4, 24*0, 32, 24, 0);

				// Move
				x += dx;
				y += dy;

			}

			// Show position
			if(dirlen <= game->selob->steps_left)
				draw_img_trans_d_sd(screen, i_icons1,
					x*32 - game->camx,
					y*24 - game->camy,
					32*0, 24*1, 32, 24, 0);
			else
				draw_img_trans_d_sd(screen, i_icons1,
					x*32 - game->camx,
					y*24 - game->camy,
					32*4, 24*0, 32, 24, 0);

		}
	}

	// Draw HUD
	for(i = 6; i >= 0; i--)
		draw_img_trans_cmap_d_sd(screen, i_player,
			0,
			0,
			0*32, i*48, 32, 48,
			0, teams[game->curplayer]->cm_player);
		
	draw_printf(screen, i_font16, 16, 32, 0, 1, "PLAYER %i TURN", game->curplayer+1);

	if(game->selob != NULL)
		draw_printf(screen, i_font16, 16, 32, 16, 1, "STEPS %i", game->selob->steps_left);

	// TODO!

	// Flip
	screen_flip();
#ifndef __EMSCRIPTEN__
	SDL_Delay(20);
#endif

}

int game_tick(game_t *game)
{
	int i;
	obj_t *ob;

	// Check if game over
	if(game->main_state == GAME_OVER)
		return 2;

	// Get coordinates
	game->cmx = (game->mx + game->camx)/32;
	game->cmy = (game->my + game->camy)/24;
	//ce = layer_cell_ptr(game->lv->layers[0], mx, my);

	// Tick objects
	for(i = 0; i < game->lv->ocount; i++)
	{
		ob = game->lv->objects[i];

		if(ob == NULL) continue;

		if(ob->f_tick != NULL) ob->f_tick(ob);

		if(ob->freeme)
			i -= level_obj_free(game->lv, ob);
	}

	// Update UI
	if(game->mx < (screen->w>>3)) game->camx -= 4;
	if(game->my < (screen->h>>3)) game->camy -= 4;
	if(game->mx >= ((screen->w*7)>>3)) game->camx += 4;
	if(game->my >= ((screen->h*7)>>3)) game->camy += 4;

	return 0;
}

int game_input(game_t *game)
{
	// Block input if waiting for object
	if(level_obj_waiting(game->lv) != NULL)
		return 0;
	
	// INPUT STARTS HERE

	// Scan keys
	while(input_key_queue_peek() != 0)
	switch(input_key_queue_pop()>>16)
	{
		case SDLK_RETURN | 0x8000:
			// Next turn!
			game_push_newturn(game, game->ab_local, 0, STEPS_PER_TURN); // TODO: Calculate next turn player

			break;
	}

	if((mouse_b & ~mouse_ob) & 1)
		game_push_click(game, game->ab_local, game->mx, game->my, game->camx, game->camy, 0);
	else if((mouse_b & ~mouse_ob) & 4)
		game_push_click(game, game->ab_local, game->mx, game->my, game->camx, game->camy, 2);

	return 0;
}

int gameloop_core(game_t *game)
{
	int i;

	// Draw
	gameloop_draw(game);

	// Process events
	if(input_poll()) return 1;

	// Parse actions
	switch(game->net_mode)
	{
		case NET_LOCAL:
			abuf_poll(game->ab_local);
			while(game_parse_actions(game, game->ab_local, NET_S2C));
			break;

		case NET_CLIENT:
			abuf_poll(game->ab_local);
			while(game_parse_actions(game, game->ab_local, NET_S2C));
			break;

		case NET_SERVER: {
			// Get client stuff
			TCPsocket nfd = SDLNet_TCP_Accept(game->ab_local->sock);
			if(nfd != NULL)
			{
				// Find a free slot
				for(i = 0; i < TEAM_MAX; i++)
				if(game->ab_teams[i] == NULL)
				{
					// Accept it
					game->ab_teams[i] = abuf_new();
					game->ab_teams[i]->sset = SDLNet_AllocSocketSet(1);
					game->ab_teams[i]->sock = nfd;
					SDLNet_AddSocket(game->ab_teams[i]->sset, (void *)game->ab_teams[i]->sock);
					break;
				}

				if(i == TEAM_MAX)
				{
					// Cannot accept new client
					SDLNet_TCP_Close(nfd);
				}

			}

			// Poll
			for(i = 0; i < TEAM_MAX; i++)
			if(game->ab_teams[i] != NULL)
			{
				abuf_poll(game->ab_teams[i]);
				while(game_parse_actions(game, game->ab_teams[i], NET_C2S));
			}

		} break;

	}

	// Update mouse stuff
	game->mx = mouse_x;
	game->my = mouse_y;
	game->mx = mouse_ox;
	game->my = mouse_oy;

	// Process ticks
	game->time_now = SDL_GetTicks();
	while(game->time_now > game->time_next)
	{
		game->time_next += TIME_STEP_MS;
		if(game_tick(game)) return 0;
	}

	if(game_input(game)) return 0;

	return -1;
}

#ifdef __EMSCRIPTEN__
void gameloop_core_cradle(void)
{
	const int net_mode = NET_LOCAL;

	int ret = gameloop_core(rootgame);
	//printf("cradle ret %i\n", ret);
	if(ret == -1) return;
	emscripten_cancel_main_loop();
}
#endif

int gameloop(const char *fname, int net_mode, int player_count, TCPsocket sock)
{
	int i;
	obj_t *ob;

#ifdef __EMSCRIPTEN__
	printf("game beginning\n");
#endif

	// Create game
	if(rootgame != NULL) { game_free(rootgame); rootgame = NULL; }
	rootgame = game_new(net_mode);
	rootgame->player_count = player_count;

	// Prepare action buffer stuff
	if(net_mode == NET_LOCAL || net_mode == NET_CLIENT)
	{
		rootgame->ab_local = abuf_new();

		if(net_mode == NET_LOCAL)
		{
			rootgame->ab_local->loc_chain = rootgame->ab_local;

		} else {
			rootgame->ab_local->sock = sock;
			rootgame->ab_local->sset = SDLNet_AllocSocketSet(1);
			SDLNet_AddSocket(rootgame->ab_local->sset, (void *)rootgame->ab_local->sock);

		}
	}

	if(net_mode == NET_SERVER)
	{
		rootgame->ab_local = abuf_new();
		rootgame->ab_local->sock = sock;

		/*
		for(i = 0; i < player_count; i++)
		{
			rootgame->ab_teams[i] = abuf_new();
		}
		*/
	}

	// Load level
	//printf("loading level\n");
	rootgame->lv = level_load(fname);
	assert(rootgame->lv != NULL); // TODO: Be more graceful
	rootgame->lv->game = rootgame;

	// Remove excess players
	for(i = 0; i < rootgame->lv->ocount; i++)
	{
		if(i < 0) continue;

		ob = rootgame->lv->objects[i];

		if(ob->f.otyp == OBJ_PLAYER
			&& ((struct fd_player *)(ob->f.fd))->team >= player_count)
		{
			i -= level_obj_free(rootgame->lv, ob);
		}
	}

	// Set "playing" state
	rootgame->main_state = GAME_PLAYING;

	// Start turn
	//printf("starting turn!\n");
	gameloop_start_turn(rootgame);

#ifdef __EMSCRIPTEN__
	//printf("inf loop begin\n");
	emscripten_set_main_loop(gameloop_core_cradle, 100.0f, 0);
	//printf("cradle set\n");
#else
	for(;;)
	{
		int ret = gameloop_core(rootgame);
		if(ret == -1) continue;
		return 0;
	}
#endif

	return 0;
}

