# PR #250, round 4 — what's queued to post

His round-3 review (CHANGES_REQUESTED, 2026-08-11) is in [../pr-round-3/received/](../pr-round-3/received/);
the analysis is in [../pr-round-3/PLAN.md](../pr-round-3/PLAN.md) and the coverage map in
[../pr-round-3/STATUS.md](../pr-round-3/STATUS.md). Nothing here is posted.

| File | Goes to |
|------|---------|
| [pr-reply-body.md](pr-reply-body.md) | Top-level comment on PR #250 |
| [pr-inline/`<id>`.md](pr-inline/) | Eight threaded replies, named by his comment id |
| [check-shas.py](check-shas.py) | Verifies every cited fork SHA exists past `a3e13a3a` |
| [assemble-review.py](assemble-review.py) | Rebuilds `REVIEW-ALL.md` from the drafts |
| [post-pr-replies.sh](post-pr-replies.sh) | Posts the threads then the body; prompts for `post` |

## Threads that get a reply

Eight of his twenty-four. The rest are covered by the body, or are the two simplification items he
agreed could wait.

| id | anchored at | reply says |
|---|---|---|
| 3756085157 | `iso15693_poller.c:859` | blocking item fixed with his discriminator — and his sketch was one step short |
| 3756085164 | `:978` | fixed; used "at N of M" rather than his "Stopped: time limit" |
| 3756085182 | `:532` | fixed; his "unreached blocks are already in the bitmap" was not so |
| 3756085187 | `:1125` | fixed; and why this stays Success while a cut sweep becomes Partial |
| 3756085228 | `scenes/nfc_magic_scene_write.c:519` | both wrong supporting claims corrected |
| 3756085255 | `:741` | helper extracted, as asked for in this pass |
| 3756085261 | `write_fail.c:344` | predicate done, taken first because the truncation fix needed it |
| 3756085272 | `:866` | fixed — and it exposed an off-by-one of ours |

## Order

1. `./check-shas.py`
2. **Push the fork branch** — 16 commits, a fast-forward from `a3e13a3a`. Verify with
   `git merge-base --is-ancestor origin/nfc-magic-iso15693 HEAD` before pushing; a force-push here would
   destroy round-2 commits his review threads are anchored to.
3. `./assemble-review.py`, re-read
4. `./post-pr-replies.sh`

No issues to file this round.
