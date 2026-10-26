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

static uint32_t x = 123456789,y = 362436000,z = 521288629,c = 7654321;

static uint64_t inline next(void) {
	x = 69069*x+12345;
	y ^= (y<<13); y ^= (y>>17); y ^= (y<<5);
	uint64_t t = UINT64_C(698769069)*z+c; c = (t>>32);
	return x+y+(z=t);
}

#define NEXT next() ^ next()

#define HARNESS_ADD
#include "harness.c"
