/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

snd_t *snd_splat[SND_SPLAT_COUNT];
achn_t achns[ACHN_COUNT];
SDL_AudioSpec audio_spec;
int audio_age = 0;
SDL_mutex *audio_mutex = NULL;

static int clampadd16(int a, int b)
{
	a += b;
	if(a < 0x0000) a = 0x0000;
	if(a > 0xFFFF) a = 0xFFFF;
	return a;

}

void snd_free(snd_t *snd)
{
	if(snd->data != snd->ldata && snd->ldata != NULL) free(snd->ldata);
	if(snd->data != snd->rdata && snd->rdata != NULL) free(snd->rdata);
	if(snd->data != NULL) free(snd->data);
	free(snd);
}

snd_t *snd_alloc(int len, int is_stereo, int freq)
{
	// Allocate
	snd_t *snd = malloc(sizeof(snd_t));

	// Initialise
	if(is_stereo)
	{
		snd->ldata = malloc(len*2);
		snd->rdata = malloc(len*2);
		// TODO: mix for mono
		snd->data = snd->ldata;

	} else {
		snd->ldata = snd->rdata = malloc(len*2);
		snd->data = snd->ldata;
	}

	snd->len = len;
	snd->freq = freq;

	// Return
	return snd;
}

snd_t *snd_load_wav(const char *fname)
{
	int i;
	char tag[4];
	int len, freq, channels, sbits, codec, balign;
	FILE *fp;
	snd_t *snd;

	// Open file for reading
	fp = fopen(fname, "rb");
	if(fp == NULL)
	{
		perror("snd_load_wav(fopen)");
		return NULL;
	}

	// Check if RIFF
	tag[3] = '\x00';
	fread(tag, 4, 1, fp);
	if(memcmp(tag, "RIFF", 4))
	{
		printf("snd_load_wav: not a RIFF WAVE file\n");
		goto fail_fp;
	}

	// Skip the length, check if WAVE
	fread(tag, 4, 1, fp);
	tag[3] = '\x00';
	fread(tag, 4, 1, fp);
	if(memcmp(tag, "WAVE", 4))
	{
		printf("snd_load_wav: not a RIFF WAVE file\n");
		goto fail_fp;
	}

	// Scan for "fmt "
	for(;;)
	{
		tag[3] = '\x00';
		if(fread(tag, 4, 1, fp) != 1)
		{
			printf("snd_load_wav: expected \"fmt \" chunk, EOF reached instead\n");
			goto fail_fp;
		}

		if(!memcmp(tag, "fmt ", 4))
			break;

		len = io_get4le(fp);
		fseek(fp, len, SEEK_CUR);
	}

	// Get info
	len = io_get4le(fp);
	if(len < 16)
	{
		printf("snd_load_wav: insufficient length for \"fmt \" chunk\n");
		goto fail_fp;
	}

	// Read "fmt " chunk
	codec = io_get2le(fp);
	if(codec != 1)
	{
		printf("snd_load_wav: unsupported codec %i\n", codec);
		goto fail_fp;
	}

	channels = io_get2le(fp);
	if(channels < 1 || channels > 2)
	{
		printf("snd_load_wav: unsupported channel count %i\n", channels);
		goto fail_fp;
	}

	freq = io_get4le(fp);
	io_get4le(fp); // Don't care about bytes per second
	balign = io_get2le(fp); // TODO: Might end up checking this, might not.
	sbits = io_get2le(fp);
	if(sbits != 8 && sbits != 16)
	{
		printf("snd_load_wav: unsupported sample bitrate %i\n", sbits);
		goto fail_fp;
	}

	// Skip the rest
	fseek(fp, len-16, SEEK_CUR);

	// Scan for "data"
	for(;;)
	{
		tag[3] = '\x00';
		if(fread(tag, 4, 1, fp) != 1)
		{
			printf("snd_load_wav: expected \"data\" chunk, EOF reached instead\n");
			goto fail_fp;
		}

		if(!memcmp(tag, "data", 4))
			break;

		len = io_get4le(fp);
		fseek(fp, len, SEEK_CUR);
	}

	// Get data
	len = io_get4le(fp);

	// Check things
	// TODO: Be more graceful
	assert((len*8) % (channels * sbits) == 0);
	assert(channels == 1 || channels == 2);

	// Allocate sound
	snd = snd_alloc(len, channels >= 2, freq);

	// Actually get said data
	if(sbits == 8)
	{
		if(channels == 2)
		{
			for(i = 0; i < len/1; i++)
			{
				snd->ldata[i] = (fgetc(fp)<<8)-0x8000;
				snd->rdata[i] = (fgetc(fp)<<8)-0x8000;
			}

		} else if(channels == 1) {
			for(i = 0; i < len/2; i++)
			{
				snd->data[i] = (fgetc(fp)<<8)-0x8000;
			}

		} else {
			printf("EDOOFUS: Channel count invalid\n");
			fflush(stdout);
			abort();
		}

	} else if(sbits == 16) {
		if(channels == 2)
		{
			for(i = 0; i < len/4; i++)
			{
				snd->ldata[i] = io_get2le(fp);
				snd->rdata[i] = io_get2le(fp);
			}

		} else if(channels == 1) {
			for(i = 0; i < len/2; i++)
			{
				snd->data[i] = io_get2le(fp);
			}

		} else {
			printf("EDOOFUS: Channel count invalid\n");
			fflush(stdout);
			abort();
		}

	}

	// TODO: Read "data" chunk
	fseek(fp, len, SEEK_CUR);

	// Close and return our sound
	fclose(fp);
	return snd;

fail_fp:
	// Close and return with failure
	fclose(fp);
	return NULL;

}

