/*

This code implements multiplication by the matrix underlying a xoshiro512 generator.
It assumes that the state has been declared as

uint64_t s[8];

The parameters that must be defined are:

- A: the shift (left).
- B: the rotation (left).

*/

#if defined(S) && defined (R) && defined(T)
const uint64_t result_ss = rotl(s[1] * S, R) * T;
#endif

const uint64_t result_plus = s[0] + s[2];
#ifdef R
const uint64_t result_pp = rotl(result_plus, R) + s[2];
#endif

const uint64_t result_star = s[1] * 0x9e3779b97f4a7c13;

const uint64_t t = s[1] << A;

s[2] ^= s[0];
s[5] ^= s[1];
s[1] ^= s[2];
s[7] ^= s[3];
s[3] ^= s[4];
s[4] ^= s[5];
s[0] ^= s[6];
s[6] ^= s[7];

s[6] ^= t;

s[7] = rotl(s[7], B);
