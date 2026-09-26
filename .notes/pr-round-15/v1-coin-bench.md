# The V1 coin — predictions, written before the first frame

18mm green PCB coin. Out of the bag it arrived in, and nothing in this project has ever read or
written it before the capture below. **But it was a favour from another developer, not a purchase,
so its arrival state is his, not the factory's.** That single fact decides how this test has to be
read, and it is worth getting straight before any frame goes out.

## Baseline as read

    UID....... E0 11 22 33 44 55 66 91
    SYSINFO... 00 0F 91 66 55 44 33 22 11 E0 00 00 1B 03 01
    DSFID 00   AFI 00   IC ref 0x01   28 blocks x 4
    hf 15 dump 0..27 -- every block 00 00 00 00, every lck 0

**The UID is a placeholder.** Bytes 1 through 6 read `11 22 33 44 55 66`, a counting sequence that no
real UID is. So `Emosyn-EM Microelectronics USA`, decoded from `uid[1] = 0x11`, says nothing about
this silicon.

**Whose placeholder is not known.** It could be how the card ships, or it could be what the sender
left on it. The same goes for the blocks being uniformly zero and for the lock flags being clear:
that is consistent with a factory state and equally consistent with a developer who wiped it before
posting it. Nothing here can tell those apart.

## The problem this creates, and it is the whole problem

The card is here to answer **whether unlock and commit do anything at all on a card that has never
had them run**. It can only answer that if nobody has run them. If the sender already did, it cannot.

**And "locked" is unsourced.** That framing originated in this project's own notes as a hedge --
"possibly still LOCKED" -- with nothing behind it, and was later hardened into a claim attributed to
the sender. He said nothing on record about the card's state. The one piece of real evidence that it
is unconfigured is its placeholder UID, which is consistent with shipping that way and says nothing
about a lock.

That makes the two outcomes worth very different amounts:

- **A refused write to block 56 is informative.** Nothing in this project has ever directly shown a
  card refusing a UID-register write, and an already-armed card would not refuse it.
- **An accepted write to block 56 is AMBIGUOUS.** It reads as "V1 cards need no unlock" and it is
  equally consistent with "this card was unlocked before it was posted". Taken at face value it is
  exactly the over-scoped conclusion this project keeps paying for.

**So the cheapest measurement is not a frame at all: ask him.** One message — did you write to it,
and did you run an unlock or commit — settles what no single-shot test on this card can. Worth doing
before spending the irreversible step.

## What is actually spendable, and it is not the UID

Writing block 56 is **reversible**: write the original half back. The irreversible act is **unlock +
commit**, because arming cannot be undone and a card that arrived locked can only be seen that way
once. The ordering below follows from that and nothing else.

## Before any write — sweep it

`hf 15 dump` covers the ADVERTISED 28 blocks. The test writes **block 56**. Nothing at or above 28
has been read here, and `slix2-gold-30mm` established this morning that an advertised count is not a
physical top — it takes writes at block 100 against a claim of 79.

    tools/sweep-read-all.sh /dev/cu.usbmodemiceman1 ~/v1coin-sweep.txt

**The decisive part is what block 56 does.** On every gen1 chip measured here, 56/57/62/63 answer no
read at any point — write-only registers, nothing to destroy. This coin advertises 28, so 56 sits
above its count and has never been looked at. If it ANSWERS a read it is memory, and writing it
destroys content no one has recorded; if it refuses, that matches the register pattern and the write
is safe.

The sweep also says whether this card aliases the way `slix2-gold-30mm` does — 128 cells mirrored
across the 8-bit space — which would change how every later read is interpreted.

## The write to the UID register — reversible, and it goes first

Unaddressed, matching the app's own backdoor and the form measured on the other five cards.
**Deliberately not the addressed variant the earlier plan named**: addressing the backdoor has never
been measured anywhere, so an addressed frame changes two variables at once and a refusal would not
say which one did it. That is what the TI OPTION misreading cost. Addressing is a separate question
and this step repeats, so it can be asked afterwards.

    hf 15 reader                              positive control BEFORE
    hf 15 wrbl --ua -b 56 -d AABBCCDD -v      write block 56, unaddressed
    hf 15 reader                              positive control AFTER -- did the UID move?

**56 alone, not 56 and 57.** One register is enough to answer the question and leaves the other half
of the UID untouched, so the restore is a single frame.

