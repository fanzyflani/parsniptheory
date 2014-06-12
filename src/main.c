/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

char *net_connect_host = NULL;

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

int menuloop(int menuid)
{
	int i;

	// Ensure our clicks don't cut through twice
	input_poll();

	for(;;)
	{
		// Clear the screen
		screen_clear(0);

		// Work out what's selected
		int sel = -1;

		if(mouse_x >= 10 && mouse_x < screen->w/2 - 10)
		{
			if(mouse_y >= 10 && mouse_y < screen->h/2 - 10)
			{
				sel = 0;

			} else if(mouse_y >= screen->h/2 + 10 && mouse_y < screen->h - 10) {
				sel = 2;

			}

		} else if(mouse_x >= screen->w/2 + 10 && mouse_x < screen->w - 10) {
			if(mouse_y >= 10 && mouse_y < screen->h/2 - 10)
			{
				sel = 1;

			} else if(mouse_y >= screen->h/2 + 10 && mouse_y < screen->h - 10) {
				sel = 3;

			}


		}

		// Draw rectangles
		draw_rect_d(screen, 10, 10, screen->w/2-20, screen->h/2-20, 64 + (sel==0 ? 0 : 2) + 8*0);
		draw_rect_d(screen, screen->w/2 + 10, 10, screen->w/2-20, screen->h/2-20, 64 + (sel==1 ? 0 : 2) + 8*1);
		draw_rect_d(screen, 10, screen->h/2 + 10, screen->w/2-20, screen->h/2-20, 64 + (sel==2 ? 0 : 2) + 8*2);
		draw_rect_d(screen, screen->w/2 + 10, screen->h/2 + 10, screen->w/2-20, screen->h/2-20, 64 + (sel==3 ? 0 : 2) + 8*3);

		if(menuid == 0)
		{
			// Draw backgrounds

			draw_img_trans_cmap_d_sd(screen, i_tiles1,
				screen->w/2 - 10 - 10 - 36*3, screen->h - 10 - 10 - 24,
				32*1, 24*0, 32, 24, 0, cm_tiles1);
			draw_img_trans_cmap_d_sd(screen, i_tiles1,
				screen->w/2 - 10 - 10 - 36*2, screen->h - 10 - 10 - 24,
				32*5, 24*0, 32, 24, 0, cm_tiles1);
			draw_img_trans_cmap_d_sd(screen, i_tiles1,
				screen->w/2 - 10 - 10 - 36*1, screen->h - 10 - 10 - 24,
				32*12, 24*0, 32, 24, 0, cm_tiles1);

			// Draw texts
			draw_printf(screen, i_font16, 16, 15, 20, 1, "HOT SEAT");
			draw_printf(screen, i_font16, 16, 15 + screen->w/2, 20, 1, "NETWORK");
			draw_printf(screen, i_font16, 16, 15, 20 + screen->h/2, 1, "LVL EDIT");
			draw_printf(screen, i_font16, 16, 15 + screen->w/2, 20 + screen->h/2, 1, "QUIT");
			draw_printf(screen, i_font16, 16, screen->w/2 - 8*(7+1+6+1), screen->h/2 - 8, 1,
				"PARSNIP THEORY");
		
		} else if(menuid == 1) {
			// Draw backgrounds
			for(i = 0; i < 2; i++)
				menu_draw_player(screen->w/2 - 10 - 32 - 28*1 + 28*i, screen->h/2 - 10 - 44, i, 0);
			for(i = 0; i < 3; i++)
				menu_draw_player(screen->w - 10 - 32 - 28*2 + 28*i, screen->h/2 - 10 - 44, i, 0);
			for(i = 0; i < 4; i++)
				menu_draw_player(screen->w/2 - 10 - 32 - 28*3 + 28*i, screen->h - 10 - 44, i, 0);

			// Draw texts
			draw_printf(screen, i_font16, 16, 15, 20, 1, "2 PLAYER");
			draw_printf(screen, i_font16, 16, 15 + screen->w/2, 20, 1, "3 PLAYER");
			draw_printf(screen, i_font16, 16, 15, 20 + screen->h/2, 1, "4 PLAYER");
			draw_printf(screen, i_font16, 16, 15 + screen->w/2, 20 + screen->h/2, 1, "GO BACK");
			draw_printf(screen, i_font16, 16, screen->w/2 - 8*(3+1+4-1), screen->h/2 - 8, 1,
				"HOT SEAT");

		} else if(menuid == 2) {
			// Draw backgrounds

			// Draw texts
			draw_printf(screen, i_font16, 16, 15, 20, 1, "CONNECT");
			draw_printf(screen, i_font16, 16, 15 + screen->w/2, 20, 1, "CREATE");
			draw_printf(screen, i_font16, 16, 15, 20 + screen->h/2, 1, "SETUP");
			draw_printf(screen, i_font16, 16, 15 + screen->w/2, 20 + screen->h/2, 1, "GO BACK");
			draw_printf(screen, i_font16, 16, screen->w/2 - 8*(7+1+4+3), screen->h/2 - 8, 1,
				"NETWORK GAME");

		}

		// Flip
		screen_flip();
		SDL_Delay(20);

		// Check if clicked
		if((mouse_ob & 1) && !(mouse_b & 1))
		switch(sel)
		{
			case 0:
				if(menuid == 0) return menuloop(1);
				if(menuid == 1) return 2;
				if(menuid == 2) return 0x10C;
				break;
			case 1:
				if(menuid == 0) return menuloop(2);
				if(menuid == 1) return 3;
				break;
			case 2:
				if(menuid == 0) return 0xED17;
				if(menuid == 1) return 4;
				break;
			case 3:
				if(menuid == 0) return -1;
				if(menuid == 1) return 0;
				if(menuid == 2) return 0;
				break;

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

	// General SDL setup
	SDL_Init(SDL_INIT_VIDEO);
	SDLNet_Init();
#ifndef WIN32
	signal(SIGINT,  SIG_DFL);
	signal(SIGTERM, SIG_DFL);
#endif
	SDL_EnableUNICODE(1);

	// Set up basic video mode
	// TODO: Video mode selector
	SDL_WM_SetCaption("Parsnip Theory - SHAREWARE (alpha 2)", NULL);
	loadicon("dat/icon.tga");
	screen_surface = SDL_SetVideoMode(320 * screen_scale, 200 * screen_scale, screen_bpp, 0);
	screen = img_new(320, 200);

	// Load palette and colourmaps
	load_palette("dat/pal1.pal");
	pal_main[0][0] = 255/5;

	// Load images
	// TODO: png support (and hence the dat/ directory)
	i_player = img_load_tga("tga/player.tga"); 
	i_tiles1 = img_load_tga("tga/tiles1.tga"); 
	i_food1 = img_load_tga("tga/food1.tga"); 
	i_icons1 = img_load_tga("tga/icons1.tga"); 
	i_font16 = img_load_tga("tga/font16.tga"); 
	i_fontnum1 = img_load_tga("tga/fontnum1.tga"); 
	cm_player = cmaps[i_player->cmidx].data;
	cm_tiles1 = cmaps[i_tiles1->cmidx].data;
	cm_food1 = cmaps[i_food1->cmidx].data;

	// Prepare teams
	for(i = 0; i < TEAM_MAX; i++)
		teams[i] = team_new(i);

	// Enter menu loop
	for(;;)
	switch(menuloop(0))
	{
		case 2:
			gameloop("dat/level.psl", NET_LOCAL, 2, NULL);
			break;

		case 3:
			gameloop("dat/level.psl", NET_LOCAL, 3, NULL);
			break;

		case 4:
			gameloop("dat/level.psl", NET_LOCAL, 4, NULL);
			break;

		case 0x105: {
			IPaddress server_ip;
			TCPsocket server_sockfd;
			if(SDLNet_ResolveHost(&server_ip, NULL, NET_PORT)==-1) return 1;
			server_sockfd = SDLNet_TCP_Open(&server_ip);
			if(server_sockfd == NULL) return 1;
			gameloop("dat/level.psl", NET_SERVER, 2, server_sockfd);
		} break;

		case 0x10C: {
			net_connect_host = text_dialogue("ENTER HOSTNAME OR IP", "");
			if(net_connect_host == NULL)
				break;

			IPaddress client_ip;
			TCPsocket client_sockfd;
			if(SDLNet_ResolveHost(&client_ip, (net_connect_host == NULL
				? "localhost"
				: net_connect_host), NET_PORT)==-1) return 1;
			client_sockfd = SDLNet_TCP_Open(&client_ip);
			if(client_sockfd == NULL) return 1;

			free(net_connect_host);
			net_connect_host = NULL;

			gameloop("dat/level.psl", NET_CLIENT, 2, client_sockfd);
		} break;

		case 0xED17:
			editloop();
			break;

		case -1:
			// Clean up
			
			// That's all folks!
			return 0;
	}
}

