NFC Magic ISO15693: the succeeded-blocks figure is one subtraction, for three callers

Three lines on the write-fail screen print how many blocks succeeded, each
spelling it (a >= b) ? a - b : 0 with no reason given at any of them.

nfc_magic_iso15693_blocks_ok carries the reason, and carries it for all three.
blocks_total has three forms, not one: the wipe reports what it MEASURED, a gen1
clone reports a logical count with the four backdoor blocks deducted, and a gen2
clone reports the SOURCE's block count. The deduction varies too -- the clone's
call passes failed_count plus over_capacity, not failed_count alone. So the note
leads with the shape rather than one caller's version of it: the total and the
deduction are counted independently, whichever pair a caller passes, and
blocks_total's own contract lives in the header rather than being restated here.

Saturating because those two are independent accountings and have disagreed: the
sweep's back-fill could leave failed_count above blocks_total and render
"Wiped 0/20, not cleared: 44". That is fixed in the sweep; this is a statement
about what reaches the user if it recurs. The counts are unsigned, so an
inversion would otherwise print near 65535.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
