/*
 * Copyright (C) 2026 Sebastiano Vigna
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdint.h>

/* An MCG with modulus 2^127 - 1 and a random 64-bit multiplier that is a
   primitive root larger than the square root of the modulus (a smaller one
   would be provably bad in dimension two), but that has not been selected
   by the spectral test: for speed only. */

#define MCG_B 0xf3777c403f78e1c2

/* The current state of the generator, in [1..2^127 - 2]. */
static __uint128_t x = ((__uint128_t)1 << 64) + 1;

static uint64_t inline next(void) {
	const __uint128_t mask = ((__uint128_t)1 << 127) - 1;
	/* Multiplying by 2^63 modulo 2^127 - 1 rotates the state left by 63 bits:
	   the lower 64 bits of the state are the upper 64 bits of the state of the
	   same MCG started from 2^63 times the seed. */
	const uint64_t result = x;
	const __uint128_t lo = (uint64_t)x * (__uint128_t)MCG_B;
	const __uint128_t hi = (uint64_t)(x >> 64) * (__uint128_t)MCG_B;
	/* The product is t * 2^64 + (uint64_t)lo < 2^191, so its bits from bit 127
	   onwards fit in 64 bits, and 2^127 = 1 modulo 2^127 - 1. */
	const __uint128_t t = hi + (lo >> 64);
	x = ((t << 64 | (uint64_t)lo) & mask) + (uint64_t)(t >> 63);
	/* Now x < 2^127 + 2^64, and x >= 2^127 with probability less than 2^-63:
	   a branch keeps the last reduction out of the loop-carried dependency chain
	   (as in lcg127prime of Alexey Voskov's SmokeRand). */
	if (__builtin_expect(x >> 127, 0)) x = (x & mask) + 1;
	return result;
}

#include "harness.c"
