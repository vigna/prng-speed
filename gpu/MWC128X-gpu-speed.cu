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

#define MWC_A1 0xff6da236274e252e

/* The state of the generator. */
typedef struct {
	uint64_t x, c;
} state_t;

#define INIT(s, seed) { s.x = splitmix64(&seed); s.c = 1; }

/* The scrambler X: the output is the most recent digit of the current state
   xored with the carry. As in the CPU version, the step is computed using
   128-bit integers, which nvcc supports in device code since CUDA 11.5,
   emulating them using 32-bit multiplications and additions with carry;
   before, it is computed using __umul64hi(). */

#if __CUDACC_VER_MAJOR__ > 11 || (__CUDACC_VER_MAJOR__ == 11 && __CUDACC_VER_MINOR__ >= 5)
__device__ static inline uint64_t next(state_t *s) {
	const uint64_t result = s->x ^ s->c;
	const unsigned __int128 t = MWC_A1 * (unsigned __int128)s->x + s->c;
	s->x = t;
	s->c = t >> 64;
	return result;
}
#else
__device__ static inline uint64_t next(state_t *s) {
	const uint64_t result = s->x ^ s->c;
	const uint64_t lo = MWC_A1 * s->x;
	const uint64_t hi = __umul64hi(MWC_A1, s->x);
	s->x = lo + s->c;
	s->c = hi + (s->x < lo);
	return result;
}
#endif

/* Each output contains two 32-bit words. */
#define RESULT_T uint64_t
#define HARNESS_ADD
#include "gpu-harness.cu"
