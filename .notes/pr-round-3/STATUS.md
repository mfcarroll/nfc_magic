# His Round 4 review — ANSWERED, PUSHED AND POSTED 2026-08-11

> **SUPERSEDED for current state.** This is the historical record of the Round 4 exchange. Since it was
> written, Pass C was taken, a further defect was found and pushed, and a host test harness was built.
> **[../NEXT-SESSION.md](../NEXT-SESSION.md) is the authority on where things stand** — in particular the
> "Next" section below is spent. What remains useful here is the coverage map against his 24 inline
> comments, the hardware results, and the conventions.

Directory naming is misleading: this is `pr-round-3/` but holds what **he** numbers Round 4, and our
reply to it lives in `pr-round-4/`. See [../pr-rounds.md](../pr-rounds.md).

Read [PLAN.md](PLAN.md) for the analysis and [received/](received/) for his review verbatim.

## Where things are

- Dev `iso15693-dev`: **12 code commits** on `a3e13a3a`'s content, plus `.notes` commits. Clean.
- Fork `nfc-magic-iso15693` = `688614e8`, **PUSHED** (fast-forward from `a3e13a3a`, no force). Reply and
  eight threaded replies posted, all verified byte-identical to the drafts. Always reset to
  `origin/nfc-magic-iso15693` before replaying — resetting to `d659a919` silently drops already-pushed
  commits and turns the next push into a force-push over his review threads.
- Builds clean at Momentum 87.15 and Unleashed 88.2, zero warnings, clang-format applied.
- **Superseded:** work has continued past this round. For current state read
  [../NEXT-SESSION.md](../NEXT-SESSION.md) — this file is the record of the Round 4 exchange, not a
  live status.

## Coverage against his 24 inline comments

All addressed except two he agreed could wait. Commit SHAs are dev-side; the fork carries one commit
per dev commit with an `NFC Magic ISO15693:` prefix.

| his comment | commit subject |
|---|---|
| `:859` **blocking**, tail-drop vs the advertised-count floor | don't drop claimed blocks the cache proves held data |
| `write_fail.c:344` details gate (asked for this pass) | one predicate for the details route |
| `:978`, `write_fail.c:126`, `:196` truncation on every screen | decide and report a wipe's outcome in one place |
| `:1125`, `:1194`, `write_fail.c:307` uid_verified + retry | same |
| `poller.h:24`, `app_i.h:129`, `:260`, `:695`, `poller.h:209` | same |
| `:532` clone data-pass clock | bound the clone's data pass by the clock too |
| `:62`, `:137`, `write_fail.c:70` the three figures | correct three figures that were estimated rather than read |
| `:853` tail-drop premise, `:866` "every exit" | time the sweep on every exit, including a lifted card |
| `CHANGELOG:39` | changelog says which mismatches are faults and which aren't |
| `scene_write.c:519` Back comment's wrong claims | correct the Back comment's supporting claims |
| `:741` three copies of the rule (asked for this pass) | one statement of the block-answered rule, not three |
| **`:711` / `:799`** hoist ticks, name the off-by-one | **deferred, Pass C** |
| **`scene_write.c:396`** two locals | **deferred, Pass C** |

Round-1 leftovers also cleared this round: the capacity claim on evidence, the block-size clamp and
re-probe by content, and four remaining drift claims.

**Commit hygiene was audited mechanically** -- for each pair, does a later commit remove a line an
earlier one in the batch added? That found 39 lines of churn: the truncation screens, uid_verified and
the contracts were each written against "a cut sweep is a qualified Success", then rewritten when that
model changed. Those four are now one commit, since they are one decision. Three overlapping lines
remain, both benign. **Re-run that audit before any future push** -- the script is in the round-4 notes
history, or reconstruct it: diff each commit with `--unified=0`, collect added and removed line text per
commit, and look for a later commit removing what an earlier one added.

## Verified on hardware

- 70/64 clone → Partial, 6 blocks named, "Card too small" — correct, the card genuinely is too small.
- 70/64 wipe → "Wipe complete / Cleared 64 blocks. / Card claims 70.", no spurious UID caveat.
- Clone with the card lifted → "Card removed before the write could finish."
- Wipe with the card lifted → timing line now emitted on **both** card-lost exits (above and below the
  advertised count), attempted − cleared == 8 on all runs.

## NOT verified — reasoned only (TRUE AS OF ROUND 4; four of these are now tested)

Needed a card that refuses a write while still answering a read, which nobody has:

- the blocking fix's positive case (a dead stretch inside the claimed range) — **now tested**
- every truncated-sweep screen, and truncation reporting as Partial — **the reporting is now tested; the
  screens are not**
- `uid_verified` false — **now tested**
- the capacity gate's discriminating case (a failed block that answers a read) — **now tested**
- a clock-cut clone with the card still present — **now tested, and it was WRONG; see the fix on the PR**

Closed by the host harness in `tools/hosttest`, not by hardware. See
[../test-bench-idea.md](../test-bench-idea.md) and `tools/hosttest/README.md`.

## Next — SPENT, see ../NEXT-SESSION.md

Kept for the record. Everything below was done: Pass C items 1-4 are committed locally, items 5 and 6 are
still held, and `.notes/pr-round-5/` now exists holding the interim comment already posted and the gen3
note.

Pass C, whenever it is taken: the two deferred simplifications above, plus the round-1 queue — the duplicated retry loop
   (he called it non-optional), the `{reason, title, body}` render table, the duplicated confirm scene,
   and the comment cut. File is ~42% comment against ~10% for the other pollers.

## Conventions carried forward

Same three as round 2 (see `../pr-round-2/README.md`), plus: **don't leak process into artifacts that
describe the present** — no "no longer" in a CHANGELOG for an unshipped feature, no "this used to be
duplicated" in a comment. The rule belongs in the comment, the incident in the commit message.
