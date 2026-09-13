#!/usr/bin/env python3
"""Find comments whose gen1 hedging the 2026-09-08 hardware session invalidated.

Six rounds of PR #250 carried "no gen1 card confirmed on either side". That is closed:
`lri2k-keychain` is gen1, the UID write is reversible, and the armed-gen1 wipe hazard has been
reproduced and recovered. Every comment written under the old premise is now either wrong or
weaker than the evidence.

CORRECTED 2026-09-11 by a second session, which reversed two things this file used to assert:

  - THE LATCH IS MEASURED, and the model was backwards. A written UID takes effect IMMEDIATELY --
    an INVENTORY in the same field session already returns it. There is no power-up latch on this
    chip, so text that HEDGES about the latch is no longer "correct hedging"; text that ASSERTS
    the power-up latch is wrong. The label on that rule says so.
  - "The backdoor registers accept writes WITHOUT acknowledging" was a PARSE, not a measurement.
    Blocks 62/63 answer with error 0x10, block not available; `hf 15 wrbl --ua` renders that as
    `( fail )`, which the first session read as an absent ACK. Do not reinstate it.

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
 (r"not hardware[-\s]?(validated|tested)|NOT-hardware-(validated|tested)|unvalidated|untested"
  r"|not validated on hardware|not tested (on|against)|none was available|no gen1 hardware was",
  "validation caveat"),
 (r"as they must be on a card that may not answer|may not answer", "the no-ACK INFERENCE (now observed)"),
 (r"inferred from proxmark|read out of proxmark|from proxmark's (send )?order|proxmark's source", "inference provenance"),
 (r"only order anyone has observed|only ordering .* observed|documented to accept", "ordering claim"),
 (r"arguments today|argument today|measurements later|an argument rather than a measurement", "argument-vs-measurement framing"),
 (r"\bcan have its UID moved\b|could be moved|might move the UID|possib(le|ility)", "hazard stated as possible (now reproduced)"),
 (r"no gen1 (hardware|silicon)|gen1 is (a )?port|faithful port", "port-not-tested framing"),
 (r"latch(es|ed|ing)?\b", "latch -- MEASURED 2026-09-11: the UID changes IMMEDIATELY, no power-up latch"),
 (r"only hardware (this PR|we) has|the only (card|silicon) (we|this PR)", "claim about what hardware exists"),
 (r"pending a gen1 card|gen1 card to test|awaiting a gen1|until a gen1 card", "waiting-on-hardware framing"),
]

# Phrasings this scanner MUST match, each taken from a site that was live in the tree and that an
# earlier version of these patterns could not see. Test against the text that is WRONG, never
# against the text already corrected: a pattern list written by reading the sites you just fixed
# encodes their vocabulary and is systematically blind to the ones you missed. That is exactly how
# five "not hardware-tested" sites, two of them consent-screen strings, survived the round whose
# whole job was to retire that claim.
SELFTEST = [
    "Gen1 is not hardware-tested.",
    "Offers the destructive, NOT-hardware-tested gen1 fallback as an explicit opt-in.",
    "That is destructive and not hardware-tested, so it needs consent.",
    "NOTE: gen1 is NOT hardware-validated.",
    "the card latched on the next power-up",
    "a card that only latches a written UID on the next power-up",
    "on the only hardware this PR has, this is a regression test",
    "flagged in the code as an open question pending a gen1 card to test against",
    "as they must be on a card that may not answer",
]

def selftest():
    bad = [s for s in SELFTEST if not any(re.search(p, s, re.I) for p, _ in PATS)]
    for s in bad:
        print("SELFTEST FAIL -- no pattern matches: %s" % s)
    print("selftest: %d/%d known-stale phrasings matched" % (len(SELFTEST) - len(bad), len(SELFTEST)))
    return 1 if bad else 0

args = sys.argv[1:]
if args and args[0] == "--selftest":
    sys.exit(selftest())
if not args:
    # Printing nothing and exiting 0 is indistinguishable from a clean pass, which is the false-PASS
    # shape this project has been caught by twice. Fail loudly instead.
    print("usage: gen1-staleness.py <files...>   |   gen1-staleness.py --selftest")
    print("  e.g. gen1-staleness.py $(git ls-files 'magic/protocols/iso15693/*') CHANGELOG.md")
    print("\nerror: no files given -- this scanner reports nothing without them, which reads as a pass")
    sys.exit(2)

if selftest():
    print("\nerror: the pattern list cannot see phrasings it is known to need; fix PATS first")
    sys.exit(2)
print()

for path in args:
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
