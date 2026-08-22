# Round 6 threaded replies — DRAFT, not posted

Keyed by his comment id. Post after the push.

---

## thread `3823546078`

Fixed, one conjunct, exactly as written. Your trace is exact including the step I would have got wrong:
`over_capacity` folding in at `:756-760` is what makes `failed_count == blocks_total`, so the guard trips
on a count the back-fill created rather than on anything the card did.

Two tests -- the cut case is Partial, and an UNCUT clone that accepted nothing is still Fail, which is the
half of the guard that has to keep working. Mutation-verified: dropping the conjunct fails the first.

`iso15693_poller.h:38` having already promised the opposite -- "whatever the counts say" -- is the
clearest statement that the code and its contract had diverged, and it is corrected in the same delta.

---

## thread `3823546093`

Fixed. This was the worst of the three back-fill comments for exactly the reason you give: it is the field
a reader consults to decide whether `pass_truncated` needs mode-gating, and `scene_write.c` mode-gates on
it. Believing the comment means deleting the gate, which puts a cut clone on the wipe-specific screen --
this round's blocker running backwards.

It now says BOTH modes, points at the clone's truncation break, and names the gate as the reason it
matters.

---

## thread `3823546100`

Fixed, and the part I had not joined up is your last line: this is *why* `has_details(WipeStopped)` is
unconditional rather than `failed_count > 0`. That is now the reason given in the field doc.

The header keeps the BOTH-modes sentence and attributes the back-fill to the clone, with the mechanism
spelled out -- the sweep breaks out of the loop and both bit-setting sites are inside the body the
deadline gates, so every set bit is below the cut and `blocks_total` is at or below it too. The unreached
blocks are outside the denominator rather than inside the numerator, which is the sentence that makes
`write_fail.c:146`'s subtraction obviously fine instead of suspicious.

---

## thread `3823546110`

Fixed. Split as you asked -- this is the entry a caller reads to learn what Partial covers, so it now says
the back-fill is the clone's and that a cut wipe records nothing above its cut, which makes the flag the
only thing that mentions those blocks there.

And the clause you quoted against the code -- "whatever the counts say" -- is true now that the Fail guard
reads `pass_truncated`. You were right that until then it was the clearest statement of the bug.

---

## thread `3823546119`

Fixed, and you are right on both counts. The refuses-every-write-answers-every-read card proves EVERY
block present -- a refused write falls through to the read, a successful read calls `wipe_note_present` --
so `blocks_total == cut_block` exactly, not "far below".

And the general argument is the one I should have made: getting past the advertised count at all requires
never accumulating `ABSENT_RUN` consecutive absences, because past the claim such a run ends the sweep. So
above the claim the gap is bounded by 8, and "far below" is unreachable in that regime.

The field now describes both cards, in opposite directions, with the mechanism for each -- the large gap
belongs to the card claiming 200 while holding 10, where the cut lands BELOW the claim because under the
claim the sweep never stops on absence alone.

---

## thread `3823546131`

Fixed with the field doc above -- both now point at the same two-card description rather than repeating a
fused one.

The 111-column line went with it, and I checked the file for the same shape: `iso15693_poller.c:169` was
at **155** columns, appended to and never rewrapped since `78e1d168`. `ReflowComments: false` is exactly
why neither of us saw it. Both rewrapped; a scan found nothing else over 110.

---

## thread `3823546143`

Fixed here and at `write_fail.c:130-133`, with the one-block change you suggested: a card that accepts
some writes and then answers reads everywhere.

And the reason code now says why the refuses-every-write card cannot reach this screen -- it clears
nothing, so the wipe short-circuits to NothingWiped first. Which is what `write_fail.c:232-233` said 100
lines away, so the two comments now agree instead of disagreeing.

---

## thread `3823546151`

Accepted, and this is the sharpest of the eleven. The wipe has no magic check at all -- menu, confirm,
sweep -- so an un-finalized gen3 card gets its UID registers AND the configuration signature zeroed, and
comes out with a moved UID no longer identifying as re-writable.

The entry now says that explicitly, keeps your framing that the reader it is written for is the person
about to wipe one, and notes that the post-write UID re-check covers the identity half for free while
nothing speaks for the signature. Still declared rather than handled.

Thank you for checking the proxmark claims against master rather than taking them on trust.

---

## thread `3823546157`

Accepted, and thank you for flagging it against yourself as well -- I would not have found it, because I
checked the shared routing when you first raised the parallel and not the Classic precondition either.

`uid_locked` set unconditionally with no zero-problems shortcut, so `problems_count >= 1` always. The
claim is now Gen2 alone, in the CHANGELOG and in `file_select.c`. Your second point stands too: I dropped
"card-derived", since half of Gen2's checks come from the source file rather than the card.

---

## thread `3823546168`

Softened, and the CardLost route is named rather than left implicit: "reported as such whenever the run
reaches its own result screen -- a card removed at the moment of the cut reports as removed instead, which
is accurate and retryable."

The NothingCloned route is gone with the guard fix, so CardLost is the only one left.

---

## thread `3823546178`

Accepted in full, including the part that inverts the conclusion.

The probe clause does not survive: it was comparing the clone against its own earlier revision while the
sentence around it compared clone against wipe, and the sweep already reads on every refused block. So per
refused block the two passes are the same shape, and the WIPE is the more expensive one on the same
geometry -- it pays an inventory every `ABSENT_RUN` absences and a full trailing re-probe, neither of
which the clone has.

