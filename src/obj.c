/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

//
// OBJ_PLAYER
//

int obj_player_f_init(obj_t *ob)
{
	struct fd_player *fde = (struct fd_player *)ob->f.fd;

	assert(fde->team >= 0 && fde->team < TEAM_MAX);
	ob->img = i_player;
	ob->cmap = teams[fde->team]->cm_player;

	ob->bx =   6;
	ob->by = -16;
	ob->bw =  20;
	ob->bh =  36;

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

	// TEST:
	// Get coordinates
	int asendx = (mouse_x + game_camx)/32;
	int asendy = (mouse_y + game_camy)/24;

	// Check if walking
	if(ob->f.ox == 0 && ob->f.oy == 0) do
	{
		// Perform A* towards target
		if(asendx != ob->tx && asendy != ob->ty)
		{
			// Do A* trace
			int dirlist[1024];
			int dirlen = astar_layer(rootlv->layers[0], dirlist, 1024,
				ob->f.cx, ob->f.cy, asendx, asendy);

			// If it works, produce a new list
			if(dirlen >= 1)
			{
				// Allocate + copy
				ob->v1 = realloc(ob->v1, sizeof(int) * dirlen);
				memcpy(ob->v1, dirlist, sizeof(int) * dirlen);

				// Set things
				ob->i1 = dirlen;
				ob->i2 = 0;
				ob->tx = asendx;
				ob->ty = asendy;
				
			}
		}

		// If it works, follow
		if(ob->v1 != NULL && ob->i2 < ob->i1) do
		{
			// Get direction
			int dir = ((int *)ob->v1)[ob->i2];
			dx = face_dir[dir][0];
			dy = face_dir[dir][1];

			// Get neighbour cell
			cx = ob->f.cx + dx;
			cy = ob->f.cy + dy;

			// Check cell
			cell_t *ce = layer_cell_ptr(rootlv->layers[ob->f.layer], cx, cy);
			if(ce == NULL) continue;
			if(ce->f.ctyp != CELL_FLOOR) continue;

			// Move
			ob->i2++;
			ob->f.cx += dx;
			ob->f.cy += dy;
			ob->f.ox -= 32*dx;
			ob->f.oy -= 24*dy;
			fde->face = dir;

			break;
		} while(0);

	} while(0); else {
		// Move
		if(ob->f.ox < 0) ob->f.ox += 3;
		if(ob->f.ox > 0) ob->f.ox -= 1;
		if(ob->f.ox > 0) ob->f.ox -= 1;
		if(ob->f.ox > 0) ob->f.ox -= 1;

		if(ob->f.oy < 0) ob->f.oy += 2;
		if(ob->f.oy > 0) ob->f.oy -= 1;
		if(ob->f.oy > 0) ob->f.oy -= 1;

	}
}

void obj_player_f_draw(obj_t *ob, img_t *dst, int camx, int camy)
{
	int i;
	struct fd_player *fde = (struct fd_player *)ob->f.fd;

	// Draw parts
	for(i = 6; i >= 0; i--)
		draw_img_trans_cmap_d_sd(dst, ob->img,
			camx + ob->f.cx*32 + ob->f.ox,
			camy + ob->f.cy*24 + ob->f.oy - 21,
			(fde->face&3)*32, i*48, 32, 48,
			0, ob->cmap);

}

void obj_player_f_free(obj_t *ob)
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

};

void obj_free(obj_t *ob)
{
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
	ob->tx = 0;
	ob->ty = 0;
	ob->v1 = NULL;
	ob->i1 = 0;
	ob->i2 = 0;

	// Return
	return ob;
}

obj_t *obj_new(int otyp, int flags, int cx, int cy, int layer)
{
	// Allocate
	obj_t *ob = obj_alloc(otyp, flags, cx, cy, 0, 0, layer, NULL, obj_fptrs[otyp].fdlen);

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

obj_t *obj_load(FILE *fp)
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

