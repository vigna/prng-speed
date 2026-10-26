/* Mark A. Overton's RomuDuoJr ("Romu: Fast nonlinear pseudo-random number generators
   providing high quality", arXiv:2002.11331, 2020; https://www.romu-random.org/),
   in the reference code. */

#include <stdint.h>

#define ROTL(d,lrot) ((d<<(lrot)) | (d>>(8*sizeof(d)-(lrot))))

uint64_t xState = 1, yState = 2; // set to nonzero seed

uint64_t next() {
	uint64_t xp = xState;
	xState = 15241094284759029579u * yState;
	yState = yState - xp;  yState = ROTL(yState,27);
	return xp;
}

#include "harness.c"
