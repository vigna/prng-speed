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
	uint32_t a, b, c, counter;
} state_t;

#define INIT(s, seed) { const uint64_t r = splitmix64(&seed); s.a = r; s.b = r >> 32; s.c = splitmix64(&seed); s.counter = 1; }

/* SFC32 as in PractRand 0.96 (src/RNGs/sfc.cpp), with shifts {21, 9, 3}. */

__device__ static inline uint32_t next(state_t *s) {
	enum {BARREL_SHIFT = 21, RSHIFT = 9, LSHIFT = 3};
	const uint32_t tmp = s->a + s->b + s->counter++;
	s->a = s->b ^ (s->b >> RSHIFT);
	s->b = s->c + (s->c << LSHIFT);
	s->c = ((s->c << BARREL_SHIFT) | (s->c >> (32 - BARREL_SHIFT))) + tmp;
	return tmp;
}

#include "gpu-harness.cu"
