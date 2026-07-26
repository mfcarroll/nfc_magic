# ISO15693/SLIX capability matrix & port roadmap

_Where our app sits vs the stock Flipper NFC app and proxmark3, and what to build next._
_From a 3-source catalog (our app / stock Flipper+SDK / proxmark). 2026-07-26._

## The reframe

Most of what we'd want is **already written — in the Momentum SDK we link against and the stock NFC
app**, not just in proxmark. Our fork uses **none** of it: it sits on the raw `iso15693_3` layer only.

- The SDK `SlixPoller` already exposes: `get_nxp_system_info` (0xAB), `read_signature` (0xBD),
  `get_random_number` (0xB2), `set_password` (all 5 types, 0xB3), `write_block(s)`
  (`slix_poller.h:80-149`). The generic poller exposes read/write block(s), inventory, get_system_info,
  get_blocks_security, `send_frame` (`iso15693_3_poller.h:63-183`).
- The **stock NFC app** already ships: read, info, save, **emulate**, **block-write to card**, and a
  full **SLIX privacy-unlock UI** (manual hex + TonieBox preset `{0x5B6EFD7F, 0x0F0F0F0F}`, default
  `0x0F0F0F0F`) — `helpers/slix_unlock.c`, `nfc_scene_slix_unlock*.c`.

**The ONE thing the stock app fundamentally cannot do is change a genuine card's UID** — its "Edit UID"
only rewrites the in-memory dump and re-saves the `.nfc` (`nfc_scene_set_uid.c:34`). That magic
backdoor is the single piece of code we must own, and it's what makes us a *clone* tool, not a reader.

**SLIX "password auth" is not crypto** — it's a plain XOR of the 4-byte password with a 2-byte
GET_RANDOM value (`../proxmark3/armsrc/iso15693.c:3282`), and the SDK already implements it. So every
`slix*` operation is a straightforward protocol port, not a cryptographic problem.

## 3-way matrix

Status: **✅ have** · **stock** = in the built-in NFC app/SDK (reusable) · **pm3** = proxmark only ·
**✗** = nobody. "Next for us" = recommended action.

| Capability | Our app | Stock Flipper | proxmark | Next for us |
|---|---|---|---|---|
| Read / inventory / blocks / security | ✅ (raw layer) | ✅ (SlixPoller: +type, +NXP sysinfo, +sig) | ✅ | migrate to SlixPoller |
| Info + full block data | ✅ | ✅ | ✅ | parity reached |
| Save to `.nfc` | ✅ (generic) | ✅ (richer typed slix) | ✅ | richer save on migration |
| **Block-DATA write to card** (0x21) | ✗ | ✅ (skip-locked/equal) | ✅ | **port — Phase 2, high value** |
| Gate writes on lock bits (0x2C) | ✗ | ✅ | ✅ | port with the writer |
| AFI write (0x27) / DSFID write (0x29) | ✗ | ✗ (SDK reads only) | ✅ | port from pm3 (send_frame) |
| **Change genuine UID (magic backdoor)** | **✅ gen1+gen2** | ✗ (dump-only) | ✅ | **our differentiator** |
| Magic V3 UID (0x10/0x11) + finalize (0x14/0x15) | ✗ | ✗ | ✅ | port — plain write_block |
| **SLIX privacy unlock** (0xB2+0xB3) | ✗ | ✅ (full UI) | ✅ | **reuse stock scenes** |
| NXP system info (0xAB) | ✗ | ✅ | ✅ | free on migration |
| Originality signature read (0xBD) | ✗ | ✅ | ✅ | free on migration |
| Signature **verify** (offline ECDSA) | ✗ | ✗ | ✅ | port as honesty check |
| Password set — read/write/destroy/EAS types | ✗ | SDK-callable (UI only wires Privacy) | ✅ | menu + byte-input |
| EAS enable/disable/read | ✗ | ✗ (constants only) | ✅ | low value for cloning |
| Emulate a tag | ✗ | ✅ (listener) | ✅ | lean on stock app |
| **Forge signature / ICODE-DNA AES** | ✗ | ✗ | ✗ | **infeasible (crypto)** |
| Sniff / raw samples / demod / trace | ✗ | ✗ | ✅ | infeasible (needs pm3 FPGA/trace) |
| Read a password OFF a card | ✗ | ✗ | ✗ | impossible by design (write-only) |

