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

/* The CPU harness. The including file must define next(), returning a 64-bit
   output, or NEXT, an expression returning a 64-bit output (32-bit generators
   glue two outputs, usually as next() ^ next()). It may also define

   - INIT, a statement initializing the generator;
   - TAIL, a statement executed at the end;
   - HARNESS_ADD, to accumulate outputs with an addition instead of a xor, as
     the operation must differ from the last one of the output: otherwise, the
     compiler could merge the two and skip part of the computation;
   - HARNESS_DOUBLE, if NEXT returns a double;
   - OUTPUTS, the number of 64-bit outputs computed by each evaluation of NEXT
     (default 1), as for the vectorized sources, which compute 1024.

   The harness evaluates NEXT as many times as its argument says, accumulating
   the results, and prints the user time per 64-bit output (ns/word). */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifndef INIT
#define INIT
#endif

#ifndef TAIL
#define TAIL ;
#endif

#ifndef NEXT
#define NEXT next()
#endif

#ifndef OUTPUTS
#define OUTPUTS 1
#endif

uint64_t get_user_time(void) {
	struct rusage rusage;
	getrusage(0, &rusage);
	return rusage.ru_utime.tv_sec * 1000000000ULL + rusage.ru_utime.tv_usec * 1000ULL;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "USAGE: %s ITERATIONS\n", argv[0]);
		exit(1);
	}
	const long long int n = strtoll(argv[1], NULL, 0);
	if (n <= 0) {
		fprintf(stderr, "The number of iterations must be positive\n");
		exit(1);
	}

	INIT // Here you can optionally inject initialization code

	const uint64_t start_time = get_user_time();

	uint64_t e = -1;
#ifdef HARNESS_DOUBLE
	union { double d; uint64_t i; } uu;

	// NEXT returns a double
	for(int64_t i = n; i-- != 0;) {
		uu.d = NEXT; // Measurement
		e ^= uu.i;
	}
#else
	// NEXT returns a 64-bit integer
	for(int64_t i = n; i-- != 0;) {
#ifdef HARNESS_ADD
		e += NEXT; // Measurement
#else
		e ^= NEXT; // Measurement
#endif
	}
#endif

	const uint64_t time_delta = (get_user_time() - start_time);
	const double words = (double)n * OUTPUTS;
	printf("%s: %.03f s, %.03f GB/s, %.03f words/ns, %.03f ns/word\n", argv[0], time_delta / 1E9, (words * 8.) / time_delta, words / time_delta, time_delta / words);

	const volatile uint64_t unused = e;

	TAIL // Here you can add instructions that must be executed at the end, usually to avoid dead-code elimination

	return 0;
}
