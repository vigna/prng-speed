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

/* The state of the generator. As for the other generators, all threads use
   the same generator, starting from different states: the increment is fixed
   (one), rather than different for each thread. */
typedef struct {
	uint64_t state;
} state_t;

#define INIT(st, seed) { st.state = splitmix64(&seed); }

#define INC 1

/* pcg32_random_r() of the minimal C implementation of PCG32 ((c) 2014 M.E.
   O'Neill, https://www.pcg-random.org/, Apache License 2.0), with a fixed
   increment. */

__device__ static inline uint32_t next(state_t *s) {
	const uint64_t oldstate = s->state;
	s->state = oldstate * UINT64_C(6364136223846793005) + INC;
	const uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	const uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

#define HARNESS_ADD
#include "gpu-harness.cu"
