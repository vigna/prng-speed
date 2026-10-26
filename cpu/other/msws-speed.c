/* Bernard Widynski's Middle Square Weyl Sequence RNG (arXiv:1704.00358): 32 bits
   per step, so two steps make a 64-bit output. */

#include <stdint.h>

static uint64_t x, w;

static uint32_t inline next(void) {
	x *= x; /* square x */
	x += (w += 0xB5AD4ECEDA1CE2A9);
	x = (x >> 32) | (x << 32); 
	return x;
}

#define NEXT next() ^ next()

#define HARNESS_ADD
#include "harness.c"
