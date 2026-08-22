# DRAFT issue — NOT FILED

Target: `xMasterX/all-the-plugins`. Use the **Enhancement** form
(`.github/ISSUE_TEMPLATE/03_enhancement.yml`), which auto-applies `type/enhancement`.

Not the Bug form: its Reproduction field is required and we cannot reproduce this — no gen3 card exists on
either side of the PR. And the app no longer says anything untrue (the CHANGELOG states that a wipe
overwrites those blocks), so what is missing is a safeguard rather than a correction. #252 and #253 are
both `type/bug` because both were observed on hardware; this one is not.

Paste each block below into the matching form field.

---

## Title

```
NFC Magic: an ISO15693 wipe silently overwrites gen3 magic registers — detectable in two block reads, unlike the armed-gen1 case
```

---

## App

```
NFC Magic
```

---

## Describe the enhancement.

```
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
```

---

## Anything else?

```
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

gen1 keeps its UID in blocks 56/57 with unlock/commit in 62/63. The ISO15693 wipe clears them
deliberately: on a gen2 card they are ordinary user data, and sparing them would leave real data behind
on the card most people actually have. That trade is documented in the poller and is not what this issue
questions.

The hazard is narrower. A wipe cannot *arm* a gen1 UID change by itself — arming needs `0x6996` in the
commit block and a wipe writes zero — but that says nothing about a card left **already armed** by an
earlier gen1 UID write. There, zeroing 56/57 can move the UID.

**No pre-flight check is possible for it.** gen1 has no signature to read, and an armed card is
indistinguishable from any other ISO15693 card without writing to it. Nor does write ordering help: the
registers latch on the next power-up, so anything that would de-arm the card is itself a write to the
blocks in question.

So the mitigation already implemented is the only one available — re-read the UID after the wipe behind a
field power-cycle, and report a change as Partial rather than promising the UID survived. Detection after
the fact, which for this case is the best there is.

| | registers | pre-flight check | current state |
|---|---|---|---|
| gen3 | `0x10`/`0x11`, `0x14`/`0x15` | **yes** — signature at `0x14`/`0x15` | declared in the CHANGELOG only |
| armed gen1 | 56/57, 62/63 | **no** — nothing to read | post-wipe UID re-check reports a change |

Filed together so the gen1 half is not read as an oversight sitting beside a gen3 half that gets a check
gen1 cannot have.

### Not blocking, and not tested on hardware

Neither case affects the gen2 cards this feature was built and validated against. No gen3 card and no
gen1 card exist on either side of #250, so neither of these has been exercised on hardware — the gen3
mechanism above is read out of proxmark's source and the app's own control flow, not observed.
```
