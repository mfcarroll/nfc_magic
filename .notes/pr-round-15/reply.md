# Round 15 — main reply (DRAFT, NOT POSTED)

Post between the `~~~~` markers. **No internal process in the payload.**

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

**No chip in this PR requires an addressed write.** All five are measured taking an unaddressed one.

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

**Addressed data-block writes**, which is what #251 asks for. Every chip here accepts them and four
answer nothing at all to a UID one byte wrong. The wipe retakes its address after writing 56 or 57,
because on a gen1 card those two blocks are the UID and it moves immediately — measured on two chips.
Without that, every later frame carries an address the card no longer answers to, the sweep's absent
run trips, and it reports a card shorter than the one in the field.

**The clone's identity writes are addressed too.** WRITE AFI and WRITE DSFID are standard commands,
so an unaddressed one lands on a tag of any size, and a changed AFI can drop that tag out of a
selective inventory.

**Two reporting fixes that came out of the same work.** A gen1 clone told the user that blocks
56/57/62/63 "differ from the source" whatever the source was — but a source below block 57 has no
such blocks, and on every gen1 chip measured those four addresses answer no read at all, so they are
registers outside the memory map rather than blocks with something to displace. The claim is now
made only when the source actually reached them. And a gen1 clone that lost nothing is no longer
reported as Partial: the counts said "28/28, not written 0" under a Partial banner with a note
saying nothing had been skipped.

## What this does to the scope

The OPTION flag is what makes a TI Tag-it writable, and it is separable from everything else here.
The addressing is the safety fix #251 was filed as.

I have kept both, and the reason is the range. A bystander does not have to be touching the antenna
on ISO15693 — a wallet or a badge holder is enough — and a clone's payload or a wipe's zeros landing
on someone's other card is the kind of damage this PR has been careful about throughout.

**It does not close #251.** The 1-slot INVENTORY_T5 and the missing STAY QUIET are untouched, and the
issue's worst consequence cannot be fixed this way at all: the post-wipe UID re-read can still be
answered by a bystander, and that check exists to discover whether the UID changed, so it cannot be
aimed at a UID already in doubt. The backdoor sequences also stay unaddressed — they are the magic
sequences, measured to work that way on all five cards, and nothing has been measured about
addressing them.

## The bench

Five cards across four chips — the five-chip figure above is the frame measurements, which include
two cards this round did not run the app against. The runs that decide it:

- a **TI Tag-it** wipe and clone, which is the card that could not be written at all before this
- an **armed gen1 ST LRi2K** wipe, where the UID moves under the sweep — 58/58 and the identity
  change reported, unchanged from before, which is the point: the re-address is what stops the
  addressing from breaking that path
- a **70-block source onto 64-block silicon**, on the second TI card, where the six blocks past the
  top burn their retries and their read-backs fail, so they are reported as refused while every
  real block is verified against a source in which no two blocks are alike
- regressions on the **EM-Marin** gen2 card and an **NXP SLIX** gen1 card, clone and wipe

## Where this stands

I think it is ready. The capability gap that was blocking it is closed, and what remains on #251 is
the inventory, which is a different change.
~~~~