Both other figures corrected too: the 4-byte payload is the sample card rather than the code, and 8x is
the payload ratio where the frame is 2.5-4x, with ~15ms of the 40-70ms being `furi_delay_ms` that no
payload touches. The rule is unchanged; what was backwards was which pass it has to be defensible for.

---

## thread `3823546188`

Accepted. Two independent disassembly facts, and I had fused them: the prefix property says a dropped
block WAS read, the memset says an unread entry reads as zeros. Neither implies the other and the keep
branch leans on both.

Now two load-bearing uses plus a pointer to the memset for the third, which is where the read-back note
already had it right.

---

## thread `3823546205`

Fixed. Three-way now: `has_details` -> Details; else retryable -> Exit; else no right button at all.

You are right that the two-way statement is dangerous specifically because of the instruction it closes
with -- someone adding a reason and reasoning from it concludes a non-retryable reason without details
gets an Exit, which is not what happens.

---

## thread `3823546224`

Took your first reading, and put the reasoning where you said a reader would look for it -- at the branch
ordering in `scene_write.c` rather than in the predicate.

Re-running a wipe against a card whose identity has already moved does not obviously help, and on gen1 it
is another pass over the very blocks that moved it. So the withholding is deliberate, and the comment says
so at the point that decides it. The truncation note still reaches Details there; only the button is
withheld.

---

## thread `3823546230`

Count corrected to six reachable reason codes, with your table's reasoning in the comment.

The `NothingWiped` case gets the paragraph rather than a corrected number, because your second point is
the interesting one: `uid_verified` is false there BY CONSTRUCTION, and the short-circuit's justification
-- no write landed that could have moved the UID -- is the one inference this file declines to draw
anywhere else. The sweep did send three WRITE BLOCKs each at 56 and 57 before giving up, and
`write_identity`'s own comment says a tag can apply a write without answering.

Filed rather than fixed, as you asked: gen1, no card, and the short-circuit predates this PR.

---

## thread `3823546236`

Fixed with `<=`, and your reasoning for which sentence is right at equality is the one in the comment: at
`cut == advertised` the sweep attempted the claimed range and nothing beyond it, and block N itself was
not attempted, so the first branch's wording is the accurate one.

---

## thread `3823546245`

Yes. And inside a function this delta edited, in the hunk directly below the one that swapped its local
array for the shared one.

Five call sites now, and I checked -- no hand-rolled zero loops left in the file. I will not pretend to be
surprised twice.

---

## thread `3823546254`

Both done, and the `+ over_capacity` analysis is exactly right: it survives non-zero only down the
`failures_are_top_tail` branch, which requires an uncut pass, which forces `list_upto` to the full range,
by which point every one of those blocks already has its bit set. The sum read as though the terms were
complementary when one strictly contains the other.

The tally went with it. `nfc_magic_partial_details_any_index` sits beside `append_indices` and takes the
same range, so "asked over exactly the range that will be printed" is structural rather than a promise in
a comment -- and it removes the only raw bit arithmetic in the scene layer, which is also the test site
that let the poller's bitmap idiom become a pair rather than a trio.

And your "Related" is fixed: the `list_upto` comment now says which mode has the problem the bound was
written for, and why the bound is applied in both anyway.

---

## thread `3823546259`

Done as a pair, and for your reason rather than the line count. Both clears sit inside the densest
reasoning in the file, three and five lines from a set, where `|=` and `&= ~` differ by two characters and
a slip reads as correct either way.

Four sets and two clears behind named calls; three raw references remain and all three are intentional
(the field declaration and one inside each helper). The test site is gone, absorbed by `any_index` on the
other thread, so a trio was never needed.

---

## thread `3823546264`

We converged, and you saved me the wrong round rather than merely re-scoping it.

I had built the `switch(reason)` version and measured it at +25 code / +21 comment before stopping -- for
your reason: seven of twelve bodies are dynamic, and `wipe_stopped` arriving dynamic with a conditional
third line moved the ratio further from a table. So I had reached "not the fuller form" independently and
had not reached what the right form was.

`{reason -> title}` is in: one lookup, one hoisted call, twelve identical calls gone. +12 comment / +8
code, which I am reporting rather than dressing up -- the lookup is longer than the calls it deletes, and
what it buys is the invariant stated once. The reasoning against the fuller form is in the file now so the
next reader does not re-derive it and reach for it.

The twelve `widget_add_string_multiline_element` calls varying only in (x, y) I have left: the y values
carry the line-budget reasoning you measured for us in round 4, and I would rather move those in the
comment cut than bury them in a table.

---

## thread `3823546276`

Fixed at all three sites. `COUNT_OF` is in the fake's `furi.h` too, copied from
`furi/core/core_defines.h`.

You are right about the failure mode -- widening the array to `uint16_t` for a block index above 255 would
silently shorten all three loops -- and this file does contemplate that elsewhere, so the note about why
now sits at the array rather than at the loops.

---

## thread `3823546285`

Fixed with the CHANGELOG claim. Classic dropped from the sentence; the ISO15693 rationale and the
Gen1/Gen4/USCUID-UL clause are unchanged, as you suggested.
