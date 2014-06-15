/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

//
// 
//

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
	if(g->f_draw != NULL)
		g->f_draw(g, g->sx + sx, g->sy + sy);
}

