/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

int main(int argc, char *argv[])
{
	int i;

	// chdir
	chdir_to_exe(argv[0]);

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
	SDL_WM_SetCaption("Parsnip SERVER - SHAREWARE (alpha 10)", NULL);
	screen_surface = SDL_SetVideoMode(320 * screen_scale, 200 * screen_scale, screen_bpp, 0);
	screen = img_new(320, 200);

	// Load palette and colourmaps
	load_palette("dat/pal1.pal");
	pal_main[0][0] = 255/5;

	// Load images
	if(!load_graphics())
		return 0;

	// Prepare teams
	for(i = 0; i < TEAM_MAX; i++)
		teams[i] = team_new(i);

	// Enter network setup loop
	//netloop(NET_SERVER);
	IPaddress server_ip;
	TCPsocket server_sockfd;
	if(SDLNet_ResolveHost(&server_ip, NULL, NET_PORT)==-1) return 1;
	server_sockfd = SDLNet_TCP_Open(&server_ip);
	if(server_sockfd == NULL) return 1;
	gameloop(NET_SERVER, server_sockfd);

	return 1;
}

