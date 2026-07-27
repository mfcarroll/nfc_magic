# Changelog

## Unreleased — SLIX (magic ISO15693) `iso15693-v2`

Adds magic **ISO15693 / SLIX (NfcV)** support: detect an ISO15693 tag, show Info (UID /
manufacturer / chip / system info), and perform a magic **backdoor UID write** (gen1 or gen2),
verified by read-back. The write frames are a byte-for-byte port of proxmark3's `SetTag15693Uid` /
`SetTag15693Uid_v2`. This is a **UID-only writer**, not a full card cloner. See `.notes/` for the
analysis, the byte-level protocol reference, and the on-hardware validation plan.

### Added
- **SLIX Info** — UID, manufacturer, chip type, GET SYSTEM INFO (memory / DSFID / AFI / IC ref), and
  the **full block data** the poller reads during activation (scrollable, `*` marks a locked block).
  Chip decode now tells **SLI / SLIX / SLIX2** (and the -S / -L variants) apart via the UID
  type-indicator bits, matching the SDK's `iso15693_get_type` and proxmark's UID table.
- **SLIX Save to file** — read a card and save it to a plain ISO15693-3 `.nfc` (UID + system info +
  blocks); the read/dump half of a full clone.
- **SLIX Write UID** — magic backdoor UID write with a **confirmation screen** (shows the new UID and
  warns that the gen1 step can overwrite data on a non-magic tag) before the irreversible write.

### Changed / hardened
- The write **verifies after an RF field power-cycle** (`NfcCommandReset`), like proxmark's
  `switch_off()` + `getUID`, so a card that only latches the new UID after a reset is not misreported
  as a failure.
- The destructive **gen1 fallback only runs if the gen2 write left the card's UID unchanged**, so a
  partially-written gen2 card is not clobbered.
- The entered UID is **forced to start with `0xE0`** (a valid ISO15693 UID) before writing.
- Detect / Info / write popups now **time out** instead of hanging forever when no card is present.
- Scenes are labelled generically **"ISO15693 / NfcV"** (a non-NXP or non-magic tag is no longer
  mislabelled "SLIX").
- Harmless `#ifndef` fallback for `ISO15693_3_FDT_WRITE_POLL_FC` as belt-and-braces for an SDK that
  might lack it (it is present in the Momentum SDKs this app targets).

### Fixed
- A non-magic ISO15693 tag (the common case) now shows a dedicated **"Not a magic tag"** message
  instead of a generic write error with a Retry that looped forever; a removed card shows a distinct
  **"Card removed"** message.

## 2.0

Major release. Adds magic **Ultralight / NTAG (USCUID-UL)** support, and reworks the magic
**MIFARE Classic (Gen2)** wipe & clone with honest **Success / Partial / Fail** reporting.

### Added

- **Magic Ultralight / NTAG (USCUID-UL) support** — the app can now write, clone and wipe magic
  Ultralight-family tags, not just MIFARE Classic:
  - **Write / full clone** with the transport auto-selected from detection — **direct** (CUID/ATS,
    ISO14443-3 + `A2`) or **backdoor** (raw wakeup). ACK-only, continue-on-fail; the UID page is
    written last; live "Writing X/N" progress; **Partial Write** with a Details list of the pages
    that didn't take (and resume if the card is briefly removed mid-write).
  - **Wipe** to factory default.
  - **Auth with Password (PWD-AUTH)** to write protected/locked tags (single attempt, so it can't
    burn `AUTHLIM`).
  - **"Write anyway"** for tags that don't confirm as magic.
  - **Detection** of UL11 / UL21 (incl. Mikron "Ultra"), NTAG213/215/216, UL-C, UL-5, and
    backdoor-mode tags (both wakeup sequences), with a raw-config view for unrecognised presets;
    family-first scanner.
- **Gen2 / MFC wipe & clone now report Success / Partial / Fail** — a **Partial** screen plus a
  **Details** list of the exact blocks that couldn't be written/wiped (it was previously always
  "Success").
- **"No keys found"** screen when a wipe has no usable keys (instead of a doomed write-check).
- **Exit → menu** button on the Wipe / Write / Dump failure screens.

### Changed

- Gen2 / MFC wipe & clone outcomes are tracked **per block** (counts + the not-done block list).
- A wiped **block 0** preserves the card's real **SAK/ATQA** (a 1K mis-detected as 4K is no longer
  stamped 4K) and zeroes the UID.
- **Write-check warnings** reworked: one shared Gen2/Classic handler and consistent
  **Back / Next / Skip** buttons.
- Clearer rejection message for a cross-family wrong dump.

### Fixed

- **False "Success"** on a Gen2/MFC **wipe** with missing keys, and on a **clone** that could write
  nothing (unknown keys, or a read-only block 0).
- **Stale-key carry-over** between operations — the dictionary attack now re-reads the card fresh
  each time instead of replaying the previous tag's keys.
- **Per-sector → per-block** failure tracking — a read-only block 0 no longer reports the whole of
  sector 0 as failed.
- Dead **"Next"** button on a write-check warning whose first item wasn't the UID warning.
- Gen2 write-check **"Back"** did nothing (it targeted the wrong menu); the misleading **"Retry"**
  label → **"Back"** (the button always navigates back).
- **KeysDict** memory leak when backing out of a write-check.
- The wipe-failure screen could show **"No keys found"** for an unrelated failure (uninitialised
  scene state).

## 1.12

### Added

- **Gen2 CUID / ATS + static-nonce detection** — classify magic MIFARE Classic sub-types (direct
  CUID, ATS-fingerprinted, and CUID with a static nonce).
- **Gen1 4- and 7-byte UID** handling, including writing a 7-byte MIFARE Classic dump to a Gen1 tag.
- **Length-aware wipe & write guards** — validate block counts before writing.

### Changed

- Updated for new firmware API / SDK.

### Fixed

- Gen4 max block number for the NTAG protocol.

## 1.11

### Changed

- Description / metadata update.

### Fixed

- Gen4 poller fix.

## 1.10

- Upstream sync and maintenance.

## 1.9

### Changed

- Gen4 sync.

### Fixed

- Minor UI fix.

## 1.8

### Added

- **Gen4 (UMC / GTU) magic-card support.**

## 1.7

### Changed

- Reverted not-release-ready Gen2 changes (Gen4 still pending).

### Fixed

- UI and newline fixes.

## 1.6

### Changed

- Incremental updates; Gen2 preparation added, then reverted as not release-ready.

## 1.5

### Changed

- Reworked Back-button event handling; GUI cleanup.

### Fixed

- Incorrect total-block usage (#102); ufbt build compatibility.

## 1.4

- Maintenance / upstream sync.

## 1.3

### Fixed

- New-API compatibility fixes.

## 1.1

- Early release updates; new-API compatibility pass.

## 1.0

### Added

- Initial release in the plugin pack: write & wipe magic cards (**Gen1a**, **Gen2**) with
  block-0 / UID editing.
