/*
    Copyright (c) 2015 Orson Peters <orsonpeters@gmail.com>

    This software is provided 'as-is', without any express or implied warranty. In no event will the
    authors be held liable for any damages arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose, including commercial
    applications, and to alter it and redistribute it freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that you wrote the
       original software. If you use this software in a product, an acknowledgment in the product
       documentation would be appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be misrepresented as
       being the original software.

    3. This notice may not be removed or altered from any source distribution.

	Modified to return uint64_t by Sebastiano Vigna.

	Modified by Sebastiano Vigna in November 2026 to compute four blocks at a time, following
	ideas from the ChaCha implementation of Alexey L. Voskov in SmokeRand
	(https://github.com/alvoskov/SmokeRand, generators/chacha.c, MIT License), who pointed out
	that the previous version was far from the possible speed:

	- the quarter-round is a long chain of dependent instructions, so a single block leaves most
	  execution units idle: the four blocks of a refill are independent chains (Voskov's AVX2
	  version computes four blocks in two 256-bit chains; here they are four 128-bit chains, as in
	  the AES-CTR generator, which also has four blocks in flight, and as there the loops over
	  the blocks and the rounds are completely unrolled);

	- the rows of the input block are built in registers and added back from registers at the end
	  of the rounds, instead of being assembled in an array, copied to the output buffer, and added
	  from memory (as in Voskov's code); the 64-bit block counter is incremented with a 64-bit
	  vector addition.

	The output sequence is unchanged: block k is still the ChaCha block with counter k.
*/

#include <cstdint>
#include <cstring>
#include <limits>
#include <cstddef>
#include <ostream>
#include <istream>

// Number of blocks computed at a time.
#define CHACHA_BLOCKS 4

template<size_t R>
class ChaCha {
public:
    typedef uint64_t result_type;

    explicit ChaCha(uint64_t seedval, uint64_t stream = 0);
    template<class Sseq> explicit ChaCha(Sseq& seq);

    void seed(uint64_t seedval, uint64_t stream = 0);
    template<class Sseq> void seed(Sseq& seq);

    uint64_t operator()();
    void discard(unsigned long long n);

    template<size_t R_> friend bool operator==(const ChaCha<R_>& lhs, const ChaCha<R_>& rhs);
    template<size_t R_> friend bool operator!=(const ChaCha<R_>& lhs, const ChaCha<R_>& rhs);

    template<typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const ChaCha<R>& rng);

    template<typename CharT, typename Traits>
    friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, ChaCha<R>& rng);

    static constexpr uint64_t min() { return std::numeric_limits<uint64_t>::min(); }
    static constexpr uint64_t max() { return std::numeric_limits<uint64_t>::max(); }

private:
    // Number of 64-bit outputs of a refill.
    static constexpr int OUTPUTS = 8 * CHACHA_BLOCKS;

    void generate_blocks();

    alignas(16) uint32_t block[16 * CHACHA_BLOCKS];
    uint32_t keysetup[8];
    uint64_t ctr;
};


template<size_t R>
inline ChaCha<R>::ChaCha(uint64_t seedval, uint64_t stream) {
    seed(seedval, stream);
}

template<size_t R>
template<class Sseq>
inline ChaCha<R>::ChaCha(Sseq& seq) {
    seed(seq);
}

template<size_t R>
inline void ChaCha<R>::seed(uint64_t seedval, uint64_t stream) {
    ctr = 0;
    keysetup[0] = seedval & 0xffffffffu;
    keysetup[1] = seedval >> 32;
    keysetup[2] = keysetup[3] = 0xdeadbeef;      // Could use 128-bit seed.
    keysetup[4] = stream & 0xffffffffu;
    keysetup[5] = stream >> 32;
    keysetup[6] = keysetup[7] = 0xdeadbeef;      // Could use 128-bit stream.
}

template<size_t R>
template<class Sseq>
inline void ChaCha<R>::seed(Sseq& seq) {
    ctr = 0;
    seq.generate(keysetup, keysetup + 8);
}


template<size_t R>
inline uint64_t ChaCha<R>::operator()() {
    int idx = ctr % OUTPUTS;
    if (idx == 0) generate_blocks();
    ++ctr;

    uint64_t result;
    memcpy(&result, &block[idx * 2], sizeof result);
    return result;
}

template<size_t R>
inline void ChaCha<R>::discard(unsigned long long n) {
    int idx = ctr % OUTPUTS;
    ctr += n;
    // If idx is zero, the buffer is stale, as operator() refills it lazily (the previous
    // version did not check this case, and returned stale outputs after, e.g., discard(1)
    // on a new generator).
    if ((idx == 0 || idx + n >= OUTPUTS) && ctr % OUTPUTS != 0) generate_blocks();
}

#if defined(__SSE2__) && !defined(CHACHA_NO_SIMD)
#include "emmintrin.h"

