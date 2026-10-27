#!/usr/bin/env python3
"""Usage: summarize-gpu.py [--markdown|--tsv] [DIR]. Reads DIR/{speed-gpu,system.txt,clocks.csv,*.sass}
(test-gpu.sh writes speed-gpu and clocks.csv, compile-gpu.sh writes system.txt
and, if cuobjdump is available, the SASS dumps; all but speed-gpu are optional;
default DIR: the directory of this script) and prints, for every generator, the minimum over
the five runs for each number of threads per block of the time per 32-bit output
of the whole GPU (ps), the same time multiplied by the number of multiprocessors (the
time of a single multiprocessor, analogous to a core of a CPU), the corresponding
throughput, the number of threads per block of the minimum, and the registers per
thread of the kernel. With --markdown, it prints the two times in GitHub Markdown,
with links to the sources relative to the root of the repository
(results/update.py puts them in README.md); with --tsv, one line per generator,
with full precision (read by the script that updates the website)."""
import collections, glob, os, re, sys
from urllib.parse import quote
args = sys.argv[1:]
fmt = args.pop(0)[2:] if args and args[0] in ("--markdown", "--tsv") else "text"
D = args[0] if args else os.path.dirname(os.path.abspath(__file__))

# Generator (binary name without -gpu-speed), name on the website, source
GENS = (("MWC64X_32", "MWC64X/32", "MWC64X_32.c"),
        ("MWC128X_32", "MWC128X/32", "MWC128X_32.c"),
        ("xoroshiro64**", "xoroshiro64**", "xoroshiro64starstar.c"),
        ("xoroshiro64*", "xoroshiro64*", "xoroshiro64star.c"),
        ("xoshiro128++", "xoshiro128++", "xoshiro128plusplus.c"),
        ("xoshiro128**", "xoshiro128**", "xoshiro128starstar.c"),
        ("xoshiro128+", "xoshiro128+", "xoshiro128plus.c"),
        ("MWC128X", "MWC128X (64-bit outputs)", "MWC128X.c"),
        ("sfc32", "SFC32 (PractRand)", ""),
        ("pcg32", "PCG32", ""),
        ("xorwow", "XORWOW (cuRAND)", ""),
        ("philox4x32-10", "Philox4x32-10 (cuRAND, curand4())", ""),
        ("MRG32k3a", "MRG32k3a (cuRAND)", ""))

runs = collections.defaultdict(lambda: collections.defaultdict(list))
for l in open(f"{D}/speed-gpu"):
    if "words/ns" in l:
        g = l.split(":")[0].removeprefix("./").removesuffix("-gpu-speed")
        t = int(l.split(" x ")[1].split()[0])
        # ns per word from words/ns, which is printed with more significant digits
        runs[g][t].append(1 / float(l.split(", ")[2].split()[0]))
