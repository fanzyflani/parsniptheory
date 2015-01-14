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
	SDL_WM_SetCaption("Parsnip SERVER - OPEN SOURCE (alpha 13)", NULL);
	screen_surface = SDL_SetVideoMode(
		screen_width * screen_scale,
		screen_height * screen_scale,
		screen_bpp, 0);
	screen = img_new(screen_width, screen_height);

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

