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

## Not done (needs hardware / out of scope)
See [hardware-plan.md](hardware-plan.md). Headline: the write→latch→read-back behaviour on a real
magic card (settles whether the power-cycle fix is sufficient and whether the gen1 gate fully
protects data). Also: the residual clobber-of-a-normal-tag risk when a user consents to the gen1
step — if hardware confirms it, split into explicit gen2-only / gen1 actions. Feature gaps
(privacy/password, EAS, AFI/DSFID/block-clone, gen3/V3, save-load) remain out of scope.

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
