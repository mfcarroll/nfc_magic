# Round 8 — 16 thread replies

**Status: DRAFT. Not posted.**

Rules for this file: **quote text, never a line number** — his anchors move. Each reply carries at
most three things: the disposition, anything HE got wrong, anything I found while doing it.
Everything else is padding to the person who wrote the finding.

---

## 01 — `scenes/nfc_magic_scene_write_confirm.c`, the wipe title — comment 3989702099

~~~~
Fixed, and you are right that it was unintended — the commit you quote argues the title purely as an
ISO15693 scope statement, which is the tell.

Not the one-token version, though. `is_wipe` goes back to `"Wipe card?"` and the ISO15693 title is
set inside the `iso15693_wipe` branch, next to the `text_height` that branch already sets. That is
the shape the file uses for the write-UID variant. A nested ternary would leave a protocol-specific
string on the line whose whole job is the protocol-neutral default, which is how this happened the
first time.

Two comment lines say why the title is not taken from `is_wipe`: the union is right for the body
dispatch and wrong for anything naming a generation, and that distinction is invisible at the
declaration.
~~~~

---

## 02 — `iso15693_poller.c`, "Every block write in this file funnels through here" — comment 3989702111

~~~~
Confirmed and fixed, and checking it turned up two more senders. The single path this comment
described is four:

- `WRITE AFI` (0x27) and `WRITE DSFID` (0x29), from the clone's identity pass. **Standard** ISO15693
  commands, so unlike a block write at index 56 there is no size floor — any compliant tag in the
  field applies them. A changed AFI can drop a tag out of a selective inventory too.
- the gen1 backdoor, which is your finding.
- the gen2 backdoor, `0xE0` — proprietary, so a conforming tag should reject it. The only one of the
  four with a floor under it.

I did not take "every **data-block** write here, gen1 mentioned separately". The scope now lives on
`ISO15693_MAGIC_FLAGS`, which is the define that makes a frame unaddressed and where a reader of any
of the four sites lands; this comment is two lines pointing at it. A second site carrying a partial
copy is how the first one got stale.

The SDK half is re-verified rather than carried forward: `iso15693_3_poller_write_block` appends
`SUBCARRIER_1 | DATA_RATE_HI` and nothing else.

#251 gets a comment with the AFI/DSFID part.
~~~~

---

## 03 — `write_fail.c`, "The UID was never touched" — comment 3989702126

~~~~
Deleted. Both halves hold.

The second one has a consequence worth stating: since neither rendered string mentions the UID, the
screen never made the claim the comment said it was making. So the app is accidentally right, and
there is no behaviour to fix here — which is the better of the two outcomes, because the poller's
position is that it could not have said so honestly anyway.

What is left is the part that earns its place: why a cut wipe gets reported on this screen rather
than on WipeStopped. Net −2 lines.
~~~~

---

## 04 — `iso15693_poller.h`, the `<= 7` bound — comment 3989702137

~~~~
Re-scoped to the bullet it sits in, and your worked case checks out exactly. I traced it rather than
taking it: the below-claim branch is `if(!claimed_range_attempted) { ...card-present probe...;
continue; }` and it never clears `absent_run`. 200 claimed, 10 held, clock at block 150 — run of 140,
tail-drop clears 10..149, total stuck at 10, cut at 150.

Your second point is the more valuable one and I have cited it in place of the old mechanism. The
deadline's position fixes `block` as the exclusive end of the attempted range and says nothing about
the run length; what holds the gap to seven above the claim is the tripped-run handling, where a run
reaching the threshold is either re-probed back under it or ends the sweep.

The below-claim bullet now carries the counterexample and says outright that the bound does not hold
there, so the two halves cannot drift apart again.
~~~~

---

## 05 — `write_fail.c`, the `blocks_total` pointer — comment 3989702145

~~~~
Fixed at the header rather than at the pointer, which is where you put the fault.

