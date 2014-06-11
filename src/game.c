/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

int game_camx = 0;
int game_camy = 0;
int game_mouse_x = 0;
int game_mouse_y = 0;
int game_mouse_ox = 0;
int game_mouse_oy = 0;

obj_t *game_selob = NULL;

int game_player_count = 0;
int game_curplayer = 0;

static void gameloop_start_turn(void)
{
	int i;
	obj_t *ob;
	struct fd_player *fde;
	int obcount = 0;

	// Grant steps to all players of this team
	for(i = 0; i < rootlv->ocount; i++)
	{
		// Get object
		ob = rootlv->objects[i];
		fde = (struct fd_player *)(ob->f.fd);
		
		// Do the compare
		if(fde->team == game_curplayer)
		{
			obcount++;
			ob->steps_left = STEPS_PER_TURN;
			game_camx = ob->f.cx*32+16 - screen->w/2;
			game_camy = ob->f.cy*24+12 - screen->h/2;

		} else {
			ob->steps_left = -1;

		}
	}

	// Move to the next player if this fails
	if(obcount == 0)
	{
		game_curplayer++;
		if(game_curplayer >= game_player_count)
			game_curplayer = 0;

		gameloop_start_turn();
	}
}

static int gameloop_next_turn(void)
{
	// Deselect object
	game_selob = NULL;

	// Take note of the current player
	int oldcp = game_curplayer;

	// Move to the next player
	game_curplayer++;
	if(game_curplayer >= game_player_count)
		game_curplayer = 0;
	
	// Start their turn
	gameloop_start_turn();

	// If it's the same player, they've won
	return game_curplayer != oldcp;
}

