/*  Written in 2019 by Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#include <stdint.h>

// You can play with this value on your architecture, provided that it divides
// 1024: on an AMD EPYC 9R45 (Zen 5) the best value is 16 with clang and 32
// with gcc (with 32, clang is several times slower)
#define XOSHIRO256_UNROLL (16)

/* The current state of the generators. */
static uint64_t s[4][XOSHIRO256_UNROLL];

static __inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

#ifdef HARNESS_DOUBLE
double result[1024];
#else
uint64_t result[1024];
#endif

#ifdef HARNESS_DOUBLE
static inline double next(double * const restrict array, int len)
#else
static inline uint64_t next(uint64_t * const restrict array, int len)
#endif
{
 	uint64_t t[XOSHIRO256_UNROLL];

	for(int b = 0; b < len; b += XOSHIRO256_UNROLL) {
#ifdef HARNESS_DOUBLE
		for(int i = 0; i < XOSHIRO256_UNROLL; i++) array[b + i] = ((rotl(s[0][i] + s[3][i], 23) + s[0][i]) >> 11) * 0x1.0p-53;
#else
		for(int i = 0; i < XOSHIRO256_UNROLL; i++) array[b + i] = rotl(s[0][i] + s[3][i], 23) + s[0][i];
#endif

		for(int i = 0; i < XOSHIRO256_UNROLL; i++) t[i] = s[1][i] << 17;

		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[2][i] ^= s[0][i];
		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[3][i] ^= s[1][i];
		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[1][i] ^= s[2][i];
		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[0][i] ^= s[3][i];

		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[2][i] ^= t[i];

		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[3][i] = rotl(s[3][i], 45);
	}

	// This is just to avoid dead-code elimination
	return array[0] + array[len - 1];
}

#define INIT for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[0][i] = UINT64_C(1) << i;
#define NEXT next(result, 1024);
/* Each evaluation of NEXT computes 1024 outputs. */
#define OUTPUTS 1024

#include "harness.c"
