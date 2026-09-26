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

The original reading changed one variable and took the 0x01 as a refusal of the unaddressed form.
Its control was `hf 15 wrbl`, which silently forces OPTION on for any tag whose manufacturer byte is
TI's — so the frame that "worked addressed" differed in two bits, not one.

**Nothing here requires an addressed write.** Five cards covering four identified chips are each
measured taking an unaddressed one.

## What went in

**The OPTION flag, decided by what the card says.** The first write of a run goes out without it; a
0x03 refusal turns it on for the rest of the operation. Not decided from the UID, although proxmark
does it that way — a UID is not a statement about silicon in an app whose purpose is changing it. A
clone writes the source's UID onto the card, so a TI card cloned from an NXP image stops looking
like TI immediately before the data pass that needs the flag.

**A read-back, because the flag costs the acknowledgement.** ISO15693-3 10.3.1: with OPTION set the
card answers a write only after the reader sends a standalone EOF, and the SDK has no call for one —
its encoder appends exactly one EOF as the tail of the frame. So the write lands and nothing comes
back. Before this, a wipe zeroed all 64 blocks of a TI card and then reported that nothing had been
cleared. Silence from a card that asked for the flag is now settled by reading the block back and
comparing it. An in-band error frame is still taken at face value: the card that stays silent is
waiting for something we cannot send, the card that answers has decided.

[👤] I'd argue that SDK limit is a real gap in the firmware for iso15693, but it's one we can work
around, and much better to work around it than expand the scope of this to requiring a firmware
upgrade, even if that means we aren't able to get the write acknowledgments back directly.

**Addressed data-block writes**, which is what #251 asks for. Every card here accepts them and four
answer nothing at all to a UID one byte wrong. The wipe retakes its address after writing 56 or 57,
because on a gen1 card those two blocks are the UID and it moves immediately — measured on two chips.
Without that, every later frame carries an address the card no longer answers to, the sweep's absent
run trips, and it reports a card shorter than the one in the field.

**The clone's identity writes are addressed too.** WRITE AFI and WRITE DSFID are standard commands,
so an unaddressed one lands on a tag of any size. The AFI is the worse of the two: a reader can
inventory selectively on it, so an installation with a mixed tag population uses it to make its door
readers see door cards and not stock labels. Change a bystander's AFI and it stops answering the
inventory its own system runs — the card still reads fine to anything generic, but to the system
that owns it the card has simply gone.

## A clone could destroy the identity of the card it was copying onto

The gen2 verify proves the card's UID **matches** the target, not that the card is magic. A card
already wearing the source's UID satisfies it without anything having happened — and re-cloning the
same file is exactly how a card comes to wear it.

So the second clone of a 64-block file onto a gen1 SLIX took the gen2 path with blocks 56 and 57 in
scope, wrote the file's data straight into the UID registers, and left the card answering to
`39 A5 39 5A 38 A5 38 5A` — the file's own bytes fed through the UID mapping — under a screen that
said "Cloned 30/64, Not written: 34" and nothing else.

What identifies the card is the same write that does the damage. A write to 56 or 57 that moves the
UID to precisely the value that write implies can only mean those addresses are registers, so the
card is gen1 whatever path the run took to get there. The run converts from that point: it stops
feeding the file into the registers, carries on with everything above them, and afterwards writes the
target UID back through the gen1 sequence. The end state is the one an ordinary gen1 clone produces
— a shape already tested and already reported correctly — rather than a new one.

