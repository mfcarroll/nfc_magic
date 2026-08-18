# Next session — Round 5 is ANSWERED, PUSHED and POSTED; await his Round 6

## Where things stand

PR #250, `nfc_magic_dev` on branch `iso15693-dev`. **His Round 5 is fully answered, pushed and posted
(2026-08-17.)** Nothing is queued and nothing is held back except the two items below.

Fork `nfc-magic-iso15693` = **`64326417`**, 31 commits, all signed and GitHub-verified, pushed
fast-forward from `f8eb8164` with no force at any point. Dev `iso15693-dev` = **`800b9c8`**, 48 commits
in the round, all signed, clean, and **pushed to `origin/iso15693-dev`** as a backup.

What went up, in history order — three groups, each commit one decision:

1. **Pass C 1-4 + the clone-confirm prose fix** (`a157cec` `0c17b2f` `b0a9952` `954a1e2` `71fa6e7`).
   They could NOT be held back: `6eec333` edits `block_held_data`, which calls
   `iso15693_poller_block_is_empty` — a helper Pass C item 1 introduced. 8 call sites at HEAD, 0 at the
   old fork head. The earlier plan to push "Round 5 only" was not achievable and the reply says so.
2. **`fe7305d`**, the gen3 CHANGELOG declaration.
3. **This round's eleven fixes**, `b818ccc` through `27d973c`.
4. **`3c88cf1`**, a twelfth fix from the lowered-budget hardware run — see the follow-up comment.

