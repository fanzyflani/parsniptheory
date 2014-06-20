/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

//
// BAG
//
static void gui_bag_f_draw(widget_t *g, int sx, int sy)
{
	gui_draw_children(g, sx, sy);
}

static void gui_bag_f_free(widget_t *g)
{
	widget_t *sg, *sg2;

	for(sg = g->fchild; sg != NULL; sg = sg2)
	{
		sg2 = sg->nsib;
		gui_free(sg);
	}
}

int gui_bag_init(widget_t *g, void *ud)
{
	g->f_draw = gui_bag_f_draw;
	g->f_free = gui_bag_f_free;

	return 1;
}

//
// GENERAL SUPPORT
//

void gui_reparent(widget_t *gp, widget_t *gc)
{
	// Check if relationship already established
	if(gc->parent == gp) return;

	// Detach child from pre-existing parent
	if(gc->psib != NULL) { gc->psib->nsib = gc->nsib; }
	if(gc->nsib != NULL) { gc->nsib->psib = gc->psib; }
	if(gc->parent != NULL) {
		if(gc->parent->lchild == gc) { gc->parent->lchild = gc->psib; }
		if(gc->parent->fchild == gc) { gc->parent->fchild = gc->nsib; }
	}

	gc->psib = gc->nsib = NULL;
	gc->parent = NULL;

	// Attach child to new parent
	assert(gp->lchild == NULL || gp->lchild->nsib == NULL);
	assert((gp->lchild == NULL ? gp->fchild == NULL : gp->fchild != NULL));
	if(gp->lchild != NULL) { gp->lchild->nsib = gc; gc->psib = gp->lchild; }
	if(gp->fchild == NULL) { gp->fchild = gc; }
	gc->parent = gp;
	gp->lchild = gc;

	assert((gp->lchild == NULL ? gp->fchild == NULL : gp->fchild != NULL));

}

void gui_free(widget_t *g)
{
	// Call the widget's "free" function (if it exists)
	if(g->f_free != NULL)
		g->f_free(g);

	// Deparent
	gui_reparent(NULL, g);
	while(g->fchild != NULL)
		gui_reparent(NULL, g->fchild);

	// Free
	free(g);

}

widget_t *gui_new(int (*f_init)(widget_t *g, void *ud), widget_t *parent, int w, int h, void *ud)
{
	// Allocate
	widget_t *g = malloc(sizeof(widget_t));

	// Set defaults
	g->parent = NULL;
	g->fchild = g->lchild = NULL;
	g->psib = g->nsib = NULL;

	g->sx = 0; g->sy = 0;
	g->w = w; g->h = h;

	g->f_free = NULL;
	g->f_draw = NULL;
	g->f_mouse_b = NULL;
	g->f_mouse_m = NULL;
	g->f_mouse_f = NULL;

	g->i1 = 0;

	g->v1 = NULL;

	// Set parent
	gui_reparent(parent, g);

	// Call init
	if(!f_init(g, ud))
	{
		// OK, destroy this and return NULL
		free(g);
		return NULL;

	} else {
		// Return our widget
		return g;

	}
}

void gui_draw(widget_t *g, int sx, int sy)
{
	if(g != NULL && g->f_draw != NULL)
		g->f_draw(g, sx, sy);
}

void gui_draw_children(widget_t *g, int sx, int sy)
{
	if(g == NULL) return;

	for(g = g->fchild; g != NULL; g = g->nsib)
		gui_draw(g, g->sx + sx, g->sy + sy);
}

int gui_mouse_b(widget_t *g, int mx, int my, int mb, int db, int ds)
{
	widget_t *sg;

	for(sg = g->fchild; sg != NULL; sg = sg->nsib)
	if(mx >= sg->sx && mx < sg->sx + sg->w)
	if(my >= sg->sy && my < sg->sy + sg->h)
	if(gui_mouse_b(sg, mx - sg->sx, my - sg->sy, mb, db, ds))
		return 1;

	if(g->f_mouse_b != NULL)
		return g->f_mouse_b(g, mx, my, mb, db, ds);

	return 0;
}

int gui_mouse_m(widget_t *g, int mx, int my, int mb, int dx, int dy)
{
	widget_t *sg;

	for(sg = g->fchild; sg != NULL; sg = sg->nsib)
	if(mx >= sg->sx && mx < sg->sx + sg->w)
	if(my >= sg->sy && my < sg->sy + sg->h)
	if(gui_mouse_m(sg, mx - sg->sx, my - sg->sy, mb, dx, dy))
		return 1;

	if(g->f_mouse_m != NULL)
		return g->f_mouse_m(g, mx, my, mb, dx, dy);

	return 0;
}

void gui_mouse_auto(widget_t *g, int sx, int sy)
{
	int i;

	// Mouse coordinates
	int mx = mouse_x - sx;
	int my = mouse_y - sy;
	int dx = mouse_x - mouse_ox;
	int dy = mouse_y - mouse_oy;

	// Mouse motion
	gui_mouse_m(g, mx, my, mouse_ob, dx, dy);

	// Mouse change states
	for(i = 0; i < 32; i++)
	{
		if(((mouse_b & ~mouse_ob)>>i) & 1)
			gui_mouse_b(g, mx, my, mouse_b, i, 1);
		else if(((mouse_ob & ~mouse_b)>>i) & 1)
			gui_mouse_b(g, mx, my, mouse_b, i, 0);

	}

}

