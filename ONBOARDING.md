# NFC Magic — ISO15693 magic support — team onboarding

Welcome. This is a one-page orientation; the deeper docs live in [`.notes/`](.notes/README.md).

## What this is

A **Flipper Zero external app** — a fork of the `nfc_magic` app from xMasterX/all-the-plugins, synced
to upstream **v2.0**. Upstream writes/clones/wipes magic MIFARE Classic (Gen1/Gen2), magic Ultralight
(USCUID-UL), and Gen4 (UMC) cards. Our fork adds a new tag family:

> **Magic ISO15693 / NfcV.** Detect an ISO15693 tag, show **Info**, and **clone / write / wipe** a
> magic ISO15693 card — the "Chinese magic" gen1/gen2 backdoor, ported byte-for-byte from proxmark3
> and verified by a power-cycled read-back.

It's a **full clone**, not just a UID writer: UID + all data blocks + identity (IC ref / geometry /
AFI / DSFID), handled through the same Check → menu → Write-from-`.nfc` flow as the other magic types.
Development lives on branch **`iso15693-dev`**.

## Repo layout

| Path | What |
|------|------|
| [`application.fam`](application.fam) | FAP manifest (appid `nfc_magic_dev`, name "NFC Magic Dev"). `sources` excludes the dev-only `tools/`. |
| [`nfc_magic_app.c`](nfc_magic_app.c), [`nfc_magic_app_i.h`](nfc_magic_app_i.h) | App entry point + shared app state / views / scene enums. |
| [`scenes/`](scenes/) | UI scenes. [`nfc_magic_scene_config.h`](scenes/nfc_magic_scene_config.h) registers every scene (`ADD_SCENE` line + matching `.c`). ISO15693 scenes are `nfc_magic_scene_iso15693*`. |
| [`magic/nfc_magic_scanner.c`](magic/nfc_magic_scanner.c) | Card **detection** — decides which magic family a tapped card is. |
| [`magic/protocols/`](magic/protocols/) | One poller per family: `gen1a`, `gen2`, `gen4`, `uscuid_ul`, and **`iso15693`** (our work). |
| [`tools/`](tools/) | Dev-only test harnesses (proxmark probe + Flipper ground-truth) and sample `.nfc`. Not part of the FAP. |
| [`.notes/`](.notes/README.md) | Developer docs: protocol reference, primer, capability matrix, hardware plan, worklog. |

The ISO15693 logic is small and self-contained: [`magic/protocols/iso15693/`](magic/protocols/iso15693/)
(`iso15693_poller` = the magic write state machine, `iso15693_info` = manufacturer/chip decode,
`iso15693_data` = a thin wrapper over the SDK's `Iso15693_3Data`) plus the `iso15693*` scenes. It
builds on the raw SDK `iso15693_3` layer.

## What we've achieved

- **Detection** of any ISO15693 tag as a magic candidate, routed to a dedicated menu (Write / Wipe /
  Write UID / Info).
- **Info** — UID / manufacturer / GET-SYSTEM-INFO / full block data (scrollable, `*` = locked), with
  NXP **SLI / SLIX / SLIX2** decode. (Saving a read to `.nfc` is the stock NFC app's job, not this one.)
- **Full clone from a saved `.nfc`** — UID (magic backdoor) + all data blocks + identity. Writes
  **every** source block and reports only real data loss (non-empty blocks that won't fit → Partial;
  empty over-capacity → clean Success). Impersonates larger/other geometries where the card allows.
- **Wipe** (zero data blocks, UID untouched) and **manual Write UID**.
- **Safety/robustness** — power-cycle read-back verify; gen1 fallback only after a gen2 no-op, behind
  a confirm; gen1 fidelity surfaced (warns pre-write and reports Partial post-write when blocks
  56/57/62/63 are affected); honest "Not a magic tag" / "Card removed" outcomes; popup timeouts.
- **Builds clean** under `-Werror`; links; passes `APPCHK`.

**Maturity:** the **gen2** path is hardware-validated end-to-end (byte-identical clones across
28/56/64/70-block geometries, cross-checked with a Proxmark3). The **gen1** path is a faithful
proxmark port but **not yet hardware-validated** — no gen1 magic ISO15693 card was available.

## Build & run

No `ufbt` requirement; the app is symlinked into `applications_user/` of a local firmware checkout
(a sibling `../Momentum-Firmware`, which supplies the SDK + toolchain). From that checkout:

```bash
FBT_NO_SYNC=1 ./fbt fap_nfc_magic_dev                               # -> build/f7-firmware-C/.extapps/nfc_magic_dev.fap
FBT_NO_SYNC=1 ./fbt launch APPSRC=applications_user/nfc_magic_dev   # build, install & run on a connected Flipper
```

(`FBT_NO_SYNC=1` skips the submodule sync so it works offline once the toolchain is downloaded.)
Reference sources live as siblings: `../proxmark3` (the magic-protocol ground truth) and the firmware
SDK checkout.

## Next steps

1. **Real-reader test** — clone an actual access pass and see whether the reader accepts it (UID-only
   vs. block-data / anti-clone checks).
2. **gen1 hardware validation** — needs a gen1 magic (and ideally a non-magic) ISO15693 card.
3. **Upstream PR** — the core code is PR-ready on top of v2.0; the PR must note the gen1 caveat.

## Where to go deeper

- [`.notes/protocol-reference.md`](.notes/protocol-reference.md) — byte-level map of the magic write vs proxmark3.
- [`.notes/iso15693-primer.md`](.notes/iso15693-primer.md) — ISO15693 vs SLIX, chip families, magic variants.
- [`.notes/capability-matrix.md`](.notes/capability-matrix.md) — our app vs stock Flipper NFC vs proxmark.
- [`.notes/hardware-plan.md`](.notes/hardware-plan.md) — the on-hardware test plan (what's done / pending).
- [`.notes/worklog.md`](.notes/worklog.md) — what changed, commit by commit.
- [`tools/README.md`](tools/README.md) — the proxmark + Flipper test harnesses.
