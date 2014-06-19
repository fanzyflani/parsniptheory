#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
static const int hctab[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

typedef unsigned long uLongf;
typedef int8_t Bytef;

typedef struct sm
{
	const Bytef *data;
	int len;
	int offs;
} sm_t;

static int sm_read(sm_t *sm, int bits)
{
	int v = 0;
	int s = 0;
	int obits = bits;

	assert((sm->offs + bits)/8 <= sm->len);

	while(bits > 8-(sm->offs&7))
	{
		v += ((0xFF & (int)sm->data[sm->offs>>3]) >> (sm->offs&7)) << s;
		bits -= (8-(sm->offs&7));
		sm->offs += (8-(sm->offs&7));
	}

	if(bits > 0)
	{
		v += (((0xFF & (int)sm->data[sm->offs>>3]) >> (sm->offs&7)) & ((1<<bits)-1)) << s;
		sm->offs += bits;
	}

	assert(v >= 0 && v < (1<<obits));

	return v;
}

static void gen_hufftab(int len, int *tg, int *tv, int *ts)
{
	int i, j;
	int q, p, v;
	
	// Clear tables for everything
	for(i = 0; i <= 16; i++) ts[i] = 0;
	for(i = 0; i < len; i++) tv[i] = -1;

	// Fill tables
	for(i = 1, q = 0, p = 2, v = 0; i <= 16; i++, p <<= 1, v <<= 1)
	for(j = 0; j < len; j++)
	if(tg[j] == i)
	{
		tv[q++] = j;
		ts[i]++;
		v++;
		//printf("%05X %05X %i %i %i %i\n", v, p, q, i, j, ts[i]);
		assert(v <= p);
	}

	// DEBUG: Show steps
	//printf("%i %i\n", v, p);
	for(i = 1; i <= 16; i++) { printf(" %i", ts[i]); } printf("\n");
}

static int huff_decode(int len, int *tv, int *ts, sm_t *sm)
{
	int i;
	int v, p, q;
	int sv;

	for(i = 1, sv = 0, q = 0, p = 2, v = 1; i <= 16; q += ts[i], i++, p <<= 1, v <<= 1)
	{
		//
		//printf("%05X %05X %05X %05X %i\n", v, p, sv, ts[i], q);

		sv = (sv<<1) + sm_read(sm, 1);

		if(sv < v)
		{
			//printf("> %i %i %i %i %i %i\n", len, q, sv, v, q+v-sv, tv[q+v-sv]);
			return tv[q+v-sv];
		}

		v += ts[i];
	}
	
	printf("FAIL: %05X\n", sv);
	abort();

	return 0;
}

int uncompress(Bytef *outbuf, uLongf *outlen, const Bytef *inbuf, uLongf inlen)
{
	int i, v, l, pv;

	int huff_clen_g[19];
	int huff_clen_v[19];
	int huff_clen_s[16+1];
	int huff_dist_g[32];
	int huff_dist_v[32];
	int huff_dist_s[16+1];
	int huff_lit_g[288];
	int huff_lit_v[288];
	int huff_lit_s[16+1];
	int hclen = 0;
	int hlit = 0;
	int hdist = 0;

	int outoffs = 0;

	// Prepare stream
	sm_t sm_real = {inbuf, inlen, 0};
	sm_t *sm = &sm_real;

	// Skip the first 2 bytes
	sm_read(sm, 16);

	// Now start reading!
	int final = 0;
	int ctyp = 0;
	while(!final)
	{
		// Get block info
		final = sm_read(sm, 1);
		ctyp = sm_read(sm, 2);
		printf("final=%i ctyp=%i\n", final, ctyp);
		assert(ctyp != 3);

		// Determine by type
		assert(ctyp != 0);
		assert(ctyp != 1);
		switch(ctyp)
		{
			case 0:
				// UNCOMPRESSED
				sm_read(sm, (-sm->offs)&7);
				// TODO: length and stuff
				break;

			case 1:
				// STATIC HUFFMAN
				break;

			case 2:
				// DYNAMIC HUFFMAN

				// Get lengths
				hlit = sm_read(sm, 5) + 257;
				hdist = sm_read(sm, 5) + 1;
				hclen = sm_read(sm, 4) + 4;
				printf("%i %i %i\n", hlit, hdist, hclen);
				assert(hlit >= 257 && hlit <= 286);
				assert(hdist >= 1 && hdist <= 32);
				assert(hclen >= 1 && hclen <= 19);

				// Read hclit tab
				for(i = 0; i < 19; i++) huff_clen_g[i] = 0;
				for(i = 0; i < hclen; i++) huff_clen_g[i] = sm_read(sm, 3);

				// DEBUG: Show table
				for(i = 0; i < 19; i++) { printf("%i ", huff_clen_g[i]); } printf("\n");

				// Generate hclen tab
				gen_hufftab(19, huff_clen_g, huff_clen_v, huff_clen_s);

				// Decode hlit tab
				for(i = 0; i < 288; i++) huff_lit_g[i] = 0;
				for(i = 0; i < hlit;)
				{
					v = hctab[huff_decode(19, huff_clen_v, huff_clen_s, sm)];
					assert(v >= 0 && v <= 18);
					//printf("%i %i\n", i, v);

					pv = -1;
					if(v <= 15)
					{
						pv = v;
						huff_lit_g[i++] = v;
					} else if(v == 16) {
						assert(pv != -1);
						for(l = sm_read(sm, 2)+3; l > 0; l--)
							huff_lit_g[i++] = pv;
					} else if(v == 17) {
						for(l = sm_read(sm, 3)+3; l > 0; l--)
							huff_lit_g[i++] = 0;
					} else if(v == 18) {
						for(l = sm_read(sm, 7)+11; l > 0; l--)
							huff_lit_g[i++] = 0;
					}

					assert(i <= hlit);
				}

				gen_hufftab(288, huff_lit_g, huff_lit_v, huff_lit_s);

				// Decode hdist tab
				for(i = 0; i < 32; i++) huff_dist_g[i] = 0;
				for(i = 0; i < hdist;)
				{
					v = hctab[huff_decode(19, huff_clen_v, huff_clen_s, sm)];
					assert(v >= 0 && v <= 18);

					pv = -1;
					if(v <= 15)
					{
						pv = v;
						huff_dist_g[i++] = v;
					} else if(v == 16) {
						assert(pv != -1);
						for(l = sm_read(sm, 2)+3; l > 0; l--)
							huff_dist_g[i++] = pv;
					} else if(v == 17) {
						for(l = sm_read(sm, 3)+3; l > 0; l--)
							huff_dist_g[i++] = 0;
					} else if(v == 18) {
						for(l = sm_read(sm, 7)+11; l > 0; l--)
							huff_dist_g[i++] = 0;
					}

					assert(i <= hdist);
				}
				gen_hufftab(32, huff_dist_g, huff_dist_v, huff_dist_s);
				break;

		}

		abort();

	}

	// Return success
	return 0;
}

#ifdef TEST_UDEFLATE_MAIN
int main(int argc, char *argv[])
{

}

#endif