// Get an efficient _mm_roti_epi32 based on enabled features.
#if !defined(__XOP__)
    #if defined(__SSSE3__)
        #include <tmmintrin.h>
        #define _mm_roti_epi32(r, c) (                              \
            ((c) == 8) ?                                            \
                _mm_shuffle_epi8((r), _mm_set_epi8(14, 13, 12, 15,  \
                                                   10,  9,  8, 11,  \
                                                    6,  5,  4,  7,  \
                                                    2,  1,  0,  3)) \
            : ((c) == 16) ?                                         \
                _mm_shuffle_epi8((r), _mm_set_epi8(13, 12, 15, 14,  \
                                                    9,  8, 11, 10,  \
                                                    5,  4,  7,  6,  \
                                                    1,  0,  3,  2)) \
            : ((c) == 24) ?                                         \
                _mm_shuffle_epi8((r), _mm_set_epi8(12, 15, 14, 13,  \
                                                    8, 11, 10,  9,  \
                                                    4,  7,  6,  5,  \
                                                    0,  3,  2,  1)) \
            :                                                       \
                _mm_xor_si128(_mm_slli_epi32((r), (c)),             \
                              _mm_srli_epi32((r), 32-(c)))          \
        )
    #else
        #define _mm_roti_epi32(r, c) _mm_xor_si128(_mm_slli_epi32((r), (c)), \
                                                   _mm_srli_epi32((r), 32-(c)))
    #endif
#else
    #include <xopintrin.h>
#endif

template<size_t R>
inline void ChaCha<R>::generate_blocks() {
    // ROTVn rotates the elements in the given vector n places to the left.
    #define CHACHA_ROTV1(x) _mm_shuffle_epi32((__m128i) x, 0x39)
    #define CHACHA_ROTV2(x) _mm_shuffle_epi32((__m128i) x, 0x4e)
    #define CHACHA_ROTV3(x) _mm_shuffle_epi32((__m128i) x, 0x93)

    // The rows of the input block; words 12 and 13 are the 64-bit block counter.
    const uint64_t first = ctr / OUTPUTS * CHACHA_BLOCKS;
    const __m128i a0 = _mm_set_epi32(0x6b206574, 0x79622d32, 0x3320646e, 0x61707865);
    const __m128i b0 = _mm_loadu_si128((const __m128i*) (keysetup));
    const __m128i c0 = _mm_loadu_si128((const __m128i*) (keysetup + 4));
    const __m128i d0 = _mm_set_epi32(0xdeadbeef, 0xdeadbeef, first >> 32, first & 0xffffffffu);

    __m128i a[CHACHA_BLOCKS], b[CHACHA_BLOCKS], c[CHACHA_BLOCKS], d[CHACHA_BLOCKS];
    // As in the AES-CTR generator, the pragmas unroll the loops over the blocks and the
    // rounds also under -fno-unroll-loops, which in Clang (but not in GCC) disables complete
    // unrolling of loops with a constant number of iterations.
    #pragma GCC unroll 16
    for (int j = 0; j < CHACHA_BLOCKS; ++j) {
        a[j] = a0;
        b[j] = b0;
        c[j] = c0;
        d[j] = _mm_add_epi64(d0, _mm_set_epi64x(0, j));
    }

    #pragma GCC unroll 16
    for (size_t i = 0; i < R; i += 2) {
        #pragma GCC unroll 16
        for (int j = 0; j < CHACHA_BLOCKS; ++j) {
            a[j] = _mm_add_epi32(a[j], b[j]);
            d[j] = _mm_xor_si128(d[j], a[j]);
            d[j] = _mm_roti_epi32(d[j], 16);
            c[j] = _mm_add_epi32(c[j], d[j]);
            b[j] = _mm_xor_si128(b[j], c[j]);
            b[j] = _mm_roti_epi32(b[j], 12);
            a[j] = _mm_add_epi32(a[j], b[j]);
            d[j] = _mm_xor_si128(d[j], a[j]);
            d[j] = _mm_roti_epi32(d[j], 8);
            c[j] = _mm_add_epi32(c[j], d[j]);
            b[j] = _mm_xor_si128(b[j], c[j]);
            b[j] = _mm_roti_epi32(b[j], 7);

            b[j] = CHACHA_ROTV1(b[j]);
            c[j] = CHACHA_ROTV2(c[j]);
            d[j] = CHACHA_ROTV3(d[j]);

            a[j] = _mm_add_epi32(a[j], b[j]);
            d[j] = _mm_xor_si128(d[j], a[j]);
            d[j] = _mm_roti_epi32(d[j], 16);
            c[j] = _mm_add_epi32(c[j], d[j]);
            b[j] = _mm_xor_si128(b[j], c[j]);
            b[j] = _mm_roti_epi32(b[j], 12);
            a[j] = _mm_add_epi32(a[j], b[j]);
            d[j] = _mm_xor_si128(d[j], a[j]);
            d[j] = _mm_roti_epi32(d[j], 8);
            c[j] = _mm_add_epi32(c[j], d[j]);
            b[j] = _mm_xor_si128(b[j], c[j]);
            b[j] = _mm_roti_epi32(b[j], 7);

            b[j] = CHACHA_ROTV3(b[j]);
            c[j] = CHACHA_ROTV2(c[j]);
            d[j] = CHACHA_ROTV1(d[j]);
        }
    }

    #pragma GCC unroll 16
    for (int j = 0; j < CHACHA_BLOCKS; ++j) {
        __m128i* out = (__m128i*) (block + 16 * j);
        _mm_store_si128(out, _mm_add_epi32(a[j], a0));
        _mm_store_si128(out + 1, _mm_add_epi32(b[j], b0));
        _mm_store_si128(out + 2, _mm_add_epi32(c[j], c0));
        _mm_store_si128(out + 3, _mm_add_epi32(d[j], _mm_add_epi64(d0, _mm_set_epi64x(0, j))));
    }

    #undef CHACHA_ROTV3
    #undef CHACHA_ROTV2
    #undef CHACHA_ROTV1
}
#else
template<size_t R>
inline void ChaCha<R>::generate_blocks() {
    #define CHACHA_ROTL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

    #define CHACHA_QUARTERROUND(x, a, b, c, d) \
        x[a] = x[a] + x[b]; x[d] ^= x[a]; x[d] = CHACHA_ROTL32(x[d], 16); \
        x[c] = x[c] + x[d]; x[b] ^= x[c]; x[b] = CHACHA_ROTL32(x[b], 12); \
        x[a] = x[a] + x[b]; x[d] ^= x[a]; x[d] = CHACHA_ROTL32(x[d],  8); \
        x[c] = x[c] + x[d]; x[b] ^= x[c]; x[b] = CHACHA_ROTL32(x[b],  7)

    const uint64_t first = ctr / OUTPUTS * CHACHA_BLOCKS;
    for (int j = 0; j < CHACHA_BLOCKS; ++j) {
        uint32_t input[16] = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574};
        for (int i = 0; i < 8; ++i) input[4 + i] = keysetup[i];
        input[12] = (first + j) & 0xffffffffu;
        input[13] = (first + j) >> 32;
        input[14] = input[15] = 0xdeadbeef; // Could use 128-bit counter.

        uint32_t* x = block + 16 * j;
        for (int i = 0; i < 16; ++i) x[i] = input[i];
        for (size_t i = 0; i < R; i += 2) {
            CHACHA_QUARTERROUND(x, 0, 4,  8, 12);
            CHACHA_QUARTERROUND(x, 1, 5,  9, 13);
            CHACHA_QUARTERROUND(x, 2, 6, 10, 14);
            CHACHA_QUARTERROUND(x, 3, 7, 11, 15);
            CHACHA_QUARTERROUND(x, 0, 5, 10, 15);
            CHACHA_QUARTERROUND(x, 1, 6, 11, 12);
            CHACHA_QUARTERROUND(x, 2, 7,  8, 13);
            CHACHA_QUARTERROUND(x, 3, 4,  9, 14);
        }
        for (int i = 0; i < 16; ++i) x[i] += input[i];
    }

    #undef CHACHA_QUARTERROUND
    #undef CHACHA_ROTL32
}
#endif


