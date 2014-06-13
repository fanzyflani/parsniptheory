/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

abuf_t *ab_local;
abuf_t *ab_teams[TEAM_MAX];

//
// ALLOCATIONS
//
void abuf_free(abuf_t *ab)
{
	// TODO: Close socket!
	free(ab);
}

abuf_t *abuf_new(void)
{
	// Allocate
	abuf_t *ab = malloc(sizeof(abuf_t));

	// Fill in
	ab->sock = NULL;
	ab->sset = NULL;
	ab->loc_chain = NULL;
	ab->f_cont = NULL;
	ab->rsize = 0;
	ab->wsize = 0;
	ab->state = STATE_GIVEVER;

	// Return
	return ab;
}

//
// QUERIES
//
int abuf_get_rsize(abuf_t *ab)
{
	return ab->rsize;
}

int abuf_get_rspace(abuf_t *ab)
{
	return ABUF_SIZE - ab->rsize;
}

int abuf_get_wsize(abuf_t *ab)
{
	return ab->wsize;
}

int abuf_get_wspace(abuf_t *ab)
{
	return ABUF_SIZE - ab->wsize;
}

//
// READS
//
uint8_t abuf_read_u8(abuf_t *ab)
{
	assert(ab->rsize >= 1);
	uint8_t ret = ab->rdata[0];
	ab->rsize -= 1;
	memmove(ab->rdata, ab->rdata + 1, ab->rsize);
	return ret;
}

int8_t abuf_read_s8(abuf_t *ab)
{
	return (int8_t)abuf_read_u8(ab);
}

uint16_t abuf_read_u16(abuf_t *ab)
{
	assert(ab->rsize >= 2);
	uint16_t ret0 = ab->rdata[0];
	uint16_t ret1 = ab->rdata[1];
	uint16_t ret = ret0 | (ret1<<8);
	ab->rsize -= 2;
	memmove(ab->rdata, ab->rdata + 2, ab->rsize);
	return ret;
}

int16_t abuf_read_s16(abuf_t *ab)
{
	return (int16_t)abuf_read_u16(ab);
}

void abuf_read_block(void *buf, int len, abuf_t *ab)
{
	assert(ab->rsize >= len);
	ab->rsize -= len;
	memmove(buf, ab->rdata, len);
	memmove(ab->rdata, ab->rdata + len, ab->rsize);
}

//
// WRITES
//
void abuf_write_u8(uint8_t v, abuf_t *ab)
{
	assert(ab->wsize + 1 <= ABUF_SIZE);
	ab->wdata[ab->wsize] = v;
	ab->wsize += 1;
}

void abuf_write_s8(int8_t v, abuf_t *ab)
{
	abuf_write_u8((uint8_t)v, ab);
}

void abuf_write_u16(uint16_t v, abuf_t *ab)
{
	assert(ab->wsize + 2 <= ABUF_SIZE);
	ab->wdata[ab->wsize+0] = (v&255);
	ab->wdata[ab->wsize+1] = (v>>8);
	ab->wsize += 2;
}

void abuf_write_s16(int16_t v, abuf_t *ab)
{
	abuf_write_u16((uint16_t)v, ab);
}

void abuf_write_block(const void *buf, int len, abuf_t *ab)
{
	assert(ab->wsize + len <= ABUF_SIZE);
	memmove(ab->wdata + ab->wsize, buf, len);
	ab->wsize += len;
}

//
// POLLS
//
void abuf_poll_write(abuf_t *ab)
{
	int len;

	if(ab->loc_chain != NULL)
	{
		if(ab->wsize == 0)
		{
			return;

		} else if(ab->wsize + ab->loc_chain->rsize <= ABUF_SIZE) {
			memmove(ab->loc_chain->rdata + ab->loc_chain->rsize, ab->wdata, ab->wsize);
			ab->loc_chain->rsize += ab->wsize;
			ab->wsize = 0;

		} else {
			len = ABUF_SIZE - ab->loc_chain->rsize;
			memmove(ab->loc_chain->rdata + ab->loc_chain->rsize, ab->wdata, len);
			ab->loc_chain->rsize += ab->wsize;
			ab->wsize -= len;
			memmove(ab->wdata, ab->wdata + len, ab->wsize);

		}

	} else if(ab->sock != NULL) {
		len = SDLNet_TCP_Send(ab->sock, ab->wdata, ab->wsize);

		if(len == 0)
		{
			/*
			printf("FIXME: handle socket EOF\n");
			fflush(stdout);
			abort();
			*/

		} else if(len < 0) {
			printf("FIXME: handle socket send error\n");
			fflush(stdout);
			abort();

		} else if(len == ab->wsize) {
			ab->wsize = 0;

		} else {
			assert(len <= ab->wsize);
			ab->wsize -= len;
			memmove(ab->wdata, ab->wdata + len, ab->wsize);

		}
	}
}

void abuf_poll_read(abuf_t *ab)
{
	int len;

	if(ab->loc_chain != NULL)
	{
		abuf_poll_write(ab->loc_chain);

	} else if(ab->sock != NULL) {
		SDLNet_CheckSockets(ab->sset, 0);
		if(!SDLNet_SocketReady(ab->sock)) return;
		len = SDLNet_TCP_Recv(ab->sock, ab->rdata + ab->rsize, ABUF_SIZE - ab->rsize);

		if(len == 0)
		{
			/*
			printf("FIXME: handle socket EOF\n");
			fflush(stdout);
			abort();
			*/

		} else if(len < 0) {
			printf("FIXME: handle socket recv error\n");
			fflush(stdout);
			abort();

		} else {
			assert(len + ab->rsize <= ABUF_SIZE);
			ab->rsize += len;

		}
	}
}

void abuf_poll(abuf_t *ab)
{
	abuf_poll_write(ab);
	abuf_poll_read(ab);
}