**PREDICTED: refused, UID unchanged.** That is the project's standing expectation and the reason the
card was kept. It can come out the other way, which is what makes it a test.

- **Refused, with the card answering `hf 15 reader` either side** → it really is locked, which
  nothing here has ever directly shown. The card is worth its irreversible step; go on.
- **Accepted** → do NOT conclude "no unlock needed". Go to the arming probe below first.
- **Silent AND `hf 15 reader` fails after** → the card left the field. Not a result. Reseat, rerun.

## If that write was accepted — probe whether it was already armed

An armed card refuses unlock and commit while still taking 56/57; that is measured, in band as error
`0x10` on the LRi2K. So a refusal here is the signature of a card that was armed before it arrived.

    hf 15 raw -ackw -d 02213E00000000    unlock (0x3E) -- expect REFUSED if already armed
    hf 15 reader

- **Refused** → the sender armed it. The locked-card question is unanswered and this card can no
  longer answer it. Record that and stop; it costs nothing more.
- **Accepted** → it was not armed, yet took a UID write anyway. That would be new, and it would mean
  unlock is not a precondition on this silicon.

## Unlock and commit — IRREVERSIBLE, only if the UID write was refused

    hf 15 reader
    hf 15 raw -ackw -d 02213E00000000    unlock  (block 62 / 0x3E)
    hf 15 raw -ackw -d 02213F00000000    commit  (block 63 / 0x3F)
    hf 15 reader
    hf 15 raw -ackw -d 0221380000ABCD    retry block 56
    hf 15 reader

**PREDICTED: both accepted, and 56 then takes.** Watch the responses, not just the outcome: `00 78 F0`
from either would be **the first time unlock or commit has been seen ACCEPTED on any card here**.
Every previous observation is of an already-armed card refusing them.

## Addressed, only if the backdoor refuses unaddressed

Unaddressed first is the point; if the backdoor refuses it here, the addressed form becomes worth
trying. A refusal is ASSUMED to leave the state alone — an assumption, not a measurement, and if it
is wrong the unlock has already cost this.

## If the coin refuses everything, change the CODING before concluding anything

proxmark sends the magic sequence in **1-out-of-256** reader coding (`hf 15 raw -2`). This app uses
1-out-of-4. That difference has never been controlled for, and it does not matter on the five cards
that work -- but a refusal under a coding proxmark never used would prove far less than it looks
like. Re-send with `-2` before calling the card locked.

See the provenance section in [dearm-probe-bench.md](dearm-probe-bench.md): the sequence has no
documented semantics at all, so "locked" and "armed" are this project's vocabulary rather than
anything the source supports.

## Restore

    hf 15 wrbl --ua -b 56 -d 91665544

The arrival UID is `E0 11 22 33 44 55 66 91`, so `uid[7..4]` is `91 66 55 44`. If the sweep showed
block 56 ANSWERING a read, restore what it actually held instead — that value is memory, not a
register, and the UID mapping does not apply. Record what the runs leave behind and
**whether the card ends armed**, because that is a permanent change to the only specimen of its kind
here — and, if the sender did not arm it, the end of the one chance to observe a locked card.


---

# RESULT — the bare UID write is ACCEPTED, as predicted

    hf 15 reader                          -> E0 11 22 33 44 55 66 91
    hf 15 wrbl --ua -b 56 -d AABBCCDD     ( ok )
    hf 15 reader                          -> E0 11 22 33 DD CC BB AA

`AA BB CC DD` into block 56 lands as `uid[7..4]`, exactly the mapping measured on the other cards.

**So four cards have now taken a UID-register write with no unlock and no commit in front of it** --
and this one had never been written by anyone in this project and arrived wearing a factory
placeholder UID. That is as close to a definitive negative as the cards here can produce.

**It is four CARDS, not four chips.** This coin's type line decodes `uid[1] = 0x11` from the
placeholder, so its silicon is unknown. The three named ones are ST LRi2K, NXP SLIX and NXP SLIX-S.

**And it still does not license dropping the two frames from the app.** The cards that would prove
them necessary are the ones nobody here owns; four for four is a statement about this shelf.

## What this card can STILL answer, and nothing else can

The sweep showed 62/63 refusing READS -- which proves nothing about writes. **56/57 refuse reads too
and accept writes.** Reads and writes are separate questions at these addresses, and only the write
side has ever mattered.

