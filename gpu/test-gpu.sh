#!/bin/bash -e

# Runs every GPU speed test five times for each number of threads per block in
# 128, 256, 512, and 1024, one binary at a time, on the GPU of index GPU
# (default 0) with its clocks locked at their maximum. The argument is the
# number of calls per thread. Results, and the clocks before and after the
# tests, go to speed-gpu; the time of a generator is the minimum over all runs.
# The clock does not always obey the lock (the H100 SXM locked at 1980 MHz runs
# at 1830 MHz), so clock, power, temperature, and active throttle reasons are
# sampled four times a second into clocks.csv (see summarize-gpu.py).

cd "$(dirname "$0")"

if [[ $(whoami) != root ]]; then
	echo "Must be run as root" 1>&2
	exit 1
fi

export GPU=${GPU:-0}
export CUDA_DEVICE_ORDER=PCI_BUS_ID
export CUDA_VISIBLE_DEVICES=$GPU
MAX=$(nvidia-smi -i $GPU --query-gpu=clocks.max.sm --format=csv,noheader,nounits)
PM=$(nvidia-smi -i $GPU --query-gpu=persistence_mode --format=csv,noheader)
# Drivers before R535 call the throttle reasons clocks_throttle_reasons
REASONS=clocks_event_reasons.active
nvidia-smi -i $GPU --query-gpu=$REASONS --format=csv,noheader > /dev/null 2>&1 || REASONS=clocks_throttle_reasons.active
nvidia-smi -i $GPU -pm 1
nvidia-smi -i $GPU -lgc $MAX,$MAX
nvidia-smi -i $GPU --query-gpu=timestamp,clocks.sm,clocks.max.sm,power.draw,temperature.gpu,$REASONS --format=csv -lms 250 < /dev/null > clocks.csv &
MON=$!
# The trap must reset the clocks even if the monitor has already died
trap "kill $MON 2> /dev/null || true; nvidia-smi -i $GPU -rgc; [[ $PM == Disabled ]] && nvidia-smi -i $GPU -pm 0" EXIT

export N=${1:-16777216}

# Warmup
( for i in *-gpu-speed; do
	"./$i" $((N/16))
done ) >/dev/null

# Test
( echo "calls per thread: $N"
nvidia-smi -i $GPU --query-gpu=name,driver_version,clocks.sm,clocks.max.sm --format=csv
for i in *-gpu-speed; do
	for t in 128 256 512 1024; do
		for((r=0; r<5; r++)); do
			"./$i" $N $t
		done
	done
done
nvidia-smi -i $GPU --query-gpu=clocks.sm --format=csv
nvidia-smi -i $GPU -q -d PERFORMANCE ) > speed-gpu
