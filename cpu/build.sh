#!/bin/bash
# Usage: build.sh [TABLE...]. Builds every generator of gens.tsv (or those of the
# given tables: main, fp, vect, paper, other) with GCC (into gcc/) and Clang (into
# clang/), always with -march=native and no other limitation. GCC and CLANG name
# the compilers (default gcc and clang; GXX and CLANGXX the C++ compilers).
#
# Scalar generators use -fno-unroll-loops, which keeps the harness loop rolled;
# vectorized ones (vect) do not, and GCC also gets -fdisable-tree-cunrolli,
# without which it does not vectorize them. The novl entries (GMWC) add
# -mno-avx512vl, as GCC with AVX-512 moves the 128-bit intermediate values of the
# plain generators through vector registers. The novec entries disable
# auto-vectorization, which turns the harness loop of SplitMix (both compilers)
# and of wyrand (Clang) into vector code. The cxx entries are C++. Sources are in
# subdirectories by family; -iquote . lets them include harness.c.
cd "$(dirname "$0")"
GCC=${GCC:-gcc}
CLANG=${CLANG:-clang}
GXX=${GXX:-$(echo "$GCC" | sed 's/gcc\([^/]*\)$/g++\1/')}
CLANGXX=${CLANGXX:-$(echo "$CLANG" | sed 's/clang\([^/]*\)$/clang++\1/')}
echo "gcc: $($GCC --version | head -1)"
echo "clang: $($CLANG --version | head -1)"
failed=0
for cc in gcc clang; do
	mkdir -p $cc
	while IFS=$'\t' read -r name table src kind extra; do
		(( $# )) && [[ " $* " != *" $table "* ]] && continue
		if [[ $cc == gcc ]]; then CC=$GCC CXX=$GXX; else CC=$CLANG CXX=$CLANGXX; fi
		BASE="-Wall -Wno-unused-variable -O3 -march=native -iquote ."
		case $kind in
			int) FLAGS="-fno-unroll-loops $BASE -std=c99" ;;
			novec) FLAGS="-fno-unroll-loops $BASE -std=c99"
				[[ $cc == gcc ]] && FLAGS="$FLAGS -fno-tree-vectorize" || FLAGS="$FLAGS -fno-vectorize -fno-slp-vectorize" ;;
			novl) FLAGS="-fno-unroll-loops $BASE -mno-avx512vl -std=c99" ;;
			vect) FLAGS="$BASE -ftree-vectorize -std=c99"
				[[ $cc == gcc ]] && FLAGS="$FLAGS -fdisable-tree-cunrolli" ;;
			cxx) FLAGS="-fno-unroll-loops $BASE -x c++"; CC=$CXX ;;
			*) echo "Unknown kind $kind for $name" >&2; exit 1 ;;
		esac
		echo "$cc $name: $FLAGS $extra $src"
		# A failed build must not leave the binary of a previous build in place
		rm -f "$cc/$name"
		$CC $FLAGS $extra "$src" -o "$cc/$name" || { echo "FAILED: $cc $name" >&2; failed=1; }
	done < <(grep -v '^#' gens.tsv)
done
exit $failed
