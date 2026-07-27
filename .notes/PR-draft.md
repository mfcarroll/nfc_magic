# Draft PR — Add magic ISO15693 (NfcV) support to nfc_magic

> This is the draft body for the upstream PR. The PR is built on branch **`iso15693-pr`**
> (3 commits off the v2.0 base `85a6532`). See "How to open the PR" at the bottom.

---

## Summary

Adds a new magic tag family to `nfc_magic`: **ISO15693 / NfcV**. It follows the same
Check → menu → Write-from-`.nfc` model as the app's existing magic types (MIFARE Gen1/2/4,
USCUID-UL), so ISO15693 magic cards can be **cloned, written, and wiped** from a saved dump.

The magic write frames are a byte-for-byte port of proxmark3's `SetTag15693Uid` (gen1) and
`SetTag15693Uid_v2` (gen2), verified by a power-cycled read-back.

## What's added

- **Detection** — any ISO15693 tag that activates is treated as a magic *candidate* and routed to a
  dedicated menu (Write / Wipe / Write UID / Info). Detection is non-destructive; magic is confirmed
  by the write (the same "write to confirm" model the app already uses for Classic / USCUID-UL).
- **Info** — UID, manufacturer, chip type (NXP **SLI / SLIX / SLIX2** and -S/-L variants),
  GET SYSTEM INFO (memory / DSFID / AFI / IC ref), and the full block data (scrollable, `*` = locked).
- **Clone from a saved `.nfc`** — writes UID (magic backdoor) + all data blocks + identity
  (IC ref / block geometry / AFI / DSFID) so the copy advertises the same chip.
- **Wipe** (zero data blocks, UID untouched) and **manual Write UID**.

## Design decisions

- **Built on the SDK's raw `iso15693_3` layer** (not the richer `slix` protocol). The magic backdoor
  is not part of the SDK, so the poller is a small state machine issuing raw frames, mirroring how the
  other magic pollers wrap their SDK protocols. A thin `iso15693_data` wraps `Iso15693_3Data`.
- **gen2 first, gen1 fallback.** gen2 sets UID + geometry via the `0xE0` config command (which a
  non-magic tag ignores). gen1 (WRITE BLOCK backdoor) runs **only if gen2 left the UID unchanged**,
  and is gated behind the write confirmation — a gen2 card is never touched by gen1.
- **Write every source block; report only real data loss.** On these cards `WRITE BLOCK` is gated by
  physical memory, not the advertised block count (confirmed on hardware — writes succeed past the
  reported count, and high blocks don't even read until written). So the clone attempts every block:
  a **non-empty** block that won't write → **Partial** (named); an **empty** block past the card's real
  capacity loses nothing (the card reports it as zero) → clean **Success**. A card that advertises a
  larger geometry than it physically holds (fake-flash) therefore clones faithfully for the blocks that
  fit, and honestly reports what didn't.
- **gen1 fidelity is surfaced.** The gen1 backdoor stores the UID/commit in *data* blocks 56/57/62/63,
  so a gen1 clone can't reproduce a source that uses them. If the source has data there, the confirm
  screen warns before the write; if the clone actually fell back to gen1, it reports Partial and flags
  those blocks.
- **Robustness** — verify after an RF field power-cycle (`NfcCommandReset`), so a card that only latches
  the UID after a reset isn't misreported; dedicated "Not a magic tag" / "Card removed" outcomes;
  detect / write popups time out instead of hanging.

## Limitations / not included

- ⚠️ **The gen1 path is a faithful proxmark port but is NOT hardware-validated** — no gen1 magic
  ISO15693 card was available to test against. The gen2 path is fully validated (below). gen1 changes
  are compile-tested and reviewed; the write-confirm + Partial reporting make its fidelity caveats
  explicit at runtime.
- **gen3 / "V3" magic** (the config-mode-signature variant that can permanently lock the UID) is
  detected-only groundwork, not implemented as a write path.
- SLIX-specific features that live in the SDK's richer `slix` layer — privacy mode / passwords, EAS,
  originality signature — are **out of scope** (we build on the raw `iso15693_3` layer).

## Testing / validation

- **gen2 validated end-to-end on hardware.** A sweep of synthetic sources produced **byte-identical
  clones** across chip types and geometries (28 / 56 / 64 / 70 blocks, NXP / ST / TI IC refs),
  confirmed by **both** the stock Flipper NFC read-back **and** a Proxmark3 cross-read (UID +
  block_count + IC ref matched the source every time). Wipe and the honest over-capacity reporting
  (empty tail → Success, non-empty over-capacity → Partial with the exact blocks named) were confirmed
  on the same card.
- Builds clean under `-Werror`; links; `APPCHK` passes.
- The dev test tooling used for this (a Proxmark3 characterization harness and a Flipper-CLI
  ground-truth harness) is **not part of this PR** — it lives in the dev branch's `tools/`.

## Commits

1. **`nfc_magic: add magic ISO15693 (NfcV) protocol + detection`** — the `magic/protocols/iso15693/`
   module (poller / chip-info decode / data wrapper), protocol registration, and scanner detection.
2. **`nfc_magic: ISO15693 UI — Info / Write / Clone / Wipe scenes + integration`** — the dedicated
   menu + scenes and the routing/app-state integration into the shared detect / file-select / write flow.
3. **`nfc_magic: changelog + manifest for ISO15693 support`** — CHANGELOG entry + FAP description.

---

## How to open the PR (notes for me, not part of the PR body)

- **Base:** upstream nfc_magic at **v2.0**. The `iso15693-pr` branch is 3 commits on top of `85a6532`
  (our squashed "update to v2.0" commit). Confirm the real upstream target is at v2.0; if it has moved,
  **rebase / cherry-pick the 3 commits** (`65b2efa..f8c927b` → HEAD of `iso15693-pr`) onto a branch cut
  from the real upstream, since `85a6532` is a squashed sync, not upstream's actual history.
- **Target repo:** wherever nfc_magic upstream lives (it was cloned from
  `xMasterX/all-the-plugins` → `base_pack/nfc_magic`). Open the PR there (or to the standalone repo if
  that's the canonical one).
- **Must call out in the PR:** the **gen1 path is untested on hardware** (only gen2 was validated).
- The dev branch's `.vscode/settings.json` carries a stale `slix` reference inherited from the base;
  it is not in the PR diff, so it can be ignored (or dropped from the base if desired).
- The PR uses the upstream appid (`nfc_magic`) and icon header (`nfc_magic_icons.h`); the dev branch
  uses `nfc_magic_dev` only so it can be installed alongside the built-in app. Because of that appid
  collision the PR branch can't be built in *this* firmware checkout, but the code is byte-identical to
  the hardware-validated dev build apart from the appid/name/icon-header-name.
