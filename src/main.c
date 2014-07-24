/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

#define MAIN_MENU_DUMMY    0
#define MAIN_MENU_ACTION   1
#define MAIN_MENU_GOBACK   2
#define MAIN_MENU_MOVETO   3

char *net_connect_host = NULL;
widget_t *menu_g_main = NULL;
widget_t *menu_g_game = NULL;

int menu_click_catcher = -1;

static void menu_draw_player(int x, int y, int team, int face)
{
	int i;

	for(i = 6; i >= 0; i--)
		draw_img_trans_cmap_d_sd(screen, i_player,
			x,
			y,
			face*32, i*48, 32, 48,
			0, teams[team]->cm_player);
}

static void menu_draw_tile(int x, int y, int tidx, int a2)
{
	draw_img_trans_cmap_d_sd(screen, i_tiles1,
		x, y,
		32*(tidx&15), 24*(tidx>>4), 32, 24, 0, cm_tiles1);

}

struct menu_data
{
	const char *name;
	int act, arg;
	struct menu_pic {
		void (*f_draw)(int x, int y, int a1, int a2);
		int x, y, a1, a2;

	} pics[4];

} menu_gui_data[][4] = {

	// Main menu
	{
		{
			"PLAY", MAIN_MENU_MOVETO, 2, {
				{menu_draw_player, -32-28*2-30, -31-18, 1, 0},
				{menu_draw_player, -32-28*2+30, -31-18, 6, 0},
				{menu_draw_player, -32-28*2+0, -31-18, 0, 0},
				{NULL, 0, 0, 0, 0},
			}
		},

		{
			"SETUP", MAIN_MENU_ACTION, 0x5E7, {
				{menu_draw_player, -32-28*2+0, -37-18, 5, 0},
				{menu_draw_tile, -32-28*2-32, -31, 7, 0},
				{menu_draw_tile, -32-28*2+0,  -31, 8, 0},
				{menu_draw_tile, -32-28*2+32, -31, 9, 0},
			}
		},

		{
			"LVL EDIT", MAIN_MENU_ACTION, 0xED17, {
				{menu_draw_tile, -10 - 36*3, -34, 1, 0},
				{menu_draw_tile, -10 - 36*2, -34, 5, 0},
				{menu_draw_tile, -10 - 36*1, -34, 12, 0},
				{NULL, 0, 0, 0, 0},
			}
		},

		{
			"QUIT", MAIN_MENU_ACTION, -1, {
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
			}
		},
	},

	// Hotseat menu (not used)
	{
		{
			"2 PLAYER", MAIN_MENU_ACTION, 2, {
				{menu_draw_player, -32-28*1, -44, 0, 0},
				{menu_draw_player, -32-28*0, -44, 1, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
			}
		},

		{
			"3 PLAYER", MAIN_MENU_ACTION, 3, {
				{menu_draw_player, -32-28*2, -44, 0, 0},
				{menu_draw_player, -32-28*1, -44, 1, 0},
				{menu_draw_player, -32-28*0, -44, 2, 0},
				{NULL, 0, 0, 0, 0},
			}
		},

		{
			"4 PLAYER", MAIN_MENU_ACTION, 4, {
				{menu_draw_player, -32-28*3, -44, 0, 0},
				{menu_draw_player, -32-28*2, -44, 1, 0},
				{menu_draw_player, -32-28*1, -44, 2, 0},
				{menu_draw_player, -32-28*0, -44, 3, 0},
			}
		},

		{
			"GO BACK", MAIN_MENU_GOBACK, 0, {
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
			}
		},
	},

	// Play menu
	{
		/*
		{
			"SERVERS", MAIN_MENU_ACTION, 0x101, {
				{menu_draw_tile, -32-28*2, -31, 1, 0},
				{menu_draw_player, -32-28*2, -31-21, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
			}
		},*/
		{
			"HOT SEAT", MAIN_MENU_ACTION, 2, {
				{menu_draw_tile, -32-28*2, -31, 1, 0},
				{menu_draw_player, -32-28*2-10, -31-23, 1, 0},
				{menu_draw_player, -32-28*2+10, -31-23, 2, 0},
				{menu_draw_player, -32-28*2+0, -31-18, 0, 0},
			}
		},

		{
			"CONNECT", MAIN_MENU_ACTION, 0x10C, {
				{menu_draw_player, 20, -44, 0, DIR_EAST},
				{menu_draw_player, -20-32, -44, 1, DIR_WEST},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
			}
		},

		{
			"CREATE", MAIN_MENU_ACTION, 0x105, {
				{menu_draw_tile, -32-28*1-16, -28, 1, 0},
				{menu_draw_tile, -32-28*1+16, -28, 1, 0},
				{menu_draw_tile, -32-28*1-16, -52, 5, 0},
				{menu_draw_tile, -32-28*1+16, -52, 5, 0},
			}
		},

		{
			"GO BACK", MAIN_MENU_GOBACK, 0, {
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
			}
		},
	},

	{
		{
			"", MAIN_MENU_DUMMY, 0, {
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
			}
		},

		{
			"", MAIN_MENU_DUMMY, 0, {
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
			}
		},

		{
			"", MAIN_MENU_DUMMY, 0, {
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
			}
		},

		{
			"", MAIN_MENU_DUMMY, 0, {
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
				{NULL, 0, 0, 0, 0},
			}
		},
	},
};

const char *menu_names[] = {
	"PARSNIP THEORY ",
	" HOT SEAT",
	"PLAY GAME",
};

static void menu_widget_f_free(widget_t *g)
{

}

static int menu_widget_f_mouse_b(widget_t *g, int mx, int my, int mb, int db, int ds)
{
	if(ds == 0 && db == 0)
		menu_click_catcher = g->i1;
	
	return 1;
}

static void menu_widget_f_draw(widget_t *g, int sx, int sy)
{
	int x, y, j;
	int mx = mouse_x - sx;
	int my = mouse_y - sy;
	int sel = (mx >= 0 && my >= 0 && mx < g->w && my < g->h);

	// Draw rectangle
	if(sel)
		draw_rect_d(screen, sx, sy, g->w, g->h, 64 + (sel ? 0 : 1) + 4*g->i1);
	else
		draw_border_d(screen, sx, sy, g->w, g->h, 64 + (sel ? 0 : 1) + 4*g->i1);

	// Draw GUI contents
	struct menu_data *md = (struct menu_data *)(g->v1);

	for(j = 0; j < 4; j++)
	{
		if(md->pics[j].f_draw == NULL) continue;

		x = md->pics[j].x;
		y = md->pics[j].y;
		if(x < 0) x += g->w;
		if(y < 0) y += g->h;

		md->pics[j].f_draw(sx + x, sy + y, md->pics[j].a1, md->pics[j].a2);
	}

	// Draw text
	draw_printf(screen, i_font16, 16, 1, sx + 5, sy + 10, 1, md->name);

}

static int menu_widget_f_init(widget_t *g, void *ud)
{
	g->v1 = ud;

	g->f_draw = menu_widget_f_draw;
	g->f_free = menu_widget_f_free;
	g->f_mouse_b = menu_widget_f_mouse_b;

	return 1;
}

widget_t *menu_gen_widget(struct menu_data *mdat, const char *name)
{
	int i;

	widget_t *groot = gui_new(gui_bag_init, NULL, screen->w, screen->h, NULL);
	widget_t *g;

	// Text
	g = gui_new(gui_label_init, groot, 16*strlen(name), 16, (void *)name);
	g->sx = groot->w/2 - 8*strlen(name);
	g->sy = groot->h/2 - 8;

	g = gui_new(gui_label57_init, groot, groot->w, 7, "Alpha 11 - SHAREWARE - Spread to all your friends!");
	g->sx = 0;
	g->sy = 0;

	g = gui_new(gui_label57_init, groot, groot->w, 7, "Copyright (C) 2014, fanzyflani. All rights reserved. https://fanzyflani.itch.io");
	g->sx = 0;
	g->sy = groot->h-7;

	// Widgets
	for(i = 0; i < 4; i++)
	{
		g = gui_new(menu_widget_f_init, groot, screen->w/2-20, screen->h/2-20, (void *)(mdat+i));
		g->sx = 10+((i&1) ? screen->w/2 : 0);
		g->sy = 10+((i&2) ? screen->h/2 : 0);
		g->i1 = i;
	}

	return groot;
}

int menuloop(int menuid)
{
	widget_t *g_menu = NULL;

	// Init widgets if need be
	if(menu_g_main == NULL) menu_g_main = menu_gen_widget(menu_gui_data[0], menu_names[0]);
	if(menu_g_game == NULL) menu_g_game = menu_gen_widget(menu_gui_data[2], menu_names[2]);
	menu_click_catcher = -1;

	// Ensure our clicks don't cut through twice
	input_poll();

	// Clean up past games if we still have any
	if(menuid == 0)
	{
		if(game_v != NULL) { game_free(game_v); game_v = NULL; }
		if(game_m != NULL) { game_free(game_m); game_m = NULL; }
	}

	for(;;)
	{
		// Clear the screen
		screen_plasma();

		// Draw title card
		/**/ if(menuid == 0) draw_img_trans_cmap_d_sd(screen, i_title[1],
			0, 0, 160, 200, 320, 200, 0, cmaps[i_title[1]->cmidx].data);
		else if(menuid == 2) draw_img_trans_cmap_d_sd(screen, i_title[0],
			0, 0, 0, 0, 320, 200, 0, cmaps[i_title[0]->cmidx].data);


		// Pick menu
		/**/ if(menuid == 0) g_menu = menu_g_main;
		else if(menuid == 2) g_menu = menu_g_game;

		// Draw widget
		gui_draw(g_menu, g_menu->sx, g_menu->sy);

		// TEST: Draw some of a waveform
#if 0
		for(i = 0; i < screen->w; i++)
		{
			int v;
			v = snd_sackit->ldata[i * (snd_sackit->len) / screen->w];
			v = (v+0x8000)*screen->h/0x10000;
			*IMG8(screen, i, v) = 1;
			v = snd_sackit->ldata[i * (2048) / screen->w];
			v = (v+0x8000)*screen->h/0x10000;
			*IMG8(screen, i, v) = 0;
		}
#endif

		// Flip
		screen_flip();
		SDL_Delay(20);

		// Catch clicks
		gui_mouse_auto(g_menu, g_menu->sx, g_menu->sy);

		// Check if clicked
		if(menu_click_catcher != -1)
		switch(menu_gui_data[menuid][menu_click_catcher].act)
		{
			case MAIN_MENU_MOVETO:
				return menuloop(menu_gui_data[menuid][menu_click_catcher].arg);
			case MAIN_MENU_GOBACK:
				return 0;
			case MAIN_MENU_ACTION:
				return menu_gui_data[menuid][menu_click_catcher].arg;
		}

		// Poll input
		if(input_poll()) return -1;

	}

}

void loadicon(const char *fname)
{
	int x, y;
	Uint8 mask[32*4];

	SDL_Surface *s = SDL_LoadBMP("dat/icon.bmp");

	SDL_LockSurface(s);

	memset(mask, 0, 32*4);

	for(y = 0; y < 32; y++)
	for(x = 0; x < 32; x++)
		if(((uint8_t *)(s->pixels))[x + y*s->pitch] != 5)
			mask[y*4 + (x>>3)] |= 128>>(x&7);

	SDL_UnlockSurface(s);

	//SDL_SetColorKey(s, SDL_SRCCOLORKEY, 5+8);

	SDL_WM_SetIcon(s, mask);
}

int main(int argc, char *argv[])
{
	int i;

	// chdir
	chdir_to_exe(argv[0]);

	// General SDL setup
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
	SDLNet_Init();
#ifndef WIN32
	signal(SIGINT,  SIG_DFL);
	signal(SIGTERM, SIG_DFL);
#endif
	SDL_EnableUNICODE(1);

	// Set up basic video mode
	SDL_WM_SetCaption("Parsnip Theory - SHAREWARE (alpha 11)", NULL);
	loadicon("dat/icon.tga");
	screen_surface = SDL_SetVideoMode(320 * screen_scale, 200 * screen_scale, screen_bpp, SDL_SWSURFACE);
	printf("screen %p %i %i\n", screen_surface, screen_surface->w, screen_surface->h);
	screen = img_new(320, 200);

	// Load palette and colourmaps
	load_palette("dat/pal1.pal");

	// Load images
	if(!load_graphics())
		return 1;
	// Prepare teams
	for(i = 0; i < TEAM_MAX; i++)
		teams[i] = team_new(i);

	// Initialise Lua
	if(!lint_init())
		return 1;

#ifdef __EMSCRIPTEN__
	// Load sound
#ifndef NO_AUDIO
	if(!audio_init())
		return 1;
#endif
	
	// Play music
	music_play(mod_trk1);

	// Play sound
	snd_play_splat(0, 0, 0);

	// Force a hotseat game
	printf("forcing hotseat game\n");
	switch(2)
#else
	// Do graphics setup
	if(!screen_setup())
		return 1;

	// Load sound
#ifndef NO_AUDIO
	if(!audio_init())
		return 1;
#endif
	

	// Do title
#ifndef NO_AUDIO
#ifndef NO_MUSIC
	if(titleloop())
		return 1;
#endif
#endif

	// Play music
	//music_play(NULL);
	music_play(mod_trk1);

	// Play sound
	snd_play_splat(0, 0, 0);

	// Enter menu loop
	for(;;)
	switch(menuloop(0))
#endif
	{
		case 2:
			gameloop(NET_LOCAL, NULL);
			break;

		case 0x5E7:
			switch(options_dialogue("ENABLE MUSIC?", "YES", "NO"))
			{
				case 0:
					music_play(mod_trk1);
					break;

				case 1:
					music_play(NULL);
					break;

			}

			break;

		case 0x101: {
			errorloop("Not implemented yet!");
		} break;

		case 0x105: {
			IPaddress server_ip;
			TCPsocket server_sockfd;
			if(SDLNet_ResolveHost(&server_ip, NULL, NET_PORT)==-1)
			{
				errorloop("Host lookup failed");
				break;
			}

			server_sockfd = SDLNet_TCP_Open(&server_ip);
			if(server_sockfd == NULL)
			{
				errorloop("Could not open port");
				break;
			}

			gameloop(NET_SERVER, server_sockfd);
		} break;

		case 0x10C: {
			char *host = text_dialogue("ENTER HOSTNAME OR IP", net_connect_host);
			if(host == NULL)
				break;
			if(net_connect_host != NULL)
				free(net_connect_host);
			net_connect_host = host;

			IPaddress client_ip;
			TCPsocket client_sockfd;
			if(SDLNet_ResolveHost(&client_ip, (net_connect_host == NULL
				? "localhost"
				: net_connect_host), NET_PORT)==-1)
			{
				errorloop("Host lookup failed");
				break;
			}

			client_sockfd = SDLNet_TCP_Open(&client_ip);
			//if(client_sockfd == NULL) return 1;
			if(client_sockfd == NULL)
			{
				errorloop("Could not connect");
				break;
			}

			//free(net_connect_host); net_connect_host = NULL;

			gameloop(NET_CLIENT, client_sockfd);
		} break;

		case 0xED17:
			editloop();
			break;

		case -1:
			// Clean up
			
			// That's all folks!
			return 0;
	}

#ifdef __EMSCRIPTEN__
	printf("woop woop, now we transfer\n");
#endif
}

