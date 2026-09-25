# Bench rules — how to measure so the result means something

Companion to [WRITING-RULES.md](WRITING-RULES.md). That one is about what we say; this is about what
we are entitled to say it from. Every rule here was paid for.

## 1. Every measurement needs its control, or you do not know what you measured

A card accepting a correctly-addressed write does not show it MATCHED the address -- a tag ignoring
the addressed flag entirely answers identically. Without the mis-addressed frame beside it, the
result is consistent with two opposite worlds.

`gen-2-card` got that control. `SL2S5302` did not, because I built the frame set for one card and
carried a shortened version to the next. **Build the control into the frame set, not into the
intention.**

## 2. A control that cannot fail is not a control

It has to be capable of coming out the other way. "Addressed write succeeds" proves little on its
own; "addressed write succeeds AND a one-byte-wrong address gets silence" pins it, because the second
could have failed and did not.

## 2b. Bracket a negative result with a positive one

"No answer" and "the card was not there" produce identical output. A refusal only counts as a refusal
if the card is shown answering immediately before and after -- `hf 15 reader`, the frame, `hf 15
reader`. mfcarroll did this unprompted on the SL2S5302 control; I had specified the frame alone.

## 3. Name the chip, never the family

"No power-up latch on gen1 silicon" from one armed LRi2K cost a review round. Then the same shape
went into the addressed-write note -- "measured on gen1 silicon" from one NXP SLIX -- four rounds
after learning it. **The title of a measurement note is where this leaks first.**

## 4. An absence is a result, and needs the same rigour

`slix-1k-50mm` wiped without its UID moving. That is not "nothing happened", it is the reach rule
predicting a boundary correctly -- claim 28 puts L at 35, short of 56. Absences are easy to leave
unrecorded because there is no screenshot to paste.

## 5. Predict before you run, so agreement means something

Writing down "the inventory should read `00 00 00 00 50 01 04 E0`" before zeroing block 56 turns the
run into a test. Reading the result first and explaining it afterwards does not, and cannot
distinguish a confirmed model from a flexible one.

## 6. Prerequisites get run before the thing that depends on them

The round-10 finding recorded two controls as unmeasured -- does EM-Marin still wipe, do gen1 cards
take unaddressed writes on ORDINARY blocks. Both were prerequisites for the addressing work and both
had sat for two weeks. Had EM-Marin failed, the design would have been wrong from the first line.

## 7. One card is one card

Five chips accept addressed writes. That took five cards, and the one that mattered most --
`gen-2-card`, the only gen2 that takes unaddressed writes -- was the fourth tested. **Test the card
that could kill the design first**, not the one that is convenient.

## 8. Say which transcript a claim came from

Every frame and response in `pr-round-15/addressed-writes-measured.md` is pasted verbatim, so a later
reader can re-derive the conclusion rather than trust the summary. A summary without its transcript
is an assertion.

## 9. Restore, and record the restore

Cards carry state between sessions. `gen-2-card` spent a day advertising 256 blocks after the CFG
test; `coin18` still carries a fixture's UID. Note what a run left behind, in the run's own write-up.
