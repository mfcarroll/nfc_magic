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

99 tests across seven files.

**`test_write_step.c` — 15 cases over the write state machine.** These do not call one function: they
drive the real `iso15693_poller_nfc_callback` the way the SDK does — build an `NfcGenericEvent`, call the
callback, honour the returned `NfcCommand`, power-cycle the fake tag on `NfcCommandReset`, stop on
`NfcCommandStop`. So the field resets, the two activation-error budgets and the gen2-then-gen1 sequencing
are exercised rather than assumed. The reported events are recorded, so the tests assert the whole
sequence (`CardDetected` exactly once) and not just the terminal one.

This is where the fake earns its keep: it decodes the magic backdoor frames off the wire, so
`is_gen2_magic` / `is_gen1_magic` decide whether a UID write actually takes, and a gen1 UID latches only
on the next `fake_tag_power_cycle()` — which is the entire reason every UID verify in the poller sits
behind a reset. Covers the armed-gen1 wipe reporting a UID change, a card that never returns from the
reset still getting its wipe reported (`uid_verified` false), and that a clone writes no data at all onto
a tag that refuses the gen2 UID.

**`test_outcome.c` — 15 cases over `iso15693_poller_success_or_partial`**, the single place where "what
happened" becomes "what the user is told". A pure function of the result fields, so the tests read as the
contract: which conditions qualify a result, which do not, and which of those are clone-only. Includes the
two that look like oversights and are not — `uid_verified` being absent from the Partial list (making it
Partial would flag every wipe where the user lifts the card as it completes) and the all-rejected `Fail`
guard sparing a wipe (whose failed and accepted sets are disjoint). Both would be "fixed" by a reader who
had not read the reasoning, and both now fail loudly if they are.

**`test_clone_blocks.c` — 14 cases over `iso15693_poller_write_source_blocks`**, concentrating on what may
set `clone_capacity_confirmed`, since that renders "Card too small". This is the file that found the
clock-cut capacity bug. Also pins the gen1 backdoor-block arithmetic at both boundaries: a source below
block 56 must lose nothing from its total, and a source reaching 56/57 but not 62/63 must lose exactly
two. The first is a real geometry — magic SLIX cards ship with 32 blocks, where those addresses are
outside the data space entirely.

**`test_wipe_sweep.c` — 15 cases over `iso15693_poller_wipe_blocks`**, including the geometries re-derived
by hand each review round:

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
- **the two cut-index traces**: a dropped trailing run leaving the cut above `blocks_total`, and a
  read-everywhere card cut ABOVE the count it advertises -- the "Stopped at 200 of 64" shape. Both
  exist because a screen printed `blocks_total` as if it were where the sweep stopped, and the two
  figures are independent in either direction.

**`test_write_fail_scene.c` — 15 cases over the two ISO15693 result screens.** A second set of fakes, no
radio involved: the GUI calls become recorders, so what a screen SAYS, which buttons it offers, and where
each button navigates are all data a test can assert on. The scenes are compiled verbatim, same technique
as the poller.

This exists because of how round 5 broke down. Four defects: the poller tests found the one in the poller,
and the other three were in this layer — a control labelled `Exit` that opened Details, a screen that
never said why it stopped, and a Details note promising something the wall-clock bound cannot deliver.
Two of those were only ever going to be caught by a person reading a 128x64 screen.

The load-bearing case is `test_right_button_label_matches_where_it_goes`, which asserts the invariant
rather than the instance: for **every** reason code, the right slot's label agrees with where `on_event`
sends it. Adding a reason to `is_retryable` or `has_details` cannot silently reintroduce the round-5
blocking bug — reintroducing it fails that test and names the reason index and the offending label.

All three of those defects were mutation-tested after the fact: reverting each fix in the shipped source
fails the corresponding test. A test suite that has never been seen to fail is not evidence of anything.

**`test_write_identity.c` — 10 cases over `iso15693_poller_write_identity`**, the AFI / DSFID
write-then-read-back-and-verify retry loop. The outcome was already covered in `test_outcome.c`; these
cover the mechanics, which is where the load-bearing claim lives.

