#!/usr/bin/env python3
"""Find comments whose gen1 hedging the 2026-09-08 hardware session invalidated.

Six rounds of PR #250 carried "no gen1 card confirmed on either side". That is closed:
`lri2k-keychain` is gen1, the UID write is reversible, the backdoor registers accept writes
WITHOUT acknowledging, and the armed-gen1 wipe hazard has been reproduced and recovered.
Every comment written under the old premise is now either wrong or weaker than the evidence.

STILL UNMEASURED: the latch -- whether a written UID takes effect immediately or on the next
power-up. Hedging about THAT remains correct, so the scan labels those hits rather than
flagging them, and they are left alone.

Usage:  python3 tools/gen1-staleness.py <files...>
        python3 tools/gen1-staleness.py $(git ls-files 'magic/protocols/iso15693/*') CHANGELOG.md

Run it before posting any draft and as the site list for the gen1 B-round. It has teeth:
iso15693_poller.h yields six hits, four of them real. Note one known false positive class --
"impossible" matches the possib(le|ility) pattern.

See .notes/gen1-hardware-findings.md for the findings and the work list.
"""
import re, sys
# Phrases the 2026-09-08 gen1 session invalidated or weakened.
PATS = [
 (r"no gen1 card|without a gen1 card|no card to test|nobody can test|cannot be tested|untestable", "no-card claim"),
 (r"not hardware[- ]validated|NOT hardware-validated|unvalidated|not validated on hardware", "validation caveat"),
 (r"as they must be on a card that may not answer|may not answer", "the no-ACK INFERENCE (now observed)"),
 (r"inferred from proxmark|read out of proxmark|from proxmark's (send )?order|proxmark's source", "inference provenance"),
 (r"only order anyone has observed|only ordering .* observed|documented to accept", "ordering claim"),
 (r"arguments today|argument today|measurements later|an argument rather than a measurement", "argument-vs-measurement framing"),
 (r"\bcan have its UID moved\b|could be moved|might move the UID|possib(le|ility)", "hazard stated as possible (now reproduced)"),
 (r"no gen1 (hardware|silicon)|gen1 is (a )?port|faithful port", "port-not-tested framing"),
 (r"latch(es)?\b", "latch -- STILL unmeasured, so hedging here is CORRECT"),
]
for path in sys.argv[1:]:
    txt = open(path, encoding="utf-8").read().split("\n")
    thread = "(preamble)"
    hits = []
    for i,l in enumerate(txt,1):
        m = re.match(r'^## `?([0-9]+)`? *(.*)', l)
        if m: thread = "%s %s" % (m.group(1), m.group(2)[:44])
        elif l.startswith("## "): thread = l[3:][:52]
        for pat,label in PATS:
            if re.search(pat, l, re.I):
                hits.append((thread, i, label, l.strip()[:118]))
    print("##### %s -- %d hits" % (path.split('/')[-1], len(hits)))
    last=None
    for th,i,label,l in hits:
        if th!=last: print("\n  [%s]" % th); last=th
        print("    :%-4d %-42s %s" % (i, label, l))
    print()
