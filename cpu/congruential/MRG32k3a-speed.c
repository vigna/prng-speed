/*  Written in 2019 by Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

/* Pierre L'Ecuyer's MRG32k3a ("Good parameters and implementations for
   combined multiple recursive random number generators", Oper. Res. 47(1),
   pp. 159-164, 1999), in the fast, branchless implementation based on 64-bit
   integers of https://github.com/vigna/MRG32k3a. It returns a double in
   (0..1), so the time is per double. The state is L'Ecuyer's standard seed
   (12345 in all six components); as in MRG32k3a_init6(), INIT discards one
   output to align the stream with L'Ecuyer's original implementation. */

#include <stdint.h>

static int64_t s10 = 12345, s11 = 12345, s12 = 12345, s20 = 12345, s21 = 12345, s22 = 12345;

static inline double next(void) {
	const int64_t m1 = INT64_C(4294967087);
	const int64_t m2 = INT64_C(4294944443);
	const int32_t a12 = INT32_C(1403580);
	const int32_t a13 = INT32_C(810728);
	const int32_t a21 = INT32_C(527612);
	const int32_t a23 = INT32_C(1370589);
	const int64_t corr1 = (m1 * a13);
	const int64_t corr2 = (m2 * a23);
	// This is equivalent to 2.328306549295727688e-10, but more precise as there's no rounding involved.
	const double norm = 0x1.000000d00000bp-32;

	int64_t p, r;

	/* Combination */
	r = s12 - s22;
	r -= m1 * ((r - 1) >> 63);

	/* Component 1 */
	p = (a12 * s11 - a13 * s10 + corr1) % m1;
	s10 = s11;
	s11 = s12;
	s12 = p;

	/* Component 2 */
	p = (a21 * s22 - a23 * s20 + corr2) % m2;
	s20 = s21;
	s21 = s22;
	s22 = p;

	return r * norm;
}

#define INIT next();
#define HARNESS_DOUBLE
#include "harness.c"
