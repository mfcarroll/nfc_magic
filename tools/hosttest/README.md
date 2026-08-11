# Host-side tests for the ISO15693 sweep logic

Dev-only. `application.fam` excludes `tools/` from the FAP build, so nothing here ships or affects the
firmware build.

```bash
cd tools/hosttest && make
```

## Why this exists

Several behaviours in the ISO15693 wipe and clone are *reported outcomes* — a count, a verdict, a screen
— and getting one wrong produces a confidently wrong report rather than a crash. That is the class of bug
every round of PR #250 has found. Most of them cannot be staged on hardware: they need a card that
refuses a write while still answering a read, or a stretch of memory that dies mid-sweep, or a sweep slow
enough to hit the 10-second backstop.

They are all trivial to stage against a fake tag.

## How it works, and what it does not do

`test_wipe_sweep.c` `#include`s `iso15693_poller.c` directly. The shipped file is compiled **verbatim** —
no seam, no wrapper, no `#ifdef TEST`, and the file-statics are reachable because they are in the same
translation unit. The SDK calls resolve to `fakes/` instead of the firmware headers, via `-Ifakes`.

That means a behaviour change in the sweep is a failure here, which is the point: these are regression
tests for refactors, and Pass C was five behaviour-preserving commits verified only by reading.

The one thing the host cannot reach is the radio layer — what real silicon puts on the wire. That is a
hardware question. Everything between the sweep and the radio is firmware source we have, and is checked
below rather than assumed.

## Fake fidelity, verified against firmware source

A wrong fake is worse than no test, because it asserts wrong behaviour confidently. Every semantic the
sweep depends on was read out of the firmware rather than guessed. Line numbers are Momentum 87.15;
**the entire `lib/nfc/protocols/iso15693_3/` directory is byte-identical between Momentum 87.15 and
Unleashed 88.2**, so one model covers both API versions.

| behaviour the sweep relies on | verified in | result |
|---|---|---|
| a refused write returns `Iso15693_3ErrorInternal` | `iso15693_3_i.c:42-48` — `BLOCK_UNAVAILABLE`, `BLOCK_LOCKED`, `BLOCK_ALREADY_LOCKED`, `BLOCK_WRITE`, `BLOCK_LOCK` all map to it | confirmed; fake matches |
| activation's block read stops at the first failure | `iso15693_3_poller_i.c:104-118` — `read_blocks` breaks on first non-`None`, remainder left at its zeroed allocation | confirmed; `fake_tag_cache_from_activation()` reproduces it |
| …and activation still reports success | `iso15693_3_poller_i.c:22-33` — `filter_error` maps `Timeout` and `NotSupported` to `None` | confirmed; this is what makes the activation cache prove presence only, never absence |
| `get_block_count` / `get_block_size` are the advertised values | `iso15693_3.c:356-366` — returned straight from `system_info` | confirmed; fake matches |
| `get_block_data` range-checks its index | `iso15693_3.c:368-374` — `furi_check(block_count > block_index)` | confirmed; fake `furi_check`s identically, so `block_held_data`'s comment about relying on it holds |
| the full `Iso15693_3Error` enum | `iso15693_3.h:85-102` | fake mirrors all 16 members in order, so values match too |

One semantic comes from the bench rather than from source: blocks past physical capacity **refuse writes
in-band while failing reads outright** (measured 2026-08-04). The fake models that. The sweep treats every
non-`None` error alike, so it makes no difference to the outcomes — confirmed by the tests all still
passing when the fake's error codes were corrected to match.

## What is covered

13 cases over `iso15693_poller_wipe_blocks`, including the geometries re-derived by hand each review
round:

- clean 64/64
- advertised 28 / physical 64 — the case the sweep exists for, measured on hardware
- advertised 66 / physical 64 — fake flash; the phantom tail must be dropped, not reported
- a dead stretch inside the claimed range that answered at activation — **the Round 4 blocking finding's
  positive case**, previously shipped on reasoning alone
- the same stretch dead *before* activation — the residual he named and we did not close, pinned as a test
  so closing it later shows up as a change here
- a locked block still holding data, and a locked block already empty
- an interior dropout that recovers
- advertised 0, and a full 256 blocks at the bitmap ceiling
- a card refusing every write while answering reads — cut by the wall clock, unreachable on hardware
- a card lifted mid-sweep
- the summary log line, which also pins the `ABSENT_RUN` probe cost paid past the card's top

## Not covered yet

- `iso15693_poller_write_source_blocks` (the clone loop) — same technique, not yet written. The capacity
  gate's discriminating case lives here.
- `iso15693_poller_write_step` and the terminal-event choice — needs the state machine driven rather than
  a single function called.
- Anything above the poller: the scenes and their rendering.
- The gen1 path. Its block-skipping arithmetic is testable; its wire behaviour is not, and no gen1 card
  exists on either side of the PR.
