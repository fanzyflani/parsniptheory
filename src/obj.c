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

//
// OBJ_PLAYER
//

int obj_player_f_init(obj_t *ob)
{
	struct fd_player *fde = (struct fd_player *)ob->f.fd;
	cell_t *ce;

	assert(fde->team >= 0 && fde->team < TEAM_MAX);
	ob->img = i_player;
	ob->cmap = teams[fde->team]->cm_player;

	ce = layer_cell_ptr(ob->level->layers[ob->f.layer], ob->f.cx, ob->f.cy);
	if(ce != NULL)
		ce->ob = ob;

	ob->bx =   6;
	ob->by = -16;
	ob->bw =  20;
	ob->bh =  36;

	ob->tx = ob->f.cx;
	ob->ty = ob->f.cy;
	ob->tmode = 0;

	ob->health = PLAYER_HEALTH;

	ob->skintone = 32+((rand()>>12)&7)*4;

	return 1;
}

int obj_player_f_init_fd(obj_t *ob)
{
	struct fd_player *fde = (struct fd_player *)ob->f.fd;

	fde->team = 0;
	fde->face = 0;

	return 1;
}

int obj_player_f_load_fd(obj_t *ob, FILE *fp)
{
	struct fd_player *fde = (struct fd_player *)ob->f.fd;

	fde->team = fgetc(fp);
	fde->face = fgetc(fp);

	return 1;

}

int obj_player_f_save_fd(obj_t *ob, FILE *fp)
{
	struct fd_player *fde = (struct fd_player *)ob->f.fd;

	fputc(fde->team, fp);
	fputc(fde->face, fp);

	return 1;

}

void obj_player_f_reset(obj_t *ob)
{
	//

}

