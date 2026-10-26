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

/* Philox4x32-10 from the cuRAND device API. We call curand4(), which returns
   the four 32-bit words of a block at once. The time includes cuRAND's
   bookkeeping (curand4() computes the next block in advance and selects the
   output according to the state, and the 128-bit counter is incremented with
   a chain of branches), so it is that of cuRAND's Philox4x32-10, rather than
   that of a minimal implementation. */

typedef curandStatePhilox4_32_10_t state_t;

#define INIT(s, seed) curand_init(seed, 0, 0, &s)
#define WORDS 4
#define ACCUMULATE(e, s) { const uint4 r = curand4(&s); e += r.x + r.y + r.z + r.w; }

#include "gpu-harness.cu"
