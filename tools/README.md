# tools/ — SLIX / ISO15693 magic test tooling

Dev/test helpers for the SLIX (magic ISO15693) work. **Not part of the FAP** — they don't build into
the app and needn't go upstream with it.

## `iso15693_magic_probe.py` — characterize a magic card (Proxmark3)

Guided harness (modeled on the T5577 capture campaign) that probes an unknown magic NfcV card and
records everything to a timestamped campaign dir (`campaign.log`, `manifest.json`, `raw/*.txt`).

```bash
python3 tools/iso15693_magic_probe.py --list-probes
python3 tools/iso15693_magic_probe.py --card "aliexpress-64blk" --probes info,capacity,magictype
python3 tools/iso15693_magic_probe.py --card blank1 --probes info,capacity,magictype,edgepages,impersonate --destructive
python3 tools/iso15693_magic_probe.py --dry-run --card x --probes info,capacity   # plan only
```

Probes: **info** (identity), **capacity** (physical block count vs reported — finds the phantom
tail), **magictype** (V3 signature; gen1/gen2 UID-write test), **edgepages** (write/read the last-real
& first-phantom block, aliasing check), **impersonate** (does the card accept a *standalone* CFG frame
to report another geometry / IC ref?), **writespan** (does `WRITE BLOCK` obey the *advertised* block
count or the *physical* capacity?). The write probes need `--destructive`.

`writespan` settles whether the clone app should cap writes at the target's advertised count. It's
meaningful only when the card advertises **fewer** blocks than it physically has — clone a small
source first (e.g. `slix_28` → 28 blocks) onto the physically-64 magic card, then:

```bash
python3 tools/iso15693_magic_probe.py --card blank1 --probes info,capacity,writespan --destructive
```

It write-tests a ladder of blocks around the advertised boundary (snapshot + restore each) and reports
the highest writable block: **> advertised** ⇒ writes follow physical capacity (app should write every
source block and report only true failures); **== advertised** ⇒ the card gates writes by the
advertised count (that count is the real limit). Tune the top with `--writespan-max N`.

Note the geometry side-effect: the `magictype` gen2 `csetuid` test rewrites the card's CFG block to
proxmark's default geometry (64 blk / IC 0x8B) as a side effect. The probe snapshots the run-start
geometry, tries to restore, and if the card won't take a standalone CFG restore it says so and tells
you to re-clone from the `.nfc`. `impersonate` on such a card reports "no change (standalone CFG
ignored)" — that's not a clamp; real geometry only sets via the full UID-write sequence, which is what
the app clone (and the ground-truth harness below) exercises. `--flipper-note` captures what the
Flipper app reads, for a quick proxmark-vs-Flipper diff.

## `flipper_ground_truth.py` — ground-truth the app clone on hardware (Flipper CLI)

The other half of the rig: uploads a source `.nfc` to the Flipper, has you write it onto the magic
card with the **NFC Magic app** and read it back with the **stock NFC app**, then auto-detects the
read-back file (by diffing `nfc/`), downloads it, and compares source vs read-back — identity
(UID/IC/DSFID/AFI), block-by-block data, and over-capacity (source blocks a smaller card can't hold).
Optional Proxmark3 second-opinion read. Campaign-dir output like the probe.

Needs `pyserial`, which the system python lacks — run under the tools venv (one-time setup):

```bash
python3 -m venv tools/.venv && tools/.venv/bin/pip install pyserial   # once
tools/.venv/bin/python tools/flipper_ground_truth.py                  # all tools/test_nfc/slixtest_*.nfc
tools/.venv/bin/python tools/flipper_ground_truth.py --sources tools/test_nfc/slixtest_slix_28.nfc
tools/.venv/bin/python tools/flipper_ground_truth.py --pm3-crosscheck  # add a Proxmark3 read
python3 tools/flipper_ground_truth.py --dry-run                        # parse/preview, no device
```

`flipper_bridge.py` is the reusable serial-storage layer (list/upload/download/new-file-diff); it
auto-locates `../Momentum-Firmware/scripts`. Close qFlipper / other serial sessions first — only one
CLI client can hold the port.

## `make_test_iso15693_nfc.py` — synthetic source `.nfc` files

Generates ISO15693-3 dumps with chosen identity/geometry/data to test app-side clone + impersonation
on one magic target. Output in `test_nfc/` — feed them to `flipper_ground_truth.py`, which uploads,
prompts the on-device NFC Magic → Write, and compares the read-back automatically.

```bash
python3 tools/make_test_iso15693_nfc.py --list
python3 tools/make_test_iso15693_nfc.py            # -> tools/test_nfc/*.nfc
```

The set covers: smaller impersonation (SLIX-28, LRi2K-56), same-size/different-IC (Tag-it-64),
edge-page data that fits (edgedata_64), and edge-page data that can't fit a 64-block target
(oversize_edgedata_70 vs oversize_empty_70 — Partial-with-named-blocks vs clean Success).
