# SLIX (magic ISO15693) — project notes

Working notes for the SLIX feature on branch `slix-v2`. These are developer notes, not user
docs. Written 2026-07-26.

| File | What's in it |
|------|--------------|
| [analysis.md](analysis.md) | State-of-project review: what works, what's verified, the ranked issue list. |
| [protocol-reference.md](protocol-reference.md) | Byte-level mapping of the magic backdoor write frames vs. the local proxmark3 reference. The offline ground truth. |
| [hardware-plan.md](hardware-plan.md) | The test plan for the work that can only be finished with a real magic ISO15693 card. |
| [clone-feasibility.md](clone-feasibility.md) | Scoping for expanding to data-block writes / full clone: the Flipper-SDK-vs-magic-backdoor split, the cryptographic boundary, and a phased plan. |
| [iso15693-primer.md](iso15693-primer.md) | Background: SLIX vs ISO15693, chip families, standard vs custom commands, the magic variants (gen1/gen2/V3), and how this app maps onto it. |
| [capability-matrix.md](capability-matrix.md) | 3-way capability comparison (our app / stock Flipper NFC / proxmark) + a prioritized port roadmap. Key finding: adopt the SDK slix poller — most features are already written. |
| [worklog.md](worklog.md) | Running log of the offline changes actually made, commit by commit. |

## One-paragraph status

The SLIX / ISO15693 feature is a **full magic-card clone tool**, integrated the same way as the app's
other magic types: Check → "Magic card detected" → menu (**Write** a saved `.nfc` = UID + data blocks
+ identity IC/geometry/AFI/DSFID · **Wipe** · **Write UID** manual · **Info**). The magic write is a
byte-for-byte port of proxmark3, verified by a power-cycled read-back; the code is thread-/memory-safe
and builds clean under `-Werror`. **Hardware-validated 2026-07-27**: a byte-identical clone and a wipe
were confirmed on a real 64-block magic card. Current state + what's next: see the top of
[worklog.md](worklog.md). Deeper feature gaps (V3 variant, passwords/EAS/signature) remain out of
scope — see [capability-matrix.md](capability-matrix.md).

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