## Roadmap (offline-first)

**Top move — infrastructure (compile-verifiable now):** migrate our SLIX read/info/save from the raw
`NfcProtocolIso15693_3` layer onto the SDK `NfcProtocolSlix` device+poller. This one change hands us
privacy-unlock, NXP sysinfo, signature read, per-block lock gating, and richer typed save/load **for
free**, and aligns us with the stock app. Keep `slix_poller.c`'s backdoor as the UID-write step.
Everything below gets cheaper after this. (Cost: a heavier device model + a one-time refactor of the
Info/save scenes off the raw layer.)

**Offline, compile-verifiable now (land code; verify latching on hardware later):**
1. **Block-data writer** — `iso15693_3_poller_write_block(s)`, gated on Get Block Security (0x2C),
   per-block Success/Partial/Fail like the Gen2/USCUID paths. This is clone **Phase 2** — highest-value
   new capability.
2. **AFI (0x27) + DSFID (0x29) write** — the only two standard write frames the SDK lacks; trivial
   `send_frame` ports from proxmark `CmdHF15WriteAfi`/`WriteDsfid`.
3. **Reuse the stock privacy-unlock UI** — `helpers/slix_unlock.c` + `nfc_scene_slix_unlock*.c`
   (default `0x0F0F0F0F`, TonieBox preset, manual byte-input).
4. **Magic V3 path** — `csetuid --v3` (write_block to 0x10/0x11, reversed) + `cfinalize` (0x14/0x15),
   guarded by a config-mode check; finalize is irreversible → confirm gate.
5. **Signature read + offline verify** — honesty check that flags a card whose UID-bound signature a
   magic clone can't reproduce.

**Hardware-gated (code offline, only a card confirms behavior):** all write latching — the magic UID
write we already ship, block write-back, privacy-unlock against a genuine quiet card, V3 finalize. Land
code, then run `.notes/hardware-plan.md`.

**Do NOT attempt (crypto / hardware walls):** forge the SLIX2/ICODE-DNA originality signature (ECDSA
over UID with NXP's private key), ICODE-DNA AES mutual auth, and sniff/samples/demod (need proxmark's
FPGA + trace buffer; no ST25R3916 equivalent).

## Clone one specific SLIX — exact pipeline

1. **Read** the source with the SLIX poller → UID + every block + sysinfo + privacy state.
2. If activation errors because **privacy is enabled**, disable it (0xB2 + 0xB3) — need the password
   (defaults / TonieBox preset, else 2³² brute, AUTHLIM-throttled).
3. On a **magic target**, write the captured UID via our backdoor (gen1/gen2 ✅; add V3 if the target
   is V3).
4. **Write block** every unlocked data block (skip locked); restore AFI/DSFID.

**Have:** steps 1 (plain card) + 3. **Missing (all offline-portable, none cryptographic):** block
write-back, AFI/DSFID write, privacy-unlock.

**Likely blockers, ranked:** (i) **privacy lock** on the source (need the password — access control,
not crypto); (ii) **password-protected** read/write blocks (same); (iii) **originality signature** — a
hardware magic clone presents a wrong/blank sig bound to the real UID, so any reader that *verifies*
originality will reject the clone (save the sig for an *emulated* replay, but a magic clone can't); (iv)
**ICODE DNA AES** — uncloneable, out of scope.

## Architecture recommendation
**Adopt the SDK `slix` protocol and reuse the stock privacy-unlock; do not stay on raw `iso15693_3`.**
The SDK slix layer already implements the NXP-custom command set correctly and the stock unlock UI is
drop-in, so migrating replaces hand-rolled frames with tested code and unlocks 6+ capabilities at once.
Keep the magic-backdoor UID write as our own module (the SDK will never provide it). Then port the
3–4 small missing frames (AFI/DSFID write, V3, signature-verify).
