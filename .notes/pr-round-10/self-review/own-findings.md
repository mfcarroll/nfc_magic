# Findings from the session's own mechanical checks (not agent-sourced)

## F1 — CONFIRMED, high. `iso15693_poller.c:882` asserts the power-up latch the rest of the tree retracts

`magic/protocols/iso15693/iso15693_poller.c:882`:

    // DOES move its UID. Reproduced 2026-09-08 on an armed LRi2K: the sweep zeroed 56/57, the card
    // latched on the next power-up, and the re-read below reported uid_changed as Partial.

The power-up latch is the model the second bench session MEASURED FALSE. Three sites say so:

- `iso15693_poller.c:43` — "The UID changes IMMEDIATELY ... so there is no power-up latch on this chip"
- `iso15693_poller.h:80` — "NOT because a card latches the new UID on power-up: that was the reason
  given here and gen1 silicon measured it false"
- `CHANGELOG.md:152` — "a written UID takes effect **immediately**, not on the next power-up as this
  app assumed"

**And the contradiction is inside the same comment block**, 14 lines later at `:895-896`:

    // power-cycle. The power-cycle is not what makes the change visible -- it is visible
    // immediately -- it just gives a clean re-activation to read from.

So `:882` and `:896` state opposite models within one paragraph.

Provenance: introduced by `3a8b6ea` ("the armed-card hazard is reproduced"), commit 4 of the gen1
B-round. `4ec5f8c` (commit 7) is the commit that removed the latch model — it rewrote the
surrounding lines but did not reach `:882`. This is the round-9 "fixed one copy, left the twin"
class, inside the very round that was reset and rebuilt to purge this model.

**What is actually true, and what the line should say:** the observation is still good — the sweep
zeroed 56/57 and the re-read reported `uid_changed` as Partial. Only the mechanism clause is wrong.
The UID moved immediately; the re-read caught it. Nothing about the shipped behaviour changes.

## F2 — CONFIRMED, medium. `tools/gen1-staleness.py` structurally cannot catch F1

`tools/gen1-staleness.py:40`:

    (r"latch(es)?\b", "latch -- MEASURED 2026-09-11: ..."),

`\b` after `latch` blocks the inflected forms:

    latch     -> matches
    latches   -> matches
    latched   -> MISSES
    latching  -> MISSES

Swept the shipped tree: `latch` appears at exactly three sites. The scanner flags `:43` and `.h:80`
(both correct text). The ONE wrong site, `:882`, is the ONE it cannot see — it says "latched".

Suggested pattern: `latch(es|ed|ing)?\b`.

Second, smaller point: the scanner takes files as argv and prints NOTHING when run with no
arguments, exiting 0. A bare `python3 tools/gen1-staleness.py` looks like a clean pass. That is the
`grep -q` false-pass shape the project has been bitten by twice. It should exit non-zero, or
default to its documented file list, when given no files.

Both are dev-only (`tools/`), so neither reaches the PR — but F2 is why F1 survived.

## Mechanical verification of the current tree (re-run this session, not inherited)

| check | result |
|---|---|
| host tests, from `make clean` | **108 run, 0 failed** |
| clang-format (toolchain 18.1.8, slix `.clang-format`) | **95 files checked, 0 need formatting** |
| comment-only classification, 15 shipped commits | **11 comment-only, 3 CHANGELOG-only, 1 code** (`d5e457b`, the dead `is_wipe` term) |
| intra-batch churn, all 15 shipped commits | one pair: `3a8b6ea` -> `4ec5f8c`, 3 lines, **re-wrap only, not a semantic retraction** (see F1 for the semantic one) |
| final-state PR diff | **27 files, +3922 -45** |

Firmware builds were verified by round 9 on this exact tree (both warning-free); the tree has not
changed since, so they were not re-run.
