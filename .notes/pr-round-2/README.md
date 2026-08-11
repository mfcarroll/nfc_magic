# PR #250, review round 2 — what's queued to post

mishamyte reviewed at 2026-08-05 with **CHANGES_REQUESTED**: three items marked Blocking, plus
worth-fixing, documentation-drift and simplification lists. This directory is everything written in
response and **not yet posted**. Dev-only — `tools/sync-to-fork.sh` never syncs `.notes/`.

## What's here

| File | Goes to |
|------|---------|
| [pr-reply-body.md](pr-reply-body.md) | The top-level comment on PR #250 |
| [pr-inline/`<id>`.md](pr-inline/) | Replies in his inline threads, one per file, named by GitHub comment id |
| [issue-1-unaddressed-frames.md](issue-1-unaddressed-frames.md) | New issue — unaddressed write/inventory frames hit a bystander tag |
| [issue-2-poller-timeouts.md](issue-2-poller-timeouts.md) | New issue — Gen2/USCUID don't report a card removed mid-write |
| [issue-3-write-check-back-loop.md](issue-3-write-check-back-loop.md) | New issue — Back during a Gen2 write is inescapable |
| [check-shas.py](check-shas.py) | Verifies every cited fork SHA still exists. Run before posting. |
| [link-issues.py](link-issues.py) | Substitutes real issue numbers for the `#ISSUE-*` tokens, once the issues are filed. |
| [post-pr-replies.sh](post-pr-replies.sh) | Posts the threads then the body. Prompts for `post` first. Issues are filed by hand. |
| [received/](received/) | His review verbatim — body + all 16 inline comments |

## Thread map

Seven of his sixteen inline comments get a reply. Five are fixed and resolvable; two are **not** fixed
but get replies anyway, because this round changed what they're worth.

| Comment id | Anchored at | Reply says |
|---|---|---|
| 3719022267 | `iso15693_poller.c:897` | Blocking 1 — fixed |
| 3719022269 | `iso15693_poller.c:730` | Blocking 2 — fixed |
| 3719022271 | `iso15693_poller.c:786` | Blocking 3 — fixed |
| 3719022293 | `iso15693_poller.c:657` | Wall-clock bound — fixed |
| 3719022325 | `CHANGELOG.md:85` | Stale "and the UID is unchanged" — fixed |
| 3719022273 | `iso15693_poller.c:762` | **Not fixed.** The blocking-2 fix made this one worse — re-weigh it |
| 3719022322 | `scenes/nfc_magic_scene_iso15693_write_fail.c:131` | **Not fixed.** His count of 10 render branches is now 11 |

The other nine belong to later passes and get no "deferred" reply — the body lists what each pass covers.

## Order of operations

Issues first, PR comment last — the comment links them by number, so it can only be written once they
exist.

1. `./check-shas.py` — the stack has been restructured several times; this fails if any cited fork SHA
   no longer exists. Run after any rebase.
2. **Push the fork branch.** The replies cite fork SHAs; they are dead links until it is up.
3. **File the three issues by hand**, in manifest order, so labels can be set: unaddressed frames,
   poller stall, write-check Back loop. Note the three numbers.
4. `./link-issues.py <frames#> <stall#> <backloop#>` — substitutes the real numbers for the
   `#ISSUE-FRAMES` / `#ISSUE-STALL` / `#ISSUE-BACKLOOP` tokens.
5. `./assemble-review.py` and re-read the substituted lines.
6. `./post-pr-replies.sh` — the seven threaded replies, then the top-level comment. It refuses to run
   while any token is unsubstituted or any SHA is stale.

The two issue bodies cross-reference each other, so their tokens can only be resolved after both are
filed. Step 4 rewrites the local drafts; paste those two bodies back into the filed issues if you want
those links live. The PR comment is the one that matters, and it is posted after the substitution.

## Conventions worth keeping

- **Own what he saw; don't narrate drafts he didn't.** The severity misjudgement and the sweep
  adjudication both shipped — into commit messages and a previous reply — so both are owned in the
  relevant threads. Draft revisions he never saw (an earlier 6s time bound, three wrong models of the
  poller-stall) are not mentioned: confessing to those performs diligence rather than exercising it.
- **No internal shorthand.** `B1` / `W4` / `S2` are ours, from the working brief; they appear nowhere in
  his review. Every item names itself.
- **Claims about runtime behaviour are hardware, not reasoning.** The poller-stall section went through
  three wrong code-reading models before it was tested on five card types. Reading generates the
  prediction; the test is what gets written up.