void gameloop_draw(void)
{
	int x, y, i;
	obj_t *ob;
	cell_t *ce;

	// Clear the screen
	screen_clear(0);

	// Draw level
	draw_level(screen, rootlv, game_camx, game_camy, 0);

	// Draw overlays
	for(i = 0; i < rootlv->ocount; i++)
	{
		ob = rootlv->objects[i];

		if(ob == NULL) continue;

		// Cell occupancy
		draw_border_d(screen,
			ob->f.cx*32 - game_camx,
			ob->f.cy*24 - game_camy,
			32,
			24,
			1);

		// Bounding box
		if(ob == game_selob)
		draw_border_d(screen,
			ob->bx + ob->f.ox + ob->f.cx*32 - game_camx,
			ob->by + ob->f.oy + ob->f.cy*24 - game_camy,
			ob->bw,
			ob->bh,
			2);
	}

	// TEST: Mark walkable paths
	/*
	for(y = 0; y < rootlv->layers[0]->w; y++)
	for(x = 0; x < rootlv->layers[0]->h; x++)
	{
		cell_t *ce = layer_cell_ptr(rootlv->layers[0], x, y);

		if(ce != NULL && ce->f.ctyp == CELL_FLOOR)
		{
			draw_border_d(screen,
				32*x + 8 - game_camx,
				24*y + 6 - game_camy,
				16,
				12,
				ce->ob == NULL ? 1 : 2);
		}

	}
	*/

	// Draw A* route for selected object
	if(game_selob != NULL)
	{
		// Get coordinates
		int asendx = (game_mouse_x + game_camx)/32;
		int asendy = (game_mouse_y + game_camy)/24;

		// Line trace
		int canattack = line_layer(rootlv->layers[0], &x, &y,
			game_selob->f.cx, game_selob->f.cy, asendx, asendy);
		draw_border_d(screen,
			x*32 - game_camx + 1,
			y*24 - game_camy + 1,
			30,
			22,
			2);

		// Draw attack icon if that would make sense
		if(canattack)
		{
			ce = layer_cell_ptr(rootlv->layers[0], x, y);
			if(ce != NULL && ce->ob != NULL && ce->ob->f.otyp == OBJ_PLAYER
				&& ((struct fd_player *)(ce->ob->f.fd))->team != game_curplayer)
			{
				if(game_selob->steps_left >= STEPS_ATTACK)
				{
					draw_img_trans_d_sd(screen, i_icons1,
						x*32 - game_camx,
						y*24 - game_camy,
						32*1, 24*1, 32, 24, 0);
				} else {
					draw_img_trans_d_sd(screen, i_icons1,
						x*32 - game_camx,
						y*24 - game_camy,
						32*4, 24*0, 32, 24, 0);

				}
			}
		}

		// Do A* trace
		int dirlist[1024];
		int dirlen = astar_layer(rootlv->layers[0], dirlist, 1024,
			game_selob->f.cx, game_selob->f.cy, asendx, asendy);

		// Trace
		if(dirlen >= 1)
		{
			// Get start pos
			x = game_selob->f.cx;
			y = game_selob->f.cy;

			for(i = 0; i < dirlen; i++)
			{
				// Get delta
				int dx = face_dir[dirlist[i]][0];
				int dy = face_dir[dirlist[i]][1];

				// Draw line
				if(i == game_selob->steps_left)
					draw_img_trans_d_sd(screen, i_icons1,
						x*32 - game_camx,
						y*24 - game_camy,
						32*0, 24*1, 32, 24, 0);
				else if(i < game_selob->steps_left)
					draw_img_trans_d_sd(screen, i_icons1,
						x*32 - game_camx,
						y*24 - game_camy,
						32*dirlist[i], 24*0, 32, 24, 0);
				else
					draw_img_trans_d_sd(screen, i_icons1,
						x*32 - game_camx,
						y*24 - game_camy,
						32*4, 24*0, 32, 24, 0);

				// Move
				x += dx;
				y += dy;

			}

			// Show position
			if(dirlen <= game_selob->steps_left)
				draw_img_trans_d_sd(screen, i_icons1,
					x*32 - game_camx,
					y*24 - game_camy,
					32*0, 24*1, 32, 24, 0);
			else
				draw_img_trans_d_sd(screen, i_icons1,
					x*32 - game_camx,
					y*24 - game_camy,
					32*4, 24*0, 32, 24, 0);

		}
	}

	// Draw HUD
	for(i = 6; i >= 0; i--)
		draw_img_trans_cmap_d_sd(screen, i_player,
			0,
			0,
			0*32, i*48, 32, 48,
			0, teams[game_curplayer]->cm_player);
		
	draw_printf(screen, i_font16, 16, 32, 0, 1, "PLAYER %i TURN", game_curplayer+1);

	if(game_selob != NULL)
		draw_printf(screen, i_font16, 16, 32, 16, 1, "STEPS %i", game_selob->steps_left);

	// TODO!

	// Flip
	screen_flip();
	SDL_Delay(20);

}

