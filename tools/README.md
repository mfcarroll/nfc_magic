# tools/ — ISO15693 magic test tooling

Dev/test helpers for the ISO15693 (magic) work. **Not part of the FAP** — they don't build into
the app and needn't go upstream with it.

## `iso15693_magic_probe.py` — characterize a magic card (Proxmark3)

Guided harness (modeled on the T5577 capture campaign) that probes an unknown magic NfcV card and
records everything to a timestamped campaign dir (`campaign.log`, `manifest.json`, `raw/*.txt`), and
folds each card into the standing inventory below.

```bash
python3 tools/iso15693_magic_probe.py --list-probes
# safe pass: no writes at all, and enough to rule gen3 in or out
python3 tools/iso15693_magic_probe.py --card "aliexpress-64blk" --probes info,baseline,capacity,magictype
# then the one probe that needs a write, to separate gen1 / gen2 / non-magic
python3 tools/iso15693_magic_probe.py --card "aliexpress-64blk" --probes magictype --destructive
python3 tools/iso15693_magic_probe.py --dry-run --card x --probes info,capacity   # plan only
```

Probes: **info** (identity), **baseline** (pristine snapshot + restore script — run before any write),
**capacity** (physical block count vs reported — finds the phantom tail), **magictype** (V3 signature;
gen2-then-gen1 UID-write test), **edgepages** (write/read the last-real & first-phantom block, aliasing
check), **impersonate** (does the card accept a *standalone* CFG frame to report another geometry / IC
ref?), **writespan** (does `WRITE BLOCK` obey the *advertised* block count or the *physical* capacity?).
The write probes need `--destructive`.

### Categorizing an unknown tag safely

Only one thing here is genuinely irreversible, and it is not run: `hf 15 cfinalize`, which locks a gen3
tag permanently. Everything else is recoverable **given a record of what was there** — which is the
whole reason `baseline` exists.

1. `--probes info,baseline,capacity,magictype` (no `--destructive`). Zero writes. Gives identity,
   geometry, physical-vs-advertised capacity, a restore script, and the gen3 answer — the gen3 test is
   just a read of blocks `0x14`/`0x15` against the config signature.
2. Group the tags by chip TYPE / IC ref / geometry from step 1, then write-probe **one representative
   per group** rather than every tag. That is the biggest reduction in risk available.
3. `--probes magictype --destructive` on the representative. gen2 is tried first and a success stops
   there, so a gen2 card never sees the destructive probe.

Why the order matters, verified against proxmark source rather than assumed:

| | frames sent | on a tag of another type |
|---|---|---|
| gen2 — `armsrc/iso15693.c:3216` | four **custom** `E0 09 …` | `0xE0` unimplemented → refused, nothing written |
| gen1 — `armsrc/iso15693.c:3166` | four **ordinary** `WRITE BLOCK` → 0x3E, 0x3F, 0x38, 0x39 | any writable tag **accepts** → blocks 56/57/62/63 destroyed |

### The gen1 gate

`magictype --destructive` no longer sends gen1's four frames as a block. It probes the range first, with
gen1's OWN first frame -- block `0x3E` (62, the unlock register) written to zero, which is the value gen1
writes there anyway -- and only then asks to send the rest.

That frame is chosen because it is the one that cannot do harm. It carries no arming value, and it
cannot move a UID even on an already-armed card: the UID moves when 56/57 are written, not the unlock
register. Block 56 would be the move on an armed card, and block 63 is the arming frame itself.

  refused    gen1 CANNOT work here, established without sending the arming frame. Recorded as a
             reasoned negative, which is stronger than an untested skip.
  accepted   the range is writable, so the next frame is `0x6996` into block 63. Requires confirmation
             (or `--allow-arming`), and REFUSES on a non-tty rather than proceeding silently.

Two costs it avoids when the answer is "refused". Nothing ever clears the commit register -- not this
app, not proxmark -- so a partial gen1 attempt can leave a card whose UID moves on any later write to
56/57, with no way to de-arm it and no way to read the register back (see #255). And on a card whose
user range stops below 56, what lives at 56-63 is simply unknown: an LRi2K's 56 user blocks are 1792
bits against a "2K" part, leaving eight blocks at exactly 56-63, exactly where gen1 writes. If those are
a system area reachable by WRITE BLOCK, a blind write there could be irreversible.

If the range read before being written, its prior content is put back.

And the gen1 result needs care: a gen1 write landing does **not** prove gen1 magic, because an ordinary
writable tag takes the same four frames. The distinguisher is whether the UID *moved*, which is what the
probe reads back — so `gen1_write: true` already means the UID changed, and a tag that accepted the
frames without moving its UID records as non-magic (with four blocks now overwritten).

### Which tag is this?

```bash
python3 tools/iso15693_magic_probe.py --identify                       # what is on the antenna?
python3 tools/iso15693_magic_probe.py --identify --card white-tag-3    # assert it, non-zero if not
```

Read-only. Labels live on paper and UIDs live on silicon, and some tags are physically identical while
not being interchangeable -- one may have been write-probed and another kept as an untouched control. The
second form is the precondition to run before any `--destructive` probe. A tag that is not in the
inventory reports as such rather than as a mismatch, since that is the normal state of a new arrival.

One caveat it prints for itself: a magic card's UID is not an identity. If a probe or a clone left a
magic card carrying a different UID, it will not match its own entry -- which is a reason to restore the
UID after any test, and a reason the `original` section records the UID it had when first seen.

### `physical_blocks` is a LOWER BOUND

Two ways, both seen on 2026-09-08. If every block up to the search ceiling answers a read, the binary
search terminates at its own bound and the figure is the probe's limit rather than the card's — the tool
now says so and `--capacity-max` pushes it out. And a card can hold blocks that take writes without
answering reads at all: this probe measured 56/56 on the gen1 card while the app's write-based wipe
cleared 58. `ISO15693_POLLER_WIPE_MAX_BLOCKS` states the rule — only a WRITE settles whether a block
exists — so a read-based probe under-detects by construction.

### The inventory

`tools/tag-inventory.json` is the source of truth; `.notes/tag-inventory.md` is a rendered table, rewritten
on every run — do not hand-edit it. `--render-inventory` rebuilds it and touches no hardware;
`--no-inventory` skips the update.

Each entry's **`original`** section is **write-once**: the pre-write state is recorded the first time a tag
is seen and never replaced, because by the second run this tool's own probes have written to the card. A
later run adds or corrects the classification only. That is what makes an entry citable in a validation
claim — "measured on the tag whose original config was X" stays true.

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
proxmark's default geometry (64 blk / IC 0x8B) as a side effect — so the gen2 probe is not purely a UID
write, and `baseline` emits the CFG frame that puts the original geometry back. The probe snapshots the run-start
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
tools/.venv/bin/python tools/flipper_ground_truth.py                  # all tools/test_nfc/iso15693_*.nfc
tools/.venv/bin/python tools/flipper_ground_truth.py --sources tools/test_nfc/iso15693_slix_28.nfc
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
