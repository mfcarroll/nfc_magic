# Round 13 — thread replies (DRAFT, NOT POSTED, and NOT to be posted without mfcarroll)

Prepared while he was away. **Nothing here has been reviewed by him.**

Seven threads. Post each payload between its `~~~~` markers, on the comment id named.

Rules, unchanged: disposition, anything HE got wrong, anything WE found. **No internal process** —
not when something was cut, not which commit in the push removed it. `tools/` never reaches the
fork, so the host harness is not a file he can open.

---

## `CHANGELOG.md:107` — comment 4035110553

~~~~
Both taken, and you are right on both counts. The opt-in writes 56/57/62/63; a gen3 card keeps its
UID at 0x10/0x11 and its signature at 0x14/0x15; on a card that big those four are ordinary user
data. So the opt-in can **damage** a gen3 card and the wipe can brick one, because the wipe reaches
16/17 and 20/21 along with everything else. The bullet says that, and the block numbers are back.

The part worth naming is the shape of it rather than the claim: the same edit that made the claim
stronger deleted the only record of the addresses that falsify it. Unfalsifiable and wrong arrived
together, which is a failure mode worth watching for in any trim, not just this one.
~~~~

---

## `CHANGELOG.md:21` — comment 4035110565

~~~~
Fixed. The bullet now says the wipe reports a move when it sees one, and that it cannot report the
absence of one — a card that no longer inventories is not re-checked at all.

Worth recording what this was, because it is the same defect three times in this round: a claim
corrected in one place and left standing in its twin. The consent screen was fixed and the release
notes were not; the in-band error code was scoped in one site and not the other two. A file that
contradicts itself is worse than one that is uniformly wrong, because the reader has to work out
which half is current. The three are fixed together and the round says so.
~~~~

---

## `iso15693_poller.c:1253` — comment 4035110573

~~~~
Taken in full, including the shape of the fix: it is a closed form now, stated once in the poller,
with the release notes carrying only the qualitative half.

Your derivation is right and I checked it against the code rather than reading it. Writing it out
then turned up a third gate and a bad definition, both in the under-warning direction, so what
shipped is:

    L = min(max(A + 7, claim - 1), ISO15693_POLLER_WIPE_MAX_BLOCKS - 1)
    56 reached  <=>  A >= 49  OR  claim >= 57

The run must also SURVIVE the re-probe, and that gate is what fixes what A means. Your point about a
landing read is the load-bearing half of it: `wipe_note_present` zeroes `absent_run` from three
sites and only the write path increments `wiped`, so A is the first block of the FINAL unbroken run,
not the first silence anywhere. A card reading 0-4, silent at 5, reading 6-59 and silent from 60
does reach 56/57 -- and "first silence" would have said it does not, on the one path where the UID
check never runs.

Five rounds now, each a paraphrase of something the code states exactly. That is the argument for
the form rather than for a better sentence, and it is why the fixtures changed too: every one had
staged A == claim, where the two terms collapse into one and an incomplete rule passes.
~~~~

---

## `iso15693_poller.c:855` — comment 4035110582

~~~~
Fixed at both. Each now reads "error 0x10 on the LRi2K" and points at ISO15693_MAGIC_BLK_UNLOCK for
what that code is and is not evidence of.

You have the load-bearing one right: the OPEN QUESTION's conclusion — that an armed card stays armed
while its UID moves — rests on the commit write being refused rather than silently dropped, and the
in-band evidence for that is one chip. Scoping the definition and leaving the two users general was
the wrong half to fix first.
~~~~

---

## `CHANGELOG.md:99` — comment 4035110593

~~~~
Restored, in the poller rather than the release notes, which is the option you offered and the right
one -- the CHANGELOG stays lean and the fact sits where both things that depend on it are, each now
named rather than left as "load-bearing twice below".

Your grep matches mine: nothing at HEAD stated it. One clause did NOT come back with it. The
original sentence carried "so a card's advertised block count is its capacity", and that contradicts
the premise of the sweep itself, the 36-of-64 measurement recorded beside it, the clone's reason for
not capping at the claim, and `blocks_advertised` in the header -- all of which exist because the
claim and what a card holds differ. Neither dependency needs it: "Wiped 58/58" needs only that 56/57
sit above THAT card's claim, and the reach form needs only that they can lie past a claim at all.
~~~~

---

## `nfc_magic_scene_iso15693_write_fail.c:70` — comment 4035110599

~~~~
Corrected to "whose exits are BOTH success_or_partial", naming the two: the case body and the
activation-error path that tests for that state.

Confirmed: that path reports `success_or_partial` rather than falling through to the CardLost
below it. So the invariant is better supported than the note claimed: no CardLost can be
emitted once that state is entered, by either route.
~~~~

---

## `CHANGELOG.md:42` — comment 4035110604

~~~~
Settled. "Every block the card acknowledged was written", which is what the clone actually does and
what the block-contents line thirteen rows further down has been saying all along.

Agreed it is pre-existing rather than new — "clones faithfully" had the same problem — but both
lines were rewritten this round, so leaving it would have been a choice rather than an oversight.
~~~~
