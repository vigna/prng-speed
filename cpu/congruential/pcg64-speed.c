#include <stdint.h>

// -------
// PCG code Copyright by Melissa O'Neill

typedef __uint128_t pcg128_t;
#define PCG_128BIT_CONSTANT(high,low) ((((pcg128_t)high) << 64) + low)

pcg128_t _state;

static inline pcg128_t pcg_rotr_128(pcg128_t value, unsigned int rot) {
    return (value >> rot) | (value << ((- rot) & 127));
}

#define PCG_DEFAULT_MULTIPLIER_128 \
        PCG_128BIT_CONSTANT(2549297995355413924ULL,4865540595714422341ULL)
#define PCG_DEFAULT_INCREMENT_128  \
        PCG_128BIT_CONSTANT(6364136223846793005ULL,1442695040888963407ULL)

static inline uint64_t pcg_oneseq_128_xsh_rs_64_random_r() {
	_state = _state * PCG_DEFAULT_MULTIPLIER_128 + PCG_DEFAULT_INCREMENT_128;
    return (uint64_t)(((_state >> 43u) ^ _state) >> ((_state >> 124u) + 45u));
}

#define NEXT pcg_oneseq_128_xsh_rs_64_random_r();

#include "harness.c"
