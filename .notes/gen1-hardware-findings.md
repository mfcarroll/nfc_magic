# gen1 on real silicon — 2026-09-08

Six rounds of PR #250 carried the line "no gen1 card confirmed on either side". This is the session that
ended that. Written as durable findings plus a work list, because the corrections it implies belong in a
delta AFTER the comment cut, not folded into it.

**The card:** `lri2k-keychain`, and only that one. Everything below is a single sample. See
[tag-inventory.md](tag-inventory.md) for its full record and
`tools/campaigns/iso15_20260908_025701/` for the raw pm3 output.

## How it was found, which is reusable knowledge

The seller's listing was titled *"15693 UID Changeable + **Lua Script by Iceman** Compatible ST LRi 2K
(0-55 block)"*. `proxmark3/client/luascripts/hf_15_magic.lua` sends exactly four frames:

```
02213E00000000     WRITE BLOCK 0x3E (62) = 0
02213F69960000     WRITE BLOCK 0x3F (63) = 0x6996
022138<uid hi>     WRITE BLOCK 0x38 (56)
022139<uid lo>     WRITE BLOCK 0x39 (57)
```

`0x21` is `ISO15693_WRITEBLOCK`. That is **byte-for-byte** `SetTag15693Uid` in
`armsrc/iso15693.c:3191`, i.e. what `hf 15 csetuid` sends with no flag. So **"Lua Script by Iceman" in a
listing means gen1**, and it is the cheapest way to shop for one. There is no fourth magic mechanism to
implement — our gen1 probe already covers it.

## Finding 1 — gen1 works, and is reversible

`hf 15 csetuid -u E0F1E2D3C4B5A697` set the UID; it read back exactly; the original restored via gen1.

## Finding 2 — the backdoor registers accept writes WITHOUT acknowledging

This one is directly citable and replaces an inference with a measurement.

`iso15693_poller.h`'s `gen1_attempted` says the frames' return values are discarded, "as they must be on
a card that may not answer". That was read out of proxmark's source: `SetTag15693Uid` sends all four
frames in a loop and never checks `res` or `recvlen` between them.

**Now observed.** An unaddressed zero write to block 62 — gen1's own first frame — returned no ACK, and
then the full sequence worked. So that write *was* accepted, silently. A backdoor register that accepts a
write without answering is indistinguishable, from outside, from one that refuses it.

Consequence for tooling: **no write-based probe of these registers can have a meaningful negative.** Only
the full sequence plus a power-cycled UID re-read is conclusive.

## Finding 3 — the armed-gen1 wipe hazard, demonstrated

The card was left armed by the test (`0x6996` in block 63, which nothing clears). Running the app's wipe:

```
UID changed
Wiped 58/58. The card's
UID moved. Now reads:
00000000 00000000
```

**#255's central claim held, and so did the mitigation this PR ships.** The wipe zeroed 56/57, the armed
card latched it on power-up, the identity moved, and the post-power-cycle UID re-read caught it —
`uid_verified` reached an answer and the run reported `uid_changed` as Partial rather than a clean wipe.

**It is WORSE than either document says.** The UID did not move to another *valid* identity — it moved to
**all zeros**, and an ISO15693 UID must begin with `0xE0`. The card is left with no valid identity at
all. #255 and the poller both say "moved" / "changed", which understates it.

**Recovery works and is the argument for the screen.** The card still answered inventory with an all-zero
UID, and gen1's frames are unaddressed (flags `0x02`, no UID in the frame), so it stayed reachable:
`hf 15 csetuid -u E002222450008303` restored it **byte-identically**, raw `SYSINFO` response included.
That is only possible because the original UID was recorded — which is exactly why the app prints it.

## Finding 4 — our own capacity probe under-detects, as designed-against

The wipe cleared **58 blocks against 56 advertised**, zero failures, so 56 and 57 were reached and zeroed.
That is the sweep-above-advertised design working on gen1 silicon.

