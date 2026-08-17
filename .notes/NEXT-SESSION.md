# Next session — the clock-cut fix is PUSHED; Pass C is done locally; await his Round 5

## Where things stand

PR #250, `nfc_magic_dev` on branch `iso15693-dev`. His Round 4 review is answered, pushed and posted,
and since then **one further fix has been pushed and an interim comment posted** (2026-08-12 01:14Z).

Fork `nfc-magic-iso15693` = **`f8eb8164`**, 13 commits. Awaiting his Round 5, which will be his first
look at that fix.

- The pushed commit is the clock-cut capacity fix — see [pr-round-5/interim-comment.md](pr-round-5/interim-comment.md)
  for exactly what was said about it, the harness, and the clone-confirm position.
- **It is unsigned, deliberately left so.** Signing needed a machine with Touch ID; correcting it after
  the fact would mean force-pushing the PR branch, which is not worth a signature — especially as the
  commits are expected to be squashed at merge, which discards per-commit signatures anyway. The other
  13 fork commits are signed; this one is the odd one out and that is a closed decision, not a to-do.
- This machine's fork checkout is **in line** with origin at `f8eb8164`. Nothing to reset.

**One shipped-code commit is queued to go up with the Round 5 response:** `fe7305d`, a CHANGELOG line
declaring that gen3 magic is not supported. Deliberately not pushed on its own — see
[pr-round-5/gen3-note.md](pr-round-5/gen3-note.md) for what it covers, what it omits and why, and what to
do if he asks for gen3 support. It is the only shipped-code change since the fix; everything else is
`tools/` and `.notes/`.

**Pass C items 1-4 remain committed locally and NOT pushed**, plus one prose fix found while verifying
item 4 on hardware. Five code commits, all still local:

| item | commit | comment | code |
|---|---|---|---|
| 1. retry loop + empty-block test | `2816bb5` | −2 | −13 |
| 2. hoist ticks, name the off-by-one | `15eea79` | +2 | +2 |
| 3. success route's two locals | `015d8cd` | +1 | −1 |
| 4. fold the confirm scene | `04d392d` | +3 | −39 |
| — the clone's missing-confirm claim | `d921055` | +3 (+2 CHANGELOG) | 0 |

Batch: comment +9, code −51. Both firmwares rebuilt clean from scratch (85 objects, Momentum 87.15 and
Unleashed 88.2), zero compiler warnings — the only two warnings are pre-existing fbt manifest
complaints about Momentum's unrelated `cli_bridge` and `mtp` apps. Churn audit: **0 lines** written
then rewritten inside the batch.

The branch is ~25 commits ahead of `origin/iso15693-dev` (itself never pushed this round). **Six** of
those are shipped code, in two groups, and none is up yet: the five Pass C commits above, and `fe7305d`.
Everything else is `tools/` or `.notes/`. Regenerate that classification rather than trusting this line:

```bash
for c in $(git log --format=%h 83e90cb..HEAD); do git show --stat --format= --name-only $c \
  | grep -qvE '^(tools/|\.notes/)' && echo "$c SHIPPED $(git log -1 --format=%s $c)"; done
```

Read first: [pr-rounds.md](pr-rounds.md) (directory names are NOT his round numbers), then
[pr-round-3/STATUS.md](pr-round-3/STATUS.md) (coverage, what is verified vs reasoned-only, and the
pre-push checks). His Round 4 verbatim is in `pr-round-3/received/`.

## Sequencing from here

**Pass C is local and unpushed, always.** He is reviewing `f8eb8164`; pushing into a line-anchored
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

## Raise in the reply: the clone has no consent screen on the happy path

Found by using the app, not by reading it — the operator expected a warning before an ISO15693 clone and
got none. It is **not** a regression: `20649a5` added the skip deliberately, and `CHANGELOG.md:61` has
declared it for two rounds. Before that commit ISO15693 fell through to the shared confirm, which is why
it feels like it used to be there. It did.

`d921055` corrects the prose. What is left is a design position worth stating rather than waiting for him
to find:

> On the happy path — a genuine gen2 magic card — there is no consent screen anywhere in the ISO15693
> clone. The gen1 opt-in fires only when gen2 *fails* to take the UID, so the one case where the card's
> contents are certainly replaced is the case with no gate.

Put both sides in. **For:** nothing is written until the UID reads back as the target, so a non-magic tag
is untouched; the shared confirm's text is a *bricking* warning ("may not be rewritten") that does not
apply to gen2 ISO15693; and Gen2/Classic reach the write unprompted too when their checks find nothing.
**Against:** an ISO15693 wipe prompts and a clone does not, though both destroy what is on the card.

