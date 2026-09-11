NFC Magic ISO15693: the wipe only reaches 56/57 on a card that answers everywhere

One overstatement in two places -- the CHANGELOG's wipe Limit and the wiped == 0
comment in the source -- so one finding, one commit.

"The refused writes still went to blocks 56/57" is not general. For the first of
the two causes the same sentence names, no frame is transmitted at all:
`if(advertised == 0 || block_size == 0) return 0;` sits above the loop. Nor does
it hold for a write-protected card claiming fewer than 57 blocks, traced rather
than taken on report: below the claim the absent-run trip falls through to
continue, so a 28-block dead card sweeps to block 27, where
claimed_range_attempted first becomes true, trips with a run of 28, re-probes,
finds nothing, and ends. Index 56 is never addressed.

The hazard is real on the card it is about: one that answers reads at every
address, so the sweep keeps going and does reach 56/57. Scoped to that card in
both places, with the two cases that fall short named in the source so the next
reader does not re-derive them.

Conservative direction, so nothing downstream was wrong -- but this paragraph is
the argument for why the short-circuit is a hazard rather than a safe inference,
and an argument that overstates its premise is the easiest kind to dismiss.

The source paragraph rewrapped to the .clang-format ColumnLimit of 99 rather than
the 102-104 it was at. Inserting a sentence mid-paragraph reflows it either way,
so there was no churn-free option and 99 is the configured target.

CHANGELOG +6 -4, comment +12 -9. No code.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
