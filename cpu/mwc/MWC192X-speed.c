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

#define MWC_A2 0xff4e24dd418f5433

/* The current state of the generator. */
uint64_t x, y;
uint64_t c = 1;

/* The scrambler X: the output is the most recent digit of the current state
   xored with the carry. */

static uint64_t inline next(void) {
	const uint64_t result = y ^ c;
	const __uint128_t t = MWC_A2 * (__uint128_t)x + c;
	x = y;
	y = t;
	c = t >> 64;
	return result;
}

#define HARNESS_ADD
#include "harness.c"
