# SLIX (magic ISO15693) — project notes

Working notes for the SLIX feature on branch `slix-v2`. These are developer notes, not user
docs. Written 2026-07-26.

| File | What's in it |
|------|--------------|
| [analysis.md](analysis.md) | State-of-project review: what works, what's verified, the ranked issue list. |
| [protocol-reference.md](protocol-reference.md) | Byte-level mapping of the magic backdoor write frames vs. the local proxmark3 reference. The offline ground truth. |
| [hardware-plan.md](hardware-plan.md) | The test plan for the work that can only be finished with a real magic ISO15693 card. |
| [worklog.md](worklog.md) | Running log of the offline changes actually made, commit by commit. |

## One-paragraph status

The SLIX feature detects an ISO15693 (NfcV) tag, shows Info (UID / manufacturer / chip / system
info), and performs a magic **backdoor UID write** (gen1 or gen2), verified by read-back. The core
write logic is a **byte-for-byte correct port of proxmark3** and the code is thread-/memory-safe.
It is honestly a **UID-only writer, not a card cloner**. As of the offline hardening pass it is
safe-by-consent and gives honest feedback; the remaining correctness unknowns are **hardware-gated**
(see [hardware-plan.md](hardware-plan.md)).

## Key context for anyone picking this up

- **Reference sources are local** (we work offline): proxmark3 at `../proxmark3`, the Flipper SDK
  the app compiles against at `../Momentum-Firmware-slix`.
- The app builds on the **raw `iso15693_3`** SDK layer, *not* the SDK's richer `slix` protocol
  (`../Momentum-Firmware-slix/lib/nfc/protocols/slix/`), which already models privacy/passwords/EAS/
  signature. That richer layer is unused — see the feature gaps in [analysis.md](analysis.md).
- **Builds with `fbt`**, not `ufbt` (which isn't installed). The app is symlinked into
  `applications_user/` of the local firmware checkouts, so:
  `cd ../Momentum-Firmware && FBT_NO_SYNC=1 ./fbt fap_nfc_magic_dev` (builds offline; toolchain is
  already downloaded). Confirmed clean 2026-07-26. On-hardware validation is the only step left.
- The neighbouring `../Momentum-Firmware-slix` firmware fork is **identical to stock Momentum** in
  the ISO15693 code — the SLIX feature is entirely app-side, and builds against either. Note
  `ISO15693_3_FDT_WRITE_POLL_FC` exists in **both** Momentum SDKs (it is not fork-only); the app also
  defines an `#ifndef` fallback purely as belt-and-braces for an SDK that might lack it.
