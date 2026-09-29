# Round 15 — main reply (DRAFT, NOT POSTED)

Post between the `~~~~` markers. **No internal process in the payload.** `[👤]` marks a paragraph
written by mfcarroll; it is POSTED as-is, so mishamyte can see which words are his.

Not answering a review — he has not replied to round 14. This is the addressing work, and a
correction to what round 14 told him.

~~~~
## A correction

I told you TI Tag-it HF-I Plus refuses an unaddressed WRITE BLOCK, and that addressing the writes
was therefore a functional requirement rather than only the safety fix #251 asks for. That was
wrong. What that chip refuses is a write with the **OPTION flag** clear — addressed or not.

All four combinations, on the same card, each read back from it:

| flags | addressed | OPTION | result |
|---|---|---|---|
| `0x02` | no | no | refused, error 0x01 |
| `0x22` | yes | no | refused, error 0x03, "option not supported" |
| `0x42` | no | **yes** | accepted |
| `0x62` | yes | **yes** | accepted |

Its control was `hf 15 wrbl`, which silently forces OPTION on for any tag whose manufacturer byte is
TI's — so the frame that "worked addressed" differed in two bits, not one. A second TI card behaves
the same way: an **addressed** write with the flag clear is refused `0x03` there too.

**Nothing measured here requires an addressed write.** All seven cards take an unaddressed one — the
two TI cards once the OPTION flag is set, which is orthogonal to the address.

**A second correction, smaller but user-facing.** The release notes scoped the wipe's identity hazard to
a gen1 card "left armed by an earlier UID write". Five gen1 cards here take a write to block 56 with
neither "unlock" nor "commit" sent first — one of them a card this app had never written — and no card
has accepted either frame. A user cannot know a card's history and nothing can detect it, so the hazard
is any gen1 card whose claim lets the sweep reach 56/57: the reach rule, not the card's history. The
release notes say so now, and I will put the same correction on #255, which still carries the older
wording.

[👤] I would add, it is still totally possible that gen1 cards do in fact require those frames. I assume
those went into proxmark for a reason, even if the lack of any comments or documentation in there makes
it feel like a black box. I've just never _seen_ a card in a state where those frames were required
before a block 56/57 write would take. That said, I only have a handful of gen1 cards, and no duplicates,
so my focus was on categorizing and ensuring they worked with this app initially - it's possible some
arrived in that state and I "armed" them early on without realising. Worth checking carefully on factory
fresh gen1 if either of us get any more. That requires sending carefully crafted frames and intentionally
not using the standard pm write commands.

## What went in

**The OPTION flag, decided by what the card says.** The first write of a run goes out without it; a
0x03 refusal turns it on for the rest of the operation. Not decided from the UID, although proxmark
does it that way — a UID is not a statement about silicon in an app whose purpose is changing it. A
clone writes the source's UID onto the card, so a TI card cloned from an NXP image stops looking
like TI immediately before the data pass that needs the flag.

**A read-back, because the flag costs the acknowledgement.** ISO15693-3 10.3.1: with OPTION set the
card answers a write only after the reader sends a standalone EOF, and the SDK has no call for one —
its encoder appends exactly one EOF as the tail of the frame. So the write lands and nothing comes
back. With the flag and no read-back, a wipe zeroes all 64 blocks of a TI card and then reports that
nothing was cleared. Silence from a card that asked for the flag is now settled by reading the block
back and comparing it. An in-band error frame is still taken at face value: the card that stays
silent is waiting for something we cannot send, the card that answers has decided.

[👤] I'd argue that SDK limit is a real gap in the firmware for iso15693, but it's one we can work
around, and much better to work around it than expand the scope of this to requiring a firmware
upgrade, even if that means we aren't able to get the write acknowledgments back directly.