int gameloop_tick(void)
{
	int i;
	int mx, my;
	obj_t *ob;
	cell_t *ce;

	// Get coordinates
	mx = (game_mouse_x + game_camx)/32;
	my = (game_mouse_y + game_camy)/24;
	ce = layer_cell_ptr(rootlv->layers[0], mx, my);

	// Tick objects
	for(i = 0; i < rootlv->ocount; i++)
	{
		ob = rootlv->objects[i];

		if(ob == NULL) continue;

		if(ob->f_tick != NULL) ob->f_tick(ob);

		if(ob->freeme)
			i -= level_obj_free(rootlv, ob);
	}

	// Update UI
	if(game_mouse_x < (screen->w>>3)) game_camx -= 4;
	if(game_mouse_y < (screen->h>>3)) game_camy -= 4;
	if(game_mouse_x >= ((screen->w*7)>>3)) game_camx += 4;
	if(game_mouse_y >= ((screen->h*7)>>3)) game_camy += 4;

	// Block input if waiting for object
	if(level_obj_waiting(rootlv) != NULL)
		return 0;

	// INPUT STARTS HERE

	// Scan keys
	while(input_key_queue_peek() != 0)
	switch(input_key_queue_pop()>>16)
	{
		case SDLK_RETURN | 0x8000:
			// Next turn!
			if(!gameloop_next_turn())
			{
				//gameloop_win();
				return 2;
			}

			break;
	}

	// Object select
	
	if((mouse_b & ~mouse_ob) & 1)
	{

		if(ce != NULL && ce->ob != NULL && ce->ob->f.otyp == OBJ_PLAYER
			&& ((struct fd_player *)(ce->ob->f.fd))->team == game_curplayer)
		{
			// Select object
			game_selob = ce->ob;

		} else {
			// Deselect objects
			game_selob = NULL;

		}
	}

	// Object move / attack
	if((mouse_b & ~mouse_ob) & 4)
	{
		// Check if we have an object selected

		if(game_selob != NULL)
		{
			// Move it
			game_selob->tx = mx;
			game_selob->ty = my;

			// Destroy the old list
			if(game_selob->asdir != NULL)
			{
				free(game_selob->asdir);
				game_selob->asdir = NULL;
			}

			// Mark it as "please wait"
			game_selob->please_wait = 1;

		}

	}

	return 0;
}

void game_handle_version(abuf_t *ab, int typ, int ver)
{
	// TODO
}

void game_handle_quit(abuf_t *ab, int typ)
{
	// TODO
}

void game_handle_text(abuf_t *ab, int typ, int len, char *buf)
{
	// TODO
}

void game_handle_lock(abuf_t *ab, int typ)
{
	// TODO
}

void game_handle_unlock(abuf_t *ab, int typ)
{
	// TODO
}

void game_handle_newturn(abuf_t *ab, int typ, int tid, int steps_added)
{
	// TODO
}

void game_handle_move(abuf_t *ab, int typ, int sx, int sy, int dx, int dy, int steps_used, int steps_remain)
{
	// TODO
}

void game_handle_attack(abuf_t *ab, int typ, int sx, int sy, int dx, int dy, int steps_used, int steps_remain)
{
	// TODO
}

void game_handle_select(abuf_t *ab, int typ, int cx, int cy)
{
	// TODO
}

void game_handle_deselect(abuf_t *ab, int typ)
{
	// TODO
}

void game_handle_hover(abuf_t *ab, int typ, int mx, int my, int camx, int camy)
{
	// TODO
}

