# #255 — the corrected title and body, and a comment saying what changed (DRAFT, NOT POSTED)

#255 is mfcarroll's issue, so it is corrected IN PLACE: the title and body are edited to the current
state (a dated "Updated" note at the top says what changed), and a short comment says the same, because
an edit notifies no one. Nothing here is `[👤]`; mfcarroll reads it before it goes.

**Order:** after the round-15 push, and BEFORE the reply on #250, which says "and so does #255" and "It is
on #255 as well". Edit the title and body first, then post the comment.

The shipped text it must agree with is the wipe's note in `iso15693_poller_wipe_blocks` and the Validation
bullets in the 2.3 notes.

## Title

~~~~
NFC Magic: an ISO15693 wipe silently overwrites gen3 magic registers — detectable in two block reads, unlike the gen1 case
~~~~

## Body

~~~~
> **Updated 2026-09-30**, after the latest round of #250. The gen1 half is corrected: its hazard is any
> gen1 card the wipe's sweep reaches, not one left armed, and a written gen1 UID takes effect at once
> rather than on the next power-up. The gen1 case has been reproduced on hardware, and a genuine gen3 card
> has since been put to the app's paths; see the comments.

### App

NFC Magic

### Describe the enhancement.

The ISO15693 wipe zeroes every data block the card physically holds. On a gen3 magic card four of those
blocks are not data — they are what makes the card magic — and the wipe cannot tell, because it performs
no magic detection at all. The flow is menu → confirm → sweep, with no backdoor probe and no
identification step, so any ISO15693 tag presented to Wipe gets swept.

