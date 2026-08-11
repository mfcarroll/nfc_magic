# Next session — Pass C items 1-4 are DONE locally; await his Round 5

## Where things stand

PR #250, `nfc_magic_dev` on branch `iso15693-dev`. His Round 4 review is **answered, pushed and
posted** — fork `nfc-magic-iso15693` = `688614e8`, 12 commits, reply and 8 threaded replies live.
Awaiting his Round 5.

**Pass C items 1-4 are committed locally and NOT pushed** (2026-08-11). Four commits on top of
`83e90cb`, fork untouched:

| item | commit | comment | code |
|---|---|---|---|
| 1. retry loop + empty-block test | `2816bb5` | −2 | −13 |
| 2. hoist ticks, name the off-by-one | `15eea79` | +2 | +2 |
| 3. success route's two locals | `015d8cd` | +1 | −1 |
| 4. fold the confirm scene | `04d392d` | +3 | −39 |

Batch: comment +4, code −51. Both firmwares rebuilt clean from scratch (85 objects, Momentum 87.15 and
Unleashed 88.2), zero compiler warnings — the only two warnings are pre-existing fbt manifest
complaints about Momentum's unrelated `cli_bridge` and `mtp` apps. Churn audit: **0 lines** written
then rewritten inside the batch.

Read first: [pr-rounds.md](pr-rounds.md) (directory names are NOT his round numbers), then
[pr-round-3/STATUS.md](pr-round-3/STATUS.md) (coverage, what is verified vs reasoned-only, and the
pre-push checks). His Round 4 verbatim is in `pr-round-3/received/`.

## Sequencing from here

**Pass C is local and unpushed, always.** He is reviewing `688614e8`; pushing into a line-anchored
review moves the code under him and undoes the separation he asked for.

When Round 5 arrives: **answer it first, as its own round, and push that.** Pass C goes up after, so he
never reviews a diff mixing a fix with a refactor. If his fixes touch the sweep or the confirm scene,
rebasing them over these four commits is cheap — they are small and independent — but the order matters
and it is fixes first.

## What was done, and the three judgement calls worth naming in the reply

1. **The duplicated retry loop** → `iso15693_poller_write_block_retried()`. He named it
   `iso15693_write_block_retried()`; it carries the `iso15693_poller_` prefix instead, to match all
   twelve other statics in the file. Same for `iso15693_poller_block_is_empty()`. **Tell him**, so the
   rename isn't a surprise.
2. **is-buffer-all-zero had FOUR copies, not the three he counted** — the clone's non-empty test, the
   wipe's still-holds-data test, the re-probe's, and `iso15693_poller_block_held_data`. All four now
   call the one helper.
3. **His bitmap set/clear duplication is NOT done.** He listed it (`|=` / `&= ~` at six sites once you
   count the tail loops) alongside the retry loop, but proposed no helper for it, and doing it would
   widen the diff in the file he anchors most of his comments on. It is held with items 5 and 6 below —
   say so rather than letting him find it still there.

Also fixed in passing: `226dd74` (a `notes:` commit from an earlier session) had swept an uncommitted
`scene_write.c` edit into itself. Since `sync-to-fork.sh` skips `notes:` commits, that would have sent
the hunk to the fork without the locals it uses and broken the fork build. Split into `2876ee2`
(notes only, original message and author date preserved) and `015d8cd` (the code).

## Hardware coverage for this batch

**Verified 2026-08-11, Write UID → enter UID → confirm screen** (the one render path item 4 moved):
title `Write UID?`, the UID as two space-separated 4-byte groups, both warning lines, centre button
`Write`. That screen is only reachable from the new `iso15693_write_uid` branch, which sets the title,
body, button label and 38px height together — so a correct title plus a correct label covers all four.

**Verified 2026-08-11, ISO15693 → Wipe → confirm screen**: `Wipe card?`, the three-line body, `Continue`.

**That is full coverage of the fold on the hardware that exists.** Only two routes reach this scene with
an ISO15693 card, and both are checked. An ISO15693 **clone** never reaches it at all —
`file_select.c:106` routes ISO15693 straight to `NfcMagicSceneWrite`, deliberately (the gen2 write is
harmless on a non-magic tag and data blocks land only after the UID reads back as the target, so there
is nothing to confirm up front).

The other two variants — the plain `Risky operation` clone confirm and the USCUID-UL wipe text — need a
**Gen1, Gen4 or USCUID-UL** magic card, and none exists on either side of this PR. They are safe by
construction rather than by test: for a non-ISO15693 protocol `iso15693` is false, so both new
conditions are false and control falls through the same `uscuid_ul_is_wipe_mode` / `else` chain as
before; `title` is the identical `is_wipe ? ... : ...` expression, `confirm_label` stays `"Continue"`,
`text_height` stays 54. Neither path reads a value the fold introduced. Say it that way in the reply
rather than listing them as untested — the argument is stronger than the gap.

Everything else in the batch is non-render code.

## Still held

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
  than code is going the wrong way. The strong comment reduction is **item 6**, which is held — items
  1-4 came out at comment +4 / code −51, and the +4 is three constraints that had nowhere else to live
  (tick wraparound, don't-reuse-`is_wiping`, the widget copies its string so the early free is safe).
  Naming a value is often what makes the wrong refactor look attractive, so that is exactly where the
  constraint has to be written down.
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
- **`touch` does not force an fbt rebuild.** SCons decides by content signature, not mtime, so a
  touched file recompiles nothing and a "warning-free" claim from that run proves nothing. To actually
  recompile, delete the app's object dir: `rm -rf <fw>/build/f7-firmware-{C,D}/.extapps/nfc_magic_dev`.
- **Commit early; another session can commit over your working tree.** An earlier session's `notes:`
  commit swallowed an uncommitted code edit mid-pass. Because the fork sync skips `notes:` commits, that
  silently drops code from the fork while shipping the hunk that needs it. Commit each item as it lands
  rather than leaving the tree dirty across a build.
- **Commits are SSH-signed via 1Password `op-ssh-sign`.** When the vault locks, `git commit` dies with
  `1Password: failed to fill whole buffer` / `failed to write commit object`. Retrying will not help and
  the fix is not to disable signing — every commit on this branch is signed. Ask the user to unlock.

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
