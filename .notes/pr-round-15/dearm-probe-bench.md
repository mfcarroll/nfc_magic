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