**The re-address checks its answer now**, which the same work made possible. It used to take whatever
the inventory returned, and that inventory is the SDK's 1-slot unaddressed one, so a second tag in
the field can answer it (#251) and re-addressing to a stranger points every later frame at the wrong
card. Because we know what we wrote — 56 carries uid[7..4], 57 carries uid[3..0] — there are exactly
two honest replies: unchanged, or what the write implies. A third is refused and the address held.
Not airtight, and unfixably so: a bystander holding the predicted UID would pass, because magic cards
make UIDs non-unique and a 1-slot inventory cannot tell two cards apart.

**Not by skipping those blocks whenever the UID already matches**, which is the shorter fix and is
wrong: on a real gen2 card re-cloned from a *different* image that happens to share its UID,
56/57/62/63 are ordinary memory. The skip deducts them from the total, and the run reports a clean
"Cloned 60/60" over four blocks of the file it chose not to write — trading a silent data loss for a
rare identity one.

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
budget — past physical capacity a block refuses reads outright, which is the same discriminator the
data pass already uses when it asks whether a refused block is even there. Non-destructive, unlike
the wipe's sweep, which writes because it is wiping.

None of this is a failure and none of it makes the clone Partial. They are notes on a success.

**One card here defeats that, and it is not one this PR supports.** The survey rests on a block that
answers a read existing, which holds for every gen1 and gen2 card I have. One tag answers at all 256
addresses because the top half of its address space aliases the bottom — write block 228 and block
100 changes with it — so the survey would call a 128-cell card a 256-block one, and would meet the
clone's own payload again through the alias and report it as data left over from before. That tag is
neither gen1 nor gen2: its UID sits at 0x10/0x11 in the reversed layout and block 0x14 is one bit
from the V3 config-mode signature, so it is gen3 territory, which this PR does not support. I have
left the wording alone rather than hedge it for a card the feature does not claim to handle. If it
does turn out to be gen3, it is the first such card either of us has had, and #255 stops resting on
an attributed report.

**The geometry half is also a correction to the release notes.** They said a clone writes the
source's identity — IC ref, block geometry, AFI, DSFID — "so the copy advertises the same chip",
without qualification. That is the gen2 path: the gen2 backdoor has a CFG register that programs what
the card reports and gen1 has none. A 40-block SLIX-S cloned from a 28-block SLIX source with gen1
answers to the source's UID and carries its data while still reporting 40 blocks and IC ref 0x02 —
and the type line a reader prints for it is decoded from that UID rather than from the chip. The
notes now say so.

Both sides of that comparison are claims, deliberately: the question is what a reader will see, not
what the silicon is.

## The gen1 caveat, and a register read as capacity

A gen1 clone told the user that blocks 56/57/62/63 "differ from the source" whatever the source was —
but a source below block 57 has no such blocks, and on every gen1 chip measured those four addresses
answer no read at all, so they are registers outside the memory map rather than blocks with something
to displace. The claim is now made only where the source actually reached them, from the same
expression that produces the block count, so the count and the wording cannot drift apart. And a
gen1 clone that lost nothing is no longer Partial: the counts said "28/28, not written 0" under a
Partial banner with a note saying nothing had been skipped. That reaches a plain Success.

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
aimed at a UID already in doubt. The backdoor sequences also stay unaddressed — they are the magic
sequences, measured to work that way on all five cards, and nothing has been measured about
addressing them.

## The bench

Seven cards, covering four identified chips plus one whose silicon cannot be named: `gen-2-card` has
only ever worn a cloned UID, so the type line a reader prints for it describes what was copied onto
it rather than the chip underneath. The runs that decide it:

- a **TI Tag-it** wipe and clone — the card that could not be written at all before this
- an **armed gen1 ST LRi2K** wipe, where the UID moves under the sweep: 58/58 and the identity change
  reported, unchanged from before, which is the point — the re-address is what stops the addressing
  from breaking that path
- a **70-block source onto 64-block silicon**, on the second TI card, where the six blocks past the
  top burn their retries and their read-backs fail, so they are reported as refused while every real
  block is verified against a source in which no two blocks are alike
- the **same source cloned twice onto a gen1 NXP SLIX**, which is the identity case above: the second
  run now converts, the UID reads back intact, and it reports identically to the first
- a **64-block card carrying a distinct per-block pattern**, cloned from a 28-block source, which is
  the residue case, and a **40-block SLIX-S** for the geometry one
- unaddressed-write controls, to show nothing regressed: a full 64/64 wipe on the gen2 card and 28/28
  on a gen1 NXP SLIX

## Where this stands

I think it is ready. The capability gap that was blocking it is closed, and what remains on #251 is
the inventory, which is a different change.
~~~~
