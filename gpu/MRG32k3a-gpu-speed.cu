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
#include <curand_kernel.h>

/* L'Ecuyer's MRG32k3a from the cuRAND device API. Seeds are distinct and
   subsequences are not used, as skipping ahead to a subsequence in each thread
   would make initialization very slow. */

typedef curandStateMRG32k3a_t state_t;

#define INIT(s, seed) curand_init(seed, 0, 0, &s)

__device__ static inline uint32_t next(state_t *s) {
	return curand(s);
}

#include "gpu-harness.cu"
