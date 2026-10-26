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

#include "dSFMT/dSFMT.h"
#include "dSFMT/dSFMT.c"

#define INIT dsfmt_init_gen_rand(&dsfmt_global_data, -1);

#ifdef HARNESS_DOUBLE
#define NEXT dsfmt_gv_genrand_close_open()
#else
#define HARNESS_ADD
#define NEXT dsfmt_gv_genrand_uint32() ^ dsfmt_gv_genrand_uint32()
#endif

#define HARNESS_ADD
#include "harness.c"
