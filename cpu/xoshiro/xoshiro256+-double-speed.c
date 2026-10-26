/*
 * Copyright (C) 2014 Sebastiano Vigna
 * Copyright (C) 2016 The rotating xor conspiracy.
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

/* The current state of the generator. */
static uint64_t s[4] = { 1 };

static __inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

static inline double next(void) {
#include "xoshiro256-next.c"
	// Faster if positioned here
#ifdef DOUBLE_52_BITS
	const union { uint64_t i; double d; } u = { .i = UINT64_C(0x3FF) << 52 | result_plus >> 12 };
	return u.d - 1.0;
#else
	return (result_plus >> 11) * (1. / (UINT64_C(1) << 53));
#endif
}

#define HARNESS_DOUBLE
#include "harness.c"
