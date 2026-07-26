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

Deferred: routing SLIX through the **magic_info** hub (consistency polish). Roadmap extras (AFI/DSFID
write, V3) remain in capability-matrix.md.

## Not done (needs hardware / out of scope)
See [hardware-plan.md](hardware-plan.md). Headline: the write→latch→read-back behaviour on a real
magic card (settles whether the power-cycle fix is sufficient and whether the gen1 gate fully
protects data). Also: the residual clobber-of-a-normal-tag risk when a user consents to the gen1
step — if hardware confirms it, split into explicit gen2-only / gen1 actions. Clone Phase 2/3
(block write-back, passwords) and the deeper feature gaps (privacy/EAS, AFI/DSFID, gen3/V3) remain
for the hardware session / out of scope.

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