This coin is, as far as anything known, **the only card here that has never been committed**. So a
write to 62 is the discriminator between the two models, and no other card can run it:

    hf 15 raw -ackw -d 02213E00000000      unlock, and WATCH THE RESPONSE

- **`01 10 ...`** -> block unavailable. Model (a): the registers do not exist, unlock and commit are
  frames sent into empty space, and "never needed on four cards" has a mechanism behind it.
- **`00 78 F0`** -> accepted. **The first time unlock has been seen taken on any card in this
  project.** Model (b): they exist, this card was genuinely un-committed, and the lock model is real.

Decisive either way, which is what the card was kept for. The cost is low now: if accepted the card
becomes armed, but it already behaves exactly as an armed card does, so nothing observable is lost.


## The unlock write — SILENCE at either coding, and it settles nothing

    hf 15 reader                            -> E0 11 22 33 44 55 66 91   (restored)
    hf 15 raw    -ackw -d 02213E00000000    -> command failed
    hf 15 reader                            -> E0 11 22 33 44 55 66 91
    hf 15 raw -2 -ackw -d 02213E00000000    -> command failed
    hf 15 reader                            -> E0 11 22 33 44 55 66 91

Bracketed either side, so the silence is the card's answer and not an absence. Identical under
1-of-256, so **the coding variable is eliminated** -- which also disposes of the retry-with-`-2`
guard this sheet carried.

**THE COIN DID NOT SETTLE THE QUESTION, and the plan is why.** It was designed for three outcomes --
accepted, `01 10`, or card gone -- and the card supplied a fourth. Silence discriminates nothing: it
is equally what a non-existent register does on a card that stays quiet, and what a committed one
does. The one outcome that would have proved model (b) was an ACCEPTANCE, and there wasn't one.

### What it did establish

**Two cards, two different behaviours at block 62:**

    lri2k-keychain     write to 62 -> 01 10 1E 06   in band, block unavailable
    v1-coin-green18    write to 62 -> silence, both codings

That retires an ambiguity this project has carried since the gen1 work -- "the client reported a
failure that does not separate an error frame from silence". Here it genuinely is silence, measured
with `hf 15 raw` and bracketed.

**And model (b) is weaker for it.** If the registers existed and this card had never been committed,
unlock should have been accepted. It was not. Either the sender committed it before posting, or the
model is wrong. With four cards, zero acceptances of unlock or commit ever recorded, and a bare UID
write that works on every one, **model (a) -- the registers are not there and those two frames do
nothing -- is the better-supported reading.** Not proven. Better supported.

### Still not a licence to drop the frames

Unchanged by any of this: the cards that would prove them necessary are the ones nobody here owns,
and proxmark sends them. Four for four remains a statement about this shelf.


## Commit is silent too — the card does not answer at either address

    hf 15 reader                         -> E0 11 22 33 44 55 66 91
    hf 15 raw -ackw -d 02213F69960000    -> command failed
    hf 15 reader                         -> E0 11 22 33 44 55 66 91

Bracketed. So this coin is silent at 62 AND 63, in both codings, while taking a bare UID write at 56.

## The test that should have been first — `slix-1k-coin18`

mfcarroll's, and it is better than anything above because it tests the lock model with a card already
owned instead of reasoning from an absence.

`slix-1k-coin18` is the **same form factor and visually identical** to the new coin, but it arrived
in the seller batch with `slix-1k-50x28` and **it has been written**: it was one of the four
`pm3`-marked tags the gen1 write probe ran on. So under model (b) it is COMMITTED, and the new coin
is not.

**If the two behave identically at 62/63, commitment makes no observable difference -- and model (b)
predicts one.** That is a direct test rather than an inference from silence.

    hf 15 reader
    hf 15 raw -ackw -d 02213E00000000
    hf 15 raw -ackw -d 02213F69960000
    hf 15 reader

No restore: writes to 62/63 are refused everywhere, so nothing changes. Run `--identify` first --
three tags here are visually identical and `coin18` no longer wears its factory UID.

**And the second possibility is worth saying out loud:** if they are the same product, the new coin
was never a special specimen, and "the only card that can answer whether unlock is needed" was a
description of a card we already had two of.

| card | 56 write | 62 write | committed? |
|---|---|---|---|
| `lri2k-keychain` | accepted | `01 10` in band | yes |
| `v1-coin-green18` | accepted | silence | supposedly not |
| `slix-1k-coin18` | ? | ? | yes |
