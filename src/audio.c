/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

// These 2 tables are from here: http://wiki.multimedia.cx/index.php?title=IMA_ADPCM
int ima_index_table[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
}; 

int ima_step_table[89] = { 
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 
};

achn_t achns[ACHN_COUNT];
SDL_AudioSpec audio_spec;
int audio_age = 0;
SDL_mutex *audio_mutex = NULL;

snd_t *snd_splat[SND_SPLAT_COUNT];
snd_t *snd_step[SND_STEP_COUNT];
it_module_t *mod_trk1;
sackit_playback_t *sackit = NULL;
achn_t *ac_sackit = NULL;
snd_t *snd_sackit = NULL;
int music_buffer_free = 0;

static void music_update(void);

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

int adpcm_predict(int v, int *pred, int *step)
{
	int diff;

	diff = (((v&7)*2 + 1) * ima_step_table[*step]) / 8;

	*step += ima_index_table[v];
	if(*step < 0) *step = 0;
	if(*step > 88) *step = 88;

	*pred += ((v & 8) != 0 ? -diff : diff);
	if(*pred < -0x8000) *pred = -0x8000;
	if(*pred >  0x7FFF) *pred =  0x7FFF;

	return *pred;
}

void adpcm_load_block(int16_t **wptr, int *pred, int *step, FILE *fp)
{
	int i;
	int v;

	for(i = 0; i < 4; i++)
	{
		v = fgetc(fp);
		*((*wptr)++) = adpcm_predict(v&15, pred, step);
		*((*wptr)++) = adpcm_predict((v>>4)&15, pred, step);
	}

}

