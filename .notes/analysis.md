# SLIX feature — state-of-project analysis

_Review date: 2026-07-26. Branch: `slix-v2`. Reviewed against local `../proxmark3` and
`../Momentum-Firmware-slix` SDK._

## What the feature is

A Flipper Zero external app (`nfc_magic`, fork of xMasterX/all-the-plugins), synced to upstream
v2.0, extended with **magic ISO15693 / SLIX (NfcV)** support. Flow:

```
Check Magic Tag ─▶ scanner detects any ISO15693 tag as a SLIX candidate
                   └▶ SLIX menu ─┬▶ Info   (UID / mfr / chip / system info)
                                 └▶ Write UID ─▶ byte editor ─▶ (confirm) ─▶ backdoor write + verify
```

Files: `magic/protocols/slix/{slix_poller,slix_info,slix_data}.{c,h}`, scenes
`scenes/nfc_magic_scene_slix*.c`, detection in `magic/nfc_magic_scanner.c`
(`nfc_magic_scanner_detect_slix`), routing in `scenes/nfc_magic_scene_check.c:49-51`.

## What is verified correct (offline, high confidence)

- **Both magic write sequences are byte-for-byte identical to proxmark3.** gen1 matches
  `SetTag15693Uid`, gen2 matches `SetTag15693Uid_v2`. Full byte table in
  [protocol-reference.md](protocol-reference.md).
- **The UID byte-order round-trip is correct end-to-end.** The write stores the UID MSB-first
  (`uid[0]=0xE0`), exactly as proxmark's `payload.uid`; the SDK's inventory parser reverses the
  over-the-air LSB-first bytes back to MSB-first, so the read-back `memcmp` compares like-for-like.
  A physically successful write verifies as success and cannot produce a false success.
- **CRC** is correctly delegated to `iso15693_3_poller_send_frame` (matches proxmark's `AddCrc15`).
- **Thread-/memory-safe.** The `d7aea31` per-scene-poller fix is complete; `free` is fully
  synchronised behind `furi_thread_join`, so there is no use-after-free even on Back mid-write.
  bit_buffers and `SlixData` are alloc/free-balanced. `slix_data` is app-scoped (alloc'd
  `nfc_magic_app.c:131`, freed `:207`), which is why the Info scene can read it after the per-scene
  poller is freed.
- **Build-clean against the target SDK.** Every referenced symbol resolves; all scenes are wired.

## Maturity verdict

Correct-by-reference at the byte level: **yes.** Production-safe on real cards: **not until a card
validates the write→latch→read-back behaviour** (see [hardware-plan.md](hardware-plan.md)). It is a
UID-only writer, not a cloner, and (before this pass) over-claimed "SLIX" for every ISO15693 tag.

## Ranked issues (from the multi-agent review, verified)

Legend: **[offline]** fixable without a card · **[hw]** needs a card to fully settle.

| # | Sev | Issue | Status after offline pass |
|---|-----|-------|---------------------------|
| 1 | HIGH | Non-magic NfcV tag (the common input) → generic "Something went wrong" + a Retry that loops forever. No "not a magic card" message. **[offline]** | FIXED — dedicated SLIX fail scene w/ reason (not-magic vs card-lost), no dead Retry loop. |
| 2 | MED | Auto gen2→gen1 fallback can corrupt data: gen1's `WRITE BLOCK` hits blocks `0x38/39/3E/3F` (56/57/62/63) = real data blocks on a 64-block card. **[hw]** to trigger, **[offline]** to de-risk | MITIGATED — gen1 only runs if gen2 left the card **unchanged** (read-back == original), and the whole write is behind an explicit confirm screen warning about the destructive gen1 step. Residual clobber-of-normal-tag risk is now consent-gated + documented. |
| 3 | MED | Same-session verify (no RF power-cycle) diverges from proxmark's `switch_off()`+`getUID`; a correct write could read back OLD and be reported as Fail. **[offline]** fix, **[hw]** to confirm necessity | FIXED (mechanism) — verify now happens after a `NfcCommandReset` field power-cycle, per verify step. |
| 4 | MED | Entered UID never validated to start with `0xE0`; proxmark hard-rejects. **[offline]** | FIXED — `uid[0]` forced to `0xE0` before write. |
| 5 | MED | Chip decode can't tell SLI / SLIX / SLIX2 apart; drops SLIX2. **[offline]** | FIXED — decodes `uid[3]` type indicator (bits 3-4), matching the SDK's `slix_get_type` / proxmark masks. |
| 6 | LOW | Every ISO15693 tag labelled "ISO15693 (SLIX)" even ST/TI/EM tags. **[offline]** | FIXED — generic "ISO15693 / NfcV" titles. |
| 7 | LOW | Info mode can never emit Fail → dead branch, popup hangs silently with no card. **[offline]** | FIXED — activation-error timeout emits Fail; the (previously dead) detect-failed path is now live. |
| 8 | LOW | Build depends on fork-only macro `ISO15693_3_FDT_WRITE_POLL_FC`; stale header docs; dead `SlixPollerError` enum. **[offline]** | FIXED — fallback `#define`, updated docs, removed dead enum, `CardLost` now used. |
| — | — | Groundwork: no confirm before the irreversible write. **[offline]** | ADDED — `slix_write_confirm` scene. |

### Cosmetic / accepted (not changed this pass)
- Two overlapping custom-event enums (`nfc_magic_app_i.h` base-100 vs
  `helpers/nfc_magic_custom_events.h` base-100) with colliding numeric values. Confirmed **no
  runtime collision** (each scene only compares the constant it emits). Pre-existing app-wide tech
  debt; left alone to keep this pass scoped.
- `slix_info.c` `IC id = 10/68` comments are correct decimal notation, not a bug.

## Feature gaps (out of scope / future) — all stem from using raw `iso15693_3` not SDK `slix`
- No **privacy-mode / password** path (GET RANDOM NUMBER + SET PASSWORD) → a privacy-locked
  SLIX/SLIX2 is invisible and unreadable.
- No **EAS**, **AFI/DSFID write**, **block-data read/write cloning**, **protection pointer**,
  **signature**, or **counter** handling.
- No **gen3 / "V3"** magic (blocks `0x10/0x11` + `cfinalize`) that proxmark supports.
- No **save/load** of the read tag to a `.nfc` file (the poller already reads block data during
  Info activation but nothing displays or persists it).
- No **SLIX2-specific NXP GET SYSTEM INFO** read.
