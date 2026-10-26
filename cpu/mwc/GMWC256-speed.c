/*		 
 * Copyright (C) 2020 Sebastiano Vigna 
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

#define GMWC_MINUSA0 0x34dabaf938e45b
#define GMWC_A0INV 0xac8f71df86d6542d
#define GMWC_A3 0xff76e600f25ae046

/* The current state of the generator. */
uint64_t x, y, z;
uint64_t c = 2 * (uint64_t)GMWC_MINUSA0;

static uint64_t inline next(void) {
	const uint64_t result = z;
	const __uint128_t t = GMWC_A3 * (__uint128_t)x + c;
	x = y;
	y = z;
	z = GMWC_A0INV * (uint64_t)t;
	c = (t + GMWC_MINUSA0 * (__uint128_t)z) >> 64;
	return result;
}

#define HARNESS_ADD
#include "harness.c"
