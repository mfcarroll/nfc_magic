# Round 7 reply — DRAFT, NOT POSTED

Body between the ~~~~ markers.

Per-thread replies: [thread-replies.md](thread-replies.md), all 20, drafted and not posted. Each quotes
the text rather than a line number, because the cut moves every reference in `poller.h` and
`write_fail.c`. Thread bodies are in [received/threads.json](received/threads.json).

~~~~
Thank you for the two corrections you volunteered against yourself. The `<=` revert is in, and so is the
finding you withdrew before posting — that one is the most useful thing in this round and I want to start
there rather than bury it.

## What is in this push, and why it is two things

**The comment cut is in here.** It was built before this review existed (I worked on it while you were
away) - sixteen commits, promised at the end of round 6 — so you reviewed the code without it. Round 7's
answers sit on top of it as their own commits. Reviewing them as two groups will read better than reviewing
them interleaved.

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
arithmetically impossible — `iso15693_poller.c` would have to drop from 698 comment lines to about 236.

The metric that does track the defect is how many places state the same fact. **Repeated 6-word comment
phrases across the ISO15693 surface: 135 before, 34 after.** Sites per fact: `56/57/62/63` 10 → 1,
"first activation" 6 → 1.

And the honest part: ISO15693 does carry roughly 9x the comment per line of code that `gen2_poller.c`
does (90 lines per 100 against 11), and that file is 875 lines, so the gap is not a size artefact.

**Deduplication was the wrong answer to it, and the volume objection stands.** I went back over what the
cut left behind and it is not what I would have told you it was. There are **46 comment blocks of 8 lines
or more** across the ISO15693 surface; the app before this PR had **exactly one**, at 10 lines, in
`gen2_poller_i.c`. `ISO15693_POLLER_PASS_MAX_MS` is **46 comment lines for one `#define`**, and about half
of those are a wipe-versus-clone cost derivation — which is not a hardware measurement, not the queue
argument and not a constraint note. That is your `:752` thread, still open, and it was the right thread to
hold open.

The residue is a mix rather than uniformly one thing, so here is the rule I would cut by. **Would the
sentence be equally at home in the commit message? Then it belongs only there.** And for whatever
survives that: **can I name the wrong edit it prevents?** If not, delete it. On that rule the
`view_dispatcher` queue argument stays, so does the 2026-08-04 wipe measurement, so does "do not try to
de-arm by pre-writing the commit block", so does the activation-cache prefix property. The derivations,
the rejected alternatives and the measurement narratives go.

I have inventoried all 48 blocks of 8 lines or more against that rule rather than estimating: **742 lines
in them, down to 409, with 12 of the 48 left alone.** Across the whole surface that projects to roughly
**1770 -> 1250, so about 40pct down to 30pct** — worth doing, and less than the "match `gen2_poller.c`"
target, which as above cannot be reached this way. **No code changes**, verifiable by stripping comments
from both trees and diffing. If you want it nearer your figure, the lever is the 1000-odd one- and
two-line notes rather than these blocks, and I would want your call on that rather than mine.

**I would rather do that as its own delta than fold it in here.** This round is the argument for why:
every error in it came from shortening a verified sentence, not from moving one. So the pass has to be
deletion of whole claims, not rewording of survivors — deletion cannot make a fact wrong — and that is a
different operation from what is in front of you now. If you would rather have it before merge, say so
and I will scope it to whatever you want cut, including leaving the header contracts alone, which are the
part I would defend.

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
before any future gen3 work — zeroing `0x14`/`0x15` on an un-finalized V3 card can brick it, not just
clear the signature.

**The CHANGELOG's gen3 entry now says that, attributed to him.** It had described the outcome as a moved
UID and a card that no longer identifies as re-writable — recoverable-sounding, for something permanent.
#255 still carries the softer wording; I have not edited it.

**So does the wipe confirm, which matters more.** You made the point last round that the CHANGELOG
protects the reader of release notes, not the person holding the card — and that reader was seeing only
*"Wipe card? / Zeroes every data block, including the gen1 magic blocks 56/57/62/63."* It now reads:

> **Wipe? (gen1/gen2 only)**
> Zeroes every block, including
> gen1 magic 56/57/62/63.
> This can **brick** a gen3 card!

`brick` is bold via `\e#`, which switches font rather than weight and so moves the wrap; confirmed on
device, title included — `widget_add_string_element` runs off the canvas rather than truncating. The
title is a scope statement, not a detection claim: *"Wipe gen1 / gen2 card?"* would read as the app
asserting a generation it cannot detect, and would defuse the last line for exactly the person at risk.
`text_height` also drops 54 → 38, which is a fix and not tidying — at 54 the box ran to y=67, past the
screen, so an over-long string would have drawn *under* the button instead of clipping.

**What I did not do is the probe, and that is the scope question I would rather ask than assume.** The
wipe does no magic detection at all — menu, confirm, sweep — so that line can only say what a gen3 card
would cost, never that this one is gen3. A static warning is honest about that. A probe would not be a
wording change.

The trade was defensible when the gen3 cost was a moved UID and a lost signature, with the post-wipe
re-read at least reporting the identity half. It is a worse trade against permanent destruction — the
re-read tells you nothing useful about a bricked card, so the mitigation we ship is worth less than it
looked and the pre-flight probe is worth more. Two block reads, against a signature that exists only
while the hazard does, behind the same consent shape as the gen1 opt-in.

I am not adding it uninvited; it is a feature and this round was not that. But if you would rather have
the gen3 half of #255 in this PR than as the follow-up you were offered, this is the round where the
argument for it got stronger, and I would take that direction.
~~~~