All three loose sites, the two you named plus the field doc itself. The field doc now says
`highest_present + 1` outright and that interior absences fall inside the span rather than reducing
it — they are carried by `failed_count`. `start_wipe`'s contract and the assignment site in the `.c`
follow.

With the header correct, the screen's pointer stops restating what it points at. That restatement was
exactly the duplication the consolidation was meant to remove, and it survived only because it
happened to be the accurate copy. It also clears the ragged wrap that edit left behind
("-- and / it sits at or below the cut, so a sweep").

Kept both axes, since you flagged that they are different questions: "A count, never an index" is
true and does not answer range-size-vs-tally.
~~~~

---

## 06 — `iso15693_poller.c`, "every field in the span except progress_step" — comment 3989702152

~~~~
Three confirmed, counted mechanically rather than by eye — field names between `clone_blocks_total`
and `callback`, every `instance->` reference inside `get_result`, sets diffed. Nineteen fields:
`progress_step` never copied, `clone_afi_failed` and `clone_dsfid_failed` ORed into one behind the
mode gate.

The count now points at the fields rather than restating the mechanism, since that already has an
owner nineteen lines down and repeating it here is the shape you flagged elsewhere this round.
`progress_step`'s own comment gained the half it was missing — it said what the field holds, not that
`get_result` never reports it, so "noted at its own field" would have been false for it.

The orphan is gone, and so is a second one you did not flag, above `gen1_attempted` ("// send -- a").

Comment-only, and proven rather than asserted: `gcc -fpreprocessed -dD -E -P` over the file hashes
identically before and after.
~~~~

---

## 07 — `iso15693_poller.c`, "nothing ever clears commit again" — comment 3989702161

~~~~
Taken, in the narrower form you propose. No gen1 UID-write sequence clears commit afterwards, so a
card left armed by an earlier run is still armed when the next one starts.

The sweep's own zeroing of commit is now named rather than denied, and the reason it does not save the
card is stated as ordering: 56/57 go first, while commit still holds `0x6996`.
~~~~

---

## 08 — `CHANGELOG.md`, the wipe that clears nothing — comment 3989702172

~~~~
Scoped, here and in the source — one finding, one commit, so this and your note on the source are
answered together.

Your second cause I traced rather than took, and it holds: a 28-block dead card sweeps to block 27,
where `claimed_range_attempted` first becomes true, trips with a run of 28, re-probes, finds nothing,
and ends. Index 56 is never addressed.

Both now name the card that can reach 56/57 — one that answers reads at every address — with the two
cases that fall short called out in the source so the next reader does not have to re-derive them.
~~~~

---

## 09 — `iso15693_poller.c`, the same overstatement in source — comment 3989702183

~~~~
Fixed in the same commit as the CHANGELOG, since it is one fact in two places, and near enough your
wording: "on a card the sweep reached index 56/57 on, three WRITE BLOCKs each went out there before
it gave up".
~~~~

---

## 10 — `iso15693_poller.c`, the duplicated `wiped == 0` argument — comment 3989702190

~~~~
Deleted from the scene; the poller keeps it. Your reason for that direction is the right one — it is
the poller's control flow.

Both knock-ons are worth recording because they are what duplication does rather than incidental
untidiness. "This file" was written about `iso15693_poller.c` and points at nothing in a scene that
draws no inference about writes; and the dispositions had already drifted apart, one decision in two
wordings. Deleting the copy settles both at once.

What stays is what the scene is the authority on: which wipe reason codes reach a Details route, and
that "Wipe stopped" can offer Retry without saying the identity check never reached an answer. That
last one is about what a button promises.

Net −4 lines, deletion only.
~~~~

---

## 11 — `nfc_magic_app_i.h`, "that screen" — comment 3989702200

~~~~
Fixed. The screen is named now rather than referred to.

