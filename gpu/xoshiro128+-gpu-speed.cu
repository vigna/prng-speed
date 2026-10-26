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

/* The state of the generator. */
typedef struct {
	uint32_t s[4];
} state_t;

#define INIT(st, seed) { const uint64_t r0 = splitmix64(&seed), r1 = splitmix64(&seed); st.s[0] = r0; st.s[1] = r0 >> 32; st.s[2] = r1; st.s[3] = r1 >> 32; }

__device__ static inline uint32_t rotl(const uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}

/* xoshiro128+ 1.0 (https://prng.di.unimi.it/xoshiro128plus.c). */

__device__ static inline uint32_t next(state_t *st) {
	uint32_t *s = st->s;
	const uint32_t result = s[0] + s[3];
	const uint32_t t = s[1] << 9;
	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];
	s[2] ^= t;
	s[3] = rotl(s[3], 11);
	return result;
}

#include "gpu-harness.cu"
