NFC Magic ISO15693: four simplifications from Round 7

Threads 3948189049, 3948189029, 3948189038, 3948189071.

THE SHARED WRITE TAIL, and you are right that it is the highest-value of these
because of WHERE it sits rather than how long it is. The gen2 and gen1 verify
arms ended in byte-identical nine-line tails differing only in skip_backdoor,
~80 lines apart -- the seam where a change to the gen2 arm silently fails to
reach the gen1 one. Extracted as iso15693_poller_finish_write(instance,
iso_poller, skip_backdoor), with the comment naming what the flag is FOR: a gen2
UID lives in its own register space so data blocks cannot disturb it, a gen1 UID
lives in blocks 56/57/62/63 so a gen1 clone must skip them.

THE DEAD CONJUNCT. `mode != Iso15693PollerModeInfo` cannot be false there --
Info returns unconditionally six lines up. Dropped, and the comment now says
that early return is what guarantees CardDetected's "not sent in Info mode".

ALLOC'S PARTIAL RESET LIST. It set 10 fields where start_internal sets 28,
including all 10 -- so it read as A reset list while being sixteen short of the
real one, and anyone adding a field would find two lists and have to work out
which binds. Cut to the three allocations plus running/callback/context, which
are the load-bearing lines, with a comment naming start_internal as
authoritative. Safe because running starts false and every public entry point
routes through start_internal, which asserts !running.

THE TITLE SWITCH ORDER. Partial sat eleventh where the render chain has it
fourth, and every other case lines up. Moved to chain position, which is what
makes "does every branch still have the right title?" a single-pass check --
part of the point of hoisting the titles out.

107 host tests green, and the extraction is behaviour-preserving by construction:
the same statements with one parameter substituted. NOT yet protected by a test
-- flipping skip_backdoor at either call site leaves all 107 green, which is the
seam the extraction exists to guard. The next commit fixes that.

