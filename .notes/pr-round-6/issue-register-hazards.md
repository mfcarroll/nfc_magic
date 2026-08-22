# DRAFT issue — NOT FILED

Target: `xMasterX/all-the-plugins`, same repo as #252 and #253. Reference it from the PR reply once filed.

**Title:** NFC Magic: a wipe overwrites gen3 and armed-gen1 magic registers with no warning, and only one of the two can be detected first

---

## Summary

The ISO15693 wipe zeroes every data block the card physically holds. On two magic generations, some of
those blocks are not data — they are the registers that make the card magic. The wipe cannot tell, because
it does no magic detection at all: the flow is menu → confirm → sweep, with no backdoor probe and no
identification step.

One of the two is cheaply detectable before the sweep. The other is not, and that asymmetry is the point of
this issue.

## gen3 — detectable, currently unhandled

A third ISO15693 magic generation exists (proxmark's `hf 15 csetuid --v3`, `cmdhf15.c:3405`). It keeps its
UID in blocks `0x10`/`0x11` and a configuration signature `A5 2B 44 2C` / `21 AE 93 00` in `0x14`/`0x15`,
and stays rewritable until `hf 15 cfinalize` locks it permanently.

All four of those blocks are inside the data range of even a 32-block card, so a wipe zeroes them:

- **`0x10`/`0x11`** — the UID moves.
- **`0x14`/`0x15`** — the signature goes, and proxmark reads exactly that signature to decide the tag is
  still in configuration mode (`cmdhf15.c:3540`: "signature in blocks 0x14/0x15 not found — already
  finalized or not a V3 tag"). So the card comes out no longer identifying as re-writable.

What exists today is a CHANGELOG entry declaring gen3 unsupported and saying the wipe overwrites those
blocks. That warns someone who reads release notes. Someone who picks Wipe with a gen3 card on the reader
sees only the generic "Wipe card?" confirm.

The wipe's post-power-cycle UID re-check does surface `uid_changed` afterwards, so the identity half is
*reported* — but incidentally, as it would be for any card whose UID shifts. Nothing speaks for the
signature, and nothing warns beforehand.

**Proposed fix.** Read `0x14`/`0x15` before the sweep and compare the signature. Two block reads. The
signature is present only while the card is un-finalized, which is exactly the window in which the hazard
exists — so the check is precise rather than heuristic. On a match, warn naming what the wipe is about to
overwrite, and require the same kind of explicit opt-in the gen1 fallback already uses.

Not proposing gen3 *support* — writing gen3 UIDs is a separate feature and a third generation in the
opt-in ladder. This is only about not destroying one silently.

## armed gen1 — not detectable

gen1 keeps its UID in blocks 56/57 with unlock/commit in 62/63. The wipe clears them deliberately: on a
gen2 card they are ordinary user data, and sparing them would leave real data behind on the card most
people have. That trade is documented in the poller and is not in question here.

The hazard is narrower. A wipe cannot *arm* a gen1 UID change by itself — arming needs `0x6996` in the
commit block and a wipe writes zero — but that says nothing about a card left **already armed** by an
earlier gen1 UID write. On such a card the wipe zeroing 56/57 can move the UID.

**There is no pre-flight check available.** gen1 has no signature to read, and an armed card is
indistinguishable from any other ISO15693 card without writing to it. No write ordering fixes it either:
the registers latch on the next power-up, so anything that would de-arm the card is itself a write to the
blocks in question.

So the only mitigation is the one already implemented: re-read the UID after the wipe, behind a field
power-cycle, and report a change as Partial (`uid_changed`) rather than promising the UID survived. That is
detection *after* the fact, and it is the best available.

## Why one issue

Same class — a magic generation whose registers live in ordinary data space, on a card the wipe cannot
identify — and the useful content is the contrast:

| | registers | pre-flight check | current state |
|---|---|---|---|
| gen3 | `0x10`/`0x11`, `0x14`/`0x15` | **yes** — signature at `0x14`/`0x15` | declared in the CHANGELOG only |
| armed gen1 | 56/57, 62/63 | **no** — nothing to read | post-wipe UID re-check reports a change |

Filing both together so the gen1 half is not read as an oversight when the gen3 half gets a check it
cannot have.

## Not blocking

Neither case affects the gen2 cards this feature was built and validated for. gen3 is declared unsupported
in the CHANGELOG; the armed-gen1 case is carried as an open question in the poller with the UID re-check as
its mitigation. Recording both so they are visible rather than resident in code comments.

No gen3 card is available to test against, and no gen1 card either — which is also why neither has been
exercised on hardware.