# Known generators in the order above, then the others under their file name
GENS = [x for x in GENS if x[0] in runs] + [(g, g, "") for g in sorted(runs) if g not in {x[0] for x in GENS}]
assert all(len(r) == 5 for g in runs.values() for r in g.values()) and all(len(g) == 4 for g in runs.values())
gpu = re.search(r"\(([^,]*), \d+ blocks", open(f"{D}/speed-gpu").read()).group(1)
blocks = {int(m[0]) // int(m[1]) for m in re.findall(r"(\d+) blocks x \d+ threads, at most (\d+) blocks", open(f"{D}/speed-gpu").read())}
assert len(blocks) == 1, blocks
sms = blocks.pop()

def search(pattern, text, default="unknown"):
    m = re.search(pattern, text, re.M)
    return m.group(1) if m else default

system = open(f"{D}/system.txt").read() if os.path.exists(f"{D}/system.txt") else ""
# test-gpu.sh writes the name, the driver and the clocks in the first lines of speed-gpu
driver = search(r"Driver Version\s*: (\S+)", system, search(r"^[^,\n]+, ([0-9.]+), ", open(f"{D}/speed-gpu").read()))
# system.txt of compile-gpu.sh, or the raw output of nvcc --version and gcc --version
nvcc = search(r"Cuda compilation tools, release \S+ (V\S+)", system)
host = search(r"^host compiler: (.*)$", system, search(r"^(gcc .*)$", system))
flags = search(r"^flags: (.*)$", system)
calls = search(r"^calls per thread: (\d+)", open(f"{D}/speed-gpu").read(), "unknown")

# Clock samples while a kernel runs (power draw above the idle level plus 50 W)
busy = []
if os.path.exists(f"{D}/clocks.csv"):
    # Only well-formed samples (fields may be [N/A], or the file an error message)
    rows = [l.rstrip("\n").split(", ") for l in open(f"{D}/clocks.csv")][1:]
    rows = [r for r in rows if len(r) == 6 and re.fullmatch(r"[0-9.]+ W", r[3]) and r[4].isdigit()]
    if rows:
        idle = min(float(r[3].split()[0]) for r in rows)
        busy = [r for r in rows if float(r[3].split()[0]) > idle + 50]
        clocks = collections.Counter(r[1] for r in busy)
        reasons = collections.Counter(r[5].strip() for r in busy)
        power = max(float(r[3].split()[0]) for r in rows)
        temp = max(int(r[4]) for r in rows)

def regs(g):
    try:
        m = re.search(r"Function \S*generate\S*:\s*REG:(\d+)", open(f"{D}/{g}-gpu-speed.sass").read())
        return m.group(1) if m else "?"
    except FileNotFoundError:
        return "?"

best = {g: min((min(r), t) for t, r in runs[g].items()) for g, _, _ in GENS}
if fmt == "tsv":
    print("# name\tps/32 bits\tSM ps/32 bits")
    for g, _, _ in GENS:
        print(f"{g}\t{1000 * best[g][0]!r}\t{1000 * sms * best[g][0]!r}")
    sys.exit()
if fmt == "markdown":
    print("### Comparison of 32-bit PRNGs on a GPU")
    print()
    print("| PRNG | ps/32 bits (GPU) | ps/32 bits (multiprocessor) |")
    print("|:--|--:|--:|")
    for g, name, _ in GENS:
        # Asterisks of scramblers escaped, as Markdown would take them as emphasis
        label = name.replace("*", "\\*")
        print(f"| [{label}](gpu/{quote(g + '-gpu-speed.cu')}) | {1000 * best[g][0]:.3f} | {1000 * sms * best[g][0]:.1f} |")
    # A short description of the hardware: the clock is the most frequent while kernels run
    clock = f" at {clocks.most_common(1)[0][0]}" if busy else ""
    cuda = search(r"V?(\d+\.\d+)", nvcc)
    print()
    print(f"{gpu}{clock} ({sms} multiprocessors); nvcc {cuda}; ps per 32-bit output of the whole GPU "
          f"and of a multiprocessor, minimum of five runs.")
    sys.exit()

print("GPU timings")
print()
print(f"GPU: {gpu} ({sms} multiprocessors), driver {driver}; nvcc {nvcc}; host compiler {host}")
print(f"Flags: {flags}; clock locked at the maximum by test-gpu.sh")
if busy:
    print(f"Clock samples (4 per second) while a kernel runs (power > {idle:.0f} + 50 W): {len(busy)} of {len(rows)};")
    print("  SM clock: " + ", ".join(f"{c} ({n})" for c, n in clocks.most_common()))
    print("  active throttle reasons: " + ", ".join(f"{c} ({n})" for c, n in reasons.most_common()))
    print(f"  maximum power draw {power:.0f} W, maximum temperature {temp} C")
print("Value: minimum over five runs for each number of threads per block in {128, 256, 512, 1024},")
print(f"  {calls} calls per thread, one wave of blocks; ps per 32-bit output of the whole GPU and of a multiprocessor (SM)")
print()
# Registers only if there are SASS dumps (results/gpu has none)
sass = bool(glob.glob(f"{D}/*.sass"))
print(f"{'PRNG':36}{'ps/32 bits':>11}{'SM ps/32 bits':>15}{'G words/s':>11}{'threads':>9}" + (f"{'regs':>6}" if sass else ""))
for g, name, _ in GENS:
    b = best[g]
    print(f"{name:36}{1000 * b[0]:11.3f}{1000 * sms * b[0]:15.1f}{1 / b[0]:11.0f}{b[1]:9}" + (f"{regs(g):>6}" if sass else ""))
print()
print("Spread of the five runs (max/min - 1), worst over the numbers of threads per block:")
print("  " + ", ".join(f"{g} {max(max(r) / min(r) - 1 for r in runs[g].values()):.1e}" for g, _, _ in GENS))
