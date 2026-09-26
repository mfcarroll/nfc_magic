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
