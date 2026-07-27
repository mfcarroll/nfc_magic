# ISO15693 vs SLIX — a primer for this codebase

_Why the feature is named "SLIX" but is really a generic ISO15693 tool, and how the pieces map._
_Grounded in the local SDK (`../Momentum-Firmware-slix`) and proxmark3 (`../proxmark3`). 2026-07-26._

## 1. The core distinction

**ISO15693** (a.k.a. **NfcV**) is the *standard*: the 13.56 MHz RF layer plus a core command set every
compliant tag understands. **SLIX** is *one NXP ICODE chip*; ICODE is NXP's ISO15693 product line. So:

```
SLIX  ⊂  ICODE (NXP)  ⊂  ISO15693
```

Calling a tool "SLIX" when it only speaks ISO15693 *core* commands is a misnomer. The name here is a
legacy label from proxmark's colloquial "SLIX magic card" (`../proxmark3/client/luascripts/hf_15_magic.lua:11`),
**not** a chip dependency.

The Flipper SDK separates the two layers explicitly, and **this app is built entirely on the generic
one**:
- `../Momentum-Firmware-slix/lib/nfc/protocols/iso15693_3/` — generic (inventory, system info, block
  read/write, lock, AFI/DSFID). ← **what we use.**
- `../Momentum-Firmware-slix/lib/nfc/protocols/slix/` — NXP-specific (passwords, EAS, privacy,
  signature). ← **never referenced in our code** (only in these notes).

Proof: `slix_poller_alloc` allocates `NfcProtocolIso15693_3`, not `NfcProtocolSlix`
([slix_poller.c](../magic/protocols/slix/slix_poller.c)), and Info just copies the SDK's generic
`Iso15693_3Data`.

## 2. Chip families (by UID manufacturer byte)

Every ISO15693 UID is 8 bytes, MSB-first: `uid[0]=0xE0` (fixed), `uid[1]=manufacturer` (ISO/IEC
7816-6), `uid[2..7]=serial` with the IC family in the high bytes. Families a general tool should know:

| `uid[1]` | Manufacturer | Common parts |
|----------|--------------|--------------|
| `0x04` | **NXP** (most common) | ICODE SLI / SLIX / SLIX-S / SLIX-L / SLIX2 / DNA, NTAG5 |
| `0x02` | **STMicroelectronics** | LRI64/2K, LRIS2K/64K (M24LR), ST25TV / ST25DV |
| `0x07` | **Texas Instruments** | Tag-it HF-I |
| `0x16` | **EM Microelectronic** | EM4033/4133/4233/4237, EM4425 (Echo V) |
| `0x05` | **Infineon** | my-d (SLE66r01P), SRF55 |

Our decoder ([slix_info.c](../magic/protocols/slix/slix_info.c)) is already multi-vendor: a full
manufacturer table plus a chip table covering ST/TI/EM/Infineon/NXP. The NXP SLI/SLIX/SLIX2 refinement
(via the `uid[3]` type bits) is correctly gated behind `if(vendor_id == 0x04)` and falls back to the
generic table for every other vendor. ST ST25TV, EM4425, and NXP ICODE 3 are now decoded too
(`fce00c3`).

## 3. Standard vs custom commands

**Generic — work on ANY family (this is what the app relies on):**
Inventory `0x01`, Read Block `0x20`, Write Block `0x21`, Lock Block `0x22`, Read/Write Multi
`0x23`/`0x24`, Write/Lock AFI `0x27`/`0x28`, Write/Lock DSFID `0x29`/`0x2A`, Get System Info `0x2B`,
Get Block Security `0x2C` (`iso15693_3.h:56-75`).

**Chip-specific — reserved custom range `0xA0–0xDF`, do NOT generalize:**
NXP SLIX uses `0xA2–0xBE`: EAS (`0xA2–0xA5`), GET NXP SYSTEM INFO (`0xAB`), GET RANDOM NUMBER (`0xB2`),
SET/WRITE PASSWORD (`0xB3`/`0xB4`), ENABLE PRIVACY (`0xBA`), READ SIGNATURE (`0xBD`) (`slix_i.h:19-40`).
ST/TI/EM define their own incompatible customs in the same range. **The app sends none of these** — so
despite the name it has no NXP lock-in on the wire.

