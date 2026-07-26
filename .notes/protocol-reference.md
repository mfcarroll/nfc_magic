# Magic ISO15693 backdoor write — byte-level reference

The offline ground truth for the UID-write path. App frames in
`magic/protocols/slix/slix_poller.c` vs. proxmark3 `../proxmark3`. Verified 2026-07-26.

## Sources (local)
- `../proxmark3/armsrc/iso15693.c` — `SetTag15693Uid` (gen1), `SetTag15693Uid_v2` (gen2).
- `../proxmark3/client/src/cmdhf15.c` — `CmdHF15CSetUID` (UID orientation + verify).
- `../proxmark3/include/protocols.h` — `ISO15693_WRITEBLOCK 0x21`, `ISO15693_MAGIC_WRITE 0xE0`.
- SDK inventory parse: `../Momentum-Firmware-slix/lib/nfc/protocols/iso15693_3/iso15693_3_i.c`
  (`iso15693_3_inventory_response_parse` reverses the wire UID → MSB-first, `data[0]=0xE0`).

## UID convention
`target_uid` in the app is **MSB-first**: `uid[0]=0xE0`, `uid[7]=LSB`. This is the same orientation
proxmark's `payload.uid` uses. Proxmark's client sends it to the ARM **without** reversing for
gen1/gen2 (only `--v3` reverses). So the app's direct byte mapping matches proxmark exactly.

## gen1 — `SetTag15693Uid` (flags `0x02`, cmd `0x21` WRITE BLOCK)

| Step | Wire bytes (before CRC) | App builder |
|------|-------------------------|-------------|
| unlock | `02 21 3E 00 00 00 00` | `build_gen1_frame(0x3E, 00,00,00,00)` |
| commit | `02 21 3F 69 96 00 00` | `build_gen1_frame(0x3F, 69,96,00,00)` |
| UID lo | `02 21 38 uid[7] uid[6] uid[5] uid[4]` | `build_gen1_frame(0x38, uid[7..4])` |
| UID hi | `02 21 39 uid[3] uid[2] uid[1] uid[0]` | `build_gen1_frame(0x39, uid[3..0])` |

Commit magic = `0x69 0x96` in that order. **Match: exact.**

## gen2 — `SetTag15693Uid_v2` (flags `0x02`, cmd `0xE0` + sub `0x09`)

| Step | Wire bytes (before CRC) | App builder |
|------|-------------------------|-------------|
| cfg  | `02 E0 09 47 3F 03 8B 00` | `build_gen2_frame(0x47, 3F,03,8B,00)` |
| cfg2 | `02 E0 09 52 00 00 00 00` | `build_gen2_frame(0x52, 00,00,00,00)` |
| UID  | `02 E0 09 40 uid[7] uid[6] uid[5] uid[4]` | `build_gen2_frame(0x40, uid[7..4])` |
| UID  | `02 E0 09 41 uid[3] uid[2] uid[1] uid[0]` | `build_gen2_frame(0x41, uid[3..0])` |

CFG payload `3F 03 8B` = maxblock `0x3F` (64 blocks), blocksize `0x03` (4 bytes), IC ref `0x8B`.
These are **constants in proxmark too** and describe a 64-block card — see the clobber note below.
**Match: exact.** (proxmark's inline comments on cmd[2]/cmd[3] are swapped; the app follows the
*code*, which is correct.)

## Verify round-trip (why it's correct)
- proxmark: raw inventory `carduid` (LSB-first) → `reverse_array_copy` → compare to `payload.uid`.
- app: `iso15693_3_poller_inventory` already returns MSB-first (`readback[0]=0xE0`) → `memcmp` vs
  `target_uid`. The SDK's built-in reverse plays the exact role of proxmark's `reverse_array_copy`.

## Two behavioural divergences from proxmark (addressed in the offline pass)
1. **Field power-cycle before verify.** proxmark ends each write with `switch_off()` and re-runs
   `getUID` (fresh field). The app now returns `NfcCommandReset` (field off 100 ms → on) before each
   verify inventory, so the read-back reads a freshly re-powered card. Whether any real card *needs*
   this to latch is **hardware-gated**.
2. **No auto gen2→gen1 fallback in proxmark** (operator picks the generation). The app keeps an
   auto flow but now (a) only runs the **destructive** gen1 step if gen2 left the card unchanged
   (read-back == original UID), and (b) puts the whole write behind a confirm screen warning that
   gen1 can overwrite data blocks on a non-magic tag.

## Data-clobber math (why gen1 is the destructive one)
gen1 uses the **standard** `WRITE BLOCK 0x21` at blocks `0x38/0x39/0x3E/0x3F` = decimal
**56 / 57 / 62 / 63**. The gen2 CFG itself declares a 64-block card (maxblock `0x3F`=63), so those
addresses are real, writable user-data blocks. gen2's `0xE0` is an IC-custom command a normal tag
ignores (non-destructive). Hence: gen2-first is the safe probe; gen1 is the destructive step and is
now gated + consented.

## gen3 / "V3" (not implemented — reference for the future)
proxmark `hf 15 csetuid --v3`: writes UID config to magic-V3 blocks `0x10`/`0x11` via
`hf15_magic_v3_write_blk` (with `reverse_array_copy` of each half), repeatable, then locked by
`hf 15 cfinalize` (`CmdHF15CFinalize`). See `../proxmark3/client/src/cmdhf15.c`.
