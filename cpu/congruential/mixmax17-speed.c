#include <stdint.h>

/* MIXMAX with N = 17 (K. Savvidy, "The MIXMAX random number generator", Comp.
   Phys. Commun. 196, pp. 161-165, 2015), the default engine of CLHEP (and thus
   of Geant4) and ROOT's TRandomMixMax17: a matrix generator over the field of
   size p = 2^61 - 1, ported from the C source distributed with ROOT (mixmax.h
   and mixmax.icc: iterate_raw_vec, get_next and seed_spbox; SPECIAL = 0,
   SPECIALMUL = 36; by Konstantin Savvidy, released under the GNU Lesser
   General Public License v3). Every application of the matrix computes a new
   vector of 17 elements, which are then returned as outputs starting from the
   second one.
   Each output has 61 significant bits; as in the modular rank and
   linear-complexity tests of modlin (https://github.com/vigna/modlin-rs), it is
   left-justified into 64 bits. CLHEP unrolls by hand the loop of
   iterate_raw_vec, so we ask the compiler to unroll it, too, which
   -fno-unroll-loops would otherwise prevent.

   The state is seeded by seed_spbox(1234567890123456789): the first ten raw
   outputs are then those that ROOT's unmodified source prints, which are
   checked by the tests of modlin. */

#define MIXMAX_N 17
#define MIXMAX_SPECIALMUL 36
#define MIXMAX_BITS 61
#define MERSBASE ((UINT64_C(1) << MIXMAX_BITS) - 1)

static uint64_t V[MIXMAX_N], sumtot;
static int counter;

static inline uint64_t mod_mersenne(const uint64_t k) {
	return (k & MERSBASE) + (k >> MIXMAX_BITS);
}

static inline uint64_t modadd(const uint64_t a, const uint64_t b) {
	return mod_mersenne(a + b);
}

/* Multiplication by 2^SPECIALMUL modulo p, that is, a 61-bit rotation. */
static inline uint64_t mulwu(const uint64_t k) {
	return ((k << MIXMAX_SPECIALMUL) & MERSBASE) | (k >> (MIXMAX_BITS - MIXMAX_SPECIALMUL));
}

static void iterate_raw_vec(void) {
	uint64_t temp_v = sumtot, temp_p = 0, sum = temp_v, ovflow = 0;
	V[0] = temp_v;
#pragma GCC unroll 16
	for (int i = 1; i < MIXMAX_N; i++) {
		const uint64_t temp_po = mulwu(temp_p);
		temp_p = modadd(temp_p, V[i]);
		temp_v = mod_mersenne(temp_v + temp_p + temp_po);
		V[i] = temp_v;
		sum += temp_v;
		ovflow += sum < temp_v;
	}
	sumtot = mod_mersenne(mod_mersenne(sum) + (ovflow << 3));
}

static inline uint64_t next(void) {
	if (counter < MIXMAX_N) return V[counter++] << 3;
	iterate_raw_vec();
	counter = 2;
	return V[1] << 3;
}

static void seed_spbox(uint64_t seed) {
	uint64_t sum = 0, ovflow = 0;
	for (int i = 0; i < MIXMAX_N; i++) {
		seed *= UINT64_C(6364136223846793005);
		seed = (seed << 32) ^ (seed >> 32);
		V[i] = seed & MERSBASE;
		sum += V[i];
		ovflow += sum < V[i];
	}
	sumtot = mod_mersenne(mod_mersenne(sum) + (ovflow << 3));
	counter = MIXMAX_N;
}

#define INIT seed_spbox(UINT64_C(1234567890123456789));

#include "harness.c"
