#!/bin/bash
# Usage: website.sh [CORE]. Regenerates the CPU timings of prng.di.unimi.it: times
# the generators of the tables of the website (main, fp, and vect of gens.tsv)
# with go.sh on CORE (default 8), copies the logs of the session into
# results/cpu, and runs results/update.py, which regenerates the summaries in
# results/ and the tables of README.md (tables and order of the generators come
# from gens.tsv, which is not copied). It takes less than half an hour on an
# AMD EPYC 9R45: run it under nohup. The GPU table of the website is regenerated
# by gpu/compile-gpu.sh and gpu/test-gpu.sh, on a machine with an NVIDIA GPU:
# copy gpu/{speed-gpu,clocks.csv,system.txt} into results/gpu and run
# results/update.py.
cd "$(dirname "$0")"
bash go.sh "${1:-8}" main fp vect || exit 1
cp gcc.out clang.out freq.log build.log go.log ../results/cpu/
python3 ../results/update.py
