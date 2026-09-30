#!/usr/bin/env python3
"""Intra-push churn, multiset: every line a sync point adds that is gone at the tip.
Usage: churn.py <repo> <base> <c1> ... <cN>   (consecutive fork commits, tip last)
A line added k times in the push and present t times at the tip counts max(0, k-t) as gone.
Blank lines are skipped. Per file."""
import subprocess, sys
from collections import Counter, defaultdict
repo, commits = sys.argv[1], sys.argv[2:]
def git(*a): return subprocess.run(['git','-C',repo,*a],capture_output=True,text=True,check=True).stdout
added = defaultdict(Counter); who = defaultdict(list)
for i in range(1, len(commits)):
    a, b = commits[i-1], commits[i]
    f = None
    for ln in git('diff','-U0','--no-color',a,b).splitlines():
        if ln.startswith('+++ '):
            f = ln[6:] if ln.startswith('+++ b/') else None; continue
        if ln.startswith('+') and f and ln[1:].strip():
            added[f][ln[1:]] += 1; who[(f, ln[1:])].append(i)
tip = commits[-1]; total = 0; rows = []
for f, c in added.items():
    try: t = Counter(git('show', f'{tip}:{f}').splitlines())
    except subprocess.CalledProcessError: t = Counter()
    for line, k in c.items():
        g = max(0, k - t[line])
        if g: total += g; rows.append((f, who[(f,line)], g, line))
for f, w, g, line in sorted(rows):
    print(f'{g}x  added at {",".join("%02d"%x for x in w)}  {f.split("/")[-1]}:  {line.strip()[:110]}')
print(f'TOTAL gone at tip: {total}')
