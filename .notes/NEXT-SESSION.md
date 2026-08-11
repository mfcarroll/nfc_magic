# Next session — start Pass C (simplification)

## Where things stand

PR #250, `nfc_magic_dev` on branch `iso15693-dev`. His Round 4 review is **answered, pushed and
posted** — fork `nfc-magic-iso15693` = `688614e8`, 12 commits, reply and 8 threaded replies live.
Awaiting his Round 5.

Read first: [pr-rounds.md](pr-rounds.md) (directory names are NOT his round numbers), then
[pr-round-3/STATUS.md](pr-round-3/STATUS.md) (coverage, what is verified vs reasoned-only, and the
pre-push checks). His Round 4 verbatim is in `pr-round-3/received/`.

## Before starting: is now the right time?

**Pass C is local and unpushed, always.** He is reviewing `688614e8`; pushing into a line-anchored
review moves the code under him and undoes the separation he asked for.

More than that, the order matters. He has found something in all four rounds so far, so Round 5
requesting changes is the likely case -- and if Pass C is already applied locally, his fixes land on
refactored code and he reviews a diff containing both. That is the mixing he asked us to avoid, just
deferred.

So: **if Round 5 has not arrived, prefer waiting.** If it has arrived and requests changes, do those
first as their own round, push them, and take Pass C after. Starting Pass C early only pays if he is
slow, and costs a rebase if he is not.

If starting anyway, the four items below are the ones least likely to collide with whatever he says.

## The task

Pass C, the simplification he has been deferring since Round 3. Nothing here changes behaviour.

**Do these now** — isolated, and he asked for the first:

1. **The duplicated retry loop.** Character-identical between the clone and the wipe. He calls this one
   non-optional: the copies drifted once and shipped a bug. Also duplicated: is-buffer-all-zero (three
   sites) — one `iso15693_write_block_retried()` plus one `iso15693_block_is_empty()`.
2. **`iso15693_poller.c`, the sweep's two minor items** — hoist `furi_ms_to_ticks(...)` out of the loop
   (recomputed up to 256 times from compile-time constants; keep the elapsed-form comparison, an
   absolute deadline is not wraparound-safe), and name the off-by-one:
   `const bool claimed_range_attempted = (block + 1 >= advertised);`
3. **`nfc_magic_scene_write.c:396`** — two locals so the gate and the reason read off the same fact.
   Do NOT reuse `nfc_magic_scene_write_is_wiping()`; it is also true for `uscuid_ul_is_wipe_mode`.
4. **`nfc_magic_scene_iso15693_write_confirm.c`** — 68 lines, character-identical to the shared
   `nfc_magic_scene_write_confirm.c` bar two strings, a button label and a text-box height.

**Hold these until his Round 5 has been answered**, and say so in the reply so he knows they are not
forgotten:

5. The `{reason, title, body}` table for `nfc_magic_scene_iso15693_write_fail.c` — twelve render
   branches now. Leave the eleven `const bool`s at the top alone; the table deletes them.
6. The comment cut, under his ownership model: the event enum owns the outcome contract, the
   `ISO15693_MAGIC_BLK_*` defines own the wire facts, `gen1_optin.c`'s strings own the user-facing gen1
   consequence, `ISO15693_POLLER_WIPE_MAX_BLOCKS` owns the sweep rationale. Everything else
   cross-references.

Reason for holding: his reviews are line-anchored, and both move a lot of lines in the files he is most
likely to comment on. If his review arrives after they land, every comment he anchored points at code
that no longer exists.

## Rules that cost us real time this round

- **A comment earns its place only if it records something the code cannot show AND is not already
  stated elsewhere.** The file is ~42% comment against ~10% for `gen2_poller.c` and
  `uscuid_ul_poller.c`. Report added/removed comment vs code per commit; a commit adding more comment
  than code is going the wrong way. Pass C should be strongly net-negative on comments.
- **Do not leak process into artifacts that describe the present.** No "this used to be duplicated" in
  a comment, no "no longer" in a CHANGELOG for an unshipped feature. The rule goes in the comment, the
  incident in the commit message.
- **Audit commit hygiene before pushing.** For each pair of commits, does a later one remove a line an
  earlier one in the batch added? Diff each with `--unified=0`, collect added/removed line text per
  commit, compare. Found 39 lines of churn last round. He reviews commit by commit and has flagged
  intra-batch churn twice.
- **Never reset the fork to `d659a919`.** Always reset to `origin/nfc-magic-iso15693` before replaying,
  then verify `git merge-base --is-ancestor origin/nfc-magic-iso15693 HEAD`. Resetting to the old base
  silently drops pushed commits and turns the next push into a force-push over his review threads.
- **Use exact-match, asserted string replacements when editing.** Index-based slicing broke a file
  mid-edit, and an unasserted replacement silently no-matched and cost a build cycle.
- **`cd` in a compound shell command aims the git commit at the wrong repo.** Keep them separate.

## Mechanics

- Build: `cd ../Momentum-Firmware && FBT_NO_SYNC=1 ./fbt fap_nfc_magic_dev` (API 87.15).
  CI parity: rsync into `../unleashed-firmware/applications_user/nfc_magic_dev` then `./fbt
  fap_nfc_magic_dev` (88.2). Both must be warning-free.
- Format: `../Momentum-Firmware/toolchain/current/bin/clang-format
  -style=file:../Momentum-Firmware/.clang-format -i <files>`
- Fork sync: `SYNC_SRC=<dev-sha> tools/sync-to-fork.sh ../all-the-plugins`, cwd inside the dev repo,
  one fork commit per dev commit, subject `NFC Magic ISO15693: <subject>`. Skip `notes:` commits.
- **The user pushes and posts. Never push the PR branch without an explicit go-ahead.**
- Hardware: one gen2 ISO15693 magic card. No gen1 card exists on either side.

## What a bench cannot reach

Five behaviours shipped this round on reasoning alone — they need a card that refuses a write while
still answering a read. Listed in `pr-round-3/STATUS.md`; the idea for closing that gap is in
[test-bench-idea.md](test-bench-idea.md), where host-side tests of the sweep's decision logic look more
tractable than tag simulation. Pass C is a good moment to start it, since a behaviour-preserving
refactor is exactly what regression tests are for.