The answer we settled on is that the asymmetry is right — a wipe's only product is destruction, a clone
leaves the card holding a chosen image — and the behaviour stays. **The gen1 opt-in is not up for
discussion; it carries the real consent and stays exactly as it is.** Offer the asymmetry up anyway; four
rounds say he finds this class of thing, and it lands better volunteered.

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
- **Ask before the first edit to shipped code that is not on the agreed list.** A bug you find while
  doing something else is a FINDING TO REPORT, not a licence to change the diff the reviewer is reading.
  This cost real trust on 2026-08-11: the host harness turned up a genuine "Card too small" defect and it
  was fixed in `iso15693_poller.c` without asking, during a task whose whole premise was that it touched
  no app code. The fix was correct and the decision was not ours to make. Report it, recommend it, wait.
- **Classify every commit as shipped-vs-dev-only in the message where you report it, up front.** Anything
  outside `tools/` and `.notes/` is shipped code and gets named explicitly, never left to a tail
  paragraph. Use THIS form -- pure git, no grep:
  ```bash
  for c in $(git log --reverse --format=%h <range>); do
    f=$(git show --pretty= --name-only $c -- ':(exclude)tools' ':(exclude).notes')
    [ -n "$f" ] && echo "SHIPPED $c $(git log -1 --format=%s $c)"
  done
  ```
  **The old `| grep -qv '^tools/'` form gives WRONG ANSWERS in this environment and was used to report
  to the user on 2026-08-16.** `grep` here is a shell function wrapping `ugrep`, whose `-q` combined
  with `-v` returns 1 even when non-matching lines exist -- so commits touching `magic/` and `scenes/`
  were reported as dev-only. It fails silently and plausibly, which is the worst shape. Never use
  `grep -q` for a decision in this repo; use `grep -c` and test the count, or avoid grep as above.
  The failure this prevents is not a wrong commit, it is a report the user cannot check at a glance --
  which is what turns one unasked change into "did you also push?".
- **Never give a push command in the `HEAD:branch` form.** Always name the SHA:
  `git push origin <sha>:refs/heads/<branch>`. On 2026-08-11 the fix was signed in a clone on a second
  machine (Touch ID does not work over VNC) while this machine's fork checkout held the unsigned version
  at the same branch name. `git push origin HEAD:nfc-magic-iso15693` was valid on BOTH and pushed a
  different commit depending on where it ran -- so the unsigned one landed on the PR branch and had to be
  accepted, because undoing it meant force-pushing a live review. A SHA-explicit refspec cannot do that.
  It also fails loudly instead of silently when the intended commit is not present locally.
- **Signing only works from `/Users/Shared/code/.gitconfig-base` and `.gitconfig-personal`**, so a fresh
  clone anywhere else falls back to GPG and fails with "No secret key". For an out-of-band signing run,
  set repo-locally: `gpg.format=ssh`, `gpg.ssh.program=/Applications/1Password.app/Contents/MacOS/op-ssh-sign`,
  `user.signingkey=ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIDrbH2qYmg+qPbMKs34sLMke+K/csgeWr8lymyDTh7P1`,
  plus `user.email`/`user.name` -- `--amend` rewrites the COMMITTER, and GitHub only badges Verified when
  that address is verified on the account.
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
  `1Password: failed to fill whole buffer` / `failed to write commit object`, and over VNC it fails with
  `agent returned an error` because Touch ID cannot be reached. Retrying does not help. Ask the user to
  unlock; do NOT extract the key from 1Password to sign with it directly, and do not silently disable
  signing. If they cannot sign, `git -c commit.gpgsign=false commit` is the stopgap — it leaves their
  config untouched so signing resumes by itself.
  **Note the dev branch is currently unsigned from `b921c69` onward**: the reorder rewrote every commit
  while signing was unavailable. It is private working history so it matters little, and
  `git rebase --exec 'git commit --amend --no-edit -S' 83e90cb` re-signs the run — safe precisely because
  none of it is pushed.

## Hardware checks owed for Round 5 — NOT DONE, and mostly NOT REACHABLE as-is

Round 5's headline changes are all gated on `pass_truncated`, and a healthy gen2 card never truncates:
a 64-block sweep takes ~1s against a 10s budget. So the new screens cannot be reached by using the app
normally. They are covered by the host tests instead, which is exactly what the harness exists for --
but that is source-level coverage, not a rendered screen.

**To reach them on the card we have, temporarily lower the budget.** Set
`ISO15693_POLLER_PASS_MAX_MS` to ~200 in `iso15693_poller.c`, build, flash, test, then REVERT before
pushing. A healthy card is then cut a dozen or so blocks in and every truncation path renders.

| # | with the budget lowered | expect |
|---|---|---|
| 1 | ISO15693 -> Wipe | title `Wipe stopped`; body `Cleared N blocks.` / `Stopped at block N.`; buttons **Retry** and **Details** (NOT Exit) |
| 2 | press Details on it | scroll view; `Sweep hit its time limit at block N of the M this card claims. Blocks above that were never attempted and may still hold data.` |
| 3 | press Back on it | leaves to the ISO15693 menu. This is the exit now -- there is no Exit button |
| 4 | press Retry on it | re-runs the wipe |
| 5 | ISO15693 -> clone a saved image | `Clone partial`; a `Stopped at block N` qualifier line; **Retry** offered, not Finish |
| 6 | Details on that clone | the block list stops at the cut, and the note reads `Clone hit its time limit at block N. Blocks from there up were never sent to the card -- they are counted as not written, but the card did not refuse them. Running the clone again writes them.` |

