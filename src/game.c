/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

game_t *game_m = NULL; // model (server)
game_t *game_v = NULL; // view  (client)

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
	game->settings.player_count = 0;
	game->curplayer = 0;
	game->main_state = GAME_SETUP;
	game->net_mode = net_mode;
	game->curtick = 0;

	game->time_now = game->time_next = SDL_GetTicks();

	game->selob = NULL;
	game->lv = NULL;

	// OK!
	return game;

}

void gameloop_start_turn(game_t *game)
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
		if(game->curplayer >= game->settings.player_count)
			game->curplayer = 0;

		gameloop_start_turn(game);
	}
}

int gameloop_next_turn(game_t *game)
{
	// Deselect object
	game->selob = NULL;

	// Take note of the current player
	int oldcp = game->curplayer;

	// Move to the next player
	game->curplayer++;
	if(game->curplayer >= game->settings.player_count)
		game->curplayer = 0;
	
	// Start their turn
	gameloop_start_turn(game);

	// If it's the same player, they've won
	return game->curplayer != oldcp;
}

void gameloop_draw_playing(game_t *game)
{
	int x, y, i;
	obj_t *ob;
	cell_t *ce;

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


}

void gameloop_draw(game_t *game)
{
	// Clear the screen
	screen_clear(0);

	// Check game mode
	switch(game->main_state)
	{
		case GAME_LOGIN0:
		case GAME_SETUP:
			draw_printf(screen, i_font16, 16,
				8*1, screen->h/2-32, 1,
				"CHECKING IF THIS IS");
			draw_printf(screen, i_font16, 16,
				8*2, screen->h/2-16, 1,
				"ACTUALLY A PARSNIP");
			draw_printf(screen, i_font16, 16,
				8*6, screen->h/2, 1,
				"THEORY SERVER,");
			draw_printf(screen, i_font16, 16,
				8*7, screen->h/2+16, 1,
				"PLEASE WAIT...");
			break;

		case GAME_WAIT_PLAY:
		case GAME_PLAYING:
			gameloop_draw_playing(game);
			break;

		default:
			draw_printf(screen, i_font16, 16, 0, 0, 1, "STATE %i", game->main_state);
			break;
			
	}
	

	// Flip
	screen_flip();
#ifndef __EMSCRIPTEN__
	SDL_Delay(20);
#endif

}

int game_tick_login0(game_t *game)
{
	// Check to see if we've sent what we have to
	if(game->ab_local->state == CLIENT_WAITVER)
	{
		game_push_version(game, game->ab_local, NET_VERSION);
		game->ab_local->state = CLIENT_WAITVERREPLY;
	}

	return 0;
}

int game_tick_playing(game_t *game)
{
	int i;
	obj_t *ob;

	// Get coordinates
	game->cmx = (game->mx + game->camx)/32;
	game->cmy = (game->my + game->camy)/24;
	//ce = layer_cell_ptr(game->lv->layers[0], mx, my);

	// Tick objects
	if(game->lv != NULL)
	for(i = 0; i < game->lv->ocount; i++)
	{
		ob = game->lv->objects[i];

		if(ob == NULL) continue;

		if(ob->f_tick != NULL) ob->f_tick(ob);

		if(ob->freeme)
			i -= level_obj_free(game->lv, ob);
	}

	// Update UI
	if(game->net_mode != NET_SERVER )
	{
		if(game->mx < (screen->w>>3)) game->camx -= 4;
		if(game->my < (screen->h>>3)) game->camy -= 4;
		if(game->mx >= ((screen->w*7)>>3)) game->camx += 4;
		if(game->my >= ((screen->h*7)>>3)) game->camy += 4;
	}

	return 0;
}

int game_tick(game_t *game)
{
	// Check if game over
	if(game->main_state == GAME_OVER)
		return 2;

	switch(game->main_state)
	{
		case GAME_LOGIN0:
			return game_tick_login0(game);

		case GAME_WAIT_PLAY:
		case GAME_PLAYING:
			return game_tick_playing(game);
			break;

		default:
			// TODO!
			break;
			
	}

	return 0;
}