The sharper half: **our read-based capacity probe measured 56/56 on this card and under-detected by two.**
`ISO15693_POLLER_WIPE_MAX_BLOCKS` says "only a WRITE settles whether a block exists… so a read-based
capacity probe under-detects". The probe proved its own documentation right by getting it wrong. Treat
`physical_blocks` as a **lower bound** on any card whose high blocks take writes without answering reads.

## NOT settled — the latch

Whether the UID latches on power-up or changes immediately is **still unmeasured**. `SetTag15693Uid` ends
in `switch_off()`, so every read-back sits behind a field power-cycle and cannot separate the two. The
app's `NfcCommandReset`-before-verify remains justified by the model, not by observation.

To isolate it: read the UID in the **same field session** as the write. Nothing in the current tooling
does that — it needs a raw sequence in one pm3 invocation, or an app build that skips the reset.

## SESSION 2, 2026-09-11 — THE LATCH IS SETTLED, AND THE MODEL WAS WRONG

Same card, `lri2k-keychain`, identify-asserted before and after, restored byte-identically twice.
Raw frames via `hf 15 raw` with `-k` holding the field up across commands in one client session.

### Finding 5 — the UID changes IMMEDIATELY. It does not latch on power-up.

The four-frame sequence, then an INVENTORY **in the same field session, no power-cycle**:

```
hf 15 raw -ackw -d 02213E00000000   -> 01 10 1E 06      unlock  REJECTED (error 0x10)
hf 15 raw -ckw  -d 02213F69960000   -> 01 10 1E 06      commit  REJECTED (error 0x10)
hf 15 raw -ckw  -d 02213897A6B5C4   -> 00 78 F0         blk 56  OK
hf 15 raw -ckw  -d 022139D3E2F1E0   -> 00 78 F0         blk 57  OK
hf 15 raw -ck   -d 260100           -> 00 00 97 A6 B5 C4 D3 E2 F1 E0 5F 81
```

That inventory response is UID LSB-first: **`E0F1E2D3C4B5A697`, the newly written one**. A second
inventory from a fresh invocation, field dropped in between, returned the same. So A = new and
B = new: **the change is immediate, and the power-up latch does not exist on this silicon.**

**This inverts the model the app is built on.** `iso15693_poller.h` says the field is power-cycled
before the read-back "so a card that only latches the new UID after a reset is not misreported as a
failure", and the CHANGELOG says a gen1 card "latches a written UID only on the next power-up". One
sample, one chip — but on that chip the premise is false, not merely unverified.

**The `NfcCommandReset` should NOT be removed on this evidence.** It is harmless, it re-activates the
card for a clean read, and the gen2 Write-UID doc leans on the same reasoning for a UID that lives in
a different register space and was not tested here. What changes is the JUSTIFICATION, not the code.

### Finding 6 — the backdoor registers DO answer, and 2026-09-08's "no ACK" was a parse

Blocks 62 and 63 returned `01 10 1E 06` — flags `0x01` with the error bit set, error code `0x10`,
"block not available". That is an in-band refusal, not silence. Confirmed through both command forms
on the same card minutes apart:

| form | block 62 |
|---|---|
| `hf 15 raw -ackw -d 02213E00000000` | `01 10 1E 06` — answers, error 0x10 |
| `hf 15 wrbl --ua -b 62 -d 00000000` | `( fail )` |

So session 1's "an unaddressed zero write to block 62 got no ACK" was almost certainly
`pm15_wrbl_ua` reading a `( fail )` — this exact error — as an absent acknowledgement. **The claim
"these registers accept writes WITHOUT acknowledging" is not supported and should not ship.**

### Finding 7 — on an armed card the UID writes alone move it, and the refusals do not matter

Writing **only** 56/57, with no unlock and no commit:

```
hf 15 raw -ackw -d 02213898A6B5C4   -> 00 78 F0
hf 15 raw -ckw  -d 022139D3E2F1E0   -> 00 78 F0
hf 15 raw -ck   -d 260100           -> 00 00 98 A6 B5 C4 D3 E2 F1 E0 ED 30   = E0F1E2D3C4B5A698
```