// Implement <random> interface.
template<size_t R>
inline bool operator==(const ChaCha<R>& lhs, const ChaCha<R>& rhs) {
    for (int i = 0; i < 8; ++i) {
        if (lhs.keysetup[i] != rhs.keysetup[i]) return false;
    }

    return lhs.ctr == rhs.ctr;
}

template<size_t R>
inline bool operator!=(const ChaCha<R>& lhs, const ChaCha<R>& rhs) { return !(lhs == rhs); }

template<size_t R, typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const ChaCha<R>& rng) {
    typedef typename std::basic_ostream<CharT, Traits>::ios_base ios_base;

    // Save old state.
    auto flags = os.flags();
    auto fill = os.fill();
    
    // Set flags and fill to space.
    auto space = os.widen(' ');
    os.flags(ios_base::dec | ios_base::fixed | ios_base::left);
    os.fill(space);

    // Serialize.
    for (int i = 0; i < 8; ++i) os << rng.keysetup[i] << space;
    os << rng.ctr;

    // Sestore old state.
    os.flags(flags);
    os.fill(fill);

    return os;
}

template<size_t R, typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, ChaCha<R>& rng) {
    typedef typename std::basic_istream<CharT, Traits> ::ios_base ios_base;

    // Save old flags and set ours.
    auto flags = is.flags();
    is.flags(ios_base::dec);

    // Deserialize.
    for (int i = 0; i < 8; ++i) is >> rng.keysetup[i];
    is >> rng.ctr;

    // Restore old flags.
    is.flags(flags);

    return is;
}