int game_input(game_t *game)
{
	// Block input if waiting for object
	if(game->ab_local->state != CLIENT_UNLOCKED)
		return 0;
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

	// Parse actions
	assert(game->net_mode != NET_LOCAL);

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
			TCPsocket nfd = (game->ab_local->sock == NULL
				? NULL
				: SDLNet_TCP_Accept(game->ab_local->sock));

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
			abuf_poll(game->ab_local);
			while(game_parse_actions(game, game->ab_local, NET_C2S));

			for(i = 0; i < TEAM_MAX; i++)
			if(game->ab_teams[i] != NULL)
			{
				abuf_poll(game->ab_teams[i]);
				while(game_parse_actions(game, game->ab_teams[i], NET_C2S));
			}

		} break;

	}

	// Update mouse stuff (if not locked)
	if(game->ab_local->state == CLIENT_UNLOCKED)
	{
		game->mx = mouse_x;
		game->my = mouse_y;
		game->mx = mouse_ox;
		game->my = mouse_oy;
	}

	// Process ticks
	game->time_now = SDL_GetTicks();
	while(game->time_now > game->time_next)
	{
		game->time_next += TIME_STEP_MS;
		if(game_tick(game)) return 0;
	}

	return -1;
}

#ifdef __EMSCRIPTEN__
void gameloop_core_cradle(void)
{
	// Draw
	gameloop_draw(game_v);

	// Process events
	if(input_poll()) return;

	// Process input
	if(game_input(game_v)) return;

	// Tick
	if(gameloop_core(game_v) == -1 && gameloop_core(game_m) == -1) return;

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
	if(game_v != NULL) { game_free(game_v); game_v = NULL; }
	if(game_m != NULL) { game_free(game_m); game_m = NULL; }

	// Prepare model
	if(net_mode == NET_LOCAL || net_mode == NET_SERVER)
	{
		game_m = game_new(NET_SERVER);
		assert(game_m != NULL);
		game_m->settings.player_count = player_count;
	}

	// Prepare view
	if(net_mode == NET_LOCAL || net_mode == NET_CLIENT)
	{
		game_v = game_new(NET_CLIENT);
		assert(game_v != NULL);
		// TODO: sync crap
		game_v->settings.player_count = player_count;
	}

	// Prepare action buffer stuff
	if(game_v != NULL)
	{
		game_v->ab_local = abuf_new();

		if(net_mode != NET_LOCAL)
		{
			game_v->ab_local->sock = sock;
			game_v->ab_local->sset = SDLNet_AllocSocketSet(1);
			SDLNet_AddSocket(game_v->ab_local->sset, (void *)game_v->ab_local->sock);

		}
	}

	if(game_m != NULL)
	{
		game_m->ab_local = abuf_new();

		if(net_mode != NET_LOCAL)
		{
			game_m->ab_local->sock = sock;
		}
	}

	if(net_mode == NET_LOCAL)
	{
		assert(game_v != NULL && game_m != NULL);
		game_v->ab_local->loc_chain = game_m->ab_local;
		game_m->ab_local->loc_chain = game_v->ab_local;
	}

	if(game_m != NULL)
	{
		// Load level
		//printf("loading level\n");
		game_m->lv = level_load(fname);
		assert(game_m->lv != NULL); // TODO: Be more graceful
		game_m->lv->game = game_m;

		// Remove excess players
		for(i = 0; i < game_m->lv->ocount; i++)
		{
			if(i < 0) continue;

			ob = game_m->lv->objects[i];

			if(ob->f.otyp == OBJ_PLAYER
				&& ((struct fd_player *)(ob->f.fd))->team >= player_count)
			{
				i -= level_obj_free(game_m->lv, ob);
			}
		}

		game_m->main_state = GAME_WAIT_PLAY;

		// Start turn
		//printf("starting turn!\n");
		gameloop_start_turn(game_m);
	}

	if(game_v != NULL)
	{
		// Set "login" state
		game_v->main_state = GAME_LOGIN0;
	}

#ifdef __EMSCRIPTEN__
	//printf("inf loop begin\n");
	emscripten_set_main_loop(gameloop_core_cradle, 100.0f, 0);
	//printf("cradle set\n");
#else
	for(;;)
	{
		// Draw
		gameloop_draw(game_v);

		// Process events
		if(input_poll()) return 0;

		// Process input
		if(game_input(game_v)) return 0;

		if(gameloop_core(game_m) != -1) return 0;
		if(gameloop_core(game_v) != -1) return 0;
	}
#endif

	return 0;
}

