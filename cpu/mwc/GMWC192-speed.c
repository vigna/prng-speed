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

/* m = GMWC_A2 * 2^128 + GMWC_MINUSA0 is a safe prime, but this pair has not
   been selected by its figures of merit: it is meant for timing only. */

#define GMWC_MINUSA0 0xb4fc961f
#define GMWC_A0INV 0xebb96d85ae9e9a21
#define GMWC_A2 0xffaf1260e39d9b7f

/* The current state of the generator. */
uint64_t x, y;
uint64_t c = 2 * (uint64_t)GMWC_MINUSA0;

static uint64_t inline next(void) {
	const uint64_t result = y;
	const __uint128_t t = GMWC_A2 * (__uint128_t)x + c;
	x = y;
	y = GMWC_A0INV * (uint64_t)t;
	c = (t + GMWC_MINUSA0 * (__uint128_t)y) >> 64;
	return result;
}

#define HARNESS_ADD
#include "harness.c"
