#!/bin/bash
# Usage: run.sh CORE [TABLE...]. Warmup (10^8 outputs per binary), then 5 rounds;
# in each round every GCC binary and then every Clang binary of gens.tsv (or of
# the given tables) runs once, alone, with 3*10^9 outputs (vectorized binaries:
# 3*10^6 calls of 1024 outputs), pinned to CORE with nice -n -20 (through sudo).
# Results go to gcc.out and clang.out; the frequency of CORE is sampled every
# second into freq.log; progress is logged into go.log. A run that fails stops
# the session.
cd "$(dirname "$0")"
CORE=${1:?Usage: run.sh CORE [TABLE...]}
shift
F=/sys/devices/system/cpu/cpu$CORE/cpufreq
[[ -d $F ]] || { echo "No cpufreq directory for core $CORE ($F)" >&2; exit 1; }
N=3000000000
# When run alone, start a new log (go.sh writes the start line itself)
[[ -n ${GO_SH:-} ]] || echo "$(date) start" > go.log
[[ -r $F/cpuinfo_avg_freq ]] && FREQ=$F/cpuinfo_avg_freq || FREQ=$F/scaling_cur_freq
( while true; do echo "$(date +%s) $(cat $FREQ)"; sleep 1; done ) > freq.log &
SAMPLER=$!
trap "kill $SAMPLER 2>/dev/null" EXIT
trap "exit 130" INT TERM
rm -f gcc.out clang.out
for r in warmup 1 2 3 4 5; do
	for cc in gcc clang; do
		while IFS=$'\t' read -r name table kind; do
			(( $# )) && [[ " $* " != *" $table "* ]] && continue
			[[ -x $cc/$name ]] || continue
			n=$N
			[[ $kind == vect ]] && n=$((N / 1000))
			[[ $r == warmup ]] && n=$((n / 30))
			if ! out=$(sudo nice -n -20 taskset -c $CORE "./$cc/$name" $n); then
				echo "$(date) $cc/$name failed: session stopped" | tee -a go.log >&2
				exit 1
			fi
			[[ $r != warmup ]] && echo "$r	$name	$out" >> $cc.out
		done < <(grep -v '^#' gens.tsv | cut -f1,2,4)
	done
	echo "$(date) round $r completed" >> go.log
done
echo DONE >> gcc.out
echo DONE >> clang.out
[[ -n ${GO_SH:-} ]] || echo "$(date) done" >> go.log
