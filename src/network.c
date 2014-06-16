/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

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
		if(!((tid >= 0 && tid < game->settings.player_count) || tid == 0xFF)) return;

		// Check: To override things we have to be the admin
		if(game->claim_admin != netid)
			if(tid == 0xFF ? game->claim_admin != 0xFF : game->claim_team[tid] != 0xFF)
				return;

	} else {
		assert((tid >= 0 && tid < game->settings.player_count) || tid == 0xFF);
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
	{
		abuf_bc_u8(ACT_NEWTURN, game);
		abuf_bc_u8(tid, game);
		abuf_bc_s16(steps_added, game);
	}

}

void game_handle_move(game_t *game, abuf_t *ab, int typ, int sx, int sy, int dx, int dy, int steps_used, int steps_left)
{
	cell_t *ce, *dce;

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
	{
		abuf_bc_u8(ACT_SELECT, game);
		abuf_bc_s16(cx, game);
		abuf_bc_s16(cy, game);
	}

}

void game_handle_deselect(game_t *game, abuf_t *ab, int typ)
{
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
	{
		abuf_bc_u8(ACT_DESELECT, game);
	}
}

void game_handle_hover(game_t *game, abuf_t *ab, int typ, int mx, int my, int camx, int camy)
{
	// TODO
}

int game_parse_actions(game_t *game, abuf_t *ab, int typ)
{
	// Make sure we have a byte
	char buf[257];
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

