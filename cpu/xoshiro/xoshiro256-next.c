/*

This code implements multiplication by the matrix underlying a xoshiro256 generator.
It assumes that the state has been declared as

uint64_t s[4];

The parameters that must be defined are:

- A: the shift (left).
- B: the rotation (left).

*/

// Faster with previous-state data
#ifdef R
const uint64_t result_pp = rotl(s[0] + s[3], R) + s[0];
#endif

#if defined(S) && defined (R) && defined(T)
const uint64_t result_ss = rotl(s[1] * S, R) * T;
#endif

const uint64_t result_plus = s[0] + s[3];

const uint64_t t = s[1] << A;

s[2] ^= s[0];
s[3] ^= s[1];
s[1] ^= s[2];
s[0] ^= s[3];

s[2] ^= t;

// Faster with next-state data

const uint64_t result_star = s[1] * 0x9e3779b97f4a7c13;

s[3] = rotl(s[3], B);

