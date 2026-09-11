NFC Magic ISO15693: three exceptions to the get_result mapping, not one

The struct span comment said get_result copies "every field in the span except
progress_step". The correct statement was already nineteen lines down, at
clone_afi_failed: "Two flags, one result field: get_result ORs them behind a mode
gate." Two comments in one struct, and the wrong one is the one a reader meets
first.

Counted mechanically rather than by eye -- the field names between
clone_blocks_total and callback against every instance-> reference in get_result,
sets diffed. Nineteen fields, three exceptions: progress_step is never copied, and
clone_afi_failed / clone_dsfid_failed are ORed into one result field behind the
mode gate.

The count now points at the fields rather than restating the mechanism, since the
mechanism already has an owner at clone_afi_failed. progress_step's own comment
gains the half it was missing -- it said what the field holds, not that get_result
never reports it, so "noted at its own field" would not have been true for it.

Two ragged wraps fixed while in the same struct: the 35-character orphan
"// function is the mapping, and", and "// send -- a" above gen1_attempted.

Comment-only, and checked rather than claimed: the file stripped of comments by
the toolchain's GCC hashes identically before and after.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
