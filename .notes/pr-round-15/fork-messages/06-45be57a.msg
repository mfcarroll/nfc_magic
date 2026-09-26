NFC Magic ISO15693: what a clone leaves behind, and a gen1 card it lands in

Two changes to what a clone does and what it says afterwards. They share the
survey that feeds the result screens, so they arrive together.

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

== A CLONE THAT LANDS IN A GEN1 CARD'S UID ==

The gen2 verify proves the card's UID MATCHES the target, not that the card is
magic, which is what the poller's comment says at that branch. A card already
wearing the target UID satisfies it without anything having happened -- and
re-cloning the same file is exactly how a card comes to wear it.

MEASURED. A 64-block source cloned twice onto a gen1 SLIX. The first run took
the gen1 path and was correct. The second found the card already wearing the
source's UID, so the gen2 verify passed, the data pass ran with the backdoor
blocks IN scope, and the file's blocks 56 and 57 landed in the UID registers.
The card was left answering to 39 A5 39 5A 38 A5 38 5A -- the file's own bytes
through the UID mapping -- under a screen reading "Cloned 30/64, Not written:
34" and nothing else.

WHAT CONVERTS IT. A write to 56/57 that moves the UID to precisely the value
that write implies can only mean those addresses are registers, so the card is
gen1 whatever path the run took. The write that does the damage is the same
event that identifies the card, so the run switches: it stops feeding the file
into the registers, keeps going with everything above them, and afterwards
writes the target UID back through the gen1 sequence. The end state is the one
an ordinary gen1 clone produces -- a shape already tested and already reported
correctly -- rather than a new one.

NOT BY SKIPPING those blocks whenever the UID already matches, which is the
shorter fix and is wrong: on a real gen2 card re-cloned from a DIFFERENT image
that happens to share its UID, 56/57/62/63 are ordinary memory. The skip deducts
them from the total, and the run reports a clean "Cloned 60/60" over four blocks
of the file it chose not to write -- trading a silent data loss for a rare
identity one.

NOTHING ON SCREEN SAYS ANY OF THAT, and that is deliberate. What the user needs
is what the CARD is -- gen1, with the file's data absent from those four
addresses -- and the existing gen1 caveat says exactly that. A note about the
identity being disturbed and put back would be the app narrating its own
process.

THE CONSENT CALL, named because it is one. The repair runs unlock and commit,
which arms the card, and the gen1 opt-in is not shown on this path. That screen
exists to warn that gen1 writes destroy four blocks of user data on a tag that
is NOT gen1; this card has just proved that it is, by moving its UID to the
value our write implied, so those addresses are registers rather than anyone's
data. Leaving the card answering to bytes lifted out of the file is the worse
outcome by a distance.

THE RE-ADDRESS CHECKS ITS ANSWER NOW. It took whatever the inventory returned,
and that inventory is the SDK's 1-slot unaddressed one, so a second tag in the
field can answer it (#251), and re-addressing to a stranger points every later
frame at the wrong card. Because we know what we wrote -- 56 carries uid[7..4],
57 carries uid[3..0] -- there are exactly two honest replies: unchanged, or what
the write implies. A third is refused and the address held. Not airtight, and
unfixably so: a bystander holding the predicted UID would pass, because magic
cards make UIDs non-unique and a 1-slot inventory cannot tell two cards apart.

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
to Wipe and write again. The size note leads with the conclusion rather than the
evidence. The configuration note shows BOTH sides -- what the file asked for and
what the card reports -- and ends with the two ways to get a closer copy. It no
longer asserts gen1 either: it fires whenever the two disagree, and gen1 is the
usual cause but not the only one, since a card that is not magic at all, whose
UID already matched the file's, reaches the same place with no configuration
write ever attempted.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
