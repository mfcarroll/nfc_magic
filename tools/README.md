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
& first-phantom block, aliasing check), **impersonate** (can the card report other geometries / IC
refs?). The last two write to the card — opt in with `--destructive` (they snapshot + best-effort
restore; use a blank first). `--flipper-note` captures what the Flipper app reads, for the
proxmark-vs-Flipper diff.

## `make_test_iso15693_nfc.py` — synthetic source `.nfc` files

Generates ISO15693-3 dumps with chosen identity/geometry/data to test app-side clone + impersonation
on one magic target. Output in `test_nfc/` — copy to the Flipper's `nfc/` folder, then
NFC Magic → Check → ISO15693 → Write → pick one → re-read and compare.

```bash
python3 tools/make_test_iso15693_nfc.py --list
python3 tools/make_test_iso15693_nfc.py            # -> tools/test_nfc/*.nfc
```

The set covers: smaller impersonation (SLIX-28, LRi2K-56), same-size/different-IC (Tag-it-64),
edge-page data that fits (edgedata_64), and edge-page data that can't fit a 64-block target
(oversize_edgedata_70 vs oversize_empty_70 — Partial-with-named-blocks vs clean Success).
