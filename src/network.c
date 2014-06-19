/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

char cli_psl_fname[2048];

void game_push_version(game_t *game, abuf_t *ab, int version)
{
	abuf_write_u8(ACT_VERSION, ab);
	abuf_write_u8(version, ab);
}

void game_push_quit(game_t *game, abuf_t *ab, const char *msg)
{
	// GOODBYE KIND SIR
	int len = strlen(msg);
	if(len > 255) len = 255;

	abuf_write_u8(ACT_QUIT, ab);
	abuf_write_u8(len, ab);
	abuf_write_block(msg, len, ab);

	ab->state = CLIENT_DEAD;
}

void game_push_text(game_t *game, abuf_t *ab, const char *msg)
{
	int len = strlen(msg);
	if(len > 255) len = 255;

	abuf_write_u8(ACT_TEXT, ab);
	abuf_write_u8(len, ab);
	abuf_write_block(msg, len, ab);
}

void game_push_netid(game_t *game, abuf_t *ab, int netid)
{
	abuf_write_u8(ACT_NETID, ab);
	abuf_write_u8(netid, ab);

}

void game_push_settings(game_t *game, abuf_t *ab, game_settings_t *settings)
{
	int lmapname = strlen(settings->map_name);
	if(lmapname > 255) lmapname = 255;

	abuf_write_u8(ACT_SETTINGS, ab);
	abuf_write_u8(settings->player_count, ab);
	abuf_write_u8(lmapname, ab);
	abuf_write_block(settings->map_name, lmapname, ab);
}

void game_push_claim(game_t *game, abuf_t *ab, int netid, int tid)
{
	abuf_write_u8(ACT_CLAIM, ab);
	abuf_write_u8(netid, ab);
	abuf_write_u8(tid, ab);

}

void game_push_unclaim(game_t *game, abuf_t *ab, int netid, int tid)
{
	abuf_write_u8(ACT_UNCLAIM, ab);
	abuf_write_u8(netid, ab);
	abuf_write_u8(tid, ab);

}

void game_push_startbutton(game_t *game, abuf_t *ab)
{
	abuf_write_u8(ACT_STARTBUTTON, ab);

}

