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

uint64_t a = 1, b, c, d;

#define rot(x,k) (((x)<<(k))|((x)>>(64-(k))))

static inline uint64_t next(void) {

    uint64_t e = a - rot(b, 7);
    a = b ^ rot(c, 13);
    b = c + rot(d, 37);
    c = d + e;
    d = e + a;
    return d;
}

#define NEXT next()

#include "harness.c"
