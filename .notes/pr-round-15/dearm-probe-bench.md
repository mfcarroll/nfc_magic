# Can an armed gen1 card be locked again? — predictions, written before the first frame

Raised by mfcarroll: one-way arming is odd. Nobody has ever attempted a de-arm, and the belief that
it is impossible rests on thinner evidence than its confidence suggests.

## Why this runs BEFORE the V1 coin

The coin's whole design is shaped by unlock+commit being irreversible — it is why that card is
"spendable once" and why the plan spends a reversible question first. **If anything re-locks a card,
that constraint disappears** and the coin becomes an ordinary reusable fixture. Asking afterwards
would mean having spent it under a restriction that might not exist.

## It also collects evidence the addressing work needs

Addressed WRITE BLOCK has never been sent to 56/57/62/63 as part of the magic sequence. **An IN-BAND
refusal to an ADDRESSED write proves the card received, addressed-matched and parsed that frame** —
a card that ignored the addressing would answer identically to an unaddressed one, but a card that
answers `01 10 ...` to an addressed frame has demonstrably processed it. A refusal is the expected
outcome here, and it is still the evidence.

## What is actually known, which is less than the confidence around it

- The register meanings are **inferred**. The defines say so: `written as 0; inferred: unlock`,
  `written as 0x6996; inferred: arms the UID change`. `0x6996` is the value proxmark sends.
- The irreversibility evidence is an armed card refusing writes to 62/63 — in band, error `0x10`,
  **on `lri2k-keychain` and on that card alone**. On the other two gen1 chips the client reported a
  failure that does not separate an error frame from silence.
- The `do NOT try to de-arm` comment in the wipe governs that sweep's write ORDER. It is reasoning,
  not an experiment.

## Cards, and read the UID of each before touching it

`lri2k-keychain`, `slix-1k-50x28`, `SL2S5302` — all three have had a gen1 UID write, so all three
should be armed. **Do not assume it; step A proves it per card.** Two of them wear
`E0 04 01 10 A1 A2 A3 A4` from earlier work, so `--identify` or the form factor decides which is on
the antenna, not the UID.

    hf 15 reader        record the UID -- every restore below depends on it

`hf 15 wrbl` takes `-u <uid>` for addressed and `--ua` for unaddressed and handles the byte order
itself, so no hand-built frames are needed. It silently forces the OPTION flag for TI manufacturer
bytes; none of these three is TI, so that does not apply here.

## Starting states, recorded before any probe frame

    lri2k-keychain    UID 00 00 00 00 00 00 00 00   DSFID 02
    slix-1k-50x28     UID E0 04 01 10 5E ED 00 01   DSFID 00
    SL2S5302          UID E0 04 01 10 A1 A2 A3 A4   DSFID 00

Two wear cloned UIDs from earlier work; that is fine here, since the probe needs only a KNOWN
starting value and a readable UID. **Restore each to the value above, not to its factory UID**, so
the probe leaves every card where it found it.

`lri2k-keychain` was the exception and was restored FIRST, to `E0 02 22 24 50 00 83 03`. Not
tidiness: step B sends addressed frames, and addressing to an all-zero UID is a degenerate case — an
odd result there would have been read as the backdoor refusing addressing when it was the zero UID
all along, on the one card whose in-band `0x10` the whole belief rests on.

    hf 15 wrbl --ua -b 56 -d 03830050    ( ok )
    hf 15 wrbl --ua -b 57 -d 242202E0    ( ok )
    hf 15 reader -> E0 02 22 24 50 00 83 03

**That restore IS step A for this card** — two writes to the UID registers accepted, the UID moved,
which is what "armed" means operationally. Its DSFID reads 02 against an original 00; restoring that
needs a WRITE DSFID rather than a block write and has no bearing here.

## Step A — prove the card is armed. REVERSIBLE, and it must be able to fail

    hf 15 reader                                  note UID, call its top half U7654
    hf 15 wrbl --ua -b 56 -d AABBCCDD -v
    hf 15 reader                                  expect UID high half -> AA BB CC DD

**PREDICTED: accepted, and the UID moves.** That is what "armed" means operationally. If it is
REFUSED, this card is not armed and it is the wrong specimen for this probe — stop and note it,
because a card that refuses a 56 write is itself a finding nothing here has recorded.

## Step B — try to de-arm. Refused writes change nothing, so this is free

