#!/usr/bin/env python3
"""Comment-vs-code accounting for a diff, and per-file comment ratios.

Round 6 cost +117 comment against +28 code while every individual edit was correct, so a cut has to
be measured rather than asserted. Two modes:

    comment-ratio.py diff [<git diff args>]   added/removed comment vs code (default: working tree)
    comment-ratio.py files <path>...          the standing ratio per file
"""
import subprocess
import sys


def is_comment(text):
    t = text.strip()
    return t.startswith("//") or t.startswith("/*") or t.startswith("*") or t.startswith("*/")


def diff(args):
    out = subprocess.run(
        ["git", "diff", "--unified=0"] + args, capture_output=True, text=True, check=True
    ).stdout
    tally = {"+c": 0, "+x": 0, "-c": 0, "-x": 0}
    for line in out.split("\n"):
        if line.startswith("+++") or line.startswith("---"):
            continue
        if line.startswith("+") or line.startswith("-"):
            body = line[1:]
            if not body.strip():
                continue
            sign = "+" if line[0] == "+" else "-"
            tally[sign + ("c" if is_comment(body) else "x")] += 1
    net_c = tally["+c"] - tally["-c"]
    net_x = tally["+x"] - tally["-x"]
    print("comment  +%d  -%d   net %+d" % (tally["+c"], tally["-c"], net_c))
    print("code     +%d  -%d   net %+d" % (tally["+x"], tally["-x"], net_x))
    verdict = "the right way" if net_c < 0 and net_x <= 0 else "CHECK THIS"
    print("verdict: %s" % verdict)


def files(paths):
    tot_l = tot_c = 0
    for p in paths:
        lines = [l for l in open(p).read().split("\n")]
        if lines and lines[-1] == "":
            lines = lines[:-1]
        c = sum(1 for l in lines if is_comment(l))
        tot_l += len(lines)
        tot_c += c
        print("%-52s %5d %5d  %3.0f%%" % (p.split("/")[-1], len(lines), c, 100.0 * c / len(lines)))
    if len(paths) > 1:
        print("%-52s %5d %5d  %3.0f%%" % ("TOTAL", tot_l, tot_c, 100.0 * tot_c / tot_l))


if __name__ == "__main__":
    mode = sys.argv[1] if len(sys.argv) > 1 else "diff"
    if mode == "files":
        files(sys.argv[2:])
    else:
        diff(sys.argv[2:])
