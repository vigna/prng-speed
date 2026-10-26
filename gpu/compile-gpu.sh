#!/bin/bash -e

# Builds the GPU speed tests for the GPU of this machine (-arch=native is the
# analogue of -march=native; ARCH overrides it, e.g., ARCH=sm_70 for CUDA
# before 11.5 Update 1, which lacks -arch=native). Writes the versions of the
# compilers and the flags into system.txt and, if cuobjdump is available, the
# SASS of each speed test, with the registers it uses, into NAME-gpu-speed.sass.

cd "$(dirname "$0")"
FLAGS="-O3 -arch=${ARCH:-native} -Xcompiler -Wall"
{
	echo "nvcc: $(nvcc --version | grep -i release)"
	echo "host compiler: $(${CXX:-g++} --version | head -1)"
	echo "flags: $FLAGS"
} > system.txt
for f in *-gpu-speed.cu; do
	nvcc $FLAGS -o "${f%.cu}" "$f"
	# cuobjdump -sass needs nvdisasm, which some installations lack
	if command -v cuobjdump > /dev/null; then
		cuobjdump -res-usage -sass "${f%.cu}" > "${f%.cu}.sass" 2>&1 || rm -f "${f%.cu}.sass"
	fi
done
