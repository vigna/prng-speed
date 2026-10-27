#!/usr/bin/env python3
"""Usage: summarize.py [--markdown|--tsv] [DIR]. Reads DIR/{gens.tsv,gcc.out,clang.out,freq.log,build.log,go.log}
(default: the directory of this script, where go.sh leaves them) and prints the
minimum of the five runs of every generator (ns per 64-bit output, or per
double) and cycles/B at the mean frequency of the core. With --markdown, the
tables are in GitHub Markdown, with links to the sources relative to the root of
the repository (results/update.py puts them in README.md); with --tsv, there is
one line per generator, with full precision (read by the script that updates
the website)."""
import os, statistics, sys
from urllib.parse import quote
args = sys.argv[1:]
fmt = args.pop(0)[2:] if args and args[0] in ("--markdown", "--tsv") else "text"
D = args[0] if args else os.path.dirname(os.path.abspath(__file__))
gens = [l.rstrip("\n").split("\t") for l in open(f"{D}/gens.tsv") if not l.startswith("#")]
res = {}
for cc in ("gcc", "clang"):
    runs = {}
    for l in open(f"{D}/{cc}.out"):
        if l.startswith("DONE"):
            continue
        r, name, out = l.rstrip("\n").split("\t")
        runs.setdefault(name, []).append(float(out.split()[-2]))
    bad = [k for k, v in runs.items() if len(v) != 5]
    assert not bad, f"{cc}: not five runs for {bad}"
    res[cc] = {k: min(v) for k, v in runs.items()}
f = [int(l.split()[1]) for l in open(f"{D}/freq.log") if len(l.split()) == 2]
ghz = statistics.mean(f) / 1e6 if f else float("nan")
ver = [l.strip() for l in open(f"{D}/build.log") if l.startswith(("gcc:", "clang:"))] if os.path.exists(f"{D}/build.log") else []
log = [l.rstrip("\n") for l in open(f"{D}/go.log")]
cpu = next((l.removeprefix("cpu: ") for l in log if l.startswith("cpu: ")), "unknown CPU")
core = next((l for l in log if l.startswith("cpu") and l.split(":")[0][3:].isdigit()), None)
smt = next((l.removeprefix("smt: ") for l in log if l.startswith("smt: ")), "unknown")
start = next(l for l in log if l.endswith(" start")).removesuffix(" start")
end = next((l for l in reversed(log) if l.endswith(" done")), "(not finished)").removesuffix(" done")
TITLES = {"main": "Comparison of PRNGs (prng.di.unimi.it)", "fp": "Floating-point generation (prng.di.unimi.it)",
          "vect": "Vectorized generators (prng.di.unimi.it)",
          "paper": "Remaining generators of the speed table of the MWC paper",
          "other": "Other generators (not in the tables)"}

def cols(name, p):
    """ns and cycles/B with GCC and Clang, with p decimals (None: full precision)."""
    out = []
    for cc in ("gcc", "clang"):
        if name not in res[cc]:
            out += ["-", "-"]
            continue
        ns = res[cc][name]
        out += [repr(x) if p is None else f"{x:.{p}f}" for x in (ns, ns * ghz / 8)]
    return out

def esc(s):
    """Escapes the asterisks of scramblers, which Markdown would take as emphasis."""
    return s.replace("*", "\\*")

if fmt == "tsv":
    print("# name\ttable\tGCC ns\tGCC cycles/B\tClang ns\tClang cycles/B")
    for name, table, *_ in gens:
        if any(name in res[cc] for cc in res):
            print("\t".join([name, table] + cols(name, None)))
    sys.exit()
# The captions of the tables of the website
CAPTIONS = {"main": "Comparison of PRNGs", "fp": "Comparison of floating-point PRNGs",
            "vect": "Comparison of vectorized PRNGs"}
if fmt != "markdown":
    print(f"Session: {start} - {end}")
    print(f"Machine: {cpu}; every binary runs alone on one core; SMT active: {smt}")
    print(f"  {core or 'frequency settings unknown (run.sh without go.sh)'}; mean frequency during the runs {ghz:.3f} GHz")
    for v in ver:
        print("  " + v)
    print("Flags: -fno-unroll-loops -Wall -Wno-unused-variable -O3 -march=native -std=c99")
    print("  (vectorized: -Wall -Wno-unused-variable -O3 -ftree-vectorize -march=native -std=c99,")
    print("  GCC also -fdisable-tree-cunrolli; novl: also -mno-avx512vl; see build.sh)")
    print("Value: minimum of 5 runs of 3*10^9 outputs; ns per 64-bit output (floating point:")
    print("  per double); cycles/B = ns * GHz / 8")
for t in TITLES:
    rows = [g for g in gens if g[1] == t and any(g[0] in res[cc] for cc in res)]
    if not rows:
        continue
    if fmt == "markdown":
        print(f"### {CAPTIONS.get(t, TITLES[t])}")
        print()
        print("| PRNG | ns/64 bits (GCC) | cycles/B (GCC) | ns/64 bits (Clang) | cycles/B (Clang) |")
        print("|:--|--:|--:|--:|--:|")
    else:
        print()
        print(TITLES[t])
        print(f"{'':22}{'GCC':>16}{'Clang':>16}")
        print(f"{'PRNG':22}{'ns':>8}{'cyc/B':>8}{'ns':>8}{'cyc/B':>8}")
    for name, table, src, kind, *extra in rows:
        # Vectorized generators need a third decimal (the website shows it)
        c = cols(name, 3 if kind == "vect" else 2)
        if fmt == "markdown":
            print(f"| [{esc(name)}](cpu/{quote(src)}) | " + " | ".join(c) + " |")
        else:
            print(f"{name:22}" + "".join(f"{x:>8}" for x in c))
    if fmt == "markdown":
        print()
# In Markdown the session follows the tables, which come first in README.md
if fmt == "markdown":
    compilers = ", ".join(f"`{v.split(': ', 1)[1]}`" for v in ver) or "unknown"
    print(f"CPU session of {start} to {end} on an {cpu}, every binary alone on one core (SMT active: {smt}), "
          f"mean frequency of the core {ghz:.3f} GHz; compilers {compilers}, with the flags of "
          f"[`cpu/build.sh`](cpu/build.sh). Times are the minimum of five runs of 3 × 10⁹ outputs, in ns per "
          f"64-bit output (floating point: per double); cycles/B are computed at the mean frequency.")
