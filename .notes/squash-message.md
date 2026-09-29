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
message has to describe the final state, so re-read it against the tree before posting.

**House style, measured not assumed:** `<PR title> (#NNN)` as the first line, body wrapped at **~79-80
columns** (#258's longest line is 79; our own dev messages run to 80), short paragraphs, trailers last.
#258 runs ~20 lines. This feature is much larger, so the ~105 below is defensible; the 190 an earlier
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
the data-block space, at 56/57 with two more backdoor registers at 62/63,
written with four ordinary WRITE BLOCKs -- which any writable tag accepts. So
gen1 is destructive on a non-magic card and is offered only as an opt-in after
the gen2 attempt leaves the UID unchanged. A gen1 clone skips those four
addresses, and reports Partial only where the source actually had blocks that
high.

THE WIPE SWEEPS PAST THE ADVERTISED BLOCK COUNT, because that count is
programmable rather than physical. Measured on a gen2 card: seed all 64 blocks
with per-block markers, clone a 28-block source over it, wipe -- the screen said
Success -- then read with a proxmark. Blocks 28, 40 and 63 still returned their
markers. 36 of 64 blocks survived a wipe that reported unqualified success. The
sweep runs upward until a run of blocks answers neither a write nor a read. Only
a write settles whether a block exists, and on the cards measured a read past
physical capacity fails too, which is what lets a refused write be classified
rather than guessed.

THE UID AND IDENTITY FIELDS ARE VERIFIED BY READ-BACK, not by return value. A
tag can refuse in-band with a well-formed, CRC-valid error response, so the send
returns success; and it can apply a write without answering at all. Both
directions are wrong to trust. The UID is re-read after an RF field
power-cycle -- not because a written UID latches, which it does not on any of
the three gen1 chips measured, but to read from a cleanly re-activated card --
and AFI/DSFID are re-read and compared. A data block counts as written when the
card acknowledges it, except on a card that cannot acknowledge a write (below),
where it is read back.

WRITES CARRY THE CARD'S ADDRESS (#251). Every data block, every identity field
and the gen1 backdoor sequence go out as addressed frames, so a second tag in
the field is not written by them; on ISO15693 a bystander need only be in a
wallet, not on the antenna. Measured on seven cards over four identified chips,
two of the cards of unknown silicon: all seven accept an addressed WRITE BLOCK,
and a write to a UID one byte wrong leaves its block unchanged on every one. On
NXP silicon, blocks 62/63 refuse the addressed form in band and are silent to
the unaddressed one. Writing block 56 moves a gen1 card's UID at once, so
anything that writes there re-takes the address before continuing, and accepts
only the UID that write implies.

SOME SILICON REQUIRES THE OPTION FLAG on writes and says so with its own error
code. TI Tag-it HF-I Plus, identified by that behaviour rather than by its UID,
refuses a WRITE BLOCK whose OPTION bit is clear, addressed or not; without the
flag no data block could be written to that chip at all. The flag is set from
the tag's answer rather than from its UID, because this app is in the business
of changing UIDs. It costs the acknowledgement -- ISO15693-3 10.3.1 has the card
answer only after a standalone EOF, which this SDK cannot send -- so on such a
card the block is read back and compared instead of being counted as refused.

A CLONE THAT LANDS IN A GEN1 CARD'S UID REPAIRS IT. The gen2 verify passes
whenever the card already carries the UID being written, which proves the UID
matches and nothing more; re-cloning the same file is how a card comes to be in
that state. The write that moves the UID is also what identifies the card, so
the run converts to a gen1 clone from that point and puts the intended UID back.

A CLONE ALSO REPORTS WHAT IT LEFT BEHIND. It writes the file's blocks and
nothing else, so on a larger card everything above keeps the previous
contents -- and a gen2 clone reprograms the advertised count down to the file's,
so an ordinary dump shows a clean copy over data that is still readable. The
clone reads above the file's last block and reports data left up there, a card
answering past the count it reports, and a geometry the file did not ask for.
None is a failure; they are notes on a success.

KNOWN LIMITS, in the order they matter:

- gen3 is NOT supported, and a wipe can destroy one. A gen3 card keeps its UID
  in blocks 0x10/0x11 and a configuration signature in 0x14/0x15, well inside
  any claim, and the wipe checks nothing first, so the sweep zeroes both.
  @0x6r1an0y, who wrote proxmark's ISO15693 V3 magic support, reports that on an
  un-finalized card this bricks it permanently; that is their report, not tried
  in testing. The other paths were measured on a genuine un-finalized gen3 card:
  it ignores the gen2 backdoor, so a clone or Write UID reaches the gen1 opt-in,
  and accepting that writes 56/57/62/63 as ordinary data without moving its UID.
  The wipe confirm screen carries the warning; a pre-flight probe is #255.
- A WIPE CAN MOVE A GEN1 CARD'S UID and cannot prevent it. Blocks 56/57 are the
  UID registers, and all five gen1 cards measured took a write there with
  nothing sent before it. A card's history cannot be known, so treat any gen1
  card whose sweep reaches them as exposed. The wipe re-reads the UID afterwards
  and reports a move; it cannot report the absence of one, and a wipe that
  clears nothing does not run the check at all. Tracked in #255.
- #251 is NOT closed. The inventory is still the SDK's 1-slot INVENTORY_T5 with
  no STAY QUIET, so a second tag can answer it -- including the post-wipe UID
  re-read, which addressing cannot fix by construction, since that read exists
  to discover whether the UID changed. And the gen2 backdoor cannot be
  addressed at all: four gen2 cards take it only unaddressed, so another gen2
  magic card in the field takes those frames too.
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
