NFC Magic ISO15693: blocks_total is a RANGE SIZE, and now the header says so

The round-7 consolidation cut prose from the write-fail screen on the premise that
the header owned the fact, and pointed at a header stating the opposite reading.

The pointer said blocks_total is "the size of the range up to the highest proven
block, NOT a tally of proven blocks, since interior absences fold in". The header
said "the number of blocks the card PROVED it holds" -- the tally reading. The
code agrees with the pointer: clone_blocks_total = highest_present + 1, and
wipe_note_present advances highest_present past interior absences while folding
them into failed_count. So the figure is a range size and the header had never
been updated.

Note which axis this is NOT about. "A count, never an index" at the next line is
true and settles count-vs-index; range-size-vs-tally is a different question and
that sentence does not answer it. Both are kept.

Three sites carried the loose wording:

  - poller.h, the field doc. Now states highest_present + 1 and why interior
    absences fall inside the span rather than reducing it.
  - poller.h, start_wipe's contract: "what the card proved it holds" ->
    "spans up to the highest block the card proved it holds".
  - poller.c at the assignment: "what the card proved it holds" -> "the range the
    card proved". The line sits on highest_present + 1, so the code carries the
    rest.

With the header correct, the screen's pointer stops restating what it points at.
That restatement was the duplication the consolidation was supposed to remove, and
it survived only because it was the accurate copy. It also fixes the ragged wrap
that edit left there ("-- and / it sits at or below the cut, so a sweep").

Header +9 -6, poller.c +1 -1, screen +4 -6. No code.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
