#!/bin/bash
# Usage: go.sh [CORE [TABLE...]]. Fixes the frequency of CORE (default 8) at the
# maximum, builds (build.sh), times every binary alone on CORE (run.sh), and
# restores the minimum frequency. Simultaneous multithreading should be disabled
# (go.log records whether it is active). On the benchmark machine of
# prng.di.unimi.it (an AMD EPYC 9R45) it takes about half an hour for the tables
# of the website and of the MWC paper (main fp vect paper): run it under nohup.
# It needs passwordless sudo.
cd "$(dirname "$0")"
CORE=${1:-8}
shift $(( $# > 0 ))
F=/sys/devices/system/cpu/cpu$CORE/cpufreq
[[ -d $F ]] || { echo "No cpufreq directory for core $CORE ($F)" >&2; exit 1; }
MIN=$(cat $F/scaling_min_freq)
trap "sudo tee $F/scaling_min_freq <<< $MIN > /dev/null" EXIT
sudo tee $F/scaling_min_freq < $F/scaling_max_freq > /dev/null || exit 1
{ echo "$(date) start"; echo "cpu$CORE: $(cat $F/scaling_governor) min $(cat $F/scaling_min_freq) max $(cat $F/scaling_max_freq)"; echo "cpu: $(lscpu | sed -n 's/^Model name: *//p')"; echo "smt: $(cat /sys/devices/system/cpu/smt/active 2>/dev/null || echo unknown)"; } > go.log
bash build.sh "$@" > build.log 2>&1 || echo "$(date) some builds failed: see build.log" >> go.log
echo "$(date) built $(ls gcc | wc -l) + $(ls clang | wc -l) binaries" >> go.log
GO_SH=1 bash run.sh $CORE "$@" || exit 1
echo "$(date) done" >> go.log