snd_t *snd_load_wav(const char *fname)
{
	int i, j;
	int lpred, rpred;
	int lstep, rstep;
	int16_t *wptr1, *wptr2;
	char tag[4];
	int len, freq, channels, sbits, codec, balign;
	int adlen;
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
	if(codec != 1 && codec != 17)
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
	if((codec == 17 ? sbits != 4 : sbits != 8 && sbits != 16))
	{
		printf("snd_load_wav: unsupported sample bitrate %i\n", sbits);
		goto fail_fp;
	}

	if(codec == 17 && balign % 4 != 0)
	{
		printf("snd_load_wav: unsupported block alignment %i\n", balign);
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
	assert(len % balign == 0);

	// Allocate sound
	switch(codec)
	{
		case 1:
			snd = snd_alloc(len, channels >= 2, freq);
			break;

		case 17:
			adlen = (len/balign);
			adlen *= (balign-4*channels)*2;
			snd = snd_alloc(adlen,
				channels >= 2, freq);
			//printf("len %i %i\n", snd->len, len);
			break;

		default:
			printf("EDOOFUS: Codec invalid\n");
			fflush(stdout);
			abort();
	}

	// Actually get said data
	if(codec == 17 && sbits == 4)
	{
		wptr1 = snd->ldata;
		wptr2 = snd->rdata;

		for(i = 0; i < len/balign; i++)
		{
			// Feed predictors
			lpred = io_get2le(fp);
			lstep = fgetc(fp);
			fgetc(fp);

			if(channels >= 2)
			{
				rpred = io_get2le(fp);
				rstep = fgetc(fp);
				fgetc(fp);
			}

			if(lpred >= 0x8000) lpred -= 0x10000;
			if(rpred >= 0x8000) rpred -= 0x10000;

			//printf("pred %i %i\n", lpred, lstep);

			// Actually predict things
			for(j = 0; j < (balign/(4*channels))-1; j++)
			{
				adpcm_load_block(&wptr1, &lpred, &lstep, fp);
				if(channels >= 2) adpcm_load_block(&wptr2, &rpred, &rstep, fp);
			}

		}

	} else if(sbits == 8) {
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

	} else {
		printf("EDOOFUS: Bit count invalid\n");
		fflush(stdout);
		abort();
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

achn_t *snd_play(snd_t *snd, int vol, int use_world, int sx, int sy, int fmul, int offs, int lockme)
{
	int i;
	int mutret;
	achn_t *ac;
	achn_t *ac_oldest = achns;
	int oldest_age = 0;

	if(snd == NULL) return NULL;

	// Find a channel
	for(i = 0, ac = achns; i < ACHN_COUNT; i++, ac++)
	{
		if(ac->snd == NULL) break;
		if(ac->offs >= ac->snd->len) break;

		if((!ac->nokill) && (audio_age - ac->age) > oldest_age)
		{
			oldest_age = (audio_age - ac->age);
			ac_oldest = ac;
		}
	}

	// Kill the oldest sound if we can't find a free slot
	if(ac >= achns + ACHN_COUNT)
	{
		ac = ac_oldest;

		// Fail silently if it's a nokill channel
		if(ac->nokill)
			return NULL;
	}

	// Lock audio
	if(lockme)
	{
		mutret = SDL_mutexP(audio_mutex);
		assert(mutret != -1);
	}

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
	if(lockme)
	{
		mutret = SDL_mutexV(audio_mutex);
		assert(mutret != -1);
	}

	// Return channel
	return ac;
}

// WARNING: MUST HAVE AUDIO LOCK BEFORE CALLING!
static void music_update(void)
{
	int i;

	// Ensure we have things
	if(sackit == NULL) return;

	//printf("%i %i %i\n", ac_sackit->offs, music_buffer_free, sackit->buf_len);
	// Move stuff if need be
	if(ac_sackit->offs >= (int)sackit->buf_len)
	{
		//printf("move %i\n", ac_sackit->offs);
		memmove(snd_sackit->ldata, snd_sackit->ldata + ac_sackit->offs, 2*snd_sackit->len - ac_sackit->offs);
		if(snd_sackit->rdata != snd_sackit->ldata)
			memmove(snd_sackit->rdata, snd_sackit->rdata + ac_sackit->offs, 2*snd_sackit->len - ac_sackit->offs);

		music_buffer_free += ac_sackit->offs;
		ac_sackit->offs = 0;
	}

	// Update if need be
	while(music_buffer_free >= (int)sackit->buf_len)
	{
		sackit_playback_update(sackit);
		int16_t *d0 = snd_sackit->ldata + snd_sackit->len - music_buffer_free;
		int16_t *d1 = snd_sackit->rdata + snd_sackit->len - music_buffer_free;
		int16_t *src = sackit->buf;
		music_buffer_free -= sackit->buf_len;

		if(snd_sackit->ldata != snd_sackit->rdata)
		{
			for(i = 0; i < (int)sackit->buf_len; i++)
			{
				*(d0++) = *(src++);
				*(d1++) = *(src++);
			}

		} else {
			for(i = 0; i < (int)sackit->buf_len; i++)
			{
				*(d0++) = *(src++);
			}
		}

	}

	// All done!

}

void music_free(it_module_t *mod)
{
	sackit_module_free(mod);
}

it_module_t *music_load_it(const char *fname)
{
	return sackit_module_load(fname);
}

void music_play(it_module_t *mod)
{
	int mutret;

	// Lock audio
	mutret = SDL_mutexP(audio_mutex);
	assert(mutret != -1);

	// Destroy sackit playback object if need be
	if(sackit != NULL)
	{
		sackit_playback_free(sackit);
		sackit = NULL;
	}

	// If mod is NULL, we are killing the sound
	if(mod == NULL)
	{
		if(ac_sackit != NULL)
		{
			ac_sackit->nokill = 0;
			ac_sackit->snd = NULL;
			ac_sackit = NULL;
		}

		if(snd_sackit != NULL)
		{
			snd_free(snd_sackit);
			snd_sackit = NULL;
		}

		// Unlock audio
		mutret = SDL_mutexV(audio_mutex);
		assert(mutret != -1);

		return;
	}

	// Create new objects
	if(snd_sackit == NULL)
	{
		snd_sackit = snd_alloc(65536, audio_spec.channels >= 2, 44100);
		assert(ac_sackit == NULL);
		ac_sackit = snd_play(snd_sackit, 0x80, 0, 0, 0, 0x100, 0, 0);
		ac_sackit->nokill = 1;

	}

	// Create sackit playback object
	sackit = sackit_playback_new(mod, 2048, 128,
		audio_spec.channels >= 2 ? MIXER_IT214FS : MIXER_IT214F);

	// Prime the buffer
	music_buffer_free = snd_sackit->len;

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

	snd_play(snd_splat[smp], vol, use_world, sx, sy, fmul, 0, 1);

}

void snd_play_step(int use_world, int step_cls, int sx, int sy)
{
	int vol = 0x100 - (rand() % 53);
	int fmul = 0x100 - 53/2 + (rand() % 53);
	rand(); // Try not to do groups of 3 in a row
	int smp = (rand()>>12) % 2;
	smp = (smp*2) + (step_cls%2);
	rand(); // Try not to do groups of 3 in a row

	snd_play(snd_step[smp], vol, use_world, sx, sy, fmul, 0, 1);

}

void achn_reset(achn_t *ac)
{
	ac->age = audio_age;
	ac->nokill = 0;
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

	// Update music
	music_update();

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
		sprintf(buf, "dat/splat%i.snd", i+1);
		snd_splat[i] = snd_load_wav(buf);
	}

	for(i = 0; i < SND_STEP_COUNT; i++)
	{
		sprintf(buf, "dat/step%i.snd", i+1);
		snd_step[i] = snd_load_wav(buf);
	}

	// Load music
	mod_trk1 = music_load_it("dat/trk1.it");

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
	//desired.samples = 4096; // This is usually long enough
	desired.samples = 2048;
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

	// Play music
	music_play(mod_trk1);

	// Play sound
	snd_play_splat(0, 0, 0);

	// Unpause audio and return
	SDL_PauseAudio(0);
	return 1;
}