void obj_player_f_tick(obj_t *ob)
{
	int cx, cy;
	int dx, dy;
	struct fd_player *fde = (struct fd_player *)ob->f.fd;

	// Enforce cell association
	obj_t *pob;
	cell_t *cs = layer_cell_ptr(ob->level->layers[ob->f.layer], ob->f.cx, ob->f.cy);
	cell_t *ce;
	assert(cs != NULL);
	cs->ob = ob;

	// Check if we're not "forfeited"
	if(ob->health <= 0)
	{
		ob->freeme = 1;
		return;
	}

	// Check if crouching / standing
	if(ob->f.flags & OF_CROUCH)
	{
		if(ob->crouch_trans != 8)
		{
			ob->crouch_trans++;
			if(ob->crouch_trans >= 8)
			{
				ob->crouch_trans = 8;
				ob->please_wait = 0;
			}

			return;
		}

	} else {
		if(ob->crouch_trans != 0)
		{
			ob->crouch_trans--;
			if(ob->crouch_trans <= 0)
			{
				ob->crouch_trans = 0;
				ob->please_wait = 0;
			}

			return;
		}

	}

	// Check if we have steps left
	if(ob->steps_left == 0)
	{
		// Forfeit motion
		// TODO: Allow this to continue
		free(ob->asdir);
		ob->asdir = NULL;
		ob->tx = ob->f.cx;
		ob->ty = ob->f.cy;

		// Forfeit lock
		ob->please_wait = 0;
		ob->tmode = 0;
		return;
	}

	// Check if walking
	if(ob->f.ox == 0 && ob->f.oy == 0) do
	{
		// If we're attacking something, attack it (if possible)
		ce = layer_cell_ptr(ob->level->layers[ob->f.layer], ob->tx, ob->ty);

		if(ob->tmode == 2)
		if(ce == NULL || ce->ob == NULL ||
			(ce->ob->f.otyp == OBJ_PLAYER &&
			((struct fd_player *)(ce->ob->f.fd))->team != fde->team))
		{
			// Step check
			if(ob->steps_left >= STEPS_ATTACK)
			{
				// Drop steps
				ob->steps_left -= STEPS_ATTACK;

				// Do a line trace
				line_layer(ob->level->layers[ob->f.layer], &cx, &cy,
					ob->f.cx, ob->f.cy, ob->tx, ob->ty);

				// Add a tomato object
				pob = level_obj_add(ob->level, OBJ_FOOD_TOMATO, 0, cx, cy, ob->f.layer);
				pob->tx = 32*(ob->f.cx - cx);
				pob->ty = 24*(ob->f.cy - cy);
				int dist = sqrt(pob->tx*pob->tx + pob->ty*pob->ty);
				pob->tx <<= 8;
				pob->ty <<= 8;
				pob->vx = (pob->tx*10)/dist;
				pob->vy = (pob->ty*10)/dist;
				pob->time = dist/10;
			}

			// Clear target and stuff
			ob->tx = ob->f.cx;
			ob->ty = ob->f.cy;
			ob->tmode = 0;
			ob->please_wait = 0;

			// Don't bother with the rest of this block
			break;

		}
		
		// If A* works, follow
		if(ob->tmode == 1)
		if(ob->asdir != NULL) do
		{
			// Check if at end
			if(ob->asidx >= ob->aslen)
			{
				// Destroy the path and try again
				free(ob->asdir);
				ob->asdir = NULL;

				break;

			}

			// Get direction
			int dir = ((int *)ob->asdir)[ob->asidx];
			dx = face_dir[dir][0];
			dy = face_dir[dir][1];

			// Get neighbour cell
			cx = ob->f.cx + dx;
			cy = ob->f.cy + dy;

			// Check cell
			ce = layer_cell_ptr(ob->level->layers[ob->f.layer], cx, cy);
			if(ce == NULL || ce->f.ctyp != CELL_FLOOR || ce->ob != NULL)
			{
				// Destroy the path and try again
				free(ob->asdir);
				ob->asdir = NULL;

				break;
			}

			// Move cell association
			assert(cs->ob == ob);
			cs->ob = NULL;
			ce->ob = ob;

			// Move object
			ob->asidx++;
			ob->f.cx += dx;
			ob->f.cy += dy;
			ob->f.ox -= 32*dx;
			ob->f.oy -= 24*dy;
			fde->face = dir;
		} while(0);

		// Perform A* towards target
		if(ob->asdir == NULL)
		{
			// Do A* trace
			int dirlist[1024];
			int dirlen = astar_layer(ob->level->layers[0], dirlist, 1024,
				ob->f.cx, ob->f.cy, ob->tx, ob->ty);

			// If it works, produce a new list
			if(dirlen >= 1)
			{
				// Allocate + copy
				ob->asdir = realloc(ob->asdir, sizeof(int) * dirlen);
				memcpy(ob->asdir, dirlist, sizeof(int) * dirlen);

				// Set things
				ob->aslen = dirlen;
				ob->asidx = 0;
				
			} else {
				// Release "please wait" flag
				ob->tmode = 0;
				ob->please_wait = 0;

			}
		}


	} while(0); else if(ob->steps_left >= (ob->f.flags & OF_CROUCH ? 2 : 1)) {
		// Move
		if(ob->f.ox < 0) ob->f.ox += (ob->f.flags & OF_CROUCH ? 2 : 3);
		if(ob->f.ox > 0) ob->f.ox -= 1;
		if(ob->f.ox > 0) ob->f.ox -= 1;
		if(ob->f.ox > 0 && !(ob->f.flags & OF_CROUCH)) ob->f.ox -= 1;

		if(ob->f.oy < 0) ob->f.oy += (ob->f.flags & OF_CROUCH ? 1 : 2);
		if(ob->f.oy > 0) ob->f.oy -= 1;
		if(ob->f.oy > 0 && !(ob->f.flags & OF_CROUCH)) ob->f.oy -= 1;

		if(ob->level->game->net_mode != NET_SERVER)
		if(ob->f.ox == 14 || ob->f.ox == -14 || ob->f.oy == 12 || ob->f.oy == -12)
		{
			if(((ob->f.cx^ob->f.cy)&1) != 0)
				snd_play_step(1, 1, ob->f.cx*32+16 + ob->f.ox, ob->f.cy*24+12 + ob->f.oy);
			else
				snd_play_step(1, 0, ob->f.cx*32+16 + ob->f.ox, ob->f.cy*24+12 + ob->f.oy);
		}

		// Decrease steps at end
		if(ob->f.ox == 0 && ob->f.oy == 0)
		{
			ob->steps_left -= (ob->f.flags & OF_CROUCH ? 2 : 1);
		}
	}
}

