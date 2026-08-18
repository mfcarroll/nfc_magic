# Interim comment on PR #250 — POSTED 2026-08-12 01:14Z

Not a reply to a review — this went up **before** his Round 5, because it carries a defect fix and one
question. Posted alongside fork commit `f8eb8164`, a fast-forward from `688614e8`.

Status: **POSTED AND PUSHED — HISTORICAL.** This records the state as of 2026-08-12; Round 5 has since
arrived and been answered. Do not read the fork SHA or commit count below as current.
Original note: Fork `nfc-magic-iso15693` = `f8eb8164`, 13 commits. Awaiting his Round 5,
which will be his first sight of the fix.

The pushed commit is unsigned, deliberately — see the push rules in
[../NEXT-SESSION.md](../NEXT-SESSION.md). Closed decision, not a to-do.

Verbatim below, as posted.

---

While you're looking at round 4, one fix and one question — plus a note on how the fix turned up, since
that part may be more useful than the fix.

## A clone stopped by the clock could report "Card too small"

`clone_capacity_confirmed` gates that string, and the wall-clock bound on the clone's data pass could set
it **while the card was still present and perfectly healthy**.

The mechanism is the one you'd predict from the round-3 discussion. When the clock cuts the pass, the
blocks above the cut are recorded as failures — correctly, since the partial screen derives its "cloned"
figure by subtracting them. But that leaves precisely the shape a capacity edge makes: a contiguous run at
the top, nothing written above it, no member answering a read. The shape with none of the meaning, because
those blocks refused nothing. They were never asked.

So the capacity claim now also requires that the pass ran to completion:

```c
const bool failures_are_top_tail = any_failure && wrote_any && !wrote_above_failure &&
                                   !any_failure_answered && !pass_truncated;
```

Same principle as the per-block read-probe you asked for — a claim about the user's hardware has to be
earned — applied to the one path that could reach it with no evidence at all.

I'd also had the comment wrong. It said the clock "only fires on a card that has gone". It can fire with a
card present: marginal coupling makes blocks succeed only after retries, and enough of those across a large
source spends the 10s budget while every write lands. Corrected in the same commit.

`f8eb8164`, one commit, nothing else in it.

## How it turned up: a host-side test harness

Five behaviours went in last round on reasoning alone, because no card either of us has can produce them.
That was uncomfortable, and the pattern in what your reviews keep finding is consistent — not crashes, but
reports that are wrong. So I built something to stage them.

`iso15693_poller.c` compiles unmodified into a host test binary: the test file `#include`s the `.c` so the
file-statics are reachable, and the SDK calls resolve to a programmable fake tag ahead of the firmware
headers on the include path. No seam, no `#ifdef TEST`, nothing added to the shipped code. A fake tick
counter makes the wall-clock cases deterministic — "10 seconds elapsed" becomes "the sweep did N
operations".

38 tests over the three decision points: the wipe sweep, the clone loop, and
`iso15693_poller_success_or_partial`.

The fake's semantics are read out of the firmware rather than guessed, which matters more than the tests:

- a refused write returns `Iso15693_3ErrorInternal` — `iso15693_3_i.c:42`, five block errors map to it
- activation's block read breaks at the first failure and the result is then filtered to success, because
  `iso15693_3_poller_filter_error` maps `Timeout` and `NotSupported` to `None` — `iso15693_3_poller_i.c:22,104`
- `iso15693_3_get_block_data` does `furi_check(block_count > block_index)` — `iso15693_3.c:368`

That last chain is the one the sweep's read-back note depends on, and it holds. Also worth knowing: the
whole `lib/nfc/protocols/iso15693_3/` directory is byte-identical between Momentum 87.15 and Unleashed
88.2, so one model covers both API versions.

Where the five reasoned-only behaviours now stand:

| behaviour | now |
|---|---|
| tail-drop fix's positive case (a dead stretch inside the claimed range) | tested |
| the capacity gate's discriminating case (a failed block that answers a read) | tested |
| a clone cut by the clock with the card present | tested — and it was wrong; that's the fix above |
| a truncated sweep reporting as Partial | tested at the poller; the screens are not |
| `uid_verified` false | partly — that it doesn't downgrade the outcome is tested; the path that sets it isn't |
| the gen1 path | still needs a card |

Two tests exist only to defend decisions that look like bugs: `uid_verified` is deliberately absent from
the Partial list (making it Partial would downgrade every wipe where the user lifts the card as it
completes), and the all-rejected `Fail` guard is clone-only (a wipe's failed and accepted sets are
disjoint, so `failed_count == blocks_total` there is a real partial wipe). Both would be tidied away by
someone who hadn't read the reasoning. They now fail loudly.

The outcome contract came out clean — 14 cases, no changes. Given it was consolidated out of four commits
last round when the model changed mid-flight, that seemed worth reporting as a result rather than silence.

**This is not in the PR.** It lives in a dev-only `tools/` directory that the fork sync deliberately
excludes, so none of it ships or affects the FAP build. If you'd want it in-tree I'm happy to bring it as
a follow-up PR — no app in `base_pack` has tests today, so that's your call and the repo owner's, not
something to slip into this branch. If you'd rather it stayed out, the fix above stands on its own and I'll
keep the harness on my side.

## Confirming a deliberate choice: the clone has no up-front confirm

Raising this because it reads as an oversight if you come across it cold, and it isn't one.

An ISO15693 clone goes straight to the write (`file_select.c:106`). Nothing is written until the gen2 UID
reads back as the target, so a tag that isn't magic walks away untouched — and the shared confirm's text is
a *bricking* warning ("On some cards it may not be rewritten") that doesn't describe anything gen2 ISO15693
does.

That follows the closest analogues rather than departing from them:

| flow | gate before writing |
|---|---|
| Gen2, Classic | none by default; specific card-derived warnings when there is something real to say — `gen2_write_check.c:24` returns straight to the write when the checks find nothing |
| **ISO15693 clone** | none by default; the gen1 opt-in when there is something real to say |
| Gen1, Gen4, USCUID-UL | a static blanket confirm, same text whatever the card is |

The app already has two patterns, and ISO15693 follows the one that fits it: consent deferred to the moment
a destructive path becomes real, naming the actual consequence, rather than a fixed warning shown
regardless of the card.

**I think that's right.** The one thing I'll flag against myself is that an ISO15693 *wipe* prompts while a
clone doesn't, though both replace what is on the card. My reading is that a wipe's only product is
destruction, whereas a clone leaves the card holding an image the user picked and chose a file for — and
that reasoning is now written into the comment and the changelog instead of being left implied.

So: confirming rather than asking you to adjudicate. If you'd rather the clone gated, it's a small change.

## Still deferred, not forgotten

The two simplifications from round 3 are done locally but held until this round is settled, because both
move a lot of lines in the files you anchor most of your comments to:

- the `{reason, title, body}` table for `nfc_magic_scene_iso15693_write_fail.c` — twelve render branches now
- the comment cut, under the ownership model from your round-3 note

Also still open from your round-2 list: the duplicated bitmap set/clear. You flagged it alongside the retry
loop but proposed no helper for it, and it's six one-line sites in the file you comment on most, so I left
it with the two above rather than widening the diff mid-review. The retry loop and the
is-buffer-all-zero copies **are** done locally — four copies of the latter, not the three you counted;
`iso15693_poller_block_held_data` was the fourth.
