/* Mark A. Overton's RomuTrio ("Romu: Fast nonlinear pseudo-random number generators
   providing high quality", arXiv:2002.11331, 2020; https://www.romu-random.org/),
   in the reference code. */

#include <stdint.h>

#define ROTL(d,lrot) ((d<<(lrot)) | (d>>(8*sizeof(d)-(lrot))))

uint64_t xState = 1, yState = 2, zState = 3; // set to nonzero seed

uint64_t next() {
	uint64_t xp = xState, yp = yState, zp = zState;
	xState = 15241094284759029579u * zp;
	yState = yp - xp; yState = ROTL(yState,12);
	zState = zp - yp; zState = ROTL(zState,44);
	return xp;
}

#include "harness.c"
