# NFC Magic (SLIX) — team onboarding

Welcome. This is a one-page orientation; the deeper docs live in [`.notes/`](.notes/README.md).

## What this is

A **Flipper Zero external app** — a fork of the `nfc_magic` app from xMasterX/all-the-plugins,
synced to upstream **v2.0**. Upstream writes/clones/wipes magic MIFARE Classic (Gen1/Gen2), magic
Ultralight (USCUID-UL), and Gen4 (UMC) cards. Our fork adds a new tag family:

> **Magic ISO15693 / SLIX (NfcV).** Detect an ISO15693 tag, show **Info** (UID, manufacturer, chip,
> system info), and perform a magic **backdoor UID write** — the "Chinese magic" gen1/gen2 sequence,
> ported byte-for-byte from proxmark3 and verified by reading the UID back.

Scope note: this is a **UID-only writer**, not a full card cloner (it does not read/write data blocks,
AFI/DSFID, passwords, EAS, etc. — see the gaps below). Active development lives on branch
**`iso15693-v2`**.

## Repo layout

| Path | What |
|------|------|
| [`application.fam`](application.fam) | FAP manifest (appid `nfc_magic_dev`, name "NFC Magic SLIX"). |
| [`nfc_magic_app.c`](nfc_magic_app.c), [`nfc_magic_app_i.h`](nfc_magic_app_i.h) | App entry point + shared app state / view + scene enums. |
| [`scenes/`](scenes/) | UI scenes. [`nfc_magic_scene_config.h`](scenes/nfc_magic_scene_config.h) registers every scene (add an `ADD_SCENE` line + a matching `.c`). SLIX scenes are `nfc_magic_scene_iso15693*`. |
| [`magic/nfc_magic_scanner.c`](magic/nfc_magic_scanner.c) | Card **detection** — decides which magic family a tapped card is. |
| [`magic/protocols/`](magic/protocols/) | One poller per family: `gen1a`, `gen2`, `gen4`, `uscuid_ul`, and **`iso15693`** (our work). |
| [`.notes/`](.notes/README.md) | Developer docs: analysis, protocol reference, hardware plan, worklog. |

The SLIX logic is small and self-contained: [`magic/protocols/iso15693/`](magic/protocols/iso15693/)
(`iso15693_poller` = the magic write state machine, `iso15693_info` = manufacturer/chip decode) plus the
`iso15693*` scenes.

## What we've achieved

- **Detection** of any ISO15693 tag as a SLIX candidate, routed to a dedicated SLIX menu.
- **Info** screen with UID / manufacturer / GET-SYSTEM-INFO, and NXP **SLI / SLIX / SLIX2** decode.
- **Backdoor UID write**, hardened for safety:
  - Write sequence is a **byte-for-byte port of proxmark3** (`SetTag15693Uid` / `_v2`); the
    UID byte-order round-trip is verified correct end-to-end.
  - **Verifies after an RF power-cycle** (like proxmark's `switch_off` + re-`getUID`), with a retry.
  - The destructive **gen1 fallback only runs if the gen2 write left the UID untouched**, behind a
    **confirmation screen** warning it can overwrite data on a non-magic tag.
  - Honest outcomes: a non-magic tag says **"Not a magic tag"** (not a generic error), a removed card
    says **"Card removed"**, and detect/write popups time out instead of hanging.
- **Builds clean** under `-Werror`, links, and passes `APPCHK`.

**Maturity:** correct-by-reference and compiles clean, but **not yet validated on real hardware** —
that is the next milestone.

## Build & run

This app has no `ufbt` requirement; it's symlinked into `applications_user/` of a local firmware
checkout (a sibling `../Momentum-Firmware`, which supplies the SDK + toolchain). From that checkout:

```bash
FBT_NO_SYNC=1 ./fbt fap_nfc_magic_dev                          # build -> build/f7-firmware-C/.extapps/nfc_magic_dev.fap
FBT_NO_SYNC=1 ./fbt launch APPSRC=applications_user/nfc_magic_dev   # build, install & run on a connected Flipper
```

(`FBT_NO_SYNC=1` skips the submodule sync so it works offline once the toolchain is downloaded.)
Reference sources also live as siblings: `../proxmark3` (the magic-protocol ground truth) and the
firmware SDK.

## Next steps

1. **Hardware validation** — the only remaining work. Follow [`.notes/hardware-plan.md`](.notes/hardware-plan.md).
   The headline blocker: on a genuine magic ISO15693 card, confirm **write → latch → read-back**
   succeeds and that the gen1 gate protects data. This one result settles the last open correctness
   questions.
2. **Feature gaps** (out of scope so far, all because we build on the raw `iso15693_3` layer rather
   than the SDK's richer `iso15693` protocol): privacy-mode / passwords, EAS, AFI/DSFID write,
   data-block clone, gen3/"V3" magic, and save/load to `.nfc`.

## Where to go deeper

- [`.notes/analysis.md`](.notes/analysis.md) — full state-of-project review + the ranked issue list.
- [`.notes/protocol-reference.md`](.notes/protocol-reference.md) — byte-level map of the magic write vs proxmark3.
- [`.notes/hardware-plan.md`](.notes/hardware-plan.md) — the on-hardware test plan.
- [`.notes/worklog.md`](.notes/worklog.md) — what changed, commit by commit.
- [`CHANGELOG.md`](CHANGELOG.md) — release-facing history.
