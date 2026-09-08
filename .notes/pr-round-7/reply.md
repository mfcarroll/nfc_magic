# Round 7 reply — DRAFT, NOT POSTED

Body between the ~~~~ markers.

**Per-thread replies are NOT written yet** — 20 threads, and they are the remaining piece before
anything can go up. Each needs to quote the text rather than a line number, because the cut moves
every line reference in `poller.h` and `write_fail.c`. Thread ids and bodies are in
[received/threads.json](received/threads.json).

~~~~
Thank you for the two corrections you volunteered against yourself. The `<=` revert is in, and so is the
finding you withdrew before posting — that one is the most useful thing in this round and I want to start
there rather than bury it.

## What is in this push, and why it is two things

**The comment cut is in here.** It was built before this review existed — sixteen commits, promised at the
end of round 6 — so you reviewed the code without it. Round 7's answers sit on top of it as their own
commits. Reviewing them as two groups will read better than reviewing them interleaved.

## The cut deduplicated. It did not re-verify. Round 7 is the proof

This is the finding, and it is not a comfortable one.

**Four of your twenty threads are comments the cut had already touched** — `poller.c:41`, `poller.h:160`,
`poller.h:164`, `write_fail.c:273`. The cut shortened or re-placed each one and left the claim inside it
wrong. In two cases it moved the false sentence *closer* to the text that contradicts it.

The pattern is exact: the cut treated each comment as a unit to shorten, not a claim to check.
Compression preserved truth-value.

So I am not going to tell you the cut is its own best argument. It did what it set out to do on
duplication and did nothing at all for accuracy. What survives is the narrower claim: one owner per fact
is still right, and it *raises* the cost of the surviving copy being wrong rather than lowering it —
which is exactly what these four demonstrate.

## The ratio, since it was the round-6 argument

Measured across the eight files: **−102 comment lines took the surface from 37% to 36%.** Removing
comment lowers numerator and denominator together, so reaching `gen2_poller.c`'s 9% by deduplication is
arithmetically impossible — `iso15693_poller.c` would have to drop from 698 comment lines to about 236,
and what is left after the cut is the hardware measurements, the `view_dispatcher` queue argument and the
constraint notes.

The metric that does track the defect is how many places state the same fact. **Repeated 6-word comment
phrases across the ISO15693 surface: 135 before, 34 after.** Sites per fact: `56/57/62/63` 10 → 1,
"first activation" 6 → 1.

And the honest part: ISO15693 does carry roughly 9x the comment per line of code that `gen2_poller.c`
does, and that file is 875 lines, so the gap is not a size artefact. I think the argument is about what
the residue *is*, not that the gap is imaginary.

## Your two corrections

**The `<=` revert is in, and the boundary is now pinned.** You are right about count-versus-index, and
your original wording was the accurate one. Your justification at `:96-98` is dropped as you asked — it
could not pick between the branches. What I added: a test asserting `cut_block == blocks_advertised`
renders "past the N this card claims" and **not** "of the N", plus that one block lower still reads as
inside. Both of us have had that boundary backwards once; a third round-trip is now a red test.

**The comment that cost you a false finding.** Fixed at `write_fail.c` and at the same overreach in
`app_i.h`. It is a claim about the screen it is printed on, and the code four lines down disproves it.
You are right that we had written the accurate version 90 lines up.

## What the cut had already closed

Two of your threads, and one of them differs from what you proposed:

- **The three duplicated UID loops** are folded — but into **four** call sites, not three.
  `iso15693_info_cat_uid` takes a two-policy enum, `Spaced` and `Grouped`, and the Info screen uses
  `Spaced` rather than staying out of it. Your reason for excluding it was the distinction the comment
  was drawing; my reading is that naming both widths on the enum, 23 characters against 17, makes that
  distinction enforceable instead of prose on one copy. Say if you would rather have the three-site
  version.
- **`:257`'s propagation of the "two cards that separate them" phrasing** is gone — the cut rewrote
  `start_wipe` and removed that sentence.

## This round

All twenty addressed. The `COUNT_OF` inversion you would fix first is fixed: widening lengthens the loop
into an out-of-bounds read, it cannot shorten it. `CHANGELOG.md` now says a gen3 card lands on the "Not
gen2 magic card" opt-in screen, and carries the omission you asked for — accepting that opt-in sends four
ordinary WRITE BLOCKs into 56/57/62/63, so the **clone** path can damage a gen3 card too.

The shared write tail is extracted as `iso15693_poller_finish_write(instance, iso_poller, skip_backdoor)`.
Worth reporting how that went: mutation-testing it showed **nothing tested the flag** — flipping
`skip_backdoor` at either call site left all 107 cases green. An extraction whose whole justification is
"these must stay in step", with nothing holding them in step, is worth very little. So there is now a
case asserting a gen2 clone counts the backdoor blocks and a gen1 clone deducts them, verified both ways.

## Verification

- Host tests **108**, all green — up from 106. Two new: the cut-at-the-claim boundary, and `skip_backdoor`
  at both verify arms.
- Both firmware trees clean from a deleted object dir, zero warnings, `clang-format` clean.
- **Churn: 6 lines**, and both instances are the cut writing a line that Round 7 then corrected
  (`3199bb9`→`7883953`, `58de6ba`→`b312deb`). I could have amended the cut's commits to hide that, and
  deliberately did not — the order things happened in is the finding.
- Comment/code for Round 7 alone: comment **+37**, code **−9**. Correcting a claim costs lines, same as
  round 6. The cut's own figures were comment −95, code +11.

## Still open from before

The `:752` budget thread and `PASS_MAX_MS`. And #255 has a new comment from @0x6r1an0y worth reading
before any gen3 work — zeroing `0x14`/`0x15` on an un-finalized V3 card does more than clear the
signature.
~~~~