Run all of these, recording the exact response for each. Order matters only in that the unaddressed
form comes first, matching what the app sends.

    hf 15 wrbl --ua -b 62 -d 00000000 -v          unlock, the normal value
    hf 15 wrbl --ua -b 63 -d 69960000 -v          commit, the normal value
    hf 15 wrbl --ua -b 63 -d 00000000 -v          commit CLEARED -- the obvious de-arm candidate
    hf 15 wrbl --ua -b 63 -d 96690000 -v          commit, bytes swapped
    hf 15 wrbl --ua -b 62 -d FFFFFFFF -v          unlock, a non-zero value
    hf 15 wrbl -u <UID> -b 62 -d 00000000 -v      ADDRESSED unlock
    hf 15 wrbl -u <UID> -b 63 -d 00000000 -v      ADDRESSED commit cleared

**PREDICTED: every one refused.** On `lri2k-keychain`, in band as error `0x10`. On the other two the
client may not distinguish an error frame from silence — that ambiguity is itself the thing worth
resolving, so record the raw output rather than the summary line.

## Step C — did anything change? This is the actual measurement

    hf 15 wrbl --ua -b 56 -d 11223344 -v
    hf 15 reader

- **Accepted, UID moves to 11 22 33 44** → still armed. Nothing in step B de-armed it.
- **REFUSED** → something in step B re-locked the card. Work out which by repeating step B one frame
  at a time from a re-armed card. This would be a genuinely new result.

## Step D — restore

    hf 15 wrbl --ua -b 56 -d <U7654>              the original high half
    hf 15 reader                                  confirm the original UID is back

## What each overall outcome is worth

- **Everything refused, 56 still works on all three** → the strongest form of the current belief, now
  measured on three chips instead of one. The coin stays one-shot and its sheet stands unchanged.
- **Anything accepted at 62/63** → arming is not a one-way door. The coin stops being one-shot and
  its plan should be rewritten before it is spent.
- **An in-band `01 10` to an ADDRESSED frame** → independent of the de-arm question, that is the
  evidence that addressed frames reach the backdoor registers, which the addressing work needs.


---

# RESULTS — `lri2k-keychain`

    hf 15 wrbl --ua -b 62 -d 00000000     ( fail )
    hf 15 wrbl --ua -b 63 -d 69960000     ( fail )
    hf 15 wrbl --ua -b 63 -d 00000000     ( fail )
    hf 15 wrbl --ua -b 63 -d 96690000     ( fail )
    hf 15 wrbl --ua -b 62 -d FFFFFFFF     ( fail )
    hf 15 wrbl -u E002222450008303 -b 62 -d 00000000     ( fail )
    hf 15 wrbl -u E002222450008303 -b 63 -d 00000000     ( fail )

    hf 15 wrbl --ua -b 56 -d 11223344     ( ok )   -> UID E0 02 22 24 44 33 22 11
    hf 15 wrbl --ua -b 56 -d 03830050     ( ok )   -> UID E0 02 22 24 50 00 83 03   restored

**That is NOT "no value re-locks it", and the sheet said so before mfcarroll asked what the values
were based on. Nothing.** `0x6996` is the only value with a source — it is what proxmark's
`hf_15_magic.lua` sends. `00000000`, `96690000` and `FFFFFFFF` were invented here: "clear it", "swap
the bytes", "try non-zero". There is no lock or relock function anywhere in proxmark; the script sets
a UID and there is no counterpart.

**And the probe could not have tested a value anyway.** On this card the write is refused at the
BLOCK, before the payload is considered, so five payloads against a block that refuses every write
distinguish nothing. The result is one fact repeated seven times.

So the de-arm question is not answered in the negative. It is **unasked**: no candidate value exists
to try, and no write reaches those registers on this card to try one with.

## But the tool was wrong, and half the probe got nothing

`hf 15 wrbl -v` does not print the response. Every refusal is a bare `( fail )`, so **an in-band
error frame and silence are still indistinguishable** — which is exactly the ambiguity this probe
existed to resolve, and the flag was specified on the assumption that `-v` would surface it. The
recorded `01 10` on this card must have come from `hf 15 raw`.

It costs the addressing evidence too. `( fail )` on the addressed frame reads equally as "the card
address-matched, parsed the write and refused it" and as "the card ignored the addressing", and those
are opposite conclusions.

**Re-run with raw frames and a mis-addressed control**, which is what makes it conclusive:

    hf 15 raw -ackw -d 02213E00000000                    unaddressed unlock
    hf 15 raw -ackw -d 222103830050242202E03E00000000    addressed, CORRECT uid
    hf 15 raw -ackw -d 222103830050242202E13E00000000    addressed, WRONG uid (E0->E1)
    hf 15 reader                                          positive control

