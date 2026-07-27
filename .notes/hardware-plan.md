# Hardware validation plan (needs a real card)

Everything below is blocked on physical cards + a `ufbt` build. Ordered so the first test unblocks
the most. Record results inline (date / card / outcome) as they come in.

> **STATUS 2026-07-27 — core validated.** Build OK; clone (UID + data + identity IC/geometry/AFI/DSFID),
> wipe, and the honest over-capacity reporting all confirmed on a real 64-block magic card (reports 66).
> Direct proxmark facts on that card: physical = 64 (blocks 64/65 fail `rdbl` **and** `wrbl` = phantom;
> `rdbl` fails clean while `hf 15 dump` pads zeros to the reported count); card **rejects READ MULTIPLE
> 0x23**. See worklog.md "CURRENT STATUS & IMMEDIATE NEXT STEPS" for what's next (re-clone the test
> card, capacity re-run, impersonation sweep with tools/test_nfc/, try the real reader).

## 0. Build + smoke test (needs toolchain, not a card)
- [x] Build of `nfc_magic_dev` — DONE 2026-07-26 via `cd ../Momentum-Firmware && FBT_NO_SYNC=1
      ./fbt fap_nfc_magic_dev`. Clean compile (`-Werror`, no warnings), links, `APPCHK` passes,
      `nfc_magic_dev.fap` ~134 KB. (`ufbt` not needed; app is symlinked into `applications_user/`.)
- [ ] Deploy to a Flipper (`./fbt launch APPSRC=applications_user/nfc_magic_dev`, or copy the .fap)
      and open **Check Magic Tag** with no card → no crash, popup behaves, detect times out to menu.
- [ ] Confirm the new SLIX **Info-mode timeout** returns to the menu (no infinite hang) with no card
      present. Tune `SLIX_POLLER_MAX_ACTIVATION_ERRORS` in `slix_poller.c` if the timeout feels
      too fast/slow.

## 1. THE blocker: write → latch → read-back on a genuine magic card
This single result settles findings #2 and #3 together.
- [ ] With a known **magic gen2** ISO15693 card: enter a new UID, write, confirm **Success**.
      Re-scan via **Info** → UID matches. If the same-session verify reports Fail but a re-scan shows
      the new UID, the card needs the power-cycle → confirm the `NfcCommandReset`-before-verify fix
      is doing its job (it should already be).
- [ ] With a known **magic gen1** card: same test. gen1 runs only after gen2 is a no-op — confirm
      that path reaches it and succeeds.
- [ ] Cross-check the written UID against proxmark `hf 15 info` if a PM3 is available.

## 2. Confirm the gen1 clobber gate actually protects data
- [ ] On a genuine **gen2** card holding known data in blocks 56/57/62/63: do a write that succeeds
      via gen2. Read those blocks (proxmark `hf 15 rdbl`) → data intact (gen1 never ran).
- [ ] On a **non-magic but writable** NfcV tag (e.g. plain ICODE with those blocks): attempt a
      write. Expected: gen2 no-op → gen1 runs (user consented at the confirm screen) → verify Fail →
      "not a magic tag" message. **Check whether blocks 56/57/62/63 were altered.** If they were,
      that confirms the residual clobber risk and argues for making gen1 an explicit separate action
      (see "possible follow-ups").

## 3. Verify the "not a magic card" UX end-to-end
- [ ] Tap a plain non-magic ISO15693 tag, go through Write UID → confirm → expect the dedicated
      "Not a magic tag / UID write unsupported" fail scene, **not** the generic write-fail, and no
      infinite Retry.
- [ ] Remove the card mid-write → expect the "card removed" fail reason.

## 4. Chip identification spot-checks (finding #5)
- [ ] Scan SLI, SLIX, and SLIX2 cards → Info shows the distinct model for each (was collapsed
      before). Compare with proxmark `hf 15 info`.

## 5. Timing / robustness
- [ ] Confirm `ISO15693_3_FDT_WRITE_POLL_FC` (~20 ms FDT) is adequate; proxmark additionally spaces
      frames by `DELAY_ISO15693_VICC_TO_VCD_READER` — if writes are flaky, add inter-frame spacing.
- [ ] Confirm the gen2 CFG geometry (`3F/03/8B`) matches the user's actual gen2 cards; a mismatch
      could fail the gen2 write and (pre-gate) would have triggered the gen1 clobber.

## 6. Clone Phase 1 — read / display / save (built offline; verify on hardware)
- [ ] **Block display** — scan a card via Info → the "Blocks (N x M)" list matches the card's real
      memory (cross-check with proxmark `hf 15 dump`); locked blocks show `*`.
- [ ] **Save round-trip** — SLIX menu → "Save to file" → name it → confirm a `.nfc` is written under
      `nfc/`. Then **load it in the stock NFC app** and confirm it reads back as an ISO15693 card with
      the same UID / blocks. (Saved as `NfcProtocolIso15693_3`; the NFC app may re-detect it as SLIX.)
- [ ] Edge cases: a card that doesn't report memory geometry (no block list, save still works with
      UID + system info); a large (64-block) card (long scroll, save size OK).

## 7. Full clone from a saved .nfc (built; the main flow to validate)
The SLIX feature now matches the other magic types: Check → SLIX menu → **Write** → FileSelect (pick
a saved ISO15693 `.nfc`) → Confirm → clone (data blocks via WRITE BLOCK, then UID via the backdoor).
- [ ] **Capture a source**: read a SLIX/ISO15693 card with the **stock** NFC app → Save to `.nfc`.
- [ ] **Clone**: NFC Magic → Check Magic Tag → tap the **magic target** → Write → pick the saved
      `.nfc` → Confirm → expect **Success** (or **Partial** with an N/M block count).
- [ ] **Verify**: re-read the target with the stock app → UID + block data match the source
      (cross-check with proxmark `hf 15 dump` if available).
- [ ] **Partial path**: on a target with locked/short memory, confirm the "Clone partial — N of M
      blocks" screen is accurate (locked blocks are skipped, not counted as failures).
- [ ] **gen1-on-large-card caveat**: if a ≥64-block card falls to the gen1 UID fallback, blocks
      56/57/62/63 may hold UID/commit bytes (gen1 backdoor overlaps those addresses). Check whether
      this happens in practice; if so, prefer gen2-only or reorder.
- [ ] **Manual UID** (bonus, unchanged) and **Info** still work.
- [ ] **Capacity cap** (verified 2026-07-26): a 64-block target cloned from a source the Flipper
      over-read as 66 now reports "2 block(s) beyond the 64-block target" instead of a failure.
- [ ] **Wipe**: SLIX menu → Wipe → confirm → expect all data blocks zeroed (re-read to confirm),
      UID unchanged. Partial names any block that wouldn't zero.

## Possible follow-ups (decide after hardware)
- If step 2 shows normal tags get clobbered by the consented gen1 step, split the flow into explicit
  **gen2-only (safe)** and **gen1 (destructive)** actions instead of an auto-fallback — matches
  proxmark's model.
- Add gen3/V3 support (blocks `0x10/0x11` + finalize).
- Move onto the SDK `slix` protocol to unlock privacy/password/EAS/AFI/DSFID/block-clone/save-load.