That call is about six lines: the encoder already writes SOF and EOF as literal bytes, and the
transmit path sends what it is handed. On a firmware branch carrying it, the TI card answers its
writes directly — one frame instead of two, and the answer is the card's own rather than inferred.
That last part matters more than the frame: a read-back cannot tell a write that took from a refused
write to a block that already held the value.

Not something this app can depend on, though. A FAP resolves its API imports at load time, so there
is no fallback keyed on API version — naming a symbol the firmware lacks fails the whole load. So the
read-back ships, and if that call ever lands in the minimum firmware supported here it replaces it
outright.

**Addressed data-block writes**, which is what #251 asks for. All seven cards here accept them, and all
seven filter the address: a write carrying a UID one byte wrong is unanswered, and the block it aimed at
— holding something else at the time — is unchanged when read back afterwards. Silence alone would not
have shown that; a write can land without acknowledging, which is exactly what the OPTION flag does
above. The wipe retakes its address after writing 56 or 57, because on a gen1 card those two blocks are
the UID and it moves immediately — measured on three chips. Without that, every later frame carries an
address the card no longer answers to, the sweep's absent run trips, and it reports a card shorter than
the one in the field.

**The clone's identity writes are addressed too.** WRITE AFI and WRITE DSFID are standard commands,
so an unaddressed one lands on a tag of any size. The AFI is the worse of the two: a reader can
inventory selectively on it, so an installation with a mixed tag population uses it to make its door
readers see door cards and not stock labels. Change a bystander's AFI and it stops answering the
inventory its own system runs — the card still reads fine to anything generic, but to the system
that owns it the card has simply gone.

**And so is the gen1 backdoor sequence**, which was the worst of the lot to leave open. Those four
frames are plain WRITE BLOCKs at blocks 56/57/62/63, and on any tag large enough to have them that is
user data — and they went out behind an opt-in whose warning covers only the card in the user's hand.

Measured on five cards across all three gen1 chips — ST LRi2K, NXP ICODE SLIX, NXP ICODE SLIX-S —
every frame bracketed by a reader either side, so a silence is not an absence. Block 56 takes an
addressed write and the UID moves to exactly the value that write implies; a UID one byte wrong writes
nothing — the UID is unchanged when read back, one card of each gen1 chip. So the register the sequence
exists to protect is itself filtered on the address. And at block 62, which every card refuses,
**addressing is what makes them answer at all**: the NXP parts are silent unaddressed and return a
readable `0x0F` addressed, while the LRi2K answers either form with the specific `0x10`, "block not
available". So the sequence had been going out in the one form four of these cards ignore.

The cost is the same re-address the wipe needs, for the same reason, and here it sits between the two
halves of one UID: without it block 57 goes to a card that has stopped listening, and the run ends
having written half an identity — neither the one the card had nor the one asked for. Measured end to
end: a write addressed to the UID the *previous* write produced is accepted, which is the whole of
the seam.

Unlock and commit are addressed on the safety argument alone, and I cannot validate the addressed
form of either: no card here has ever accepted one, in any form. They stay: proxmark sends them, and
the cards that would prove them necessary are ones neither of us has.

## A clone could destroy the identity of the card it was copying onto

The gen2 verify proves the card's UID **matches** the target, not that the card is magic. A card
already wearing the source's UID satisfies it without anything having happened — and re-cloning the
same file is exactly how a card comes to wear it.

So the second clone of a 64-block file onto a gen1 SLIX took the gen2 path with blocks 56 and 57 in
scope and wrote the file's data straight into the UID registers. The card came away answering to a
UID assembled out of those two blocks of the file — not the source's UID, not its own, and not
anything a user could look up — under a screen that said "Cloned 30/64, Not written: 34" and nothing
else.

What identifies the card is the same write that does the damage. A write to 56 or 57 that moves the
UID to precisely the value that write implies can only mean those addresses are registers, so the
card is gen1 whatever path the run took to get there. The run converts from that point: it stops
feeding the file into the registers, carries on with everything above them, and afterwards writes the
target UID back through the gen1 sequence. The end state is the one an ordinary gen1 clone produces
— a shape already tested and already reported correctly — rather than a new one.

