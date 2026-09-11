NFC Magic ISO15693: name the four fields start_internal skips, instead of counting 27

"covers all 28 state and reporting fields" was wrong by one, and it disagreed with
the sentence after it: the old alloc list had 11 entries, so "sixteen fields short"
implies 27. You spotted the internal inconsistency and picked the right number
from it.

Counted mechanically rather than by eye -- every instance-> assignment, memset and
iso15693_3_reset inside start_internal, deduplicated against the struct's declared
fields. 27 touched: 24 assignments plus memset(uid_readback),
memset(clone_failed_bitmap) and iso15693_3_reset(data). The struct declares 31.

Took your suggestion for what replaces it, which is better than a corrected count.
A count goes stale the next time a field is added and tells a reader nothing they
can act on; what they actually need is the RULE. So the comment now names the four
fields start_internal deliberately leaves alone and why -- poller and clone_source
are owned allocations, target_uid and original_uid are set by the caller and by
write_step, so resetting them would discard the run's own inputs -- and says a new
field belongs in the reset list unless it is one of those two shapes.

Those last two are exactly the trap: they are the ones a "someone adding a field"
reader would otherwise assume were an omission.

Net +5 comment lines, and no number left in it to go stale.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
