NFC Magic ISO15693: what a clone leaves behind, and what it says about it

The survey that reads above the source, and the result screens it feeds. They
are one change: the notes have nothing to say without the survey, and the
survey has nowhere to say it without them.

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
    true whether or not anything is left up there, and the same phantom tail
    the wipe's sweep exists for.
  - a card still reporting a GEOMETRY OR IC REF the source did not -- about how
    the copy presents. Both sides of that comparison are claims, deliberately:
    the question is what a reader will see, not what the silicon is.

THE ADVERTISED COUNT CANNOT BE THE TEST, before or after. On a magic card a
count is a claim, and deriving "these blocks hold residue" from a number the
card chose would be a guess wearing the clothes of a measurement. So the survey
READS upward from the source count and stops on an absent run or the budget.
Past physical capacity a block refuses reads outright, which is the same
discriminator the data pass already uses when it asks whether a refused block is
even there. Non-destructive, unlike the wipe's sweep, which writes because it is
wiping.

It runs whatever path the clone took. gen1 has no register that could falsify a
count, but that is an inference from the cards here about a mechanism, and this
feature exists because claims lie. Eight reads on an honest card is what not
resting on it costs.

None of this is a failure and none of it makes the clone Partial, which is for a
run where something went wrong. They are notes on a success, routed to the
counted result screen the way an over-capacity clone already is.

== WHAT THE SCREENS SAY ==

A REGISTER WRITE IS NOT EVIDENCE ABOUT MEMORY. "Card too small" is withheld
unless no block wrote above the failures, since a success up there means they
are not the card's top. On a converted run the failures reach the top and then
block 56 answers -- because it is a UID register, not memory. Counting that as a
success above the failures suppressed the capacity finding and left the weaker
gen1 qualifier in its place, so the same card fed the same oversized file
reported differently depending on which path the run took to reach it. The write
that reveals the registers no longer counts, and both paths reach the same
verdict.

THE BLOCK COUNT AND THE IC REFERENCE ARE TWO FLAGS, not one. They move
independently and can both move at once -- a 64-block file declaring IC ref 0F
onto a 28-block card reporting 01 -- so one standing in for the pair either
under-reports or leads with a number that matched. Each screen names whichever
moved, both when both did. "Geometry register" is imprecise and the wording goes
with them: the gen2 backdoor programs the block count, the block size AND the IC
reference through one CFG register, which is why gen1 can reproduce none of the
three.

ONE TITLE FOR THE NOTES PAGE. It was named after the block list, which was right
when the list was all there was; it now sits beside notes about residue, size
and configuration that have nothing to do with those blocks. The page is "Clone
notes" / "Wipe notes" throughout, and the list opens with its own words.

PARAGRAPHS ARE SEPARATED AND MARKED -- a blank line between each and a leading
"- ", because they stack and three or four run together as one wall otherwise.
The scroll element has no bullet primitive, so these are literal characters and
a wrapped line does not hang-indent.

AND THEY SAY WHAT TO DO. The residue note names non-zero data and tells the user
to Wipe and write again. The size note names what the card REPORTS before what
it holds, since the reported count is the one a reader of the card meets first,
and
gives both as counts -- naming a count against a top block index differs by one
for a reason the sentence never gives. Where the card's count and the file's are
the same number, Details says so, since otherwise the reported count reads as
arbitrary when it is usually exactly what the file asked for.

Gated on the two values being EQUAL, not on which path ran, and it stops short
of saying who set the count. The gen2 CFG frame usually did, but it goes out
before the verify and lands only on a magic card, so a tag that is not magic and
already wore the file's UID, and a gen1 clone, both arrive here with a count
nothing in this app wrote. Nor does the note appearing imply the two agree: the
three survey notes are independent, so the configuration note can sit directly
below it saying they do not. Only the summary's one-line budget makes them
exclusive, and that is priority rather than agreement.

THE CONFIGURATION NOTE SHOWS BOTH SIDES -- what the file asked for and what the
card reports -- and ends with the two ways to get a closer copy. It no longer
asserts gen1 either: it fires whenever the two disagree, and gen1 is the usual
cause but not the only one, since a card that is not magic at all, whose UID
already matched the file's, reaches the same place with no configuration write
ever attempted.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
