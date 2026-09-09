NFC Magic ISO15693: rewrap the two lines ufbt format cannot see

Threads 3948189078 and 3948189092. ColumnLimit is 99 and ReflowComments is
false, so clang-format leaves comment width alone -- you ran ufbt format and it
is clean. Same class as the 155-column line in round 6.

write_fail.c:400 was 189 columns. file_select.c:114 was 110 against 96-104 for
the surrounding block, where fdd2a5d3 rewrote the sentence without rewrapping
the paragraph.

Worth being explicit that this does NOT reopen the decision the round-7 cut
made. That pass deliberately did not rewrap these files to 99, because moving
every line would have buried a comment-only diff in churn -- and that reasoning
holds for a 104-column line. It does not hold for 189, which is twice the limit
and simply unreadable in a review pane.

No COMMENT in the ISO15693 surface now exceeds 106, the files' own de-facto
width. One line still does: iso15693_info.c:218, a 117-column string literal in
the chip-description table, there since the round-1 fixes and not among your
findings. Left alone deliberately -- splitting a data-table string across a
concatenation is churn on a table, not a readability gain, and it is not a claim
that can drift.

108 host tests unchanged.