void obj_player_f_draw(obj_t *ob, img_t *dst, int camx, int camy)
{
	int i, ii;
	int pox, poy;
	int aox, aoy, aot;
	int dox, doy;
	int dflip;
	struct fd_player *fde = (struct fd_player *)ob->f.fd;

	// Set skin tone
	for(i = 0; i < 4; i++)
		ob->cmap[16+i] = ob->skintone+i;

	// Get abs offset + direction
	dox = (ob->f.ox < 0 ? -1 : ob->f.ox > 0 ? 1 : 0);
	doy = (ob->f.oy < 0 ? -1 : ob->f.oy > 0 ? 1 : 0);
	aox = (ob->f.ox < 0 ? -ob->f.ox : ob->f.ox);
	aoy = (ob->f.oy < 0 ? -ob->f.oy : ob->f.oy);
	aot = aox*24 + aoy*32;

	// Draw parts
	for(ii = 6; ii >= 0; ii--)
	{
		i = ii;

		// Fix rendering order
		if(fde->face == DIR_EAST)
			i = (i == 3 ? 2 : i == 2 ? 3 : i);
		if(fde->face == DIR_WEST)
			i = (i == 4 ? 2 : i == 2 ? 4 : i);
		if(fde->face == DIR_NORTH)
		{
			switch(i)
			{
				case 0: i = 2; break;
				case 1: i = 0; break;
				case 2: i = 1; break;
			}
		}

		pox = 0;
		poy = 0;
		dflip = 1;
		int ramp = (aot-32*24/2);
		ramp = (ramp < 0 ? -ramp : ramp);
		ramp -= 32*24/4;
		ramp *= ramp;
		ramp = 32*24*32*24/4/4 - ramp;
		ramp >>= 14;

		switch(i)
		{
			case 0:
			case 1:
			case 2:
				poy -= ramp;
				break;
		}

		int isel = (i+1)^(1&(ob->f.cx^ob->f.cy));
		switch(isel)
		{
			case 5:
				dflip = -1;
			case 4:

				if(aot <= 24*32/2)
				{
					pox = ob->f.ox;
					poy = ob->f.oy;

				} else {
					pox = dox*32-ob->f.ox;
					poy = doy*24-ob->f.oy;
				}

				pox >>= 2;
				poy >>= 2;
				pox *= dflip;
				poy *= dflip;
				break;

			case 6:
				dflip = -1;
			case 7:

				if(aot <= 24*32/2)
				{
					pox = ob->f.ox;
					poy = ob->f.oy;

				} else {
					pox = dox*32-ob->f.ox;
					poy = doy*24-ob->f.oy;
				}

				pox *= dflip;
				poy *= dflip;

				if((aot <= 24*32/2 ? isel == 7 : isel == 6))
					poy -= ramp;
				break;
		}

		switch(i)
		{
			case 0:
			case 1:
				poy += ob->crouch_trans>>0;
				break;
			case 2:
			case 3:
			case 4:
				poy += ob->crouch_trans>>1;
				break;
		}

		draw_img_trans_cmap_d_sd(dst, ob->img,
			camx + ob->f.cx*32 + ob->f.ox + pox,
			camy + ob->f.cy*24 + ob->f.oy + poy - 21,
			(fde->face&3)*32, i*48, 32, 48,
			0, ob->cmap);
	}

	// TEST: Draw tomato
	/*
	draw_img_trans_cmap_d_sd(dst, i_food1,
		camx + ob->f.cx*32 + ob->f.ox,
		camy + ob->f.cy*24 + ob->f.oy - 4,
		(fde->face&3)*32, 0*32, 32, 32,
		0, cm_food1);
	*/

	// Draw health
	draw_57_printf(dst,
		camx + 32*ob->f.cx + ob->f.ox,
		camy + 24*ob->f.cy + ob->f.oy + ob->crouch_trans - 24,
		1, "%i", ob->health);

	// Draw team number (because colourblind people are a thing)
	draw_57_printf(dst,
		camx + 32*ob->f.cx + ob->f.ox + 30 - 4*3,
		camy + 24*ob->f.cy + ob->f.oy + ob->crouch_trans - 24,
		1, "%3i", fde->team+1);
}

void obj_player_f_free(obj_t *ob)
{
	//

}


//
// OBJ_FOOD_TOMATO
//

int obj_food_tomato_f_init(obj_t *ob)
{
	struct fd_food *fde = (struct fd_food *)ob->f.fd;

	assert(fde->team >= 0 && fde->team < TEAM_MAX);
	ob->img = i_food1;
	ob->cmap = cm_food1;

	ob->bx =   5;
	ob->by =   5;
	ob->bw =   6;
	ob->bh =   6;

	ob->tx = ob->f.cx;
	ob->ty = ob->f.cy;
	ob->tmode = 0;

	ob->please_wait = 1;

	return 1;
}

int obj_food_tomato_f_init_fd(obj_t *ob)
{
	struct fd_food *fde = (struct fd_food *)ob->f.fd;

	fde->team = 0;
	fde->face = 0;

	return 1;
}

int obj_food_tomato_f_load_fd(obj_t *ob, FILE *fp)
{
	struct fd_food *fde = (struct fd_food *)ob->f.fd;

	fde->team = fgetc(fp);
	fde->face = fgetc(fp);

	return 1;

}

