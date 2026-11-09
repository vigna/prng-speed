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

uint64_t x;

/* Sirius64 by Matteo Zapparoli (https://github.com/matteo65/Sirius64). The
   output ends with a multiplication, so the default xor accumulation of the
   harness cannot be merged with it. */

static inline uint64_t next(void) {
	uint64_t z = (x += 0x9e3779b97f4a7c15);
	z = 0x9e3779b97f4a7c15 * (z ^ (z >> 17));
	z = (z << 32) | (z >> 32);
	return 0x9e3779b97f4a7c15 * (x ^ z ^ (z >> 17));
}

#include "harness.c"
