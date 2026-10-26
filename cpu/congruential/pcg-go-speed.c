#include <stdint.h>

/* The PCG generator of the Go language (math/rand/v2, type PCG): a 128-bit LCG
   with a full 128-bit multiplier and a fixed increment, whose output is
   obtained by applying the DXSM ("double xorshift multiply") permutation to
   the new state. NumPy's PCG64DXSM (pcg64dxsm-speed.c) uses instead a 64-bit
   multiplier and a per-stream increment, and computes the output on the
   current state.

   The state is initialized as by NewPCG(1, 2): the outputs are then those
   checked by TestPCG in Go's src/math/rand/v2/pcg_test.go. */

#define PCG_GO_MUL (((__uint128_t)2549297995355413924ULL << 64) | 4865540595714422341ULL)
#define PCG_GO_INC (((__uint128_t)6364136223846793005ULL << 64) | 1442695040888963407ULL)

static __uint128_t state = ((__uint128_t)1 << 64) | 2;

static inline uint64_t next(void) {
	state = state * PCG_GO_MUL + PCG_GO_INC;
	uint64_t hi = state >> 64;
	const uint64_t lo = state;
	hi ^= hi >> 32;
	hi *= 0xda942042e4dd58b5ULL;
	hi ^= hi >> 48;
	hi *= lo | 1;
	return hi;
}

#include "harness.c"