int game_parse_actions(abuf_t *ab, int typ)
{
	// Make sure we have a byte
	char buf[257];
	int ver, len, tid;
	int steps_added;
	int steps_used;
	int steps_remain;
	int sx, sy, dx, dy;
	int cx, cy, mx, my, camx, camy;

	int bsiz = abuf_get_rsize(ab);
	if(bsiz < 1) return 0;
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
			game_handle_version(ab, typ, ver);
			return 1;

		case ACT_QUIT:
			abuf_read_u8(ab);
			len = (bsiz < 1 ? 0 : abuf_read_u8(ab));
			abuf_read_block(buf, (bsiz-1 < len ? bsiz-1 : len), ab);
			buf[bsiz] = '\x00';
			printf("QUIT received: \"%s\"\n", buf);
			game_handle_quit(ab, typ);
			return 1;

		case ACT_TEXT:
			if(bsiz < 1) return 0;
			if(bsiz < 1+(int)(ab->rdata[1])) return 0;
			abuf_read_u8(ab);
			len = abuf_read_u8(ab);
			abuf_read_block(buf, len, ab);
			buf[bsiz] = '\x00';
			printf("text received: \"%s\"\n", buf);
			game_handle_text(ab, typ, len, buf);
			return 1;

		// ACT_MAPBEG
		// ACT_MAPDATA
		// ACT_MAPEND

		case ACT_LOCK:
			if(bsiz < 0) return 0;
			abuf_read_u8(ab);
			game_handle_lock(ab, typ);
			return 1;

		case ACT_UNLOCK:
			if(bsiz < 0) return 0;
			abuf_read_u8(ab);
			game_handle_unlock(ab, typ);
			return 1;

		case ACT_NEWTURN:
			if(bsiz < 2) return 0;
			abuf_read_u8(ab);
			tid = abuf_read_u8(ab);
			steps_added = abuf_read_u16(ab);
			game_handle_newturn(ab, typ, tid, steps_added);
			return 1;

		case ACT_MOVE:
			if(bsiz < 12) return 0;
			abuf_read_u8(ab);
			sx = abuf_read_s16(ab);
			sy = abuf_read_s16(ab);
			dx = abuf_read_s16(ab);
			dy = abuf_read_s16(ab);
			steps_used = abuf_read_u16(ab);
			steps_remain = abuf_read_u16(ab);
			game_handle_move(ab, typ, sx, sy, dx, dy, steps_used, steps_remain);
			return 1;

		case ACT_ATTACK:
			if(bsiz < 12) return 0;
			abuf_read_u8(ab);
			sx = abuf_read_s16(ab);
			sy = abuf_read_s16(ab);
			dx = abuf_read_s16(ab);
			dy = abuf_read_s16(ab);
			steps_used = abuf_read_u16(ab);
			steps_remain = abuf_read_u16(ab);
			game_handle_attack(ab, typ, sx, sy, dx, dy, steps_used, steps_remain);
			return 1;

		case ACT_SELECT:
			if(bsiz < 4) return 0;
			abuf_read_u8(ab);
			cx = abuf_read_s16(ab);
			cy = abuf_read_s16(ab);
			game_handle_select(ab, typ, cx, cy);
			return 1;

		case ACT_DESELECT:
			if(bsiz < 0) return 0;
			abuf_read_u8(ab);
			game_handle_deselect(ab, typ);
			return 1;

		case ACT_HOVER:
			if(bsiz < 8) return 0;
			abuf_read_u8(ab);
			mx = abuf_read_s16(ab);
			my = abuf_read_s16(ab);
			camx = abuf_read_s16(ab);
			camy = abuf_read_s16(ab);
			game_handle_hover(ab, typ, mx, my, camx, camy);
			return 1;

		default:
			printf("FIXME: handle invalid packet %i\n", ab->rdata[0]);
			fflush(stdout);
			abort();
			break;

	}

}

int gameloop(const char *fname, int net_mode, int player_count)
{
	int i;
	obj_t *ob;

	// Free action buffers
	if(ab_local != NULL) { abuf_free(ab_local); ab_local = NULL; }
	for(i = 0; i < TEAM_MAX; i++)
		if(ab_teams[i] != NULL) { abuf_free(ab_teams[i]); ab_teams[i] = NULL; }

	// Prepare action buffer stuff

	// Initialise
	game_camx = 0;
	game_camy = 0;
	game_player_count = player_count;
	game_curplayer = 0;

	// Load level
	rootlv = level_load(fname);
	assert(rootlv != NULL); // TODO: Be more graceful

	// Remove excess players
	for(i = 0; i < rootlv->ocount; i++)
	{
		if(i < 0) continue;

		ob = rootlv->objects[i];

		if(ob->f.otyp == OBJ_PLAYER
			&& ((struct fd_player *)(ob->f.fd))->team >= player_count)
		{
			i -= level_obj_free(rootlv, ob);
		}
	}


	// Start turn
	gameloop_start_turn();
	
	for(;;)
	{
		// Draw
		gameloop_draw();

		// Process events
		if(input_poll()) return 1;

		game_mouse_x = mouse_x;
		game_mouse_y = mouse_y;
		game_mouse_ox = mouse_ox;
		game_mouse_oy = mouse_oy;

		// Process tick
		// TODO: Time this nicely
		if(gameloop_tick()) return 0;
	}

	return 0;
}