The card was armed from session 1 and nothing clears that. So the arm persists across power-cycles
and across a restore, 62/63 are refused while it holds, and the UID moves on 56/57 alone.

**This is the real reason the app must discard the per-frame return values, and it is now measured
rather than assumed: both register writes were REFUSED and the sequence worked anyway.** A poller
that acted on those errors would have aborted a run that succeeded. That argument is stronger than
the one it replaces, and it does not depend on the card being silent.

### What this does to the built round

Four of its seven commits state the superseded model. The round needs rebuilding from these findings
rather than patching, since a series that asserts a thing and then retracts it is exactly the
intra-batch churn the reviewer has flagged twice.

## The card is a REUSABLE FIXTURE

It stays armed after a wipe (a wipe writes zero to block 63, not `0x6996`, so it cannot re-arm but does
not clear the arm either). So: restore the UID, run a test, restore again. The hazard on demand, which
has never been possible here before. Restore command is in the inventory entry.

Treat it as the project's most valuable tag and do not reassign its label — `--identify` matches on the
UID, so a moved UID makes it unrecognisable until restored.

## Two process lessons from the session itself

**Do not infer a fact from the absence of a mention.** The keychain was recorded as "NOT marked pm3"
because the tag description had not said "note" or "no note" for it. It *was* marked. Everything
downstream — the seller-annotation hypothesis, whether a negative gen1 result contradicted his claim —
rested on a fact invented out of silence. Ask, or record it as unknown.

**Replace a note, do not append to it.** That wrong claim survived five corrections, because each was
appended and the entry still *opened* with it — which is what `--identify` printed back, long after the
mark was confirmed. Six revisions cannot fix a false first sentence. This is the same defect as the
comment drift the round-7 cut exists to fix, in our own notes: a superseded claim left at the front reads
as current. The inventory now keeps the current state in `note` and the history in `note_history`.

## WORK LIST — for a delta AFTER the comment cut

The cut was promised as one decision with nothing else in it. None of this goes in it.

- [ ] **`.notes/NEXT-SESSION.md` harness table**: the gen1 row reads "modelled, not settled — the latch
      behaviour is our inference… Needs a gen1 card." Update to: the sequence and the armed-card hazard
      are now measured; **the latch specifically is still the inference**, so the row should narrow
      rather than flip.
- [ ] **#255**: add the hardware observation. Its "not tested on hardware" caveat is now false for the
      armed-gen1 half, and "moved" should become "left without a valid UID". The gen3 half is untouched.
- [ ] **`iso15693_poller.h`, `uid_changed`**: same wording correction — the observed outcome is an
      all-zero UID, not a different valid one.
- [ ] **`gen1_attempted`'s doc**: it says the return values are discarded "as they must be on a card that
      may not answer". That can now cite an observation rather than reading as an assumption.
- [ ] **CHANGELOG**: the gen1 wipe hazard is described as a possibility. It has been reproduced, and the
      user-visible outcome is a card with no valid UID. Worth one clause.
- [ ] **`tools/README.md`**: record that `physical_blocks` is a lower bound (Finding 4).
- [ ] **Consider a `gen1` verification test in `tools/hosttest`** — the fixture now makes the model
      checkable, though the harness is host-side and cannot drive a card.
- [x] **THE GEN3 HAZARD IS WORSE THAN WE DOCUMENT** — **DONE in Round 7, commit `c8beff5`.**
      The CHANGELOG entry is corrected and attributed, and the wipe confirm now carries
      "This can \e#brick\e# a gen3 card!" behind a "Wipe? (gen1/gen2 only)" title. #255's own
      text still carries the softer wording — editing an issue means posting, so it is not done.

- [ ] **Do NOT hold the merge for any of this.** mishamyte said explicitly not to hold for gen1 cards,
      and that still stands. This strengthens the PR's claims; it does not block them.

## SEQUENCING — this list is a B-round, so it comes BEFORE the C/D comment passes