Watch the **line width** on 1 and 5: `Stopped at block %u.` replaced `Stopped at %u of %u.` and should
be no wider, but it has not been seen rendered.

**The regression half is DONE — all five passed, 2026-08-17**, on the 70/64 gen2 card against this
delta (not cited from Round 4):

| # | test | result |
|---|---|---|
| 7 | 70/64 clone | `Clone partial` / `Cloned 64/70 blocks` / `Not written: 6` / `Card too small`; **Finish** + Details -- confirms an uncut partial is still non-retryable after `is_retryable` learned to read the flag |
| 8 | Details on it | `Blocks not written` / `64 65 66 67 68 69` -- all six, so `list_upto` does not clip an uncut run |
| 9 | clone, card lifted | `Card removed before the write could finish.`; **Retry + Exit** -- the right-slot rule correctly yields Exit where `has_details` is false |
| 10 | 70/64 wipe | `Wipe complete` / `Cleared 64 blocks.` / `Card claims 70.`; Finish only -- the 8-block phantom tail still drops after `i < advertised` came out |
| 11 | wipe, card lifted | `Card removed...`; Retry + Exit |

Also observed and correct: the wipe's progress popup ends at **70/70** while the result says 64 cleared.
The denominator is the advertised count and blocks 64-69 were genuinely attempted, so "70 of 70
attempted" is true. Operator read it as covering the claim, which is what it means. Not a defect, not
touched this round -- `iso15693_poller.c:900` gates progress on `block < advertised` by design.

## Backlog — needs cards we do not have yet

- **The six truncation screens (1-6 above).** Need the lowered-budget build. Not blocked on cards.
- **`source_uses_gen1_blocks`** -- the one backdoor-predicate site the gen2 card cannot reach, since it
  sits behind the gen1 opt-in and so needs a card that FAILS gen2. **Any ordinary ISO15693/NfcV tag
  does it** (on order as of 2026-08-17): select a source with data in 56/57/62/63, present the plain
  tag, reach the gen1 opt-in, confirm the extra warning renders -- then **press Back, do NOT accept**.
  Accepting writes the gen1 sequence into four blocks of an ordinary tag and destroys what is there.
- **Other gen2 magic silicon** (inbound) -- everything verified so far is one sample. Re-run 7-11.
- **gen1 magic candidates** (inbound, unconfirmed as gen1). What they would settle is unchanged: the
  armed-card wipe hazard, the unlock/commit reading inferred from proxmark's send order, and the UID
  re-read that reports a change without preventing one. He has said explicitly not to hold the merge
  for these, and we agree.

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

## The host test harness — READ THIS BEFORE TOUCHING THE POLLER

`tools/hosttest`, **55 tests, dev-only**. Full detail in [../tools/hosttest/README.md](../tools/hosttest/README.md).

```bash
cd tools/hosttest && make
```

It compiles the shipped `iso15693_poller.c` **verbatim** — the test files `#include` the `.c` so the
file-statics are reachable, and the SDK calls resolve to a fake tag via `-Ifakes`. No seam, no
`#ifdef TEST`, nothing added to the app. Four files: the wipe sweep (13), the clone loop (13), the
terminal-outcome contract (14), and the write state machine (15, driving the real poller callback the way
the SDK does).

**Three things that matter more than the test count:**

1. **Run it after ANY change to the poller, and before claiming anything about coverage.** It found a real
   defect on its first run over the clone loop — the clock-cut "Card too small" claim, now `f8eb8164` on
   the PR.
2. **If Round 5 asks for poller changes, update the tests in the same commit.** A test that still passes
   because it was never updated is worse than no test — it reads as coverage and isn't.
3. **The fake's semantics were verified against firmware source, with citations in the README.** If you
   change the fake, re-check them; a wrong fake asserts wrong behaviour confidently.

Where the previously reasoned-only behaviours now stand — five of six tested, one honestly out of reach:

| behaviour | now |
|---|---|
| tail-drop fix's positive case | tested |
| capacity gate's discriminating case | tested |
| clock-cut clone, card present | tested — and it was wrong; that is the fix on the PR |
| truncated sweep reporting Partial | tested at the poller; the *screens* are not |
| `uid_verified` false | tested |
| the gen1 path | **modelled, not settled** — the latch behaviour is our inference from proxmark's send order, so the tests show the app is right *given the model*, not that the model is. Needs a gen1 card. |

Still uncovered and worth knowing: `write_identity`'s retry mechanics, every scene and its rendering, and
the radio layer below the SDK. [test-bench-idea.md](test-bench-idea.md) has the state of the simulator
idea — Option B is what got built; Option A's better lead is the firmware's own listener, not the
proxmark, with two gates recorded against it.
