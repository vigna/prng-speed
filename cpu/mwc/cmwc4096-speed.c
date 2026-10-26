#include <stdint.h>

static uint32_t Q[4096], c = 123;
int p = 4095;

/* Marsaglia's CMWC4096 */

static inline uint32_t next(void) {
	p = (p + 1) & 4095;
	const uint64_t t = INT64_C(18782) * Q[p] + c;
	c = t >> 32;
	uint32_t x = t + c;
	if (x < c) {
		x++;
		c++;
	}
	return Q[p] = 0xfffffffe - x;
}

/* The state is filled using SplitMix: with an all-zero Q, outputs are degenerate
   for tens of millions of steps. */
static inline uint64_t splitmix64(uint64_t *x) {
	uint64_t z = (*x += 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

#define INIT { uint64_t seed = 0; for(int i = 0; i < 4096; i++) Q[i] = splitmix64(&seed); }

#define NEXT next() ^ next()

#define HARNESS_ADD

#include "harness.c"
