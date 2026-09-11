# Round 8 — 16 thread replies

**Status: DRAFT. Not posted.**

Rules for this file: **quote text, never a line number** — his anchors move. Each reply carries at
most three things: the disposition, anything HE got wrong, anything I found while doing it.
Everything else is padding to the person who wrote the finding.

---

## 01 — `scenes/nfc_magic_scene_write_confirm.c`, the wipe title — comment 3989702099

~~~~
Fixed, and yes — unintended.

Not the one-token version, though. `is_wipe` goes back to `"Wipe card?"` and the ISO15693 title is
set inside the `iso15693_wipe` branch, alongside the `text_height` it already sets there — the shape
the file uses for the write-UID variant. A ternary would leave a protocol-specific string on the line
whose whole job is the protocol-neutral default, which is how this happened the first time.
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

One consequence worth stating: since neither string mentions the UID, the screen never made the
claim the comment said it was making. So there is no behaviour to fix here, only the sentence.

Net −2 lines; the reason a cut wipe lands on this screen rather than WipeStopped stays.
~~~~

---

## 04 — `iso15693_poller.h`, the `<= 7` bound — comment 3989702137

~~~~
Re-scoped to the bullet it sits in. I traced the 200/10 case rather than taking it, and it comes out
at your numbers exactly.

Your correction of the MECHANISM is the more valuable half and the tripped-run handling is what the
comment cites now — a reader checking the bound needs the code that provides it.

The below-claim bullet carries the counterexample and says outright that the bound does not hold
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

With the header correct the screen's pointer stops restating it — and that clears the ragged wrap the
earlier edit left there ("-- and / it sits at or below the cut, so a sweep").

Both axes kept: "A count, never an index" is true and does not answer range-size-vs-tally.
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
Taken, in the narrower form you propose, with the sweep's own zeroing of commit named rather than
denied and ordering given as the reason it does not save the card.
~~~~

---

## 08 — `CHANGELOG.md`, the wipe that clears nothing — comment 3989702172

~~~~
Scoped, here and in the source — one finding, one commit, so this and your note on the source are
answered together.

Your second cause I traced rather than took, and it holds: a 28-block dead card sweeps to block 27,
where `claimed_range_attempted` first becomes true, trips with a run of 28, re-probes, finds nothing,
and ends. Index 56 is never addressed.

Both now name the card that can reach 56/57, with the two cases that fall short called out in the
source so the next reader does not re-derive them.
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
Deleted from the scene; the poller keeps it, for the reason you give. Both knock-ons go with the
copy, including the drifted disposition.

What stays is what the scene is the authority on: which wipe reason codes reach a Details route, and
that "Wipe stopped" can offer Retry without saying the identity check never reached an answer — that
last one is about what a button promises rather than about the poller's control flow.

Net −4 lines, deletion only.
~~~~

---

## 11 — `nfc_magic_app_i.h`, "that screen" — comment 3989702200

~~~~
Fixed — NothingWiped is named rather than referred to.
~~~~

---

## 12 — `scenes/nfc_magic_scene_write_confirm.c`, "the only warning" — comment 3989702205

~~~~
Taken as you wrote it. The replacement distinguishes the two rather than dropping one: the title
warns about scope and cannot say anything about cost.
~~~~

---

## 13 — `iso15693_poller.c`, the split doc comment — comment 3989702212

~~~~
Moved, and the ragged wrap with it. The rejoined line lands at 103 columns, inside the 100–105 the
rest of that paragraph runs at, so it restores the original shape rather than setting a new width.
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
for both `test_*.c` and `hosttest`. That was the only one.

On the larger ask: yes, and as its own PR — the sizing is in my reply on the main thread.
~~~~

---

## 15 — `iso15693_poller.c`, "all 28 state and reporting fields" — comment 3989702233

~~~~
27, confirmed mechanically — every `instance->` assignment, `memset` and `iso15693_3_reset` inside
`start_internal`, deduplicated against the struct's declared fields. 24 assignments plus
`memset(uid_readback)`, `memset(clone_failed_bitmap)` and `iso15693_3_reset(data)`. The struct
declares 31, and the four left alone are the four you name.

I took your suggestion rather than correcting the number — a count goes stale the next time a field
is added. The comment names the four and why, and states the rule behind them: a new field belongs in
the reset list unless it is an owned allocation or set by the caller.
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

What those sites keep is local — which line the timed-out string goes in, and which one qualifier of
four the y=20 body spends its third line on. Decisions about content rather than about pixels.

**`ISO15693_POLLER_MAX_BLOCKS`.** In, in the header beside `BLOCK_BITMAP_SIZE`, with all four sites
using it. You flag it as against the grain of that earlier commit and it is, so: that preference is
about not naming a value whose expression already reads clearly at its one site. This is the opposite
case. Four sites, and the expression does not say what it means — a scene computing `BITMAP_SIZE * 8`
to bound a list is reaching into the poller's storage layout to recover a protocol fact.

`WIPE_MAX_BLOCKS` stays, defined as it, keeping the 21 lines of hardware argument for using that
bound as the sweep's ceiling. That argument is about the wipe and would be wrong to hoist.
~~~~
