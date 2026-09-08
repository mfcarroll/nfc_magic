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

## The card is a REUSABLE FIXTURE

It stays armed after a wipe (a wipe writes zero to block 63, not `0x6996`, so it cannot re-arm but does
not clear the arm either). So: restore the UID, run a test, restore again. The hazard on demand, which
has never been possible here before. Restore command is in the inventory entry.

Treat it as the project's most valuable tag and do not reassign its label — `--identify` matches on the
UID, so a moved UID makes it unrecognisable until restored.

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
- [ ] **Do NOT hold the merge for any of this.** mishamyte said explicitly not to hold for gen1 cards,
      and that still stands. This strengthens the PR's claims; it does not block them.