In band from the correct address plus silence from the wrong one proves the card is filtering on the
address at block 62. Silence from both would instead revise what is recorded about this chip
answering in band at all.


## The raw re-run — ADDRESSING IS PROVEN AT THE BACKDOOR REGISTERS

    hf 15 raw -ackw -d 02213E00000000                    -> (4) 01 10 1E 06
    hf 15 raw -ackw -d 222103830050242202E03E00000000    -> (4) 01 10 1E 06
    hf 15 raw -ackw -d 222103830050242202E13E00000000    -> command failed  (silence)
    hf 15 reader                                          -> E0 02 22 24 50 00 83 03

Three results in four frames, and the third is what makes the second mean anything.

**The addressed frame was address-matched and parsed.** It returned the identical in-band error to
the unaddressed one -- flags `01`, error `10` (block not available), CRC `1E 06`. A card that
ignored the addressing could not distinguish the two, but a card that answers the SAME error to both
has evidently processed the addressed form on its merits.

**And the card filters on the address.** One byte wrong -- `E0` to `E1` in the last UID byte -- and it
says nothing at all, bracketed by a `hf 15 reader` that proves the card was present and answering
throughout. Silence there is refusal, not absence.

**So the refusal at 62 is because the card is ARMED, not because of the addressing.** Addressing is
transparent at these registers. That is the evidence the gen1 backdoor addressing change needed and
did not have: addressed WRITE BLOCK reaches block 62, is parsed there, and is filtered by UID.

Incidentally `01 10 1E 06` is byte for byte what a TI Tag-it returned through the standalone EOF
earlier the same day for blocks past its capacity -- the same error from different silicon, which is
a small corroboration that both readings are of a real response rather than a fragment.

## What is still unmeasured for the addressing change

**unlock or commit ACCEPTED addressed.** Every card on the bench is armed, so every one refuses them
whatever the form. Only an un-armed card can show one taken -- which is the V1 coin, and is why the
addressed variant belongs in that spend.

The reason to expect it to work is now much stronger than before this run: the frame is received,
address-matched and parsed at that exact block, and the refusal is on a ground unrelated to
addressing.

**One chip.** ST LRi2K. `slix-1k-50x28` and `SL2S5302` should get the same four frames, substituting
each card's own UID, before this is written up as anything broader.


## A better reading of the whole thing — those registers may simply NOT EXIST

`0x10` is not "refused" and not "locked". The SDK names it
`ISO15693_3_RESP_ERROR_BLOCK_UNAVAILABLE`, and in ISO15693-3 it means the addressed block **is not
available — it does not exist**. The card is not declining the write; it is saying there is nothing
at that address.

Line that up with everything else on the bench and a simpler model appears:

- **unlock and commit have NEVER been observed accepted, on any card, in this project.** Not once.
- 56/57 accept writes on every gen1 card here **without** unlock or commit having succeeded first.
- 56/57 answer no read ever, so they are write-only registers; 62/63 answer `0x10`, so they are
  absent.

**Which would mean there is no "arming" on this silicon at all.** These cards' UID registers are
simply writable, unlock and commit are no-ops sent into empty address space, and the
locked-then-armed model is imported from a card type nobody here owns. Every observation fits that,
and it needs no state change to have ever occurred.

The competing reading is that a locked card reports `0x10` for a register that exists but is
protected. Implementations do vary. But nothing here has ever seen the other state, so the
lock model rests on no observation of this project's own.

**This is what the V1 coin actually tests**, and it reframes that bench. Not "is this card still
locked" but **"does unlock or commit do anything on any card"**. If they are ACCEPTED there, 62/63
exist and the model is real. If they answer `0x10` there too while 56 still takes a write, the model
is probably wrong for this whole family, and the app has been sending two frames into nothing.

**Do not act on this in the app yet.** Those two frames are harmless when refused, proxmark sends
them, and some card somewhere presumably needs them. But it changes what the coin is for, and it is
worth knowing before the addressing change decides how many frames it has to address.


## Where the sequence comes from, and what proxmark says about it — NOTHING

