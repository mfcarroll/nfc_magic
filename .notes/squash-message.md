# DRAFT — the squash commit message to offer mishamyte at merge time

**Never reaches the fork** (`sync-to-fork.sh:17` excludes `.notes/`), so there is nothing to remove.

## What this is for, since two earlier readings of it here were wrong

`xMasterX/all-the-plugins` squash-merges. The squash body defaults to GitHub's **concatenation of the
branch's commit messages** — not the PR description, which is never used (#258's body is 9949 chars
against a 799-char squash message). And the merger can override that default in the merge box, which is
exactly what he did on #258: an 799-byte purpose-built message where the concatenation would have been
4064.

Ours would concatenate to roughly **1,000 lines** across the branch. Nobody wants that in `git log`, so
an override is near-certain — which means the only lever we have is to **hand him a message**. He is the
person clicking merge; a PR comment is how he gets it.

**TIMING: not yet.** Post it when merge nears — an approval, or him asking whether it is done. The
message has to describe the final state, and the gen1 B-round plus the C/D passes will change it.

**House style, measured not assumed:** `<PR title> (#NNN)` as the first line, body wrapped at **~79-80
columns** (#258's longest line is 79; our own dev messages run to 80), short paragraphs, trailers last.
#258 runs ~20 lines. This feature is much larger, so the ~60 below is defensible; the 190 an earlier
version of this file had was not.

**What belongs in it:** what the feature is, the design fact that shapes everything (the two
generations are not symmetrical), the hardware measurement that changed the design, and the limits a
future maintainer must not rediscover. **What does not:** anything already in the code (the
`view_dispatcher` queue bound, the de-arming argument, the cache-prefix property), the derivations, and
every trace of process.

---

~~~~
NFC Magic: ISO15693 / NfcV support (#250)

Adds magic ISO15693 / NfcV to the app's existing magic-card framework: detection
in the "Check Magic Tag" scan, Info, clone from a .nfc, wipe, and manual UID
write. The UID writes are ported from proxmark3 armsrc/iso15693.c
(SetTag15693Uid / _v2), also GPLv3; the clone flow, capacity handling and
warnings are built on top.

Detection cannot be non-destructive here: magic status on ISO15693 is only
confirmable by writing, so any activating NfcV tag is treated as a candidate and
the destructive paths are gated behind explicit consent instead.

THE TWO GENERATIONS ARE NOT SYMMETRICAL, and that shapes the rest. A gen2 UID
lives in a separate backdoor register space, so data-block writes cannot disturb
it and the clone writes the UID first, then the blocks. A gen1 UID lives INSIDE
the data-block space, at 56/57 plus unlock/commit at 62/63, written with four
ordinary WRITE BLOCKs -- which any writable tag accepts. So gen1 is destructive
on a non-magic card, is offered only as an opt-in after the gen2 attempt leaves
the UID unchanged, and a gen1 clone must skip those four blocks and therefore
reports Partial rather than a clean Success.

THE WIPE SWEEPS PAST THE ADVERTISED BLOCK COUNT, because that count is
programmable rather than physical. Measured 2026-08-04 on a gen2 card: seed all
64 blocks with per-block markers, clone a 28-block source over it, wipe -- the
screen said Success -- then read with a proxmark. Block 20 was zeroed; blocks
28, 40 and 63 still returned their markers. 36 of 64 blocks survived a wipe that
reported unqualified success. The sweep now runs upward until a run of blocks
answers neither a write nor a read, and only a write settles whether a block
exists: past physical capacity, reads fail too, which is what lets a refused
write be classified rather than guessed at.

Writes are verified by read-back rather than by return value, across the board.
A tag can refuse in-band with a well-formed, CRC-valid error response, so the
send returns success; and it can apply a write without answering at all. Both
directions are wrong to trust, so the UID is re-read after an RF field
power-cycle, since a card latches a written UID only on the next power-up.

KNOWN LIMITS, in the order they matter:

- gen3 is NOT supported, and a wipe can destroy one. @0x6r1an0y, who wrote
  proxmark's ISO15693 V3 magic support, reports that zeroing blocks 0x14/0x15 on
  an un-finalized V3 card does not merely clear the signature but bricks the
  card permanently. No gen3 card exists on either side of this PR, so that is
  attributed, not observed. The wipe confirm screen carries it; a pre-flight
  probe is #255.
- gen1 is validated on a single card (ST LRi2K, 2026-09-08): the UID write works
  and is reversible, and its backdoor registers accept writes WITHOUT
  acknowledging -- so no write-based probe of them can have a meaningful
  negative. Whether the UID latches on power-up or immediately is still
  unmeasured, and the header says so at each affected entry.
- Writes and inventory are unaddressed (#251). The SDK builds WRITE BLOCK with
  no ADDRESSED flag and no UID, and its inventory is 1-slot, so a second
  ISO15693 tag in the field receives the writes too and can answer the
  read-back. Split out rather than fixed here.
- A source much larger than the target can lose its "Card too small" verdict to
  the pass clock, since that pass is dominated by failing blocks and a
  genuinely-too-small card presents one long run of them. Left as under-claiming
  deliberately: a cut run reports Partial with Retry rather than asserting a
  verdict about the user's hardware that the pass never finished testing.

Refs #251, #255. #252 and #253 are pre-existing behaviours in the shared write
scene, found while doing this work and filed separately.
~~~~

---

## Leftover PR-body material

Optional polish only — the PR page is what a reader sees and never enters git history. The sections
below were drafted when this file was aimed at the wrong artefact; keep them if the body ever gets
rewritten, and note the AI-disclosure decision is still open.
