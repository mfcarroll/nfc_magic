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

**No value re-locks it.** Five values across both registers, unaddressed, plus both addressed —
refused every time, while block 56 still takes a write immediately afterwards. On this chip the card
stays armed and the de-arm question is answered in the negative.

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
