# Round 7 — first assessment. NOTHING ANSWERED, NOTHING PUSHED.

His review: **2026-09-07, `COMMENTED`, 20 threads.** Verbatim in [received/](received/). He also pushed a
merge of upstream `dev` into the PR branch on 2026-09-08 (`d31f5162`) to clear conflicts, and said so in
a thread comment — housekeeping, not a code change of ours.

**He says: "Nothing blocking. Twelve claim-accuracy items, five simplifications, two long lines."**

## THE THING THAT MATTERS MOST: he reviewed the PRE-CUT code

Our comment cut is 16 commits sitting unpushed on dev. His Round 7 is against `049029c9`, which does not
contain it. Three consequences, in order of importance:

**1. The cut closes exactly ONE of Round 7 — verified per thread, not scanned.** A first pass with a
text-matching scan said three, and that was wrong: the scan matched blockquote fragments that the cut had
rewrapped, so "rewritten" got read as "fixed". The record of that mistake stays here because it is the
same error as everything below — treating a changed comment as a corrected one.

- **`write_fail.c:336` (three duplicated UID loops) — CLOSED, but not in the shape he proposed.** He
  suggested `nfc_magic_iso15693_uid_cat` covering three sites, with the Info screen's spaced form
  "deliberately stay[ing] out of it". We folded ALL FOUR behind a two-policy enum
  (`iso15693_info_cat_uid`, `Iso15693UidFormatSpaced` / `Grouped`). Defensible — the two widths, 23 vs
  17 characters, are now named on the enum where both callers see them, instead of living as prose on
  one copy — but it is a DIVERGENCE from his suggestion, not agreement with it. Say so.
- **`write_fail.c:271` — STILL PRESENT** at `:273`, verbatim: "short-circuits to this screen before any
  of the truncation reporting".
- **`poller.h:178` — STILL PRESENT** at `:164-166`, and the cut COMPRESSED the refuted reasoning while
  keeping it word for word.

**2. The cut PRESERVED FOUR claims he has now flagged, and that is the finding that matters.**

Not two, as first written here — `poller.c:41`, `poller.h:160`, `poller.h:164` and `write_fail.c:273`.
The pattern is consistent and worth naming exactly: **the cut treated each comment as a unit to SHORTEN,
not a claim to CHECK.** Compression preserved truth-value. Several of these sentences are now tighter,
better-placed, and still wrong — and in two cases the false sentence ended up NEARER the text that
contradicts it.

So the honest framing for the reply is not "the cut is its own best argument". It is: the cut did what it
set out to do on duplication and did nothing at all for accuracy, and Round 7 is the proof. The
ownership model still stands — one owner per fact is right — but it makes the surviving copy
authoritative, which raises the cost of that copy being wrong rather than lowering it.

The two clearest cases:

- **`poller.h:160`** — he flags "Two different cards **separate them, in opposite directions**" as
  re-introducing the round-6 error in the framing sentence directly above its own correction. **The cut
  kept that sentence and tightened the words around it.** Arguably it made it worse: the false framing
  now sits closer to the bullet that contradicts it. `blocks_total` is `highest_present + 1` and is
  structurally `<=` the cut, which the line above says.
- **`poller.c:41`** — the `COUNT_OF` rationale is inverted, "in the comment whose whole job is to stop
  someone reverting this". Widening the element type makes a `sizeof`-bounded loop run *more* iterations,
  not fewer — an out-of-bounds read. The cut moved and tightened this block and left the inversion.

That is worth conceding plainly in the reply. It is also the sharpest available argument FOR the cut: a
pass that reduces duplication without re-verifying leaves the surviving copy authoritative and wrong.

**3. His line references will drift once the cut lands.** Every `poller.h` and `write_fail.c` number in
his review moves. Answer per-thread on GitHub (the threads anchor themselves) and quote the text, not the
line.

## The open sequencing decision is now RESOLVED by his reply

The brief has carried: "push now as an unprompted round, or hold until he replies so the delta can be
framed as an answer." **He has replied.** So the cut can go up framed as a partial answer to Round 7
rather than as an unprompted round — which was the better of the two options and is now available.

Recommended: **push the cut as-is**, and in the reply name the Round 7 threads it already closes and the
ones it does not. Do NOT fold Round 7's fixes into it; it was promised as one decision with nothing else
in it, and that promise is worth more than landing them a round earlier. Round 7 then answers as its own
delta on top, where the line numbers are stable.

## TWO CORRECTIONS HE OWES US — both to be accepted, one is a CODE revert

**`partial_details.c:92` — he asks us to REVERT the `<` → `<=` we made for him in round 6 (`8c9f9b3`).**
His round-6 finding was wrong and he says so: `blocks_advertised` is a COUNT, `cut_block` is an INDEX, so
at equality the claimed blocks are 0..N-1 and the cut sits at index N — the first block *past* the claim.
The wording he sent us away from was the accurate one. He also wants the three-line justification at
`:89-91` dropped as a non-sequitur: "block N itself was not attempted" is equally true of both branches,
so it cannot pick between them. **Agree on both counts** — and note this is a code change, so it needs a
test check.

**A comment in our round-6 delta cost him a false finding.** He read "the wipe short-circuits to
NothingWiped before any truncation reporting", believed it, worked up a finding, and withdrew it before
posting. He flags it as the strongest argument for the cut, since we had written the accurate version of
the same fact 90 lines earlier. Accept it as such.

## Verified-still-present after the cut, so these are real work

`CHANGELOG.md:147` (user-facing: a gen3 card lands on the gen1 opt-in screen titled "Not gen2 magic
card", not "not a magic tag" — our own `nfc_magic_app_i.h:107-109` says so), `app_i.h:145`,
`poller.h:160`, `poller.c:41`, `poller.c:184`, `poller.c:187`, `poller.c:546`, `write_fail.c:167`,
`write_fail.c:241`, `write_fail.c:392` (**189 columns**), `file_select.c` (one line over 106), plus the
simplifications at `poller.c:1434`/`:1499`/`:1512` and `write_fail.c:94`, and `partial_details.c:92`.

On the long lines: the cut deliberately did NOT rewrap these files to 99 columns, because moving every
line would have buried the cut in churn. That reasoning holds for a 104-column line and does not hold for
189 — same class as the 155-column line he flagged in round 6.

## Also on the thread, not from him

`0x6r1an0y` — who wrote proxmark's ISO15693 magic V3 support — turned up on 2026-08-29, offered a set of
tags, and added a substantive note to #255: zeroing `0x14`/`0x15` on an un-finalized V3 card does more
than clear the signature. Read that comment properly before the gen3 work. mishamyte also said he has a
batch of V2 tags for testing.

## Held open from earlier rounds

The `:752` budget thread and `PASS_MAX_MS`, which he says this round's findings continue.