Two comments were posted in total. The follow-up
([#issuecomment-5322865570](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-5322865570))
carries `3c88cf1`'s two findings; its first version went out before review and was replaced by EDITING
that same comment, so the id is stable and there is no orphaned text on the thread.

Posted: the reply as [#issuecomment-5321680598](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-5321680598),
plus **24 threaded replies covering 24 of his 26 threads**. The two without a reply are the two he
marked as needing none (`write_fail.c:71`, `poller.c:58` — both him recording a verification).
Drafts are [pr-round-5/reply.md](pr-round-5/reply.md) and
[pr-round-5/thread-replies.md](pr-round-5/thread-replies.md); his review verbatim is in
[pr-round-5/received/](pr-round-5/received/).

**Line numbers in his Round 5 comments no longer match the current diff** — `:341` renders at `:377`,
`:855` at `:917`, `:687` at `:752`. The thread IDs are the stable handle and they are recorded beside
each comment in `pr-round-5/received/inline-comments.md`. Use IDs, not lines, when replying.

## What is still held, and why

Only two things, both flagged to him in the reply so they are not surprises:

1. The `{reason, title, body}` render table for `nfc_magic_scene_iso15693_write_fail.c` — twelve render
   branches. Leave the eleven `const bool`s at the top alone; the table deletes them.
2. The comment cut, under his ownership model: the event enum owns the outcome contract, the
   `ISO15693_MAGIC_BLK_*` defines own the wire facts, `gen1_optin.c`'s strings own the user-facing gen1
   consequence, `ISO15693_POLLER_PASS_MAX_MS` owns the run-budget rationale. Everything else
   cross-references.

The bitmap set/clear duplication he listed in Round 4 belongs with item 2, not with fixes — it moves a
lot of lines in the file he anchors most comments on. Told him that.

**The comment cut is now more pressing, not less.** This round put `iso15693_poller.c` at **43%**
comment against ~8% for `gen2_poller.c` and `uscuid_ul_poller.c`. Round 5's own commits were +163
comment / +45 code; the whole push was +170 / −6 because Pass C removed 51 lines. Both figures are in
the reply — do not quote only the flattering one.

## One open question he put back to us

On the `CHANGELOG.md:42` thread he raised, as our call: `blocks_total < advertised` is fake flash **or**
dead memory and the app cannot tell which, yet it resolves that toward reassurance — success chime, the
word "complete". **We did not change it**, deliberately: it is a behaviour change on a hardware-verified
screen, in a delta already large.

The proposal is on record in that thread: use the free third line when `blocks_total < advertised` to say
the shortfall went unexplained ("Some claimed blocks did not answer."), staying out of the `>` case,
which is unambiguously benign. If he says yes it goes in the next push with a hardware check; if he does
not answer, file it so it does not evaporate.

Read first: [pr-rounds.md](pr-rounds.md) (directory names are NOT his round numbers). Round 4 and its
answer are in `pr-round-3/` and `pr-round-4/`; Round 5 keeps both halves in `pr-round-5/`, which is the
shape to copy.

## Settled in earlier rounds — do not re-litigate

- **The clone has no consent screen on the happy path, and that is correct.** He agreed explicitly in
  Round 5: consent deferred to the moment a destructive path becomes real, naming the actual
  consequence, beats a fixed warning describing a hazard gen2 ISO15693 does not have. The wipe/clone
  asymmetry is answered by a wipe's only product being destruction. **The gen1 opt-in carries the real
  consent and does not change.**
- **The host harness is out of this PR**, and whether `base_pack` grows a test directory is xMasterX's
  and mishamyte's call, not ours. He flagged it to them rather than answering for them.
- **Helper names carry the `iso15693_poller_` prefix** even where he proposed a shorter name, to match
  the other statics in the file. He has seen this and not objected.
- **Pass C's confirm-scene fold was hardware-verified 2026-08-11** (Write UID and Wipe confirm screens);
  the plain clone confirm and USCUID-UL wipe text need cards nobody on the PR has, and are safe by
  construction — for a non-ISO15693 protocol the new conditions are false and control falls through the
  pre-existing chain unchanged.

## Rules that cost us real time — cumulative, all rounds

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
  **The whole dev branch is signed as of 2026-08-17**, and so are all 30 fork commits except the one
  deliberate exception (`f8eb8164`, unsigned, a closed decision). `git rebase --exec 'git commit --amend
  --no-edit -S' <base>` re-signs a run and is safe while nothing is pushed; verify with
  `git log --format='%h %G? %s'`, and note `U` (good signature, key not in local allowed_signers) is the
  expected state here, not a problem — GitHub reports `verified: true`.

## Hardware: BOTH halves are done — 2026-08-17

**The card is physically 64 blocks.** "70/64" is its STATE, not its geometry: the advertised count is
whatever the last clone's CFG frame programmed, and the worklog records one physically-64 card
impersonating 28/56/64/70 on demand. **Run the clone before the wipe** -- the wipe reports "Card claims
70" only because the clone left it claiming 70. A past result was traced to exactly this ordering
artifact (worklog 2026-07-27), so it is a trap, not a detail.

**Regression half, at the real budget — all five passed:**

| test | result |
|---|---|
| clone the 70-block source (FIRST) | `Clone partial` / `Cloned 64/70 blocks` / `Not written: 6` / `Card too small`; **Finish** + Details — an uncut partial is still non-retryable after `is_retryable` learned to read the flag |
| Details on it | `Blocks not written` / `64 65 66 67 68 69` — all six, so `list_upto` does not clip an uncut run |
| clone, card lifted | `Card removed before the write could finish.`; **Retry + Exit** — the right-slot rule yields Exit where `has_details` is false |
| wipe (AFTER the clone) | `Wipe complete` / `Cleared 64 blocks.` / `Card claims 70.`; Finish only — the 8-block phantom tail still drops after `i < advertised` came out |
| wipe, card lifted | `Card removed...`; Retry + Exit |

**Truncation half, via a temporary `ISO15693_POLLER_PASS_MAX_MS` of 200 — all six screens rendered**, and
the run found two real defects, now fixed in `3c88cf1`:

- the "Wipe stopped" screen named where it stopped and never said WHY, while offering Retry. Now
  `Timed out at block 23.` — folded into the existing line, because a fourth line at y=13 lands its
  bottom rows inside the button box and this body already reaches three.
- **"Running the clone again writes them" was false.** The bound is a wall clock, not a position, so a
  consistently slow card is cut in the same place every time; only a transient clears on a retry.
  Observed directly — a retried wipe stopped at the same block. Applied to the wipe too, whose Retry
  button came from his Round 4 reasoning.

**How to run the truncation half again:** set `ISO15693_POLLER_PASS_MAX_MS` to ~200 in
`iso15693_poller.c`, `FBT_NO_SYNC=1 ./fbt launch APPSRC=applications_user/nfc_magic_dev` from the
Momentum tree, then **`git checkout --` the file immediately** — the device keeps the installed build, so
the wrong constant never sits in the working tree where a concurrent session could commit it. Reinstall a
clean build afterwards.

Also observed and correct: the wipe's progress popup ends at 70/70 while the result says 64 cleared. The
denominator is the advertised count and blocks 64-69 were genuinely attempted, so "70 of 70 attempted" is
true. `iso15693_poller.c` gates progress on `block < advertised` by design.

Two things still unrendered, both needing a card that answers reads at every address: the cut landing
ABOVE the advertised count, and the `Stopped at 200 of 64`-shaped string that used to produce.

## Backlog — needs cards we do not have yet

- **`source_uses_gen1_blocks`** — the one backdoor-predicate site the gen2 card cannot reach, since it
  sits behind the gen1 opt-in and so needs a card that FAILS gen2. **Any ordinary ISO15693/NfcV tag does
  it** (on order as of 2026-08-17): select a source with data in 56/57/62/63, present the plain tag,
  reach the gen1 opt-in, confirm the extra warning renders — then **press Back, do NOT accept**.
  Accepting writes the gen1 sequence into four blocks of an ordinary tag and destroys what is there.
- **Other gen2 magic silicon** (inbound) — everything verified so far is one sample. Re-run the
  regression five.
- **gen1 magic candidates** (inbound, unconfirmed as gen1). What they would settle is unchanged: the
  armed-card wipe hazard, the unlock/commit reading inferred from proxmark's send order, and the UID
  re-read that reports a change without preventing one. He said explicitly not to hold the merge for
  these, and we agree.
- **The `Timed out at block N.` string has not been seen rendered** — it is one character wider than the
  `Stopped at block N.` that was. Check it on the next truncation run.

## Mechanics

- Build: `cd ../Momentum-Firmware && FBT_NO_SYNC=1 ./fbt fap_nfc_magic_dev` (API 87.15).
  CI parity: rsync into `../unleashed-firmware/applications_user/nfc_magic_dev` then `./fbt
  fap_nfc_magic_dev` (88.2). Both must be warning-free.
- Format: `../Momentum-Firmware/toolchain/current/bin/clang-format
  -style=file:../Momentum-Firmware/.clang-format -i <files>`
- Fork sync: `SYNC_SRC=<dev-sha> tools/sync-to-fork.sh ../all-the-plugins`, cwd inside the dev repo,
  one fork commit per dev commit. Skip `notes:` commits. **The script overlays and NEVER DELETES** --
  so a file removed or renamed in dev must be `git rm`'d in the fork by hand, at the commit that removed
  it. Missed once already: Pass C deleted `nfc_magic_scene_iso15693_write_confirm.c` and the fork would
  have kept a stale copy. Check every sync with:
  `git diff --name-status -M <last-synced-dev-sha>..HEAD -- magic scenes views helpers assets *.c *.h CHANGELOG.md | grep -v '^M'`
- **Fork subject convention: STRIP the dev scope prefix, do not stack it.** Dev subjects are
  `iso15693: ...` / `changelog: ...` / `nfc_magic: ...` / `scene_write: ...`; the fork subject is
  `NFC Magic ISO15693: <subject with that prefix removed>`. Getting this wrong produces
  `NFC Magic ISO15693: iso15693: ...`, which is what **17 of the commits pushed 2026-08-17 read** --
  left uncorrected on purpose, because fixing them means force-pushing over a live review and an ugly
  subject line is worth far less than his review anchors. One sed does it:
  `sed -E 's/^(iso15693|changelog|nfc_magic|scene_write): //'`
- Also adapt the BODY for publication: no third-person references to the reviewer ("he counted three" ->
  "you counted three"), and drop mentions of `tools/hosttest`, whose files are not in the pack.
- **The user pushes and posts. Never push the PR branch without an explicit go-ahead** -- and when told
  to "post the reply", confirm the VENUE before sending. "Post it here" once meant this chat and was
  read as the PR thread, which put an unreviewed comment in front of the maintainer. An outward-facing
  send is not undoable by apology; ask if the target is not explicit.
- Hardware: one **physically 64-block** gen2 ISO15693 magic card (advertised count is programmable --
  see the hardware section). More cards inbound as of 2026-08-17: other gen2 silicon, gen1 candidates,
  and ordinary ISO15693 tags. No gen1 card confirmed on either side yet.

## The host test harness — READ THIS BEFORE TOUCHING THE POLLER

`tools/hosttest`, **59 tests, dev-only**. Full detail in [../tools/hosttest/README.md](../tools/hosttest/README.md).

```bash
cd tools/hosttest && make
```

It compiles the shipped `iso15693_poller.c` **verbatim** — the test files `#include` the `.c` so the
file-statics are reachable, and the SDK calls resolve to a fake tag via `-Ifakes`. No seam, no
`#ifdef TEST`, nothing added to the app. Four files: the wipe sweep (15), the clone loop (14), the
terminal-outcome contract (15), and the write state machine (15, driving the real poller callback the way
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
