/* Pierre L'Ecuyer's LFSR258 ("Tables of maximally equidistributed combined
   LFSR generators", Math. Comp. 68(225), pp. 261-269, 1999). */

#include <stdint.h>

static uint64_t x1 = -1ULL, x2 = -1ULL, x3 = -1ULL, x4 = -1ULL, x5 = -1ULL;

static uint64_t inline next() {
	uint64_t b;

	b = ((x1 << 1) ^ x1) >> 53;
	x1 = ((x1 & 18446744073709551614ULL) << 10) ^ b;
	b = ((x2 << 24) ^ x2) >> 50;
	x2 = ((x2 & 18446744073709551104ULL) << 5) ^ b;
	b = ((x3 << 3) ^ x3) >> 23;
	x3 = ((x3 & 18446744073709547520ULL) << 29) ^ b;
	b = ((x4 << 5) ^ x4) >> 24;
	x4 = ((x4 & 18446744073709420544ULL) << 23) ^ b;
	b = ((x5 << 3) ^ x5) >> 33;
	x5 = ((x5 & 18446744073701163008ULL) << 8) ^ b;
	return x1 ^ x2 ^ x3 ^ x4 ^ x5;
}

#define HARNESS_ADD
#include "harness.c"