Traced 2026-09-26 after mfcarroll asked what the de-arm values were based on. The whole of the
external source is `proxmark3/client/luascripts/hf_15_magic.lua`:

    hf 15 raw -2 -c -d 02213E00000000        WRITE BLOCK 0x3E (62) = 00 00 00 00
    hf 15 raw -2 -c -d 02213F69960000        WRITE BLOCK 0x3F (63) = 69 96 00 00
    hf 15 raw -2 -c -d 022138<uid[7..4]>     WRITE BLOCK 0x38 (56)
    hf 15 raw -2 -c -d 022139<uid[3..0]>     WRITE BLOCK 0x39 (57)

`0x02` is high data rate and unaddressed; `0x21` is WRITE BLOCK, a STANDARD ISO15693 command rather
than anything proprietary; `-c` appends the CRC.

**The surrounding commentary, in its entirety:** `--- Set UID on magic command enabled on a ICEMAN
based REPO` and a `print('Using backdoor Magic tag function')`. Copyright 2018, Christian Herrmann,
v1.0.6, described as "This script tries to set UID on a IS15693 SLIX magic card".

**Nothing anywhere mentions unlock, commit, arming, locking, what `0x6996` means, or why 62 and 63.**
There is no C implementation either -- `6996` appears nowhere else in the client except unrelated ATR
and APDU tables.

So **"unlock" and "commit" are this project's names**, which the defines already admit with
`inferred:`. The locked-then-armed model is ours on top of four undocumented writes labelled "set
UID". That is worth knowing before spending a card to test a state that may only exist in our
vocabulary.

Compare proxmark's V3 support in `client/src/cmdhf15.c`, which is properly worked: named block
constants, a config-mode signature (`A5 2B 44 2C` / `21 AE 93 00`) distinct from finalize values
(`A5 2B 44 2C` / `69 E2 5D 00`), and comments carrying the raw frames. The V1 magic got none of that
treatment.

### One real divergence, and it is a live variable for the coin

**proxmark sends all four frames in 1-out-of-256 reader coding** -- that is what `-2` selects. This
app uses the SDK's encoder, which is **1-out-of-4** (`bit_patterns_1_out_of_4` in
`furi_hal_nfc_iso15693.c`).

Never noted before, and evidently it does not matter for the five cards that work. But it is a
variable this project has never controlled, and **if the V1 coin refuses the sequence, try 1-of-256
before concluding it is locked** -- that is the coding the sequence was discovered with, and a
refusal under a coding proxmark never used would prove much less than it appears to.


## The V3 precedent — which argues AGAINST the "no arming" reading above

Asked by mfcarroll whether chip schematics could supply the missing value. They cannot, and the
reason matters: **these are not NXP or ST parts.** A genuine SLIX has no writable UID registers at
56/57 -- that is what makes a magic card magic -- so the silicon is an unbranded clone emulating
SLIX, and NXP's and ST's datasheets describe a part that by definition lacks the backdoor. There is
no schematic because there is no acknowledged chip.

But the question turned up something better in proxmark's V3 support, which IS documented:

    "This operation is irreversible." After finalize the configuration...
    arg_lit0("y", "yes", "Confirm the irreversible finalize operation")
    "run `hf 15 cfinalize` afterwards to lock the UID permanently."

**V3 implements an explicit one-way config lock, and the community treats it as one.** The mechanism
is the same shape as what V1 shows: after the commit-equivalent, the config registers stop accepting
writes. For V3 that is by design -- the point of finalizing is to make the card permanently
indistinguishable from a genuine tag.

**So the "62/63 probably never existed" reading recorded above is under-informed, and this is the
correction.** Both models remain live:

- **(a) no arming** -- 62/63 never existed, unlock and commit are no-ops, these cards always accepted
  UID writes. Supported by `0x10` meaning "block unavailable" and by neither frame ever being seen
  accepted.
- **(b) armed** -- 62/63 existed, commit locked them permanently, exactly as V3's finalize does.
  Supported by a sibling magic family in the same command space doing precisely this, on purpose.

`0x10` does not discriminate: a card that locked a register away could report it unavailable with the
same code. **The V3 precedent tilts toward (b)**, which is the opposite of where the previous section
leaned, and neither section should be quoted without the other.

**The V1 coin is the discriminator**, and this is what makes it worth spending: on a card that has
never been committed, model (b) predicts unlock and commit ACCEPTED, and model (a) predicts `0x10`
from both while 56 still takes a write.

### Cheaper than any of this

**Ask the developer who sent the coin.** He chose to send a V1 specimen and described it as locked,
so he may know the command set outright, or know that it is undocumented. One message settles more
than a blind search over 256 blocks times a 32-bit payload times a command byte.
