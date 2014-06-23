/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

int mouse_x = 160;
int mouse_y = 100;
int mouse_b = 0;
int mouse_ox = 160;
int mouse_oy = 100;
int mouse_ob = 0;

// We are not a key charity. (NO PUN THERE)
// Also, you only get 31 chars.
// If it overflows, tough.
#define KEY_QUEUE_SIZE 64

// is_released:1, key_sym:15, key_unicode:16
uint8_t key_state[SDLK_LAST];
uint32_t key_queue[KEY_QUEUE_SIZE];
int key_queue_head = 0;
int key_queue_tail = 0;

void input_key_queue_flush(void)
{
	// Clear the list
	key_queue_tail = key_queue_head;

	// Also clear the key_state array
	memset(key_state, 0, sizeof(key_state));

	// ALT: Turn everything into an unpress
	//int i;
	//for(i = key_queue_head; i != key_queue_tail; i = (i+1) % KEY_QUEUE_SIZE)
	//	key_queue[i] |= 0x80000000;

}

void input_key_queue_push(uint32_t key)
{
	// SANITY CHECK: 0 <= code < SDLK_LAST
	// (otherwise force code to 0)
	if(((key>>16) & 0x7FFF) >= SDLK_LAST) key &= 0x8000FFFF;

	// Add to queue
	key_queue[key_queue_head] = key;

	// Advance head
	key_queue_head = (key_queue_head + 1) % KEY_QUEUE_SIZE;

	// Advance tail if need be (that is, drop a key off the end)
	if(key_queue_tail == key_queue_head)
		key_queue_tail = (key_queue_tail + 1) % KEY_QUEUE_SIZE;

}

uint32_t input_key_queue_peek(void)
{
	// Check if empty
	if(key_queue_head == key_queue_tail)
		return 0;

	// Return key
	return key_queue[key_queue_tail];

}

uint32_t input_key_queue_pop(void)
{
	// Get key
	uint32_t ret = input_key_queue_peek();

	// If empty, return
	if(ret == 0) return 0;

	// Advance tail
	key_queue_tail = (key_queue_tail + 1) % KEY_QUEUE_SIZE;

	// Return
	return ret;

}

int input_poll(void)
{
	SDL_Event ev;
	int ret = 0;

	mouse_ox = mouse_x;
	mouse_oy = mouse_y;
	mouse_ob = mouse_b;

	while(SDL_PollEvent(&ev))
	switch(ev.type)
	{
		case SDL_QUIT:
			ret = 1;
			break;

		case SDL_KEYDOWN:
			if(ev.key.keysym.sym < SDLK_LAST)
				key_state[ev.key.keysym.sym] = 1;

			input_key_queue_push(0
				|(0<<31)
				|(((uint32_t)(ev.key.keysym.sym&0x7FFF))<<16)
				|(uint32_t)(ev.key.keysym.unicode&0xFFFF));

			break;

		case SDL_KEYUP:
			if(ev.key.keysym.sym < SDLK_LAST)
				key_state[ev.key.keysym.sym] = 0;

			input_key_queue_push(0
				|(1<<31)
				|(((uint32_t)(ev.key.keysym.sym&0x7FFF))<<16)
				|(uint32_t)(ev.key.keysym.unicode&0xFFFF));

			break;

		case SDL_MOUSEBUTTONDOWN:
			mouse_b |= (1<<(ev.button.button-1));
			break;

		case SDL_MOUSEBUTTONUP:
			mouse_b &= ~(1<<(ev.button.button-1));
			break;

		case SDL_ACTIVEEVENT:
			if(ev.active.gain == 0 && ev.active.state == SDL_APPINPUTFOCUS)
			{
				mouse_x = 160;
				mouse_y = 100;
			}
			break;

		case SDL_MOUSEMOTION:
			if(ev.motion.x < screen_ofx) break;
			if(ev.motion.y < screen_ofy) break;
			if(ev.motion.x >= screen_ofx + 320*screen_scale) break;
			if(ev.motion.y >= screen_ofy + 200*screen_scale) break;

			mouse_x = (ev.motion.x - screen_ofx) / screen_scale;
			mouse_y = (ev.motion.y - screen_ofx) / screen_scale;

			break;

	}

	return ret;
}

