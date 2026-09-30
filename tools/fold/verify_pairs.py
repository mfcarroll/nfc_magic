#!/usr/bin/env python3
"""Check a fold commit by commit: same authors, dates and messages, and nothing changed but the mapped
blobs. Writes the old->new SHA map beside the index map.

    verify_pairs.py <indexmap> <range base> <old ref> <new ref> [--skipped SHA] [--message-changed SHA]

--skipped: a commit the rewrite dropped (a commit filter's skip_commit). --message-changed: one whose
message a --msg-filter rewrote on purpose."""
import argparse, subprocess
def git(*a): return subprocess.run(['git', *a], capture_output=True, text=True, check=True).stdout
ap = argparse.ArgumentParser()
for a in ('indexmap', 'base', 'old', 'new'): ap.add_argument(a)
ap.add_argument('--skipped'); ap.add_argument('--message-changed')
x = ap.parse_args()
skipped = git('rev-parse', x.skipped).strip() if x.skipped else None
msg_changed = git('rev-parse', x.message_changed).strip() if x.message_changed else None
old = [c for c in git('rev-list', '--reverse', f'{x.base}..{x.old}').split() if c != skipped]
new = git('rev-list', '--reverse', f'{x.base}..{x.new}').split()
assert len(old) == len(new), (len(old), len(new))
m = {}
for l in open(x.indexmap):
    c, mode, blob, path = l.split(maxsplit=3); m.setdefault(c, {})[path.strip()] = blob
meta = '--format=%an%x00%ae%x00%ad%x00%cn%x00%ce%x00%cd'
bad = mapped = 0; pairs = []
for o, n in zip(old, new):
    if git('log', '-1', meta, o) != git('log', '-1', meta, n): bad += 1; print('META', o[:7])
    if o != msg_changed and git('log', '-1', '--format=%B', o) != git('log', '-1', '--format=%B', n):
        bad += 1; print('MESSAGE', o[:7])
    d = git('diff', '--name-only', o, n).split(); exp = m.get(o, {})
    for p in d:
        if p not in exp or git('rev-parse', f'{n}:{p}').strip() != exp[p]: bad += 1; print('UNEXPECTED', o[:7], p)
        else: mapped += 1
    for p in exp:
        if p not in d and git('rev-parse', f'{o}:{p}').strip() != exp[p]: bad += 1; print('MISSING', o[:7], p)
    pairs.append(f'{o[:7]} {n[:7]}')
open(x.indexmap.replace('indexmap-', 'shamap-'), 'w').write('\n'.join(pairs) + '\n')
print(f'{len(old)} pairs, {mapped} mapped blob changes, {bad} problems')