gen3 (proxmark's `hf 15 csetuid --v3`) keeps its UID in blocks `0x10`/`0x11` and a configuration
signature in `0x14`/`0x15`, and stays rewritable until `hf 15 cfinalize` locks it permanently. All four
are inside the data range of even a 32-block card, so a wipe:

- moves the UID, by zeroing `0x10`/`0x11`
- destroys the signature at `0x14`/`0x15`, which is what proxmark reads to decide the tag is still in
  configuration mode

So the card comes out with a changed UID and no longer identifying as re-writable, from an operation that
gave no indication it was going to touch anything but data.

**Proposed change: a pre-flight check.** Read `0x14`/`0x15` before the sweep and compare against the
gen3 signature `A5 2B 44 2C` / `21 AE 93 00`. That is two block reads, and the signature exists only
while the card is un-finalized — which is exactly the window in which the hazard exists — so the check is
precise rather than heuristic. On a match, warn naming what the wipe is about to overwrite and require an
explicit opt-in, the same shape as the gen1 fallback's existing consent screen.

This is deliberately *not* a request for gen3 support. Writing gen3 UIDs is a separate feature and would
mean a third generation in the opt-in ladder. This is only about not destroying one silently.

### Anything else?

### Where this comes from

Raised during review of #250 (ISO15693 / NfcV support). That PR declares gen3 unsupported in its
CHANGELOG, including that a wipe overwrites those blocks — which warns someone who reads release notes,
but not someone who picks Wipe with a gen3 card on the reader. They see only the generic "Wipe card?"
confirm.

The wipe's post-power-cycle UID re-check does surface `uid_changed` afterwards, so the identity half is
*reported* — but incidentally, exactly as it would be for any card whose UID shifts. Nothing speaks for
the signature, and nothing warns beforehand.

### proxmark references

Verified against proxmark3 master:

| fact | location |
|---|---|
| `hf 15 csetuid --v3` | `client/src/cmdhf15.c:3405` |
| `hf 15 cfinalize` ("Finalize a magic V3 tag (irreversible)") | `client/src/cmdhf15.c:4316` |
| UID in `ISO15_MAGIC_V3_BLK_UID_LO/HI` = `0x10`/`0x11` | `client/src/cmdhf15.c:3313-3320` |
| signature `A5 2B 44 2C` / `21 AE 93 00` in `0x14`/`0x15` | `client/src/cmdhf15.c:3313-3320` |
| "signature in blocks 0x14/0x15 not found — already finalized or not a V3 tag" | `client/src/cmdhf15.c:3540` |

### The related case that cannot be checked for

Recording it here rather than separately, because it is the same class — a magic generation whose
registers live in ordinary data space, on a card the wipe cannot identify — and because the contrast is
the useful part.

gen1 keeps its UID in blocks 56/57, with two more registers at 62/63 that proxmark writes first and this
app calls unlock and commit — what they do is inferred. The ISO15693 wipe clears them
deliberately: on a gen2 card they are ordinary user data, and sparing them would leave real data behind
on the card most people actually have. That trade is documented in the poller and is not what this issue
questions.

The hazard is any gen1 card the sweep reaches. On all three gen1 chips measured — ST LRi2K, NXP ICODE SLIX
and NXP ICODE SLIX-S — blocks 56/57 took a write with no unlock or commit sent before it: five cards, one
of them a card this app had never written, and whether any had been armed first cannot be told. So zeroing
56/57 can move the UID of any gen1 card the sweep reaches. What bounds it is the reach rule — how far the
sweep runs, given what the card claims and where it stops answering reads — not the card's history, which
nobody can know.

**No pre-flight check is possible for it.** gen1 has no signature to read, and a gen1 card is
indistinguishable from any other ISO15693 card without writing to it. Nor does write ordering help. A
written UID takes effect at once — an inventory in the same field session already returns it, on all three
chips — and writes to 62/63 are refused on all three, so there is nothing to clear on the way past.

So the mitigation already implemented is the only one available — re-read the UID after the wipe behind a
field power-cycle, and report a change as Partial rather than promising the UID survived. Detection after
the fact, which for this case is the best there is. The power-cycle is not what makes the change visible,
since that is immediate; it gives the read a cleanly re-activated card.

| | registers | pre-flight check | current state |
|---|---|---|---|
| gen3 | `0x10`/`0x11`, `0x14`/`0x15` | **yes** — signature at `0x14`/`0x15` | declared in the CHANGELOG only |
| gen1 | 56/57, 62/63 | **no** — nothing to read | post-wipe UID re-check reports a change |

Filed together so the gen1 half is not read as an oversight sitting beside a gen3 half that gets a check
gen1 cannot have.

### Not blocking

Neither case affects the gen2 cards this feature was built and validated against. The gen1 case has been
reproduced on hardware: an ST LRi2K, the one card tested whose sweep reaches 56/57, reported "Wiped
58/58", its UID moved to all zeros at once, and the post-wipe re-read reported it. The gen3 mechanism above
was read out of proxmark's source and the app's own control flow; a genuine gen3 card has since been put
to the app's clone and Write UID paths (see the comments), and the brick itself is deliberately untested.
~~~~

## Comment

~~~~
Updated the title and body to the current state. Two claims in the gen1 half were wrong, both measured in the latest round of #250: the hazard is not limited to a card left armed — on all three gen1 chips, blocks 56/57 take a write with no unlock or commit before it, so it is any gen1 card the wipe's sweep reaches — and the registers do not latch on the next power-up; a written UID takes effect at once. The gen1 case has also been reproduced on hardware since. The gen3 half is unchanged.

**A gen3 card is now in hand.** A genuine gen3 tag was kindly provided by @0x6r1an0y — un-finalized, its `0x14`/`0x15` configuration signature an exact match to proxmark's V3 config mode. Put to #250's paths: the gen2 backdoor leaves its UID unchanged, since the UID lives at `0x10`/`0x11`, so a clone or Write UID lands on the gen1 opt-in; accepting that writes 56/57/62/63 as ordinary data, with no identity move, the configuration signature untouched, and it restored byte-identical. That is the cost the release notes describe, now measured rather than reasoned.

The brick is deliberately not tested. Per @0x6r1an0y, zeroing `0x14`/`0x15` on an un-finalized card bricks it, and the pre-flight probe this issue asks for is the fix, not something to confirm by destroying a card. This card carries the exact config-mode signature proxmark checks before `cfinalize`, so nothing but that probe stands between it and a wipe. The wipe hazard and the probe both stand.
~~~~
