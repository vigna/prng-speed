#include <stdint.h>

/* The PCG64 generator of NumPy (numpy.random.PCG64, the bit generator
   returned by numpy.random.default_rng()): a 128-bit LCG with a full 128-bit
   multiplier and a per-stream increment, whose output is obtained by applying
   the XSL-RR ("xorshift low, random rotation") permutation to the new state.
   NumPy's PCG64DXSM (pcg64dxsm-speed.c) uses instead a 64-bit multiplier and
   the DXSM permutation, and computes the output on the current state.

   With the initialization below, the outputs are those of numpy.random.PCG64
   after setting its state to {"state": 2**64 + 2, "inc": PCG_NUMPY_INC}. */

#define PCG_NUMPY_MUL (((__uint128_t)2549297995355413924ULL << 64) | 4865540595714422341ULL)
#define PCG_NUMPY_INC (((__uint128_t)6364136223846793005ULL << 64) | 1442695040888963407ULL)

static __uint128_t state = ((__uint128_t)1 << 64) | 2;
static __uint128_t inc = PCG_NUMPY_INC;

static inline uint64_t next(void) {
	state = state * PCG_NUMPY_MUL + inc;
	const uint64_t x = (uint64_t)(state >> 64) ^ (uint64_t)state;
	const unsigned int rot = state >> 122;
	return (x >> rot) | (x << (-rot & 63));
}

#include "harness.c"