`iso15693_3_poller_send_frame` returns `Iso15693_3ErrorNone` whether or not the tag applied the write. A
tag refusing in band answers with a well-formed, CRC-valid error frame, and the SDK has no response
parser for these two commands the way `write_block` has one. So the send tells you nothing in either
direction and GET SYSTEM INFO is the only thing that can. The fake therefore models a refusal as
"swallow the write and still answer None" — a fake that reported refusals as errors could not test any of
this. Also pinned: that holding the right value is not enough (the target must ADVERTISE the field, or
the copy no longer reports the identity the source did), that an unreachable verify fails closed, and
that a transient is ridden out by the retries rather than reported.

**`test_write_scene.c` — 15 cases over the write scene's ROUTING.** Which screen each worker event sends
the user to, and which of the five magic protocols swallows Back. A routing table expressed as nested
branches, which is the shape that goes wrong quietly.

Its centrepiece is the mode-gate from round 5: `pass_truncated` used to be wipe-only, and once a cut CLONE
could set it, the branch sending a truncated run to the wipe-specific screen had to start asking which
mode it was in. That was the one piece of new behaviour in that round with no coverage at all. Also pinned:
that a moved UID outranks a cut sweep, that ISO15693 treats a lost card as terminal while the others resume
searching, and that Back is swallowed ONLY for ISO15693 and only after a card is found — swallowing it for
the other four would be a trap, since their pollers can stop advancing with the card gone.

`fake_write.c` holds link-only stubs for that scene: poller lifecycle, popup text, blink, icons. Every one
is reachable only from `on_enter`/`on_exit`, never from the routing, which reads plain fields off
`NfcMagicApp`. That boundary is deliberate — these tests assert which screen an event routes to, not what a
poller did to get there — and if the routing ever starts depending on a poller, the compile breaks there
and forces the question.

## A build trap that made this suite lie

`-MMD -MP -MF` was in place from the start, with a comment saying it exists to make an edit to the code
under test trigger a rebuild. **It did not work, and the failure was silent.**

The rule compiled the test and its fake in ONE `cc` invocation sharing a single `-MF`, so the fake's
dependency list overwrote the test's. Every depfile named only the fake and its headers — never the test,
never `iso15693_poller.c`, never a scene. `make` after a poller-only edit re-ran a **stale binary** and
printed a pass that meant nothing. It was found by mutation testing: two deliberately broken versions of
`write_identity` both "passed".

Fixed by compiling every translation unit separately, each with its own depfile. If you touch the
Makefile, check it still holds:

```bash
grep -c iso15693_poller.c build/test_write_identity.d   # must be non-zero
grep -c scenes/ build/test_write_fail_scene.d           # must be non-zero
```

The lesson generalises past this repo: a test suite that has never been observed to fail is not evidence,
and neither is a build system whose dependency tracking has never been observed to fire.

## Not covered yet

- The scenes' LAYOUT, as opposed to their content: the recorders capture x/y, font and alignment, but
  nothing asserts that N lines of FontSecondary actually fit above the button box. That arithmetic is
  documented in the write-fail scene and was measured by the reviewer, not by a test.
- The other scenes: the write scene's routing, the gen1 opt-in, the confirm screens.
- The gen1 path on the wire. Its block-skipping arithmetic is covered in `test_clone_blocks.c` and its
  latch-on-power-up behaviour is *modelled* in `test_write_step.c` — but the model is our inference from
  proxmark's send order, not a documented contract, so those tests confirm the app behaves correctly
  given the model rather than that the model is right. Settling that needs a gen1 card.
- The radio layer below the SDK — what real silicon puts on the wire. See `.notes/test-bench-idea.md`.

## Reasoned-only behaviours, before and after

The five from `.notes/test-bench-idea.md`, which shipped on reasoning because no available card produces
them:

| behaviour | now |
|---|---|
| tail-drop fix's positive case | **tested** — `test_wipe_sweep.c`, dead stretch inside the claimed range |
| the capacity gate's discriminating case | **tested** — `test_clone_blocks.c`, a failed block that answers a read |
| a clone cut by the clock, card still present | **tested** — and it was wrong; see the `pass_truncated` fix |
| truncated sweep reporting as Partial | **tested** — `test_outcome.c`; the *screens* remain untested |
| `uid_verified` false | **tested** — `test_write_step.c`, a card that never returns from the field reset |
| the gen1 path | **modelled, not settled** — the latch-on-power-up behaviour is tested against our inference from proxmark's send order; confirming the inference needs a card |
