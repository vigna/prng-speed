#include <stdint.h>

// -------
// PCG code Copyright by Melissa O'Neill

typedef __uint128_t pcg128_t;
pcg128_t _state;
static uint64_t inc = 0xdeadbeefdeadf00d;

static inline uint64_t pcg_output_cm_128_64(pcg128_t state) {
	uint64_t hi = state >> 64;
	uint64_t lo = state;

	lo |= 1;
	hi ^= hi >> 32;
	hi *= 0xda942042e4dd58b5ULL;
	hi ^= hi >> 48;
	hi *= lo;
	return hi;
}
 
static inline uint64_t pcg_dxm() {
	uint64_t result = pcg_output_cm_128_64(_state);
	_state = _state * 0xda942042e4dd58b5 + inc;
	return result;
}

#define NEXT pcg_dxm();

#include "harness.c"
