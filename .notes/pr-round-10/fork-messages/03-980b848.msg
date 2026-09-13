NFC Magic ISO15693: the wipe sweep, an armed gen1 card, and how far past the claim it reaches

Three corrections in iso15693_poller_wipe_blocks, all about the same run.

THE SWEEP REACHES 56/57 FROM A CLAIM OF 49, NOT 57. The old bound was the threshold
for block 56 sitting INSIDE the advertised count, and it missed the other route: the
sweep does not stop at that count, it keeps writing until
ISO15693_POLLER_WIPE_ABSENT_RUN blocks in a row answer nothing. A card silent from
block A is still attempted through A+7, and a write that lands resets the run, so 56
and 57 both go at 49. Measured by sweeping the advertised count against the poller:
44 through 48 leave both untouched, 49 through 52 zero both. The old wording called
a card advertising 49-56 safe when the sweep reaches its UID registers.

Only one of the gen1 cards available is large enough for that: at 28 and 40 blocks
the run opens at the claim and trips seven blocks later, nowhere near 56. The hazard
is narrower than this file said.

THE SWEEP NEVER ZEROES COMMIT. "This sweep does reach commit and zero it" got the
reaching half right -- it writes zeros at every index up to the ceiling, so it gets
to 63 like any other block -- and the zeroing half wrong. An armed card refuses
62/63 with error 0x10. The armed-card wipe reported "Wiped 58/58" on a card
advertising 56, so 56 and 57 were accepted and 62 and 63 were attempted and refused,
and the card was still armed afterwards. The ORDER clause goes with it: ordering
decides nothing when commit cannot be cleared at all, and the de-arm warning below
now carries the measured reason instead.

THE LIVE DENOMINATOR MOVES BELOW THE GEOMETRY GUARD. The advertised count is set
into blocks_total early and deliberately, so the popup does not read "Wiping 33 / 0"
for the whole pass, and is overwritten with the measured figure at the end. The
guard sat between the two, so a card that advertises blocks but reports a block size
of 0 returned with the claim still in blocks_total and carried it into a terminal
event -- which blocks_total's own contract forbids. One statement, eight lines down.
Nothing renders it today; the rule is what is being kept.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