**The re-address checks its answer.** After a write to 56 or 57 the run has to discover what the
card answers to, and the only tool for that is an inventory. The SDK's is 1-slot and unaddressed, so
with a second tag in the field it can come back with the bystander's UID instead (#251) — and taking
that at face value would aim every later frame at the wrong card, at the exact moment this one's
identity is in doubt.

What makes the answer checkable is that we know what we just wrote. Block 56 carries uid[7..4] and
block 57 carries uid[3..0], so there are only two replies this card can honestly give: the UID
unchanged, or the UID that write implies. Anything else did not come from us, so the run logs it and
keeps the address it already had.

Not airtight, and it cannot be: a bystander that happens to hold the predicted UID would pass.
Magic cards make UIDs non-unique by construction and a 1-slot inventory cannot tell two cards apart.

**The obvious shortcut would have been to skip those four blocks whenever the card already wears the
UID being written — and it is the wrong fix**, which is why the app reacts to what the card does
instead. On a real gen2 card re-cloned from a *different* image that happens to share its UID,
56/57/62/63 are ordinary memory holding the file's data. Skipping them there would also deduct them
from the total, so the run would report a clean "Cloned 60/60" while four blocks of the file went
unwritten — a silent data loss, in exchange for avoiding a much rarer identity one.

## What a clone leaves behind, and what it reports

A clone writes the source's blocks and leaves the rest alone, which is right — destroying what the
user did not ask about is the wipe's job. But nothing told them what the rest holds, and on a gen2
card the CFG frame reprograms the advertised count **down** to the source's. A 64-block card filled
with a distinct per-block pattern, cloned from a 28-block source, afterwards reports 28 blocks: `hf
15 dump` shows a clean copy, while blocks 28, 40 and 63 all still read back the old pattern and 64
fails. The residue is invisible to the ordinary tool, which is what makes it worth reporting.

Three findings, and they are separate — conflating them either warns about every clone onto a wiped
card or says nothing about the case above:

- blocks above the source still holding **non-zero data** — about the user's data. Zeros up there are
  space, not residue, and raise nothing.
- a card **answering reads above the count it now reports** — about its size, true whether or not
  anything is left up there, and the same phantom tail the wipe's sweep exists for
- a card still reporting a **geometry or IC ref the source did not** — about how the copy presents

**The advertised count cannot be the test.** On a magic card a count is a claim, and deriving "these
blocks hold residue" from a number the card chose would be a guess wearing the clothes of a
measurement. So the survey reads upward from the source count and stops on an absent run or the
budget — on the cards measured a block past physical capacity refuses reads outright, which is the
same discriminator the data pass already uses when it asks whether a refused block is even there.
Non-destructive, unlike the wipe's sweep, which writes because it is wiping.

None of this is a failure and none of it makes the clone Partial. They are notes on a success.

**One card here defeats that, and it is not one this PR supports.** The survey rests on a block that
answers a read existing, which holds for every gen1 and gen2 card I have. One tag answers at all 256
addresses because its address space aliases — write block 228 and block 100 changes with it — so the
survey would call a 128-cell card a 256-block one, and would meet the clone's own payload again
through the alias and report it as data left over from before. I have left the wording alone rather
than hedge it for a card the feature does not claim to handle.

[👤] That tag appears similar to gen3, but doesn't match the configuration patterns expected by the
gen3 code in proxmark. The seller told me there is no support for it in proxmark and it requires
custom writer software, so it may be a proprietary variant. Not worth worrying about for this PR.

