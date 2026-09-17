NFC Magic ISO15693: the three succeeded-blocks subtractions say why they saturate

Three lines on the write-fail screen print how many blocks succeeded, and all
three spelled it (a >= b) ? a - b : 0 with no reason given at any of them. The
reason sat 600 lines away in the poller, beside the bug that motivated it rather
than the code that depends on it.

nfc_magic_iso15693_blocks_ok carries it. blocks_total and failed_count are not
two halves of one count: the total is what the run MEASURED -- the wipe reports
the range the card proved, not the range it advertised -- or, on a gen1 clone, a
logical count with the four backdoor blocks deducted, while failures are
recorded at TRUE block indices. Two accountings, and they have disagreed before:
the sweep's back-fill could leave failed_count above blocks_total and render
"Wiped 0/20, not cleared: 44".

The comment is explicit that this is NOT a second fix for that -- it is fixed in
the sweep -- but a statement about what reaches the user if it ever recurs.
These counts are unsigned, so an inversion prints as a number near 65535.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
