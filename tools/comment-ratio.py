#!/usr/bin/env python3
"""Comment-vs-code accounting for a diff, and per-file comment ratios.

Round 6 cost +117 comment against +28 code while every individual edit was correct, so a cut has to
be measured rather than asserted. Two modes:

    comment-ratio.py diff [<git diff args>]   added/removed comment vs code (default: working tree)
    comment-ratio.py files <path>...          the standing ratio per file
    comment-ratio.py blocks [--min N] [--list] <path>...
                                              where the mass actually is: lines sitting in runs of N+
                                              consecutive comment lines. A cut works on those runs, so
                                              the ratio alone does not say what is available -- a file
                                              can be 66% comment and hold almost none of it in blocks.
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


def blocks(args):
    """Comment lines sitting in runs of `min_run` or more, which is what a deletion pass can work on."""
    min_run, show = 10, False
    paths = []
    i = 0
    while i < len(args):
        if args[i] == "--min":
            min_run = int(args[i + 1]); i += 2
        elif args[i] == "--list":
            show = True; i += 1
        else:
            paths.append(args[i]); i += 1

    tot = {"lines": 0, "comment": 0, "blk_lines": 0, "blk_count": 0}
    per_file = []
    for p in paths:
        lines = open(p).read().split("\n")
        if lines and lines[-1] == "":
            lines = lines[:-1]
        runs, start = [], None
        for n, l in enumerate(lines):
            if is_comment(l):
                if start is None:
                    start = n
            elif start is not None:
                runs.append((start + 1, n - start)); start = None
        if start is not None:
            runs.append((start + 1, len(lines) - start))
        big = [r for r in runs if r[1] >= min_run]
        c = sum(1 for l in lines if is_comment(l))
        blk = sum(r[1] for r in big)
        per_file.append((p, len(lines), c, blk, big))
        tot["lines"] += len(lines); tot["comment"] += c
        tot["blk_lines"] += blk; tot["blk_count"] += len(big)

    print("%-46s %6s %6s %5s %8s %6s" % ("file", "lines", "cmnt", "%", "in %d+" % min_run, "blocks"))
    for p, n, c, blk, big in sorted(per_file, key=lambda r: -r[3]):
        print("%-46s %6d %6d %4.0f%% %8d %6d"
              % (p.split("/")[-1], n, c, 100.0 * c / n if n else 0, blk, len(big)))
    print("%-46s %6d %6d %4.0f%% %8d %6d"
          % ("TOTAL", tot["lines"], tot["comment"],
             100.0 * tot["comment"] / tot["lines"] if tot["lines"] else 0,
             tot["blk_lines"], tot["blk_count"]))
    if show:
        for p, _n, _c, _blk, big in per_file:
            if not big:
                continue
            print("\n%s" % p)
            for ln, size in sorted(big, key=lambda r: -r[1]):
                print("  %s:%-5d  %3d lines" % (p, ln, size))


if __name__ == "__main__":
    mode = sys.argv[1] if len(sys.argv) > 1 else "diff"
    if mode == "files":
        files(sys.argv[2:])
    elif mode == "blocks":
        blocks(sys.argv[2:])
    else:
        diff(sys.argv[2:])