**The geometry half is also a correction to the release notes.** The 2.3 entry said a clone writes
the source's identity — IC ref, block geometry, AFI, DSFID — "so the copy advertises the same chip",
without qualification. That is the gen2 path: the gen2 backdoor has a CFG register that programs what
the card reports and gen1 has none. A 40-block SLIX-S cloned from a 28-block SLIX source with gen1
answers to the source's UID and carries its data while still reporting 40 blocks and IC ref 0x02 —
and the type line a reader prints for it is decoded from that UID rather than from the chip. The
notes now say so.

Three gen1 cards — ST LRi2K, NXP ICODE SLIX, NXP ICODE SLIX-S — all read as **Emosyn-EM
Microelectronics USA** while carrying one written UID, and as three correct manufacturers once each
was back on its own. Same cards, same reader, minutes apart, and nothing changed but the UID.

## The gen1 caveat, and a register read as capacity

A gen1 clone told the user that blocks 56/57/62/63 "differ from the source" whatever the source was —
but a source below block 56 has no such blocks, and on every gen1 chip measured those four addresses
answer no read at all, so they are registers outside the memory map rather than blocks with something
to displace. The claim is now made only where the source actually reached them, from the same
expression that produces the block count, so the count and the wording cannot drift apart. And a
gen1 clone that lost nothing is no longer Partial: the counts said "28/28, not written 0" under a
Partial banner with a note saying nothing had been skipped. That case is no longer Partial.

The screens also stopped reading a **register write as evidence about memory**. "Card too small" is
withheld unless no block wrote above the failures, since a success up there means they were not the
card's top — but on a converted run the failures reach the top and then block 56 answers, because it
is a UID register. The same card fed the same oversized file reported differently depending on which
path the run took to reach it. Both paths now reach the same verdict.

## What this does to the scope

The OPTION flag is what makes a TI Tag-it writable, and it is separable from everything else here.
The addressing is the safety fix #251 was filed as.

I have kept both, and the reason is the range. A bystander does not have to be touching the antenna
on ISO15693 — a wallet or a badge holder is enough — and a clone's payload or a wipe's zeros landing
on someone's other card is the kind of inadvertent damage this PR has been careful about throughout.

**It does not close #251.** The 1-slot INVENTORY_T5 and the missing STAY QUIET are untouched, and the
issue's worst consequence cannot be fixed this way at all: the post-wipe UID re-read can still be
answered by a bystander, and that check exists to discover whether the UID changed, so it cannot be
aimed at a UID already in doubt.

**And one frame set could not be addressed on any card tested.** The gen2 backdoor is `0xE0`, proprietary, so a
conforming tag rejects it on the command and there is no standard frame for a bystander to take.
That covers conforming tags. Another gen2 magic card parses `0xE0 09` exactly as the target does,
and the sequence programs the configuration register as well as the UID, so a bystander of that kind
comes away with a different block count, block size and IC reference on top of a different identity.

Measured on all four gen2 cards here: each takes the unaddressed form and refuses the addressed one —
with the correct UID, with and without the OPTION flag, and with the address bit set but no UID in
the frame. All four also accept an addressed ordinary WRITE BLOCK and go silent on a one-byte-wrong
address, so the frames are well formed and the cards' addressing works. The backdoor is not reachable
that way, and there is no addressed form of it to send.

So those frames stay as they are, and the limit is worth stating rather than arguing away: another
gen2 magic card in the field takes them and nothing in the app can stop it. Narrower than an
unaddressed ordinary write, which any writable tag takes; not zero.

## The bench

Seven cards, covering four identified chips plus two whose silicon cannot be named. Both of those
have only ever worn a written UID, so the type line a reader prints for them describes what was put
on rather than the chip underneath — one of them arrived that way, from the developer who wrote
proxmark's ISO15693 V3 magic support. The runs that decide it:

- a **TI Tag-it** wipe and clone — the card that could not be written at all before this
- a **gen1 ST LRi2K** wipe, where the UID moves under the sweep: 58/58 and the identity change
  reported, unchanged from before, which is the point — the re-address is what stops the addressing
  from breaking that path