int obj_food_tomato_f_save_fd(obj_t *ob, FILE *fp)
{
	struct fd_food *fde = (struct fd_food *)ob->f.fd;

	fputc(fde->team, fp);
	fputc(fde->face, fp);

	return 1;

}

void obj_food_tomato_f_reset(obj_t *ob)
{
	//

}

void obj_food_tomato_f_tick(obj_t *ob)
{
	//struct fd_food *fde = (struct fd_food *)ob->f.fd;
	cell_t *ce;
	
	// Advance
	ob->tx -= ob->vx;
	ob->ty -= ob->vy;
	ob->time--;

	if(ob->time == 0)
	{
		// Stop waiting
		ob->please_wait = 0;

		// Do damage
		ce = layer_cell_ptr(ob->level->layers[ob->f.layer], ob->f.cx, ob->f.cy);
		int splatidx = ((rand()>>12)&3);
		if((ce->splatters[FOOD_TOMATO] & (1<<splatidx)) == 0)
		{
			ce->splatters[FOOD_TOMATO] |= 1<<splatidx;
			ce->splatpos[FOOD_TOMATO][splatidx] = (rand()>>5)&255;
		}

		if(ob->level->game->net_mode != NET_SERVER)
			snd_play_splat(1, ob->f.cx*32+16, ob->f.cy*24+12);

		if(ce->ob && ce->ob->f.otyp == OBJ_PLAYER)
		{
			//printf("DAMAGED\n");
			ce->ob->health -= 10;
		}

		// Dampen
		ob->vx >>= 2;
		ob->vy >>= 2;

	}

	if(ob->time <= -15)
	{
		// Free
		ob->freeme = 1;
	}

	// TODO!

}

void obj_food_tomato_f_draw(obj_t *ob, img_t *dst, int camx, int camy)
{
	struct fd_food *fde = (struct fd_food *)ob->f.fd;
	int sx, sy;

	sx = fde->face & 3;
	sy = 0;

	if(ob->time < 0)
	{
		sy = 1;
		sx = (-ob->time)/4;
	}

	draw_img_trans_cmap_d_sd(dst, ob->img,
		camx + ob->f.cx*32 + (ob->tx>>8),
		camy + ob->f.cy*24 + (ob->ty>>8) - 4,
		sx*32, sy*32, 32, 32,
		0, ob->cmap);

}

void obj_food_tomato_f_free(obj_t *ob)
{
	//

}

//
// General object support
//

struct {
	int fdlen;
	int (*f_init)(obj_t *ob);
	int (*f_init_fd)(obj_t *ob);
	int (*f_load_fd)(obj_t *ob, FILE *fp);
	int (*f_save_fd)(obj_t *ob, FILE *fp);
	void (*f_reset)(obj_t *ob);
	void (*f_tick)(obj_t *ob);
	void (*f_draw)(obj_t *ob, img_t *dst, int camx, int camy);
	void (*f_free)(obj_t *ob);
} obj_fptrs[OBJ_COUNT] = {
	// OBJ_FREE
	{0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},

	// OBJ_PLAYER
	{
		sizeof(struct fd_player),

		obj_player_f_init,

		obj_player_f_init_fd,
		obj_player_f_load_fd,
		obj_player_f_save_fd,

		obj_player_f_reset,

		obj_player_f_tick,
		obj_player_f_draw,

		obj_player_f_free,
	},


	// OBJ_FOOD_TOMATO
	{
		sizeof(struct fd_food),

		obj_food_tomato_f_init,

		obj_food_tomato_f_init_fd,
		obj_food_tomato_f_load_fd,
		obj_food_tomato_f_save_fd,

		obj_food_tomato_f_reset,

		obj_food_tomato_f_tick,
		obj_food_tomato_f_draw,

		obj_food_tomato_f_free,
	},

};

void obj_free(obj_t *ob)
{
	cell_t *ce;

	// Detach from level
	if(ob->level != NULL && ob->level->layers[ob->f.layer] != NULL)
	{
		ce = layer_cell_ptr(ob->level->layers[ob->f.layer], ob->f.cx, ob->f.cy);
		if(ce != NULL && ce->ob == ob)
			ce->ob = NULL;

	}

	// Free internal object data if necessary
	if(ob->f_free != NULL)
		ob->f_free(ob);

	// Free object runtime / file data
	if(ob->f.fd != NULL) free(ob->f.fd);

	// Free object
	free(ob);

}