void game_push_hover(game_t *game, abuf_t *ab, int mx, int my, int camx, int camy)
{
	abuf_write_u8(ACT_HOVER, ab);
	abuf_write_s16(mx, ab);
	abuf_write_s16(my, ab);
	abuf_write_s16(camx, ab);
	abuf_write_s16(camy, ab);

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
			return;

		} else if(!game_1button) {
			// Deselect objects
			abuf_write_u8(ACT_DESELECT, ab);
			return;

		}
	}
	
	if(button == 2 && game_1button)
	{
		// Deselect objects
		abuf_write_u8(ACT_DESELECT, ab);
		return;
	}

	// Object move / attack
	if(game_1button ? button == 0 : button == 2)
	{
		// Check if we have an object selected

		if(game->selob != NULL)
		{
			// Check destination
			dce = layer_cell_ptr(game->lv->layers[0], mx, my);

			if(dce != NULL && dce->ob == NULL)
			{
				// Do A* trace
				int dirlist[1024];
				int dirlen = astar_layer(game->lv->layers[0], dirlist, 1024,
					game->selob->f.cx, game->selob->f.cy, mx, my);

				// Check if in range
				if(dirlen <= game->selob->steps_left)
				{
					// Move it
					abuf_write_u8(ACT_MOVE, ab);
					abuf_write_s16(game->selob->f.cx, ab);
					abuf_write_s16(game->selob->f.cy, ab);
					abuf_write_s16(mx, ab);
					abuf_write_s16(my, ab);
					abuf_write_u16(0, ab);
					abuf_write_u16(game->selob->steps_left, ab);
				}

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

void game_handle_version(game_t *game, abuf_t *ab, int typ, int ver)
{
	int i;

	printf("Version received: %i\n", ver);

	assert(typ == NET_C2S || typ == NET_S2C);

	if(ver != NET_VERSION)
	{
		game_push_quit(game, ab, "Version mismatch");

	} else if(typ == NET_C2S) {
		// Check if we're in the right state
		if(game->main_state != GAME_SETUP)
		{
			// Nope. Kick!
			game_push_quit(game, ab, "Game started");
			return;

		}

		// Push the version back
		game_push_version(game, ab, NET_VERSION);
		game_push_text(game, ab, "Connected!");

		// Push the settings
		game_push_settings(game, ab, &(game->settings));

		// Push the client's netid
		game_push_netid(game, ab, ab->netid);

		// Give admin if admin is unclaimed
		if(game->claim_admin == 0xFF)
			game->claim_admin = ab->netid;

		// Push claims
		for(i = 0; i < TEAM_MAX; i++)
			if(game->claim_team[i] != 0xFF)
				game_push_claim(game, ab, game->claim_team[i], i);

		if(game->claim_admin != 0xFF)
			game_push_claim(game, ab, game->claim_admin, 0xFF);

		// Move to setup mode
		// TODO: Push current setup state
		ab->state = CLIENT_SETUP;

	} else if(typ == NET_S2C) {
		// Acknowledge, and move to the setup phase
		ab->state = CLIENT_SETUP;
		game->main_state = GAME_SETUP;

	} else {
		game_push_quit(game, ab, "Don't send the version here!");

	}
}

void game_handle_claim(game_t *game, abuf_t *ab, int typ, int netid, int tid)
{
	assert(typ == NET_C2S || typ == NET_S2C);

	if(typ == NET_C2S)
	{
		netid = ab->netid;
		//if(!((tid >= 0 && tid < game->settings.player_count) || tid == 0xFF)) return;
		if(!((tid >= 0 && tid < TEAM_MAX) || tid == 0xFF)) return;

		// Check: To override things we have to be the admin
		if(game->claim_admin != netid)
			if(tid == 0xFF ? game->claim_admin != 0xFF : game->claim_team[tid] != 0xFF)
				return;

	} else {
		//assert((tid >= 0 && tid < game->settings.player_count) || tid == 0xFF);
		assert((tid >= 0 && tid < TEAM_MAX) || tid == 0xFF);
		assert((netid >= 0 && netid < TEAM_MAX) || netid == 0xFE);

	}

	if(tid == 0xFF) game->claim_admin = netid;
	else game->claim_team[tid] = netid;

	// Relay
	if(typ == NET_C2S)
	{
		abuf_bc_u8(ACT_CLAIM, game);
		abuf_bc_u8(netid, game);
		abuf_bc_u8(tid, game);
	}
	
}

void game_handle_unclaim(game_t *game, abuf_t *ab, int typ, int netid, int tid)
{
	assert(typ == NET_C2S || typ == NET_S2C);

	if(typ == NET_C2S)
	{
		netid = ab->netid;
		if(!((tid >= 0 && tid < game->settings.player_count) || tid == 0xFF)) return;

		// Check: To override things we have to be the admin
		if(game->claim_admin != netid)
			if(tid == 0xFF || game->claim_team[tid] != netid)
				return;

	} else {
		assert((tid >= 0 && tid < game->settings.player_count) || tid == 0xFF);
		assert((netid >= 0 && netid < TEAM_MAX) || netid == 0xFE);

	}

	if(tid == 0xFF) game->claim_admin = 0xFF;
	else game->claim_team[tid] = 0xFF;

	// Relay
	if(typ == NET_C2S)
	{
		abuf_bc_u8(ACT_UNCLAIM, game);
		abuf_bc_u8(netid, game);
		abuf_bc_u8(tid, game);
	}

}

void game_handle_startbutton(game_t *game, abuf_t *ab, int typ)
{
	char buf[MAP_BUFFER_SIZE];
	char *mbuf = NULL;
	int buf_offs;
	int i;
	int len;
	FILE *fp;
	obj_t *ob;

	assert(typ == NET_C2S || typ == NET_S2C);

	if(typ == NET_C2S)
	{
		if(!(game->main_state == GAME_SETUP)) return;

		// Check: To begin, we have to be an admin
		if(game->claim_admin != ab->netid)
			return;

	} else {
		assert(game->main_state == GAME_SETUP);

	}

	printf("GAME COMMENCING!\n");
	game->main_state = GAME_LOADING;

	// Relay
	if(typ == NET_C2S)
	{
		abuf_bc_u8(ACT_STARTBUTTON, game);

		// Also relay the map stuff
		// But first we have to load it!

		// Load level
		//printf("loading level\n");
		snprintf(buf, 511, "lvl/%s.psl", game->settings.map_name);
		game->lv = level_load(buf);
		assert(game->lv != NULL); // TODO: Be more graceful
		game->lv->game = game;

		// Remove excess players
		for(i = 0; i < game->lv->ocount; i++)
		{
			if(i < 0) continue;

			ob = game->lv->objects[i];

			if(ob->f.otyp == OBJ_PLAYER
				&& (((struct fd_player *)(ob->f.fd))->team >= game->settings.player_count
				|| game->claim_team[((struct fd_player *)(ob->f.fd))->team]==0xFF))
			{
				i -= level_obj_free(game->lv, ob);
			}
		}

		// Save the map to a temporary file
		//
		// getpid(2) states:
		// Though the ID is guaranteed to be unique, it should NOT be used for con‐
		// structing temporary file names, for security reasons; see mkstemp(3)
		// instead.
		//
		// But because Windows is a thing, I have no choice.
#ifdef WIN32
		sprintf(buf, "svr-psl.%i", getpid());
#else
		strcpy(buf, "/tmp/svr-psl.XXXXXX");
		mktemp(buf);
#endif
		printf("fname \"%s\"\n", buf);
		if(!level_save(game->lv, buf))
		{
			// TODO: Be more graceful
			printf("LEVEL FAILED TO SAVE - ABORTING.\n");
			abort();
		}

		// Reload the map
		level_free(game->lv);
		game->lv = level_load(buf);
		assert(game->lv != NULL); // TODO: Be more graceful
		game->lv->game = game;

		// Load it as a file
		fp = fopen(buf, "rb");
		buf_offs = 0;
		for(;;)
		{
			len = fread(buf, 1, MAP_BUFFER_SEND, fp);
			assert(len >= 0); // TODO: Be more graceful
			if(len == 0) break;
			mbuf = realloc(mbuf, buf_offs + len);
			memcpy(mbuf + buf_offs, buf, len);
			buf_offs += len;
		}
		fclose(fp);

		// Now send the map
		len = buf_offs;
		assert(len <= 0xFFFFFF);
		abuf_bc_u8(ACT_MAPBEG, game);
		abuf_bc_u16(len & 0xFFFF, game);
		abuf_bc_u8((len>>16) & 0xFFFF, game);

		buf_offs = 0;
		while(buf_offs < len)
		{
			abuf_bc_u8(ACT_MAPDATA, game);
			if(len - buf_offs >= MAP_BUFFER_SEND)
			{
				abuf_bc_u16(MAP_BUFFER_SEND, game);
				abuf_bc_block(mbuf + buf_offs, MAP_BUFFER_SEND, game);
				buf_offs += MAP_BUFFER_SEND;

			} else {
				abuf_bc_u16(len - buf_offs, game);
				abuf_bc_block(mbuf + buf_offs, len - buf_offs, game);
				break;

			}
		}

		// Free and end
		free(mbuf);
		abuf_bc_u8(ACT_MAPEND, game);

		// Start turn
		printf("starting turn!\n");
		game->curplayer = game->settings.player_count-1;
		gameloop_next_turn(game, -1, STEPS_PER_TURN);

		// All sorted! Now move on.
		game->main_state = GAME_PLAYING;
	}

}

void game_handle_quit(game_t *game, abuf_t *ab, int typ, const char *msg)
{
	// Mark connection dead
	ab->state = CLIENT_DEAD;

	// Disconnect if possible
	if(ab->sock != NULL)
	{
		SDLNet_TCP_Close(ab->sock);
		ab->sock = NULL;
	}

	if(ab->loc_chain != NULL)
	{
		ab->loc_chain = NULL;
	}

	// Client-specific stuff
	if(typ == NET_S2C)
	{
		// Mark game dead
		game->main_state = GAME_OVER;
		errorloop(msg);
	}
}

void game_handle_text(game_t *game, abuf_t *ab, int typ, const char *msg)
{
	// TODO
}

void game_handle_netid(game_t *game, abuf_t *ab, int typ, int netid)
{
	assert(typ == NET_C2S || typ == NET_S2C);
	if(typ == NET_C2S) { game_push_quit(game, ab, "S->C packet only"); return; }

	// Set netid
	assert((netid >= 0 && netid < TEAM_MAX) || netid == 0xFE);
	game->netid = netid;
}

void game_handle_settings(game_t *game, abuf_t *ab, int typ, int player_count, const char *map_name)
{
	int i;

	assert(typ == NET_C2S || typ == NET_S2C);

	printf("settings: players=%i, mapname=\"%s\"\n", player_count, map_name);
	
	if(typ == NET_C2S)
	{
		if(ab->netid != game->claim_admin) return;
		if(!(player_count >= 2 && player_count <= TEAM_MAX)) return;

	} else {
		assert(player_count >= 2 && player_count <= TEAM_MAX);

	}

	game->settings.player_count = player_count;
	strncpy(game->settings.map_name, map_name, 256);
	game->settings.map_name[255] = '\x00';

	if(typ == NET_C2S)
	{
		if(game->ab_local != NULL && game->ab_local->loc_chain != NULL)
			game_push_settings(game, game->ab_local, &(game->settings));

		for(i = 0; i < TEAM_MAX; i++)
			if(game->ab_teams[i] != NULL)
				game_push_settings(game, game->ab_teams[i], &(game->settings));
	}

}

void game_handle_mapbeg(game_t *game, abuf_t *ab, int typ, int len)
{
	assert(typ == NET_C2S || typ == NET_S2C);

	if(typ == NET_C2S)
	{
		return;

	} else {
		assert(game->mapfp == NULL);

	}

	// Write to temporary file
#ifdef WIN32
	sprintf(cli_psl_fname, "cli-psl.%i", getpid());
#else
	strcpy(cli_psl_fname, "/tmp/cli-psl.XXXXXX");
	mktemp(cli_psl_fname);
#endif
	game->mapfp = fopen(cli_psl_fname, "wb");
	assert(game->mapfp != NULL);

}

void game_handle_mapdata(game_t *game, abuf_t *ab, int typ, int len, const char *data)
{
	assert(typ == NET_C2S || typ == NET_S2C);

	if(typ == NET_C2S)
	{
		return;

	} else {
		assert(game->mapfp != NULL);

	}

	// Write data
	int wlen = fwrite(data, len, 1, game->mapfp);
	assert(wlen == 1);
}

void game_handle_mapend(game_t *game, abuf_t *ab, int typ)
{
	assert(typ == NET_C2S || typ == NET_S2C);

	if(typ == NET_C2S)
	{
		return;

	} else {
		assert(game->mapfp != NULL);

	}

	// Close mapfp
	fclose(game->mapfp);
	game->mapfp = NULL;

	// Load map data
	game->lv = level_load(cli_psl_fname);
	assert(game->lv != NULL); // TODO: Be more graceful
	game->lv->game = game;

	// Start turn
	printf("starting turn!\n");
	game->curplayer = game->settings.player_count-1;
	gameloop_next_turn(game, -1, STEPS_PER_TURN);
	game->main_state = GAME_PLAYING;
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
	assert(typ == NET_C2S || typ == NET_S2C);

	if(typ == NET_C2S)
	{
		if(!(game->claim_team[game->curplayer] == ab->netid)) return;
		tid = -1;
		steps_added = STEPS_PER_TURN;

	} else if(typ == NET_S2C) {

	} else return;

	// Next player
	if(!gameloop_next_turn(game, tid, steps_added))
	{
		game->main_state = GAME_OVER;
		printf("Game over!\n");

		// Broadcast
		if(typ == NET_C2S)
		{
			abuf_bc_u8(ACT_NEWTURN, game);
			abuf_bc_u8(0xFF, game);
			abuf_bc_s16(0, game);
		}

	} else {

		// Broadcast
		if(typ == NET_C2S)
		{
			abuf_bc_u8(ACT_NEWTURN, game);
			abuf_bc_u8(game->curplayer, game);
			abuf_bc_s16(steps_added, game);
		}
	}

}

void game_handle_move(game_t *game, abuf_t *ab, int typ, int sx, int sy, int dx, int dy, int steps_used, int steps_left)
{
	cell_t *ce, *dce;

	ce = layer_cell_ptr(game->lv->layers[0], sx, sy);
	dce = layer_cell_ptr(game->lv->layers[0], dx, dy);

	if(typ == NET_C2S)
	{
		if(!(game->claim_team[game->curplayer] == ab->netid)) return;
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
	{
		abuf_bc_u8(ACT_MOVE, game);
		abuf_bc_s16(sx, game);
		abuf_bc_s16(sy, game);
		abuf_bc_s16(dx, game);
		abuf_bc_s16(dy, game);
		abuf_bc_u16(steps_used, game);
		abuf_bc_u16(steps_left, game);
	}

	// TODO: Lock this
}

void game_handle_attack(game_t *game, abuf_t *ab, int typ, int sx, int sy, int dx, int dy, int steps_used, int steps_left)
{
	cell_t *ce, *dce;

	assert(typ == NET_C2S || typ == NET_S2C);

	ce = layer_cell_ptr(game->lv->layers[0], sx, sy);
	dce = layer_cell_ptr(game->lv->layers[0], dx, dy);

	if(typ == NET_C2S)
	{
		if(!(game->claim_team[game->curplayer] == ab->netid)) return;
		if(!(ce != NULL)) return;
		if(!(ce->ob != NULL)) return;
		if(!(dce != NULL)) return;
		if(!(dce->ob != NULL)) return;

	} else if(typ == NET_S2C) {
		assert(ce != NULL);
		assert(ce->ob != NULL);
		assert(dce != NULL);
		assert(dce->ob != NULL);

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
	{
		abuf_bc_u8(ACT_ATTACK, game);
		abuf_bc_s16(sx, game);
		abuf_bc_s16(sy, game);
		abuf_bc_s16(dx, game);
		abuf_bc_s16(dy, game);
		abuf_bc_u16(steps_used, game);
		abuf_bc_u16(steps_left, game);
	}

	// TODO: Lock this
}

void game_handle_select(game_t *game, abuf_t *ab, int typ, int cx, int cy)
{
	cell_t *ce;

	assert(typ == NET_C2S || typ == NET_S2C);

	ce = layer_cell_ptr(game->lv->layers[0], cx, cy);

	if(typ == NET_C2S)
	{
		if(!(game->claim_team[game->curplayer] == ab->netid)) return;
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
	{
		abuf_bc_u8(ACT_SELECT, game);
		abuf_bc_s16(cx, game);
		abuf_bc_s16(cy, game);
	}

}

void game_handle_deselect(game_t *game, abuf_t *ab, int typ)
{
	assert(typ == NET_C2S || typ == NET_S2C);

	if(typ == NET_C2S)
	{
		if(!(game->claim_team[game->curplayer] == ab->netid)) return;
		//

	} else if(typ == NET_S2C) {
		//

	} else return;

	// Deselect
	game->selob = NULL;

	// Broadcast
	if(typ == NET_C2S)
	{
		abuf_bc_u8(ACT_DESELECT, game);
	}
}

void game_handle_hover(game_t *game, abuf_t *ab, int typ, int mx, int my, int camx, int camy)
{
	assert(typ == NET_C2S || typ == NET_S2C);

	if(typ == NET_C2S)
	{
		if(!(game->claim_team[game->curplayer] == ab->netid)) return;

	} else if(typ == NET_S2C) {
		// Ignore hover events when we have focus
		if(game->claim_team[game->curplayer] == game->netid) return;

	} else return;

	// Move mouse
	game->mx = mx;
	game->my = my;
	game->camx = camx;
	game->camy = camy;

	// Broadcast
	if(typ == NET_C2S)
	{
		abuf_bc_u8(ACT_HOVER, game);
		abuf_bc_s16(mx, game);
		abuf_bc_s16(my, game);
		abuf_bc_s16(camx, game);
		abuf_bc_s16(camy, game);
	}
}

int game_parse_actions(game_t *game, abuf_t *ab, int typ)
{
	// Make sure we have a byte
	char buf[MAP_BUFFER_SIZE+1];
	int ver, len, tid;
	int netid;
	int player_count;
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
			game_handle_quit(game, ab, typ, buf);
			return 1;

		case ACT_TEXT:
			if(bsiz < 1) return 0;
			if(bsiz < 1+(int)(ab->rdata[1])) return 0;
			abuf_read_u8(ab);
			len = abuf_read_u8(ab);
			abuf_read_block(buf, len, ab);
			buf[len] = '\x00';
			printf("text received: \"%s\"\n", buf);
			game_handle_text(game, ab, typ, buf);
			return 1;

		case ACT_NETID:
			if(bsiz < 1) return 0;
			abuf_read_u8(ab);
			netid = abuf_read_u8(ab);
			game_handle_netid(game, ab, typ, netid);
			return 1;
			
		case ACT_SETTINGS:
			if(bsiz < 2) return 0;
			if(bsiz < 2+(int)(ab->rdata[2])) return 0;
			abuf_read_u8(ab);
			player_count = abuf_read_u8(ab);
			len = abuf_read_u8(ab);
			abuf_read_block(buf, len, ab);
			buf[len] = '\x00';
			game_handle_settings(game, ab, typ, player_count, buf);
			return 1;

		case ACT_CLAIM:
			if(bsiz < 2) return 0;
			abuf_read_u8(ab);
			netid = abuf_read_u8(ab);
			tid = abuf_read_u8(ab);
			game_handle_claim(game, ab, typ, netid, tid);
			return 1;

		case ACT_UNCLAIM:
			if(bsiz < 2) return 0;
			abuf_read_u8(ab);
			netid = abuf_read_u8(ab);
			tid = abuf_read_u8(ab);
			game_handle_unclaim(game, ab, typ, netid, tid);
			return 1;

		case ACT_STARTBUTTON:
			if(bsiz < 0) return 0;
			abuf_read_u8(ab);
			game_handle_startbutton(game, ab, typ);
			return 1;

		case ACT_MAPBEG:
			if(bsiz < 3) return 0;
			abuf_read_u8(ab);
			len = abuf_read_u16(ab);
			len += ((int)abuf_read_u8(ab))<<16;
			game_handle_mapbeg(game, ab, typ, len);
			return 1;

		case ACT_MAPDATA:
			if(bsiz < 2) return 0;
			if(bsiz < 2+((int)ab->rdata[1])+(256*(int)ab->rdata[2])) return 0;
			abuf_read_u8(ab);
			len = abuf_read_u16(ab);
			assert(len <= MAP_BUFFER_SIZE);
			abuf_read_block(buf, len, ab);
			game_handle_mapdata(game, ab, typ, len, buf);
			return 1;

		case ACT_MAPEND:
			if(bsiz < 0) return 0;
			abuf_read_u8(ab);
			game_handle_mapend(game, ab, typ);
			return 1;

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