Two robustness facts the SDK gives us for free:
- Get System Info / Read Blocks / Get Block Security are **optional** — errors are filtered, so a bare
  read-only tag still yields UID + manufacturer (`iso15693_3_poller_i.c:96-126`).
- **Geometry is never assumed** — `block_count`/`block_size` come from Get System Info and buffers are
  sized at runtime (`iso15693_3_poller_i.c:103-126`). A generic dump/restore adapts to any tag.

## 4. Magic variants

The magic property is a **clone-silicon** property, generic to magic ISO15693 and *family-agnostic*
w.r.t. the identity it emulates. proxmark's own doc section is a stub; the C source is authoritative.

| Variant | How the UID is set | Repeatable? | Notes |
|---------|--------------------|-------------|-------|
| **gen1** | standard WRITE-BLOCK `0x21` to backdoor blocks `0x3E`(unlock=0), `0x3F`(commit=`0x6996`), `0x38`=uid[7..4], `0x39`=uid[3..0] | no (one-shot) | `SetTag15693Uid`, `../proxmark3/armsrc/iso15693.c:3188` |
| **gen2** | vendor magic cmd `0xE0`/`0x09` to refs `0x47`/`0x52`/`0x40`/`0x41`; ref `0x47` also forces reported geometry to 64-blk/4-byte/IC-ref-`0x8B` | no | `SetTag15693Uid_v2`, `iso15693.c:3238` |
| **gen3 / "V3"** | standard WRITE-BLOCK to config blocks `0x10`/`0x11` (UID **reversed**) | **yes** | locked permanently by a separate finalize writing sig blocks `0x14`=`A5 2B 44 2C` / `0x15`=`69 E2 5D 00` — irreversible. `../proxmark3/client/src/cmdhf15.c:3302-3555` |

**Byte-order differs per variant** — gen1 `0x38`=uid[7..4]; V3 `0x10`=uid[4..7] *reversed*. Do not copy
gen1 ordering into a V3 implementation.

Detection: gen1/gen2 have **no read probe** — they're inherently write-and-verify (which our scanner
does: any activating ISO15693 tag is an unconfirmed candidate). Only an **un-finalized V3** tag is
non-destructively identifiable: read `0x14`/`0x15` with the OPTION flag and match `{A5 2B 44 2C}` /
`{21 AE 93 00}` (`cmdhf15.c:3362-3373`).

## 5. How our app maps onto ISO15693

| Feature | Layer | Generic? |
|---------|-------|----------|
| Detect | any ISO15693 tag = candidate (`nfc_magic_scanner.c`) | ✅ generic |
| Info (UID / mfr / chip / system info / blocks / lock) | standard core commands | ✅ generic |
| Save to `.nfc` | `nfc_device_save` typed `NfcProtocolIso15693_3` | ✅ generic |
| Write **UID** | magic backdoor gen1 + gen2 (forces `uid[0]=0xE0`) | magic-clone-generic, ✅ family-agnostic |
| Write **data blocks** | — | ❌ **not implemented** (the main gap) |
| V3 magic variant | — | ❌ not implemented |
| SLIX password / EAS / privacy / signature | — | ❌ not implemented (would be per-family, out of scope) |

**Takeaway:** to make this "a general ISO15693 magic reader/writer" is mostly *reframing* — the read/
info/save/UID-write pipeline is already generic. The real capability gaps are **block-data write-back**
(a generic WRITE-BLOCK `0x21` loop, gated on `block_security` lock bits, with the OPTION flag for TI)
and the **V3** variant — both code-draftable but **hardware-gated** to verify. See
[capability-matrix.md](capability-matrix.md) and [hardware-plan.md](hardware-plan.md).
