NFC Magic ISO15693: one UID formatter for the four screens printing one

The one code change in this delta, asked for in Round 6.

Four sites formatted a UID by hand, and the interesting part is that three of
them implemented the SAME two-group layout in two different spellings: the two
write-fail branches wrote `if(i == 4)` before the byte, write_confirm wrote
`if(i == 3)` after it. Only the Info screen genuinely differs, and the reason
for the difference -- 23 characters against 17, and 128px of line -- was
written down at exactly one of the four.

So: iso15693_info_cat_uid with two named policies, Spaced and Grouped, and the
width argument stated on the enum where both callers can see it. iso15693_info.c
is where it goes because that file is already the ISO15693 presentation helper.

Honest accounting: this is the one commit in the delta that ADDS. Comment +5,
code +11 across the shipped files. Four loops of three or four lines become four
one-line calls, but the function, the enum and the declaration cost more than
the loops did. I am reporting that rather than dressing it up, the same way as
the render table last round. What it buys is that the layout is one decision,
and that a screen cannot quietly start rendering UIDs differently from its
neighbour -- which is not hypothetical, since two spellings of the same rule is
what was already there.

The reason it is worth the eleven lines: it is now TESTED. test_uid_format.c
asserts both policies' output and both widths, which is coverage that could not
exist while the format was four inline loops. Three mutations, all caught:

  i == 4 -> i == 3 in the group break        3 of 5 cases fail
  "%02X" -> "%X"                             5 of 5 cases fail
  drop the i > 0 guard (lead with a space)   3 of 5 cases fail

Host tests 101 -> 106. That count change is deliberate and is this commit's
doing; the comment-only commits in this delta all held at exactly 101.

