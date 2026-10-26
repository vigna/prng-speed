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

#define MWC_A3 0xffcccc66

/* The state of the generator. */
typedef struct {
	uint32_t x, y, z, c;
} state_t;

#define INIT(s, seed) { const uint64_t r = splitmix64(&seed); s.x = r; s.y = r >> 32; s.z = splitmix64(&seed); s.c = 1; }

/* The scrambler X: the output is the most recent digit of the current state
   xored with the carry. */

__device__ static inline uint32_t next(state_t *s) {
	const uint32_t result = s->z ^ s->c;
	const uint64_t t = MWC_A3 * (uint64_t)s->x + s->c;
	s->x = s->y;
	s->y = s->z;
	s->z = t;
	s->c = t >> 32;
	return result;
}

#define HARNESS_ADD
#include "gpu-harness.cu"