- a **70-block source onto 64-block silicon**, on the second TI card, where the six blocks past the
  top burn their retries and their read-backs fail, so they are reported as refused while every real
  block is verified against a source in which no two blocks are alike
- the **same source cloned twice onto a gen1 NXP SLIX**, which is the identity case above: the second
  run now converts, the UID reads back intact, and it reports identically to the first
- a **64-block card carrying a distinct per-block pattern**, cloned from a 28-block source, which is
  the residue case, and a **40-block SLIX-S** for the geometry one
- **a gen1 Write UID on each of the three gen1 chips**, to a target differing from each card's own
  UID in BOTH halves, so half a UID could not pass as a whole one: plain Success on all three, and
  the frames alone run separately on the SLIX-S
- the unaddressed writes behind "nothing measured here requires an addressed write", on all seven:
  full wipes on the 64-block gen2 card (64/64) and a gen1 NXP SLIX (28/28), the backdoor's own
  unaddressed frames on the LRi2K and SLIX-S, and an unaddressed WRITE BLOCK on the gen2 sticker and —
  with the OPTION flag — on both TI cards
- **the address filter on each of the seven**, read back rather than inferred: a write aimed one byte
  wrong at a block holding something else, the block unchanged afterwards, and then the same frame
  with the right address changing it — so the silence is the address and not a malformed frame
- **the gen2 backdoor in four flag and address combinations on each of the four gen2 cards**, which
  is what settles that it cannot be addressed

## Where this stands

That closes the list I gave you in the comment-cut round. The cut, the simplification pass, the
release-notes trim, the addressed writes and the re-test on hardware are all in — which was the
condition I put on the sixth item, the squash message, since it has to describe the final state. The
2.3 notes grow again with this round, from 117 lines to 157.

Four commits close the round. 09 and 10 change comments only: 09 corrects ones from earlier rounds
that had gone false — among them the gen1 clone contract, which said such a clone never gets a clean
Success when a file ending before block 56 does, and a capacity rule stated as a law that one tested
tag breaks — plus one string: the gen1 opt-in said gen1 "writes the UID to blocks 56/57/62/63", when
the UID goes to 56/57. 10 takes the history out of comments that told it, without changing what any
of them requires. 11 and 12 are small hardening fixes. The failure bitmap checks the index it is
given, and the two switches on the write-fail reason list every reason with no default — the shape
you gave the poller's write-state switch — so a reason added without an answer is a build error, and
a screen entered without a reason crashes instead of saying "Not a magic tag".

[👤] I have a squash message drafted. I'll wait until you're ready to merge in case there are further
changes still, then post it as its own comment.

**One thing tested since, out of scope on purpose.** A genuine gen3 card kindly sent by 0x6r1an0y
— un-finalized, its configuration signature an exact match to proxmark's V3 config mode. I put the
app's paths to it. The gen2 backdoor left its UID untouched, so a clone or Write UID lands on the
gen1 opt-in exactly as the release note says; accepting that wrote 56/57/62/63 as ordinary data and
moved no identity, since a gen3 card keeps its UID in a separate register. The configuration blocks
were never touched, and it restored byte-identical. So the note's account of what those paths cost
on a gen3 card is measured now rather than reasoned. I left the brick itself untested — it is
irreversible, and a warning does not need it confirmed. Recorded on #255, and gen3 support stays out
of scope.

Three things are deliberately not in this PR, so they are not waiting on me:

- **#251 is not closed**, for the reasons above — the inventory is a different change.
- **#255 stays open.** The gen3 pre-flight probe it asks for is not in this PR and is not part of
  this work. I will put the gen1 wipe-hazard correction above onto that issue, and what the gen3 card
  showed under these paths, since the issue tracks both and its wording has the same problem the
  release notes did.
- **The host-test harness** stays out, as its own PR, for the size reason I gave before.

I think it is ready.

[👤] Or at least close. Claude may be slightly more confident than me. :)
~~~~
