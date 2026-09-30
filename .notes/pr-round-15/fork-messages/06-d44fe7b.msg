NFC Magic ISO15693: what a clone leaves behind, and what it says about it

The survey that reads above the source, and the result screens it feeds. They
are one change: the notes have nothing to say without the survey, and the survey
has nowhere to say it without them.

== WHAT THE CLONE LEAVES BEHIND ==

A clone writes the source's blocks and leaves the rest alone, which is right --
destroying what the user did not ask about is the wipe's job. But nothing told
them what the rest holds, and on a gen2 card the CFG frame reprograms the
advertised count DOWN to the source's.

MEASURED. A 64-block card carrying a distinct per-block pattern, cloned from a
28-block source, afterwards reports 28 blocks: `hf 15 dump` shows a clean copy
while blocks 28, 40 and 63 all still read back the old pattern and 64 fails. The
residue is invisible to the ordinary tool, which is what makes it worth saying.

THREE FINDINGS, AND THEY ARE SEPARATE. Conflating them either warns about every
clone onto a wiped card or says nothing about the case above:

  - blocks above the source still holding NON-ZERO data -- about the user's
    data. Zeros up there are space, not residue, and raise nothing.
  - a card ANSWERING READS above the count it now reports -- about its size,
    true whether or not anything is left up there, and the same phantom tail the
    wipe's sweep exists for.
  - a card still reporting a BLOCK COUNT OR IC REF the source did not -- about
    how the copy presents. Claims on both sides, because a reader shows claims.

THE ADVERTISED COUNT CANNOT BE THE TEST, before or after: on a magic card it is
whatever was last programmed. So the survey READS upward from the source count
and stops on an absent run or the budget. On the cards measured a block past
physical capacity refuses reads outright, which is the same discriminator the
data pass already uses when it asks whether a refused block is even there.
Non-destructive, unlike the wipe's sweep, which writes because it is wiping. A
run of absences is the card's top only while the card is still there, so when
the survey stops on one it asks: a card that has left keeps the data it showed,
but makes no claim about where it ends. The popup's last progress frame now
waits for the survey as well, so it does not read 100% while the card is still
being asked something.

It runs whatever path the clone took: that gen1 cannot misstate its count is
known only for the cards here, and the check costs one absent run of reads past
the card's top.

None of this is a failure and none of it makes the clone Partial, which is for a
run where something went wrong. They are notes on a success, routed to the
counted result screen the way an over-capacity clone already is, with the same
success tone and Finish. Which outcomes are successes is written once, for the
tone and the button both, so a success added later cannot get one without the
other.

A GEN1 CLONE THAT LOST NOTHING GETS A NOTE THERE TOO, where its file reached
56/57/62/63 with nothing stored in them. Every block that held data was written,
so it is complete, but the copy differs from the file in one way a reader can
see: those four are the card's registers, so asking for them returns no block
where the file had one. It is the same shape as an empty tail past the card's
end, and it is reported the same way.

== WHAT THE SCREENS SAY ==

THE BLOCK COUNT AND THE IC REFERENCE ARE TWO FLAGS, not one. They move
independently and can both move at once -- a 64-block file declaring IC ref 0F
onto a 28-block card reporting 01 -- so one standing in for the pair either
under-reports or leads with a number that matched. Each screen names whichever
moved, both when both did. The note says "configuration register" rather than
"geometry": the gen2 backdoor programs the block count, the block size AND the
IC reference through one CFG register, which is why gen1 can reproduce none of
the three. The block-count flag compares the count and not the block size: the
notes print counts, so a size that alone disagreed would read as two equal
numbers, and on the cards measured no clone that succeeds carries one -- a gen2
magic card handed 8-byte blocks took that geometry through its CFG frame and
then refused every write, and a gen1 SLIX refused them outright.

ONE TITLE FOR THE NOTES PAGE, "Clone notes" / "Wipe notes". The page now carries
notes about residue, size and configuration beside the block list, so the title
no longer names the list, and the list opens with its own label. That label is
decided from the capacity facts alone. The old title also asked the
over-capacity reason code, which adds nothing: that reason comes only with a
Success carrying an over-capacity count, which is exactly what the capacity
facts test, so the term goes, with the two locals it needed.

PARAGRAPHS ARE SEPARATED AND MARKED -- a blank line between each and a leading
"- ", because they stack and three or four run together as one wall otherwise.
The scroll element has no bullet primitive, so these are literal characters and
a wrapped line does not hang-indent. One helper opens every entry, so the format
is one rule.

AND THEY SAY WHAT TO DO. The residue note names non-zero data and tells the user
to Wipe and write again. The size note names what the card REPORTS before what
it holds, since the reported count is the one a reader of the card meets first,
and gives both as counts -- naming a count against a top block index differs by
one for a reason the sentence never gives. Where the card's count and the file's
are the same number, Details says so, since otherwise the reported count reads
as arbitrary when it is usually exactly what the file asked for.

Gated on the two values being EQUAL, not on which path ran, and it stops short
of saying who set the count. The gen2 CFG frame usually did, but it goes out
before the verify and lands only on a magic card, so a tag that is not magic and
already wore the file's UID, and a gen1 clone, both arrive here with a count
nothing in this app wrote.

THE CONFIGURATION NOTE SHOWS BOTH SIDES -- what the file asked for and what the
card reports -- and ends with the two ways to get a closer copy. It says "this
card", not "this gen1 card": it fires whenever the two disagree, and gen1 is the
usual cause but not the only one, since a card that is not magic at all, whose
UID already matched the file's, reaches the same place with no configuration
write ever taking.

FOUR COMMENTS THAT COUNTED WHAT THIS ADDS TO now count it: the block-size clamp
gains the survey as a fourth caller, the write-fail screen a thirteenth body and
a third at (4, 20), and the write scene a third success the bare popup cannot
hold.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
