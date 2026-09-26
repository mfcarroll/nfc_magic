# Two reporting gaps a gen1 clone exposed — measured 2026-09-25

Both found by cloning a 28-block source onto larger cards and reading the result back. Neither is a
failure of the write; both are things the user is not told and can act on.

## 1. A gen1 clone does not reproduce geometry, and says nothing

`SL2S5302`, 40 blocks, IC ref `0x02`, cloned from a 28-block SLIX source with gen1. Afterwards it
answers to the source's UID and carries its data, and still reports **40 blocks, IC ref 0x02**. The
type line a reader prints is decoded from the UID, not the chip.

That is correct behaviour -- the gen2 backdoor has a CFG register that programs what the card
reports, and **gen1 has none** -- but the app reported a bare Success, and the release notes claimed
the clone writes "the source's identity (IC ref / block geometry / AFI / DSFID), so the copy
advertises the same chip" without qualification. The notes are fixed; the screen is not.

**Gate it on the mismatch.** A 28-block source onto a 28-block card matches by luck and there is
nothing to say -- which is why the `slix-1k-50x28` clone's bare Success was fully honest. Compare
the source's reported geometry against the card's and speak only if they differ. Both sides are
CLAIMS, and claims are the right subject here: the question is what a reader will see.

## 2. Blocks above the source keep the previous card's data, and the claim hides it

`gen-2-card`, 64 physical, filled with `wipeseed_64` so every block was distinct, then cloned from
the same 28-block source over gen2:

```
before   claims 64   rdbl 64 -> fail   rdbl 69 -> fail
after    claims 28   rdbl 28 -> 5A 1C A5 1C   rdbl 40 -> 5A 28 A5 28
                     rdbl 63 -> 5A 3F A5 3F   rdbl 64 -> fail
```

The gen2 CFG frame reprogrammed the advertised count **down** to the source's 28, so `hf 15 dump`
now shows a clean 28-block card while blocks 28-63 hold the previous card's data. **The residue is
invisible to the ordinary tool**, which is what makes it worth reporting: the user cannot easily
find it.

Note the asymmetry. On gen1 the card keeps reporting its own size, so the residue is at least
visible; on gen2 the claim is rewritten over it.

### Why the advertised count cannot be the test

Raised by mfcarroll and it is the point the design turns on: the count before the clone is a claim
too, and on a magic card a claim is a costume. Deriving "blocks 28-39 hold residue" from a number the
card chose would be a guess dressed as a measurement.

**So probe.** Past physical capacity a block refuses reads outright -- measured, and the same
discriminator the clone already uses when it asks whether a refused block is even there. Read upward
from the source count until `ISO15693_POLLER_WIPE_ABSENT_RUN` blocks answer nothing, or the budget
stops it. Non-destructive: the wipe sweeps with writes because it is wiping.

Measured in both directions now. Reads succeed above the advertised count (above) and fail above the
physical top (`rdbl 64` here, and `black-tag`'s blocks 64-69 refusing writes with their read-backs
failing).

### Two things the rule must get right

- **Probe regardless of which path was used.** `clone_used_gen1` is known by then, so "we cannot
  tell" is not the reason. The reason is that "gen1 claims are truthful" is an inference from five
  cards about a mechanism, and the whole wipe design exists because claims lie. Eight reads on a card
  that is telling the truth is the price of not resting on that.
- **Existing is not residue.** Blocks above the source that read as ZEROS are space, not data, and
  must not raise a warning. Only non-zero content counts, and the message should name the range.

# 3. A gen1 card reached down the gen2 path — measured 2026-09-25

The gen2 verify passes whenever the card already WEARS the target UID, because then it proves only
that the UID matches. A re-clone is exactly how a card comes to wear it.

`slix-1k-50x28`, gen1, cloned twice from `wipeseed_64`:

```
first clone    gen1 opt-in shown, registers skipped    Cloned 28/60    CORRECT
second clone   no opt-in, gen2 path, registers in scope
               -> UID becomes 39 A5 39 5A 38 A5 38 5A
```

That is the file's blocks 56 (`5A 38 A5 38`) and 57 (`5A 39 A5 39`) through the UID mapping. The
screen said "Cloned 30/64, Not written: 34" and nothing about the identity.

**And the re-address is what completed it.** Block 56 landed and moved the UID; the re-address picked
up the new identity; block 57 went out addressed to THAT and landed too. Without it the UID would
have been half-replaced. The mechanism built to stop addressing breaking the armed-gen1 wipe is what
let the clone finish destroying the identity cleanly -- it has no opinion about whether it should.

## The fix, and the one that was rejected

**Rejected** (mine): skip the backdoor blocks whenever the UID already matched. mfcarroll's objection
is decisive -- it breaks a real gen2 card re-cloned from a DIFFERENT image sharing its UID. Those four
are ordinary memory there, the skip deducts them from the total, and the run reports a clean
"Cloned 60/60" over four blocks of the file it chose not to write. A silent data loss traded for a
rare identity loss.

**Taken** (mfcarroll's): watch and react. A write to 56/57 that moves the UID to exactly what that
write implies can only mean those addresses are registers. The write that does the damage is the same
event that identifies the card, so the run converts to a gen1 clone from there, keeps going above the
registers, and puts the target UID back afterwards. The end state is one already tested and already
reported correctly.

## The re-address now checks its answer

Also mfcarroll's: can we predict the new UID rather than trust an inventory? Not replace it -- the
prediction only holds IF the card is gen1, which is the thing being tested -- but it makes the answer
checkable. Two honest replies: unchanged, or what the write implies. A third is a bystander answering
the 1-slot inventory (#251) and is refused. Covers the wipe too, since it is one mechanism.

Not airtight, and unfixably so: a bystander holding the predicted UID passes, because magic cards
make UIDs non-unique and a 1-slot inventory cannot tell two cards apart.

## Bench: PASSES

Same two clones after the fix. Second run: no opt-in, converts, and the UID reads back
`E0 04 01 10 5E ED 00 01` -- intact. Reported as "Cloned 28/60 / gen1: 56/57/62/63 differ", identical
to the first run, which is the point.

**A Details paragraph saying the identity had been disturbed and restored was written and then cut**,
on mfcarroll's objection: it narrates what the app did rather than what the card is, and the gen1
caveat already says the only part that matters. Same fault this project keeps removing from its prose,
found in the UI.