void snd_play(snd_t *snd, int vol, int use_world, int sx, int sy, int fmul, int offs)
{
	int i;
	int mutret;
	achn_t *ac;
	achn_t *ac_oldest = achns;
	int oldest_age = 0;

	// Find a channel
	for(i = 0, ac = achns; i < ACHN_COUNT; i++, ac++)
	{
		if(ac->snd == NULL) break;
		if(ac->offs >= ac->snd->len) break;

		if((audio_age - ac->age) > oldest_age)
		{
			oldest_age = (audio_age - ac->age);
			ac_oldest = ac;
		}
	}

	// Kill the oldest sound if we can't find a free slot
	if(ac >= achns + ACHN_COUNT)
		ac = ac_oldest;

	// Lock audio
	mutret = SDL_mutexP(audio_mutex);
	assert(mutret != -1);

	// Play a sound
	ac->age = audio_age++;
	ac->snd = snd;
	ac->offs = offs;
	ac->suboffs = 0;
	ac->freq = (snd->freq<<12)/(audio_spec.freq>>4);
	ac->freq = (ac->freq * fmul)>>8;
	ac->vol = vol;
	ac->use_world = use_world;
	ac->sx = sx;
	ac->sy = sy;

	// Unlock audio
	mutret = SDL_mutexV(audio_mutex);
	assert(mutret != -1);
}

void snd_play_splat(int use_world, int sx, int sy)
{
	int vol = 0x100 - (rand() % 53);
	int fmul = 0x100 - 53/2 + (rand() % 53);
	rand(); // Try not to do groups of 3 in a row
	int smp = rand() % 3;
	rand(); // Try not to do groups of 3 in a row

	snd_play(snd_splat[smp], vol, use_world, sx, sy, fmul, 0);

}

void achn_reset(achn_t *ac)
{
	ac->age = audio_age;
	ac->freq = 0;
	ac->offs = 0;
	ac->suboffs = 0;
	ac->vol = 0;
	ac->sx = 0;
	ac->sy = 0;
	ac->use_world = 0;

	ac->snd = NULL;
}

static void audio_calc_vol(const achn_t *ac, int *lvol, int *rvol)
{
	// TODO: care about use_world
	*lvol = *rvol = ac->vol;
}

static void audio_callback(void *userdata, Uint8 *stream, int len_samples)
{
	//uint8_t *a8;
	uint16_t *a16;
	int16_t *ldata, *rdata;
	int i, j;
	int chns = audio_spec.channels;
	int bias = 0;
	int lvol, rvol;
	achn_t *ac;
	int mutret;

	len_samples /= chns;

	// Lock audio
	mutret = SDL_mutexP(audio_mutex);
	assert(mutret != -1);

	// Select according to whatever
	switch(audio_spec.format)
	{
		case AUDIO_S8:
			bias = -0x80;
		case AUDIO_U8:
			printf("TODO: 8bit output\n");
			fflush(stdout);
			abort();
			break;

		case AUDIO_S16SYS:
			bias = -0x8000;
		case AUDIO_U16SYS:
			len_samples /= 2;
			for(i = 0, a16 = (uint16_t *)stream; i < len_samples*chns; i++)
				*(a16++) = 0x8000;

			for(i = 0, ac = achns; i < ACHN_COUNT; i++, ac++)
			{
				if(ac->snd == NULL) continue;
				//printf("ac->snd %p %i\n", ac, ac->offs);

				audio_calc_vol(ac, &lvol, &rvol);
				ldata = ac->snd->ldata;
				rdata = ac->snd->rdata;

				for(j = 0, a16 = (uint16_t *)stream; j < len_samples; j++, a16 += chns)
				{
					if(ac->offs >= ac->snd->len) break;
					a16[0] = clampadd16(a16[0], (lvol * (int)ldata[ac->offs])>>8);
					if(chns >= 2) a16[1] = clampadd16(a16[1], (rvol * (int)rdata[ac->offs])>>8);
					ac->suboffs += ac->freq;
					ac->offs += ac->suboffs>>16;
					ac->suboffs &= 0xFFFF;
				}
			}

			for(i = 0, a16 = (uint16_t *)stream; i < len_samples*chns; i++)
				*(a16++) += bias;
			break;

		default:
			printf("FATAL: Sound format not supported\n");
			fflush(stdout);
			abort();

	}

	// Unlock audio
	mutret = SDL_mutexV(audio_mutex);
	assert(mutret != -1);

}

int audio_init(void)
{
	int i;
	char buf[64];
	SDL_AudioSpec desired;

	// Reset audio channels
	for(i = 0; i < ACHN_COUNT; i++)
		achn_reset(achns + i);

	// Load sounds
	for(i = 0; i < SND_SPLAT_COUNT; i++)
	{
		sprintf(buf, "wav/splat%i.wav", i+1);
		snd_splat[i] = snd_load_wav(buf);
	}

	// Create mutex
	audio_mutex = SDL_CreateMutex();
	assert(audio_mutex != NULL);


	// Set up SDL audio
	// Note, some systems hate 44100Hz, so we'll go for 48000Hz
	// Let's just ignore the fact that sackit is currently hardcoded for 44100Hz
	// (which I'll probably end up fixing)
	desired.freq = 48000;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 4096; // This is usually long enough
	desired.callback = audio_callback;
	desired.userdata = NULL;

	if(SDL_OpenAudio(&desired, &audio_spec) != 0)
	{
		printf("audio_init: couldn't open: %s\n", SDL_GetError());
		return 0;
	}

	printf("audio: %iHz, %i channels, %i samples, fmt %04X\n"
		, audio_spec.freq
		, audio_spec.channels
		, audio_spec.samples
		, audio_spec.format
	);

	// Play sound
	snd_play_splat(0, 0, 0);

	// Unpause audio and return
	SDL_PauseAudio(0);
	return 1;
}

