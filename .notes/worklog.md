# Worklog — offline hardening pass

Branch `slix-v2`. Started 2026-07-26. All changes are static-review-only (no `ufbt` build here);
each is matched to the SDK API + existing patterns by inspection. On-hardware validation is tracked
in [hardware-plan.md](hardware-plan.md).

## Planned commits (offline)
1. docs: add `.notes/` (analysis, protocol reference, hardware plan, worklog).
2. #6 generic "ISO15693 / NfcV" labels.
3. #4 enforce entered UID starts with `0xE0`.
4. #5 SLI / SLIX / SLIX2 chip decode.
5. #2 + #3 + #1(poller) rewrite the write path: power-cycle-before-verify state machine,
   gated gen1 fallback, distinct Success / Fail / CardLost outcomes; #7 Info-mode timeout.
6. #1(UI) SLIX-specific write-fail scene (not-magic vs card-lost).
7. groundwork: `slix_write_confirm` scene before the irreversible write.
8. #8 cleanup: portability `#define`, stale header docs, dead enum removal.
9. docs: update this worklog + CHANGELOG.

## Done (offline pass complete)

Grouped into buildable commits (each keeps the app self-consistent):

- [x] **docs** — `.notes/` added (analysis, protocol reference, hardware plan, worklog).
- [x] **SLIX info** (#5, #6) — generic "ISO15693 / NfcV" labels; new `slix_info_get_chip_info_ex`
      decodes SLI / SLIX / SLIX2 from the `uid[3]` type-indicator bits (mask `0x18`: `0x10`=SLIX,
      `0x08`=SLIX2), matching the SDK `slix_get_type` / proxmark masks.
      Files: `scenes/nfc_magic_scene_slix.c`, `scenes/nfc_magic_scene_slix_info.c`,
      `magic/protocols/slix/slix_info.{c,h}`.
- [x] **SLIX poller core** (#2, #3, #7, #8) — rewrote the write path as a 3-state machine
      (`Start → VerifyGen2 → VerifyGen1`): reads the original UID, sends gen2, returns
      `NfcCommandReset` to power-cycle, verifies; falls back to gen1 **only if the UID is unchanged**;
      each verify runs on a freshly re-powered field. Added an activation-error **timeout**
      (`SLIX_POLLER_MAX_ACTIVATION_ERRORS`, ~5-7 s) emitting `CardLost`. Added the portability
      `#ifndef ISO15693_3_FDT_WRITE_POLL_FC` fallback, refreshed the header docs, removed the dead
      `SlixPollerError` enum, and `SlixPollerEventCardLost` is now used.
      Files: `magic/protocols/slix/slix_poller.{c,h}`.
- [x] **SLIX write UX** (#1, #4, groundwork) — `slix_write_input` forces `uid[0]=0xE0` and routes to a
      new **`slix_write_confirm`** scene (shows the UID + destructive-gen1 warning); the write scene
      maps the three poller outcomes to a new **`slix_write_fail`** scene with a reason
      (not-magic vs card-lost), no infinite Retry.
      Files: `scenes/nfc_magic_scene_slix_write_input.c`, `nfc_magic_scene_slix_write.c`,
      new `..._slix_write_confirm.c`, new `..._slix_write_fail.c`,
      `scenes/nfc_magic_scene_config.h`, `nfc_magic_app_i.h` (reason enum).
- [x] **docs** — CHANGELOG "Unreleased — SLIX" section; this worklog.

## Post-review follow-ups (offline, verified by build)
An adversarial diff review (compile / logic / flow) returned **GO** — no build-breakers, no
correctness bugs (the clean `-Werror` build confirms the compile dimension empirically). Two `LOW`
robustness/UX items it raised were worth fixing offline because they de-risk the hardware test:

- **Verify inventory retry** (`slix_poller.c`) — the read-back after the field power-cycle now
  retries up to `SLIX_POLLER_VERIFY_ATTEMPTS` (3) with a short delay, so a card momentarily slow to
  answer post-reset isn't misreported as `CardLost` on an otherwise-successful write.
- **Confirm-screen layout** (`slix_write_confirm.c`) — the 8 spaced UID bytes always overflowed
  128px and wrapped, pushing the data-loss warning to a 5th line the box dropped. Now the UID is a
  compact two-group line and the warning is three short lines, so nothing clips.

The one remaining review note is INFO/by-design: the gated gen1 fallback can still reach a gen2 card
whose gen2 write silently failed (readback == original) — documented, user-gated at the confirm
screen, and covered by hardware-plan.md step 2.

## Clone — Phase 1 (offline, the read/dump half)
Per [clone-feasibility.md](clone-feasibility.md). Both build clean under `-Werror`; behaviour still
needs on-hardware confirmation (hardware-plan.md).

- [x] **Phase 1a — block display** (`fa51633`) — the Info screen now lists the full block data the
      poller already reads during activation (scrollable, `*` marks a locked block). Read-only
      display of data we already hold; no protocol change.
- [x] **Phase 1b — save to `.nfc`** (`a8bb65a`) — new "Save to file" SLIX menu item reads the card
      (get-info scene branched on a read-intent scene state) → `nfc_device_set_data(source_dev,
      NfcProtocolIso15693_3, ...)` → `nfc_device_save()` via a name-input scene mirroring
      `gen1_save_name`. Produces a plain ISO15693-3 `.nfc` (UID + system info + blocks). File
      round-trip (does it re-load correctly?) is a hardware-plan item.

**Deliberately NOT built offline: Phase 2 (write-back clone).** Writing data blocks is destructive
(standard `WRITE BLOCK` to real user blocks) and correctness can't be verified without a card, so
building it blind risks shipping card-corrupting logic. Left for the hardware session — the code
shape is straightforward (`iso15693_3_poller_write_block(s)` + per-block partial reporting like the
Gen2/USCUID paths), but it must be written and tested with a real tag in hand. Phase 3 (passwords)
likewise needs hardware.

## Genericness pass (offline) — the feature is generic ISO15693, not SLIX
A multi-agent audit (app / ISO15693 landscape / magic variants) confirmed the read/info/save/UID-write
pipeline is **already generic ISO15693** — it runs entirely on the SDK's `NfcProtocolIso15693_3` layer
and never sends an NXP-specific command. "SLIX" was a naming legacy. Actioned the safe offline items:

- [x] **Primer** (`1abfaa3`) — `.notes/iso15693-primer.md`: SLIX vs ISO15693, chip families, standard
      vs custom commands, magic variants, and how each app feature maps.
- [x] **De-SLIX user-facing strings** (`12cfb96`) — app title → "NFC Magic ISO15693", detect popup →
      "Detecting ISO15693", protocol name → "ISO15693 / NfcV". Internal `slix_*` symbols left as-is
      (cosmetic; rename opportunistically, not a dedicated pass).
- [x] **Broader chip decode** (`fce00c3`) — ST ST25TV, EM4425, NXP ICODE 3 (the last previously
      mis-decoded as SLI). Static-table edit, verified against proxmark's uidmapping.

Deferred/hardware-gated from the audit: full symbol rename (churn, low value); block-data write-back
and the V3 variant (draftable but need a card) — technical specifics captured in
[clone-feasibility.md](clone-feasibility.md).

## Full clone integration (matches the app's model)
Confirmed the app's model: it's a **writer/cloner** — Check → detected → per-type menu → Write →
FileSelect (a saved `.nfc` from the stock app) → write to the card. Not a reader/emulator (that's the
stock app). Integrated SLIX the same way (builds clean; on-hardware test = hardware-plan.md §7):

- [x] **Clone engine** (`fc9ed08`) — `slix_poller_start_clone(source)`: writes writable data blocks
      (WRITE BLOCK, skip locked, count failures) then the UID backdoor; Success/Partial/Fail/CardLost.
- [x] **Shared write flow** (`d3a0a91`) — SLIX branch in the shared `Write` dispatcher clones from
      `source_dev`; FileSelect accepts an ISO15693 `.nfc`; SLIX menu = Write (file) / Write UID
      (manual) / Info. Retired the off-model "Save to file" + its scaffolding.

- [x] **Capacity-aware clone + Wipe** — the clone caps writes at the target's block count and reports
      "N beyond target" distinctly from real failures (fixes the "2 of 66" on a card whose Flipper
      read over-reported the block count). **Wipe** (zero every writable block, UID untouched, like
      proxmark `hf 15 wipe`) added as a SLIX menu item.

- [x] **magic_info hub routing** — a detected ISO15693 tag now goes through the shared "Magic card
      detected" screen (More → SLIX menu), matching the other types.

### Field validation (2026-07-26, real hardware)
Clone and Wipe both confirmed working: cloned an EM ISO15693 source onto a 64-block magic target
(UID + data blocks verified), wipe zeroed all blocks (re-read = all zeros).

Resolved the "66 vs 64 blocks" puzzle — **not a Flipper bug.** The SDK computes `block_count = byte + 1`
(spec-correct). Two different cards had been conflated: the *original* is an EM Microelectronic tag
(uid `E0 16 3C…`, IC ref `0F`) that genuinely reports **66 blocks** with data only in blocks 0-1; the
proxmark JSON (IC `8B`, 64 blocks, gen1 backdoor artifacts in 56/57/63) was the *magic target*. So the
"2 beyond capacity" tail is real 66→64 shortfall, and those 2 blocks are zeros — clone is faithful.
Caveat on record: the gen2 clone stamps IC ref `8B` / 64-block geometry (fixed CFG), so a clone won't
match a reader that checks IC ref or exact block count (see capability-matrix H3).

- [x] **Identity clone** (`b2f7440`) — a clone now advertises the source's chip identity, not the fixed
      magic default: gen2 CFG carries the source's IC ref + block count/size (the card parrots these in
      Get System Info), and AFI/DSFID are set via standard WRITE AFI (0x27) / WRITE DSFID (0x29).
      Best-effort (UID + data are the core); gen1 fallback has no geometry block; a source larger than
      the target's physical memory is still reported as "N beyond capacity". This is the fake-flash
      trick — the card can report a bigger/different size than it physically has.

Field-verified 2026-07-27 (proxmark, on the identity-cloned 64-block card that now reports 66): IC ref
/ geometry / AFI / DSFID / UID / data all match the source byte-for-byte. Blocks 64/65 are **phantom**
— reads FAIL (no response) and writes FAIL; they do NOT return zeros. (An earlier note wrongly said
"read back as zeros" — that was the Flipper zero-filling an unreadable block in its `.nfc`, not the
card answering.) No aliasing: writing block 64 left block 0 intact. So over-reporting a larger size
works for any reader that doesn't deep-read the phantom tail; a reader that reads blocks 64/65 gets a
failure on the clone (and likely on the original too, if it is also over-reporting).

Roadmap extras (repeatable V3 magic; full field-by-field mismatch verification after write) remain in
capability-matrix.md.

## CURRENT STATUS & IMMEDIATE NEXT STEPS (2026-07-27)

**The SLIX/ISO15693 feature is complete and hardware-validated for the core use case.** Works exactly
like the app's other magic types, builds clean under `-Werror`, and a byte-for-byte identity clone was
confirmed on a real card:
- Detect → "Magic card detected: ISO15693 / NfcV" → More → menu (Write / Wipe / Write UID / Info).
- **Clone from a saved `.nfc`** (UID + data blocks + **identity**: IC ref / geometry / AFI / DSFID) —
  confirmed byte-identical to the original via proxmark `hf 15 info`/dump.
- **Wipe** (zero all blocks, UID untouched) — confirmed (re-read = all zeros).
- Write policy (updated 2026-07-27): **attempts every source block** (writes follow physical memory,
  not the advertised count — see writespan below); reports only *real* data loss. Empty blocks that
  won't fit → clean Success; non-empty blocks that won't fit → Partial naming the blocks.

**Direct proxmark validation (2026-07-27), the 64-block target that reports 66:**
- Physical = **64**, confirmed definitively: block 63 reads, blocks **64/65 fail BOTH `rdbl` and
  `wrbl`** → genuinely phantom (not empty-real). Physical clone of blocks 0-63 + UID + identity is
  faithful; the 2 phantom blocks were empty on the source so nothing is lost.
- **`rdbl` fails clean on a phantom block; `hf 15 dump` PADS zeros up to the reported count** (masks
  the boundary) — so the probe harness uses retried `rdbl` binary-search, not dump, for capacity.
- This card **does not support READ MULTIPLE (0x23)** ("command not supported") — single READ BLOCK
  (0x20) only. Worth recording per card.
- Note: gen1's UID write stamps blocks 0x38/0x39/0x3E/0x3F (56/57/62/63); a source with real data in
  those clones faithfully only via **gen2** (which uses separate config refs). Our card takes gen2.

### Harness work (2026-07-27, this session)
Two test-rig goals: (1) assess the magic cards, (2) write configs and ground-truth the app on-device.

- **Explained the "impersonate → CLAMPED to 64" confusion** — it was a *probe artifact*, not a card
  clamp. The `magictype` gen2 `csetuid` test (when it succeeds) rewrites the CFG block to proxmark's
  default geometry (64 blk / IC 0x8B) as a side effect, on both the test write *and* the UID-restore.
  So `impersonate` then read 64/0x8B as its baseline (contradicting `info`'s 66/0x0F seconds earlier)
  and mislabelled every *ignored* standalone-CFG write as a clamp. The card CAN advertise 66/0x0F —
  the app clone (full UID-write sequence) sets it; standalone CFG frames are simply ignored by it.
- **Fixed the probe** (commit `3e17e2f`): `remember_geometry()` snapshots run-start geometry before any
  destructive probe; `magictype` reports the CFG side-effect honestly and says to re-clone if it can't
  restore; `impersonate` distinguishes canonical vs live geometry and drops the bogus "CLAMPED" verdict.
- **Built the Flipper ground-truth harness** (commit `1806408`) — `flipper_ground_truth.py` +
  `flipper_bridge.py`. Uploads a source `.nfc` over the Flipper CLI, prompts the on-device NFC Magic
  Write + stock-app Read/Save, auto-detects the read-back by diffing `nfc/`, downloads it, and compares
  source vs read-back (identity, block-by-block, over-capacity). Optional pm3 second read. Needs
  pyserial → runs under `tools/.venv` (gitignored). Verdict logic verified offline; the on-device
  write/read is the only manual step. See `tools/README.md`.
- **Card state note:** the probe's destructive runs left both test cards reporting **64 blk / IC 0x8B**
  (proxmark default) with block 63 = `69 96 00 00` — re-clone from `.nfc` to restore the 66/0x0F identity.

### Ground-truth sweep result + the write-cap decision (2026-07-27)
On-device sweep of `tools/test_nfc/slixtest_*.nfc`: **5/6 byte-identical PASS** (SLIX-28 / LRi2K-56 /
Tag-it-64 / edgedata_64 / oversize_empty_70 — the physically-64 card impersonated 28/56/64/**70** blocks
on demand, across NXP/ST/TI ICs). `oversize_edgedata_70` showed 8 block mismatches — traced to a
**test-ordering artifact**: the app capped writes at the target's *currently-advertised* count (56, left
by the prior lri2k clone), so blocks 56-63 kept stale data from an earlier clone and 64-69 were phantom.
Proof it's ordering: `oversize_empty_70`, written next when the card already advertised 70, PASSED.

- **`writespan` probe (new)** settled the cap question on hardware: with the card advertising **28** but
  physically larger, WRITE BLOCK to blocks **29-34 all succeeded** → **writes are gated by physical
  memory, not the advertised count.** Bonus finding: high blocks don't even *read* until *written* (a
  pre-write read under-reports capacity) — so the only reliable capacity test is to write and check.
- **App change (commits `39b5586`, `c1f9d33`)** — per the goal "write the exact card; tell the user only
  when data truly can't fit": `slix_poller_write_source_blocks` now **attempts every source block** (no
  advertised-count cap). A block that won't take is classified by whether the source had data there —
  **non-empty → Partial** (real loss, named blocks); **empty → stays Success** (card reports it as zero,
  clone still matches). This also removes the stale-tail-on-reuse bug, and turns the old "Clone partial:
  2 of 66 blocks" (empty over-read) into a correct **byte-identical Success**. Fail-scene reworded to
  "N block(s) didn't fit the card: <list>". Builds clean; FAP links, APPCHK passes.
- **VALIDATED on-device 2026-07-27** (rebuilt FAP, full ground-truth sweep, dual-reader confirmation):
  5/6 **byte-identical PASS**; the physically-64 card advertised 28/56/64/**70** blocks across NXP/ST/TI
  ICs, confirmed by BOTH the stock NFC read-back AND the `--pm3-crosscheck` proxmark read (UID +
  block_count + IC all matched the source every time). `oversize_edgedata_70` → **PARTIAL with exactly 6
  `not-written` blocks (64-69), NO `stale`** — blocks 56-63 are now written, so the reuse-stale bug is
  gone; on-device the app showed "Clone partial / UID + data cloned. / 6 block(s) didn't fit the card:
  64 65 66 67 68 69". The same card was reused across all 6 writes with no stale confound.
- **Build hygiene** (`39b5586`): the FAP manifest now scopes `sources=["*.c*", "!tools"]` — the default
  recursive `*.c*` was sweeping in `tools/` (venv + `__pycache__` `.cpython-*.pyc` match `*.c*`), which
  broke the link. `tools/` is dev-only; excluded like the firmware excludes `lib/`.

### gen1 backdoor-block warning + honest gen1 reporting (`c9d3bfd`, 2026-07-27)
The gen1 UID fallback writes the UID/unlock/commit into **data blocks 56/57/62/63** (`0x38/0x39/0x3E/
0x3F`), so a clone that falls back to gen1 can't reproduce a source that stores data there. We surveyed
how the app's other magic types handle "might write to a wrong/non-magic card": gen1a/gen4/USCUID-UL-
backdoor confirm magic non-destructively first; gen2/Classic and USCUID-UL-direct are "write-to-confirm"
(standard writes land on any writable same-family card, behind the confirm) — SLIX is in that second
group, and its clone already writes all data via standard `0x21` before the backdoor. We looked at #5
(a non-destructive gen2 probe): **not available** — proxmark's gen2 (`SetTag15693Uid_v2`) is write-only,
ignores responses, and confirms by write-then-verify; only V3 has a read-based detector. A theoretical
"idempotent gen2 write + inspect ACK" probe is unproven and we have no non-magic ISO15693 card to test
it, so we did **not** rely on it. Chosen fix (lean, low-risk, keeps the auto-fallback behind the confirm):
- **Pre-write:** `slix_poller_source_uses_gen1_blocks()` — if the source has data in 56/57/62/63, the
  write-confirm shows a **specific** warning ("If this card is gen1 (not gen2), gen1 puts the UID in
  those blocks, so they can't be cloned"). Shown only when actually relevant.
- **Post-write:** track gen2-vs-gen1 (`clone_used_gen1`). A **clone that used gen1 → Partial**, and the
  result screen flags "blocks 56/57/62/63 hold the UID, not your file's data." A bare **Write-UID via
  gen1 stays a clean Success** (no source data to disturb).
- ⚠️ **The gen1 path is NOT hardware-validated** — only a gen2 magic card is on hand; no gen1 or non-magic
  ISO15693 tag to test with. Flagged in code comments (`slix_poller.c`) and **must be called out in the
  upstream PR**. Deferred: the just-in-time "pause before gen1 fallback" confirm (needs a gen1 card to
  test the flow) and buying a non-magic/gen1 ISO15693 tag to validate.

### Immediate next steps (in priority order)
The write-every-block change is validated (above) — the SLIX/ISO15693 clone feature is now
functionally complete and hardware-proven end-to-end. Remaining:
1. **Test the real access-pass reader** with a clone of the actual pass (does the door open? — settles
   whether the reader is UID-only or checks block data / an anti-clone signature). This is the last
   real-world unknown for the original use case.
2. **Re-clone both test cards** from their `.nfc` — the probe runs left them at 28/56 geometry and 64/0x8B.
3. **Upstream polish** (if pursuing a PR): builds clean and behaves correctly; a pass for
   naming/comments/consistency-with-other-magic-types. **PR must state the gen1 path is untested on
   hardware** (only gen2 validated). Decide scope (keep V3 out; the just-in-time gen1 pause deferred).
   See capability-matrix.md for what's intentionally deferred.
4. **Buy a non-magic and/or gen1 ISO15693 tag** to (a) validate the gen1 write path and (b) test the
   `0xE0`-is-inert-on-non-magic assumption — which would unlock the confirm-before-clobber reorder.
5. Optional harness add: an `info`-probe check for READ MULTIPLE (0x23) support (a real per-card trait).

### Deferred / out of scope (unchanged)
Repeatable **V3** magic variant (blocks 0x10/0x11 + finalize; needs a V3 card — screen with
`hf 15 rdbl -* -b 20/21` for sig `A5 2B 44 2C`/`21 AE 93 00`), full field-by-field mismatch report
after write, and the SDK-`slix`-only features (privacy/password, EAS, signature). See
capability-matrix.md.

## Build note
**Builds clean.** `ufbt` isn't installed, but the app is symlinked into `applications_user/` of the
local firmware checkouts, so it builds with that tree's `fbt`:

```
cd ../Momentum-Firmware && FBT_NO_SYNC=1 ./fbt fap_nfc_magic_dev
```

2026-07-26: full build OK (exit 0), all SLIX translation units compile with **no warnings** under
`-Werror`, links, and `APPCHK` passes → `build/f7-firmware-C/.extapps/nfc_magic_dev.fap` (~134 KB).
(`FBT_NO_SYNC=1` skips the submodule sync so it works offline; the toolchain was already downloaded.)

Correction to earlier finding #8: `ISO15693_3_FDT_WRITE_POLL_FC` is **not** fork-only — it exists in
stock Momentum-Firmware too (the two SDKs' iso15693_3 dirs are identical). The `#ifndef` fallback is
harmless (never triggers against a Momentum SDK) and still useful for a hypothetical SDK that lacks
the macro, so it stays. On-hardware validation (hardware-plan.md) is the only remaining step.
