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

uint64_t x;

/* wyrand from wyhash final 4 (https://github.com/wangyi-fudan/wyhash). */

static inline uint64_t next(void) {
	x += 0x2d358dccaa6c78a5;
	const __uint128_t t = (__uint128_t)(x ^ 0x8bb84b93962eacc9) * x;
	return	(t >> 64) ^ t;
}

#define HARNESS_ADD
#include "harness.c"
