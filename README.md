# prng-speed

Speed tests of pseudorandom number generators on CPUs and NVIDIA GPUs: the
code behind the timings of the [PRNG Shootout](https://prng.di.unimi.it/) and of
the paper [“Scrambled Multiply-with-Carry Pseudorandom Number
Generators”](https://arxiv.org/abs/2610.27313).

## Layout

- `cpu/`: speed tests on CPUs.
  - `harness.c`: the harness. A speed test defines `next()` and includes the
    harness, which calls it as many times as the argument says, accumulates the
    outputs, and prints the time per 64-bit output (`ns/word`); the comment at
    the start of the harness lists the macros a speed test can define.
  - `gens.tsv`: the generators (name, table, source, kind, extra flags). The
    tables `main`, `fp`, and `vect` are those of the website; `paper` contains
    the rest of the speed table of the MWC paper; `other` contains other
    generators of the website and past comparators.
  - `build.sh`, `run.sh`, `go.sh`, `summarize.py`: a session (see below).
  - The speed tests, `*-speed.c` (one generator each; `+` and `*` in the names
    are the scramblers, as in `xoroshiro128++-speed.c`), by family:
    - `xoshiro/`: xoshiro, xoroshiro, and xorshift generators. `*-next.c` are
      their steps, parameterized by `-DA=...` etc.; `-vect-` sources run 16
      instances of a generator at once, with 1024 outputs per call, so that the
      compiler can vectorize them.
    - `mwc/`: multiply-with-carry generators (MWC, GMWC, CMWC).
    - `congruential/`: LCGs and MCGs (PCG, Lehmer, `java.util.Random`),
      MRG32k3a, MIXMAX.
    - `f2-linear/`: other generators based on linear recurrences over **F**₂: the
      Mersenne Twister, SFMT and dSFMT (with their sources), WELL, MELG,
      LFSR258, XSadd.
    - `crypto/`: AES in counter mode, ChaCha8 and ChaCha20 (SSE2).
    - `other/`: SFC64, SplitMix, wyrand, Romu, JSF, KISS, MSWS.
- `gpu/`: speed tests on NVIDIA GPUs: the harness `gpu-harness.cu`, the speed
  tests `*-gpu-speed.cu`, and `compile-gpu.sh`, `test-gpu.sh`,
  `summarize-gpu.py` for a session.

The only 32-bit generators timed on CPUs are historical ones, such as the WELL
family: modern 32-bit generators are meant for GPUs and embedded hardware, and
are timed on GPUs. Since some file names contain `*`, the repository cannot be
checked out on Windows.

## CPU methodology

- Every binary runs alone, pinned to a core with `nice -n -20`, with the
  frequency of the core fixed at its maximum (simultaneous multithreading should
  be disabled; `go.sh` records whether it is active); the timing is the minimum
  over five runs of 3 × 10⁹ outputs, after a warmup.
- Everything is compiled with `-O3 -march=native`, with both GCC and Clang, and,
  except for the vectorized sources, `-fno-unroll-loops`: without it, compilers
  unroll the harness loop differently for different generators.
- `-march=native` may let the compiler vectorize the harness loop, which is not
  possible when the generator is embedded in other code: this happens with
  SplitMix (both compilers) and wyrand (Clang), whose scalar timings are those
  of the `novec` entries. With AVX-512, GCC moves the 128-bit intermediate values
  of the plain GMWC generators through vector registers, which doubles their
  time: the `novl` entries add `-mno-avx512vl`.
- The harness xors the outputs into an accumulator; a source that defines
  `HARNESS_ADD` accumulates by addition, which is necessary when the output ends
  with a xor (e.g., the scrambler X of MWC generators), as the compiler could
  merge the two xors and skip part of the computation. 32-bit generators glue
  two outputs by defining `NEXT` (usually as `next() ^ next()`, with
  `HARNESS_ADD`). Generators returning doubles define `HARNESS_DOUBLE`, and are
  timed per double.
- These are timings of a tight loop. In an application, register allocation may
  be different (constants reloaded, part of the state written to memory), and the
  only realistic suggestion is to try generators in the application.

## GPU methodology

- Each thread copies the state of its generator into registers, accumulates the
  outputs of 2²⁴ calls, and writes the result to global memory; states are
  initialized by a separate, untimed kernel. All threads run the same generator
  from different initial states (SplitMix of the index of the thread, or
  `curand_init(index, 0, 0, ...)` for cuRAND's generators).
- The loop is unrolled 16 times (`UNROLL`): GPUs issue instructions in order, so
  the instructions controlling the loop would cost as much as the step of the
  fastest generators.
- The number of blocks is the number of multiprocessors times the number of
  blocks a multiprocessor can run at the same time, given the registers used by
  the kernel (`cudaOccupancyMaxActiveBlocksPerMultiprocessor`): with more blocks,
  kernels using more than 32 registers would run a second wave on partially
  occupied multiprocessors.
- The timing is the minimum over five runs for each block size in {128, 256,
  512, 1024}, in ps per 32-bit output of the whole GPU (the inverse of its
  throughput; 64-bit outputs count as two 32-bit outputs). `summarize-gpu.py`
  also reports this time multiplied by the number of multiprocessors, that is,
  the time of a single multiprocessor, the unit of a GPU analogous to a core of
  a CPU.
- `test-gpu.sh` locks the clock of the GPU at its maximum with `nvidia-smi -lgc`
  and resets it at the end. The clock does not always obey: the H100 SXM runs
  at 1830 MHz with the clock locked at 1980 MHz. So `test-gpu.sh` samples the
  clock during the tests (`clocks.csv`), and `summarize-gpu.py` reports the
  clock while kernels run.
- The generators of cuRAND are timed through its device API, including its
  bookkeeping (for example, that of the counter of Philox4x32-10, and the
  conversion through a double of `curand()` for MRG32k3a, which is expensive on
  GPUs with slow double-precision arithmetic).
- As on CPUs, these are timings of a tight loop: for example, unrolling lets the
  compiler fold constants into consecutive steps, as it happens with the Weyl
  sequence of XORWOW.

## Running

CPU sessions need x86-64 Linux with cpufreq, GCC and Clang (also for C++),
`taskset`, `lscpu`, passwordless `sudo`, and Python 3.9 or later:

```sh
cpu/go.sh 8                       # fixes the frequency of core 8, builds, and runs everything
cpu/go.sh 8 main fp vect paper    # only the tables of the website and of the paper
cpu/summarize.py                  # timings from cpu/{gcc.out,clang.out,freq.log,build.log,go.log}
```

`go.sh` takes about half an hour for the tables of the website and of the paper
on an AMD EPYC 9R45. `build.sh` and `run.sh` can also be used separately: in
this case, redirect the output of `build.sh` to `cpu/build.log` if you want
`summarize.py` to report the versions of the compilers.

GPU sessions need the CUDA toolkit (11.5 Update 1 or later for `-arch=native`),
root privileges, and Python 3.9 or later:

```sh
gpu/compile-gpu.sh                # nvcc -O3 -arch=native (ARCH=sm_70 etc. for older CUDA);
                                  # also writes gpu/system.txt and, with cuobjdump, gpu/*.sass
sudo GPU=0 gpu/test-gpu.sh        # writes gpu/speed-gpu and gpu/clocks.csv
gpu/summarize-gpu.py              # timings, clock, and registers per thread
```

## Licenses

Our code is distributed under the GNU General Public License, version 3 or
later (`LICENSE`), unless a file says otherwise (the vectorized sources and the
speed test of MRG32k3a are in the public domain). Third-party code keeps its
license:

- `cpu/f2-linear/SFMT/` and `cpu/f2-linear/dSFMT/`: SFMT 1.4.1 and dSFMT 2.2.3
  by Mutsuo Saito and Makoto Matsumoto, unmodified (3-clause BSD license,
  `LICENSE.txt`).
- `cpu/f2-linear/mt19937-speed.c`: MT19937-64 by Makoto Matsumoto and Takuji
  Nishimura (3-clause BSD license; the notice is in the file).
- `cpu/crypto/chacha.h`: Orson Peters's ChaCha, modified to return 64-bit
  outputs (zlib license).
- `cpu/crypto/aesctr.h`: Samuel Neves's AES in counter mode, from
  [testingRNG](https://github.com/lemire/testingRNG) (Apache License 2.0).
- `cpu/congruential/pcg64-speed.c`, `cpu/congruential/pcg64dxsm-speed.c`, and
  `gpu/pcg32-gpu-speed.cu`: Melissa O'Neill's PCG code (Apache License 2.0).
- `cpu/congruential/mixmax17-speed.c`: a port of Konstantin Savvidy's MIXMAX
  (GNU Lesser General Public License, version 3).
- `cpu/f2-linear/well*-speed.c` (François Panneton, Pierre L'Ecuyer, and Makoto
  Matsumoto) and
  `cpu/f2-linear/melg*-64.c` (Shin Harase and Takamitsu Kimoto): free for
  personal, academic, or non-commercial purposes only; see the notices in the
  files.
