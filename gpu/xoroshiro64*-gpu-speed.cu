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
	uint32_t s[2];
} state_t;

#define INIT(st, seed) { const uint64_t r = splitmix64(&seed); st.s[0] = r; st.s[1] = r >> 32; }

__device__ static inline uint32_t rotl(const uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}

/* xoroshiro64* 1.0 (https://prng.di.unimi.it/xoroshiro64star.c). */

__device__ static inline uint32_t next(state_t *st) {
	uint32_t *s = st->s;
	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = s0 * 0x9E3779BB;
	s1 ^= s0;
	s[0] = rotl(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl(s1, 13); // c
	return result;
}

#include "gpu-harness.cu"
