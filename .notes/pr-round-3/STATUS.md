# Round 3 — state as of the end of Passes A and B

Read [PLAN.md](PLAN.md) for the analysis and [received/](received/) for his review verbatim.

## Where things are

- Dev `iso15693-dev`: **12 code commits** on `a3e13a3a`'s content, plus `.notes` commits. Clean.
- Fork `nfc-magic-iso15693`: re-synced, **12 commits fast-forward from `a3e13a3a`**, unpushed. Always
  reset to `origin/nfc-magic-iso15693` before replaying — resetting to `d659a919` silently drops round
  2's pushed commits and turns the next push into a force-push over his review threads.
- Builds clean at Momentum 87.15 and Unleashed 88.2, zero warnings, clang-format applied.
- Nothing posted for this round.

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

## NOT verified — reasoned only

Needs a card that refuses a write while still answering a read, which nobody has:

- the blocking fix's positive case (a dead stretch inside the claimed range)
- every truncated-sweep screen, and truncation reporting as Partial
- `uid_verified` false
- the capacity gate's discriminating case (a failed block that answers a read)
- a clock-cut clone with the card still present

Say this plainly in the reply. See [../test-bench-idea.md](../test-bench-idea.md).

## Next

1. Re-sync the fork (one fork commit per dev commit, `NFC Magic ISO15693: <subject>`).
2. Draft the reply + thread replies into `.notes/pr-round-4/`, same shape as round 2:
   `pr-reply-body.md`, `pr-inline/<comment-id>.md`, `check-shas.py`, `assemble-review.py`,
   `post-pr-replies.sh`. Thread ids are in `received/inline.json`.
3. Push, then post.
4. Pass C: the two deferred simplifications above, plus the round-1 queue — the duplicated retry loop
   (he called it non-optional), the `{reason, title, body}` render table, the duplicated confirm scene,
   and the comment cut. File is ~42% comment against ~10% for the other pollers.

## Conventions carried forward

Same three as round 2 (see `../pr-round-2/README.md`), plus: **don't leak process into artifacts that
describe the present** — no "no longer" in a CHANGELOG for an unshipped feature, no "this used to be
duplicated" in a comment. The rule belongs in the comment, the incident in the commit message.