Settled 2026-09-08. Eight of the nine items above are **comment and documentation corrections**: the
header wording, `gen1_attempted`'s doc, `uid_changed`, the CHANGELOG, `tools/README.md`, the harness
table. Only the hosttest is code. So this is not separate from the comment work — **it IS comment work,
of the B kind** (is it factually correct?), and B precedes C and D. See
[pr-round-7/comment-brevity-pass.md](pr-round-7/comment-brevity-pass.md) for the four operations.

Run C before this and C spends its effort protecting text that is about to be replaced: it would keep
`gen1_attempted`'s "as they must be on a card that may not answer" as load-bearing, and the gen1 round
then rewrites that exact sentence because it is now a measurement rather than an inference. Same for the
"NOT hardware-validated" notes on `iso15693_poller.h:87` and `:113`.

**And this round SHRINKS the surface, which is the argument for taking it first.** Replacing an inference
with an observation removes the hedging that made the inference honest: "modelled, not settled" becomes
"measured", the two NOT-hardware-validated warnings narrow to the latch alone, and the de-arming argument
at `iso15693_poller.c:852` keeps its conclusion while losing "the only order anyone has observed the
hardware accept". Rough estimate 20-40 lines out before C starts, and about a quarter of the 48 inventory
blocks stop being contingent.

One thing it ADDS: the latch is still unmeasured (see "NOT settled" above), so at least one new hedge
appears. Net still negative.

**The card is a reusable fixture, so this is not one-shot.** Recovery is byte-identical via
`hf 15 csetuid -u E002222450008303`. Restore, test, restore. That is what makes a B-round on real silicon
affordable at all, and it is why this does not need to wait for a second card.

**Still not a merge blocker.** mishamyte said not to hold for gen1 and that stands. Order is a preference,
not a constraint: PR description -> gen1 B-round -> C -> D.

## THE MECHANICAL SITE LIST — `tools/gen1-staleness.py`, run 2026-09-09

The work list above is thematic. This is every SITE, derived by scanning for the phrasings the
session invalidated. **11 genuinely stale, 7 correctly hedged, 1 false positive.** Re-run the tool
rather than trusting this list once anything moves.

**Stale — correct these:**

| site | what it says | why it is now wrong |
|---|---|---|
| `poller.h:94`, `:120` | "gen1 is NOT hardware-validated" | The UID write IS validated. Narrow to the latch. |
| `poller.h:216` | return values discarded "as they must be on a card that may not answer" | Observed, not inferred. A register accepted a write with no ACK. |
| `poller.h:236` | "a possible gen1 clone ... Source inspection only" | No longer source-only. |
| `poller.c:26` | "OUR INFERENCE from proxmark's send order, not a documented contract" | The ORDER is now observed to work; only what the registers *do* is still inference. Narrow, do not delete. |
| `poller.c:339` | "Magic cards **may** not answer these writes" | They do not answer. Measured. |
| `poller.c:856` | armed card "**can** have its UID moved by a wipe" | Reproduced, and recovered byte-identically. |
| `poller.c:859` | "the only order **anyone** has observed" | We have now observed it ourselves, on our own card. |
| `poller.c:1381` | "for the sake of a gen1 case **nobody can test**" | We can test it. This one is simply false. |
| `poller.c:1413` | "NOTE: gen1 path is not hardware-validated" | False for the UID write. |
| `partial_details.c:140` | "Filed rather than fixed: gen1, and **no card to test it**" | False. |
| `CHANGELOG.md:143` | gen1 "shipped as a faithful proxmark port, **not tested against gen1 hardware (none was available)**" | False, and it is the USER-FACING one. Found only after extending the scanner — "not tested against" and "none was available" matched none of the original patterns. |

**Correct as written — the latch is still unmeasured, so leave these alone:**
`CHANGELOG.md:85`, `CHANGELOG.md:97`, `poller.c:861`, `poller.c:867`, `poller.c:1373`,
`poller.h:74`, `poller.h:251`.

**False positive:** `poller.c:127` — "keeps that impossible" matches the `possib(le|ility)` pattern.

**Note the direction of travel:** eight of the ten replace a hedge with a measurement, which is
shorter. That is the evidence for the sequencing argument above — this round shrinks the surface
before C runs.
