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

#define GMWC_MINUSA0 0x68935f57
#define GMWC_A0INV 0x8316d3ad5de20399
#define GMWC_A1 0xfff0cfb4ed6366fa

/* The current state of the generator. */
uint64_t x;
uint64_t c = 2 * (uint64_t)GMWC_MINUSA0;

/* The scrambler X, which generalizes Thomas's scrambler of MWC generators: the
   output is the new digit xored with the upper half of t (the new digit xored
   with the new carry would be biased, as the range of the carry depends on the
   digit). */

static uint64_t inline next(void) {
	const __uint128_t t = GMWC_A1 * (__uint128_t)x + c;
	x = GMWC_A0INV * (uint64_t)t;
	c = (t + GMWC_MINUSA0 * (__uint128_t)x) >> 64;
	return x ^ (uint64_t)(t >> 64);
}

#define HARNESS_ADD
#include "harness.c"
