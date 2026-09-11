NFC Magic ISO15693: the <= 7 gap bound is an above-the-claim fact, not a universal one

cut_block's doc said the gap to blocks_total is held to
ISO15693_POLLER_WIPE_ABSENT_RUN - 1 "on every path". You refuted it from the
bullet six lines down, which describes a card claiming 200 while holding 10 and
says "THIS is where the gap gets large". Both were in the same field doc and
the second one is right.

Traced it rather than taking it: the below-claim branch is
`if(!claimed_range_attempted) { ...card-present probe...; continue; }`, and it
does not clear absent_run. So under the claim the run grows without limit. His
worked case checks out exactly -- 200 claimed, 10 held, clock at block 150:
absent_run 140, the tail-drop clears 10..149, blocks_total stuck at 10, cut_block
150. A gap of 140, not 7.

You also name the wrong mechanism in the old text, and that part is worth more
than the scoping. "The deadline is tested at the TOP of the iteration" is what
makes block the exclusive end of the attempted range; it says nothing about
absent_run. What actually holds the gap to 7 above the claim is the tripped-run
handling: a run reaching the threshold is either re-probed back under it or ends
the sweep, so no iteration up there starts with more than 7 absences open. Cited
that instead, so a reader checking the bound looks at the code that provides it.

The origin is traceable. Your round-7 note scoped this to getting past the claim;
235b5e0e generalised it to "on every path" while fixing something else. Widening
a claim is the cheapest way to break one, and nothing checks for it.

The below-claim bullet now carries the counterexample and says outright that the
bound does not hold there, so the two halves cannot drift apart again. Net +3
comment lines.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
