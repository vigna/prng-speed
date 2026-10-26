/*		 
 * Copyright (C) 2014 Sebastiano Vigna 
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
#include <stdio.h>

static uint64_t a = 1, b = 1, c = 1, s = 1, counter = 1;

/* SFC64 as in PractRand 0.96 (src/RNGs/sfc.cpp), with shifts {25, 11, 3}.
   Until October 2026 this file used {25, 12, 3}, the shifts of SFC64 before
   PractRand 0.93; PractRand 0.94 uses {24, 11, 3}, and PractRand 0.96 changed
   them again to {25, 11, 3}, which its release notes do not mention. Only the
   shift amounts change, so timings are not affected. */

static inline uint64_t next(void) {
   enum {BARREL_SHIFT = 25, RSHIFT = 11, LSHIFT = 3};
	const uint64_t tmp = a + b + counter++;
	a = b ^ (b >> RSHIFT);
	b = c + (c << LSHIFT);
	c = ((c << BARREL_SHIFT) | (c >> (64-BARREL_SHIFT))) + tmp;
   return tmp;
}

#include "harness.c"