obj_t *obj_alloc(int otyp, int flags, int cx, int cy, int ox, int oy, int layer, void *fd, int fdlen)
{
	// Allocate
	obj_t *ob = malloc(sizeof(obj_t));

	// Fill in from parameters
	ob->f.cx = cx;
	ob->f.cy = cy;
	ob->f.ox = ox;
	ob->f.oy = oy;
	ob->f.otyp = otyp;
	ob->f.flags = flags;
	ob->f.layer = layer;
	ob->f.fdlen = fdlen;
	ob->f.fd = (fdlen == 0 ? NULL : malloc(fdlen));
	assert(fdlen >= 0);

	// Copy file userdata if it's there
	if(fd != NULL && fdlen > 0)
		memcpy(ob->f.fd, fd, fdlen);

	// Preset runtime info
	ob->next = NULL;
	ob->prev = NULL;
	ob->parent = NULL;

	ob->img = NULL;
	ob->cmap = NULL;

	ob->bx = 0;
	ob->by = 0;
	ob->bw = 0;
	ob->bh = 0;

	// Check things
	assert(otyp < OBJ_COUNT);
	assert(fdlen == obj_fptrs[otyp].fdlen);

	// Fill in from obj_fptrs
	ob->f_init = obj_fptrs[otyp].f_init;
	ob->f_init_fd = obj_fptrs[otyp].f_init_fd;
	ob->f_load_fd = obj_fptrs[otyp].f_load_fd;
	ob->f_save_fd = obj_fptrs[otyp].f_save_fd;
	ob->f_reset = obj_fptrs[otyp].f_reset;
	ob->f_tick = obj_fptrs[otyp].f_tick;
	ob->f_draw = obj_fptrs[otyp].f_draw;
	ob->f_free = obj_fptrs[otyp].f_free;

	// Fill in extra crap
	ob->crouch_trans = 0;
	ob->skintone = 0;
	ob->please_wait = 0;
	ob->steps_left = 0;
	ob->health = 1;
	ob->tx = 0;
	ob->ty = 0;
	ob->tmode = 0;
	ob->vx = 0;
	ob->vy = 0;
	ob->asdir = NULL;
	ob->aslen = 0;
	ob->asidx = 0;
	ob->time = 0;

	ob->level = NULL;

	ob->freeme = 0;

	// Return
	return ob;
}

obj_t *obj_new(level_t *lv, int otyp, int flags, int cx, int cy, int layer)
{
	// Allocate
	obj_t *ob = obj_alloc(otyp, flags, cx, cy, 0, 0, layer, NULL, obj_fptrs[otyp].fdlen);
	ob->level = lv;

	// Call init_fd
	if(ob->f_init_fd != NULL && !ob->f_init_fd(ob))
	{
		// Free and return NULL
		printf("obj_new: failure in f_init_fd\n");
		obj_free(ob);
		return NULL;
	}

	// Call init
	if(ob->f_init != NULL && !ob->f_init(ob))
	{
		// Free and return NULL
		printf("obj_new: failure in f_init\n");
		obj_free(ob);
		return NULL;
	}

	// Return object
	return ob;

}

obj_t *obj_load(level_t *lv, FILE *fp)
{
	// Load info
	int otyp = fgetc(fp);
	int layer = fgetc(fp);
	int flags = (int)(uint16_t)io_get2le(fp);
	int cx = (int)(int16_t)io_get2le(fp);
	int cy = (int)(int16_t)io_get2le(fp);
	int ox = (int)(int16_t)io_get2le(fp);
	int oy = (int)(int16_t)io_get2le(fp);

	// Allocate
	obj_t *ob = obj_alloc(otyp, flags, cx, cy, ox, oy, layer, NULL, obj_fptrs[otyp].fdlen);
	ob->level = lv;

	// Call load_fd
	if(ob->f_load_fd != NULL && !ob->f_load_fd(ob, fp))
	{
		// Free and return NULL
		printf("obj_load: failure in f_load_fd\n");
		obj_free(ob);
		return NULL;
	}

	// Call init
	if(ob->f_init != NULL && !ob->f_init(ob))
	{
		// Free and return NULL
		printf("obj_load: failure in f_init\n");
		obj_free(ob);
		return NULL;
	}

	// Return
	return ob;
}

int obj_save(FILE *fp, obj_t *ob)
{
	// Save info
	fputc(ob->f.otyp, fp);
	fputc(ob->f.layer, fp);
	io_put2le(ob->f.flags, fp);
	io_put2le(ob->f.cx, fp);
	io_put2le(ob->f.cy, fp);
	io_put2le(ob->f.ox, fp);
	io_put2le(ob->f.oy, fp);

	// Call save_fd
	if(ob->f_save_fd != NULL && !ob->f_save_fd(ob, fp))
	{
		// Return failure
		printf("obj_save: failure in f_save_fd\n");
		return 0;
	}

	// Return success
	return 1;
}