Worth noting how it got there: the round-7 edit removed this parenthetical's overreach and left the
pronoun pointing at the wrong end of a sentence it had just been rewritten around.
~~~~

---

## 12 — `scenes/nfc_magic_scene_write_confirm.c`, "the only warning" — comment 3989702205

~~~~
Taken as you wrote it, since the rest of the paragraph is about this being a static line rather than
a detection — which is a point about consequence.

The two are not redundant, so the replacement says which does which: the title warns about scope and
cannot say anything about cost.
~~~~

---

## 13 — `iso15693_poller.c`, the split doc comment — comment 3989702212

~~~~
Moved, and the ragged wrap with it.

The rejoined line lands at 103 columns, inside the 100–105 the rest of that paragraph runs at — so it
restores its original shape rather than setting a new width.
~~~~

---

## 14 — `partial_details.c`, the test citation — comment 3989702219

~~~~
Thank you for the revert, and the citation is out.

One correction, and it lands in my favour: the citation is not also wrong against my own harness.
`test_cut_at_the_claim_reads_as_past_it` is in `test_write_fail_scene.c`, with its runner call at the
bottom of the same file. The filename was right; only its absence from this repository was the
problem.

I swept for others across `scenes`, `magic`, `views`, `helpers`, the root sources and the CHANGELOG,
for both `test_*.c` and `hosttest`. That was the only one. The line now says why the derivation is
written out — this boundary has been got backwards twice, by both of us — and names nothing invisible.

On the larger ask: yes, and as its own PR. The sizing is in my reply on the main thread; the short
version is that the harness is 49 files and 4,898 lines against this PR's 27 and +4,123, so folding it
in here would more than double what you are reading, at round eight.
~~~~

---

## 15 — `iso15693_poller.c`, "all 28 state and reporting fields" — comment 3989702233

~~~~
27, confirmed mechanically — every `instance->` assignment, `memset` and `iso15693_3_reset` inside
`start_internal`, deduplicated against the struct's declared fields. 24 assignments plus
`memset(uid_readback)`, `memset(clone_failed_bitmap)` and `iso15693_3_reset(data)`. The struct
declares 31, and the four left alone are the four you name.

I took your suggestion rather than correcting the number, because a count goes stale the next time a
field is added and tells a reader nothing they can act on. The comment now names the four and why —
two owned allocations, and two set by the caller and by `write_step`, so resetting them would discard
the run's own inputs — and states the rule: a new field belongs in the reset list unless it is one of
those two shapes.

You are right that the last two are the trap. They are the ones a reader would otherwise read as an
omission.
~~~~

---

## 16 — `write_fail.c`, the scope line and `BITMAP_SIZE * 8` — comment 3989702234

~~~~
Both taken.

**The line budget.** Made the claim true rather than narrowing it. I enumerated every body placement
in the file first — eleven at y=13, two at y=20, one title at y=0 — so "every body" is now a closed
set of two cases, both derived at the top, both coming out at three lines. The two sites that derived
their own now point at it. You are right about the commit message: it claimed the arithmetic was
stated once, and the file did not show that.

The derivations those sites keep are the local ones — that the timed-out body already reaches three
when blocks failed, and that the y=20 body spends its third line on exactly one qualifier out of four
in priority order. Those are decisions about content rather than about pixels.

**`ISO15693_POLLER_MAX_BLOCKS`.** In, in the header beside `BLOCK_BITMAP_SIZE`, with all four sites
using it. You flag it as against the grain of that earlier commit and it is, so: that preference is
about not naming a value whose expression already reads clearly at its one site. This is the opposite
case. Four sites, and the expression does not say what it means — a scene computing `BITMAP_SIZE * 8`
to bound a list is reaching into the poller's storage layout to recover a protocol fact.

`WIPE_MAX_BLOCKS` stays, defined as it, keeping the 21 lines of hardware argument for using that
bound as the sweep's ceiling. That argument is about the wipe and would be wrong to hoist.
~~~~
