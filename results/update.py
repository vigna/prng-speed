#!/usr/bin/env python3
"""Usage: results/update.py. Regenerates, from the logs of the sessions in
results/cpu (cpu/website.sh copies them there) and results/gpu (speed-gpu,
clocks.csv, and system.txt of gpu/test-gpu.sh and gpu/compile-gpu.sh), their
summaries (summary.txt), their timings with full precision (timings.tsv, read by
the script that updates the website), and the tables of README.md between the
markers <!-- BEGIN TIMINGS --> and <!-- END TIMINGS -->."""
import os, subprocess, sys
R = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(R)

def run(script, *args):
    return subprocess.run([sys.executable, f"{TOP}/{script}", *args], check=True, capture_output=True, text=True).stdout

md = []
for d, script in (("cpu", "cpu/summarize.py"), ("gpu", "gpu/summarize-gpu.py")):
    D = f"{R}/{d}"
    with open(f"{D}/summary.txt", "w") as f:
        f.write(run(script, D))
    with open(f"{D}/timings.tsv", "w") as f:
        f.write(run(script, "--tsv", D))
    md.append(run(script, "--markdown", D))
BEGIN, END = "<!-- BEGIN TIMINGS -->\n", "<!-- END TIMINGS -->"
readme = open(f"{TOP}/README.md").read()
a, b = readme.index(BEGIN) + len(BEGIN), readme.index(END)
with open(f"{TOP}/README.md", "w") as f:
    f.write(readme[:a] + "\n".join(md) + readme[b:])
