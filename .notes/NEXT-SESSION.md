# Next session — THE COMMENT CUT IS DONE LOCALLY AND NOT PUSHED. Awaiting a go-ahead.

## Where things stand

PR #250, `nfc_magic_dev` on branch `iso15693-dev`. **Round 6 answered, pushed and posted** —
GitHub records the reply at 2026-08-22T18:08Z, so use that date, not the 08-20 some of these notes carry.

**The comment cut is BUILT: sixteen commits on dev, on top of `04d5f8a`, all signed, nothing pushed.**
Eight of them are the cut itself; the rest are the corrections and citations found afterwards, plus notes. Results and the full argument are in
[pr-round-7/comment-cut-plan.md](pr-round-7/comment-cut-plan.md) under "EXECUTED". Headline: comment
**−95**, code **+11**, seven of eight commits comment-only and proven so, zero intra-batch churn, both
firmwares warning-free, host tests **101 → 106**.

**mishamyte has NOT replied.** He was asked to pick the cut's scope and whether he wants the gen3
pre-flight probe (#255) as its own PR. The cut was done on the stated preference (option 1) rather than
waiting, which was the plan. If he asks for the narrow version, this delta is wider than he wanted —
that is the known risk and it was taken deliberately.

**The open decision is WHEN to send it:** push now as an unprompted round, or hold until he replies so
the delta can be framed as an answer. Not a technical question; the work is finished either way.

### The finding to lead with, whenever it goes

The ratio is the wrong metric and this pass has the numbers to say so: **−102 comment lines moved the
surface from 37% to 36%**, because removing comment lowers numerator and denominator together. Reaching
`gen2_poller.c`'s 9% by deduplication is arithmetically impossible. The metric that DOES track the
defect is how many places state the same fact — **repeated comment phrases went 135 → 34**. Concede the
real part: ISO15693 carries ~9x the comment per line of code, and `gen2_poller.c` is 875 lines, so the
gap is not a size artefact. Argue about what the residue IS, not that the gap is imaginary.

Fork `nfc-magic-iso15693` = **`049029c9`**, 41 commits, all signed and GitHub-verified, fast-forward with
no force at any point. PR shows 72 commits. Dev `iso15693-dev` = **`9da594f`**, clean, all signed.

**His Round 6 was `COMMENTED`, not `CHANGES_REQUESTED`** — the first time in six rounds. He verified all
three Round 5 blockers by tracing them, tabulated all thirteen reason codes through the button rule, and
answered our open question: keep the capacity guard AND keep 10s, because a clone-specific budget is the
real fix and not this PR's.

Posted: [#issuecomment-5381850091](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-5381850091)
plus **22 threaded replies covering all 22 of his threads** — every one verified byte-identical by
re-fetching. Drafts in [pr-round-6/reply.md](pr-round-6/reply.md) and
[pr-round-6/thread-replies.md](pr-round-6/thread-replies.md); his review verbatim in
[pr-round-6/received/](pr-round-6/received/).

What went up — ten commits, one decision each, **zero intra-batch churn**:

1. `d43c0a8` **blocking** — the Fail guard judged on counts alone, so a cut clone that accepted nothing
   reported "no data block took". One conjunct: `!instance->pass_truncated`.
2. `8c9f9b3` the fourth `block_is_empty` site, the dead `+ over_capacity` term, the `cut == advertised`
   off-by-one
3. `79f1629` the bitmap set/clear pair
4. `08c45ac` `{reason -> title}` — the re-scoped render table
5. `8f64fa7` the back-fill is clone-only; two truncation archetypes unfused
6. `3171e66` the budget's cost arithmetic (it inverts), and `COUNT_OF`
7. `8b40231` three result-screen claims, and why `WipeUidChanged` withholds Retry
8. `bc0dc69` three user-facing CHANGELOG errors
9. `0850837` two minor claims and a 155-column comment line
10. `9da594f` cite #255 from the gen1 open question and the gen3 entry

## The comment cut — what it was, now that it is built

This was committed to, in writing, at the end of the posted reply. It is the last item on the deferred
queue and the round's own evidence made it the priority:

**Fixing eleven false comments cost +117 comment against +28 code, and took `iso15693_poller.c` from 43%
to 44%.** Every round that corrects a claim adds the explanation that makes it correct, so the metric moves
the wrong way even when each edit is right. Three of his eleven threads were comments contradicting *other
comments in the same delta*; two were user-facing. **That is a duplication problem, not a density one** —
the same fact in three places drifts in two.

The reply asked him to choose the scope and stated a preference. **Option 1 is what was built**, on the
stated preference, because he had not answered:

1. **The full ownership model.** The event enum owns the outcome contract, the `ISO15693_MAGIC_BLK_*`
   defines own the wire facts, `gen1_optin.c`'s strings own the user-facing gen1 consequence,
   `ISO15693_POLLER_PASS_MAX_MS` owns the budget rationale. Everything else cross-references instead of
   restating. This targets the mechanism that produced Round 6.
2. **Narrow** — only the facts that have already drifted twice: the back-fill's scope, the two truncation
   archetypes, the prefix property, the cost figures. Leaves the mechanism intact.

**Do it as its own delta with nothing else in it**, so the diff reads as one decision. That was promised in
the reply.

Also in that pass, per his notes:
- the **compact-UID formatter's four copies** (`iso15693_info.c:18`, `write_fail.c:287`, `:305`,
  `write_confirm.c:39`). The fold relocated one, it did not add one — so it is not against this delta.
- the twelve `widget_add_string_multiline_element` calls varying only in `(x, y)`. Deliberately left in
  Round 6: those y values carry the line-budget arithmetic he measured for us in Round 4, and they should
  move in the comment cut rather than be buried in a table.

**The plan is written up in [pr-round-7/comment-cut-plan.md](pr-round-7/comment-cut-plan.md)** — the
measured per-file ratios, the seven known duplicated facts as a work list, what must NOT be cut, and how to
verify it. Start there rather than from this section.

**Tests first, then the cut.** `tools/hosttest` now covers the result screens and the write scene's
routing, so a comment-only pass is verifiable as behaviour-preserving rather than read-and-hoped. If the
cut touches code, update the tests in the same commit.

## Deferred DELIBERATELY, not forgotten — 2026-08-22

**A full `/code-review` pass over the PR was scoped and NOT run**, to save tokens in a fresh weekly
window. Revisit in a burn window, ideally after the next round lands so it reviews the final code once
rather than twice. The scope decided at the time, and the reasoning, so it does not need re-deriving:

- **Run it over the ISO15693 surface, max effort** — `iso15693_poller.c/.h`, `iso15693_info.c/.h`, the
  six ISO15693 scenes, `scene_write.c`, `write_confirm.c`, `file_select.c`, `nfc_magic_app_i.h`. About
  4,000 lines.
- **Not the full `origin/main..HEAD` range** (74 files, 7,146 insertions, 221 commits): most of the rest
  is light touches on gen2/gen4/USCUID-UL that six rounds have already passed, so it roughly doubles the
  spend to re-review code that is not new.

### Both documentary items are now CLOSED — 2026-08-24

Surfaced 2026-08-22 while checking whether any code change was outstanding. Neither was a functional fix
and both are done.

1. **#255's mitigation claim had a hole** — it described the post-wipe UID re-read as unconditional while
   `iso15693_poller.c`'s `wiped == 0` short-circuit skips it entirely, which is the path an armed gen1
   card would need it on. Fixed in three places: the comment at the short-circuit (`65e741a`), the
   CHANGELOG's wipe entry (`7570ef7`), and the issue itself —
   [#255 comment 5389538269](https://github.com/xMasterX/all-the-plugins/issues/255#issuecomment-5389538269),
   posted 2026-08-24 and verified byte-identical to
   [pr-round-7/issue-255-followup.md](pr-round-7/issue-255-followup.md). That comment also discharges
   mishamyte's Round 6 "file it rather than fix it" ask, which the Round 6 reply had answered with a code
   note while saying "Filed" — a loose end he could have found.
2. **#251 is now cited**, at the two choke points where its frames are built rather than at the functions
   the issue names: `write_block_retried` (every block write; the SDK sets no ADDRESSED flag and no UID)
   and `verify_inventory` (every UID read-back; single-slot, so a bystander can answer). `27d939b`. The
   release-notes half was decided in favour of shipping it — `8e9ba69` adds it to "Validation (at 2.1)"
   beside the gen3 entry. Re-verified against the current SDK rather than trusting the Round 2 report:
   `iso15693_3_poller_i.c:237` write_block, `:132` inventory, both still as filed.

**Nothing is owed outward now.** Everything else waits on him.

## Open, waiting on him

- **The comment cut's scope** — asked at the end of the reply.
- **Whether he wants the gen3 pre-flight probe as its own PR.** Filed as
  [#255](https://github.com/xMasterX/all-the-plugins/issues/255) (`type/enhancement`, filed 2026-08-20)
  carrying both register hazards: gen3 is detectable via the `0x14`/`0x15` signature, armed gen1 is not.
  The code cites it from both sites.

## Settled in earlier rounds — do not re-litigate

- **The capacity guard stays, and so does 10s.** He answered this in Round 6: dropping the guard trades a
  fabrication the user cannot check for one they can, and 20s doubles the Back-swallowed window on every
  card to buy a diagnosis on one shape of card. A clone-specific budget is the real fix and not this PR's.
- **The render table is `{reason -> title}` and it is DONE.** The fuller `{reason, title, body}` form is
  dead and he agrees: only four of twelve bodies are static, and `wipe_stopped` arriving dynamic moved the
  ratio further away. Do not revisit it.
- **`WipeUidChanged` withholds Retry deliberately**, and the reasoning lives at the branch ordering in
  `scene_write.c` where a reader will look for it. Not an oversight.
- **`NothingWiped` has no `uid_verified` route, and that is filed rather than fixed** — gen1, no card, and
  the `wiped == 0` short-circuit predates this PR.

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
  stated elsewhere.** Report added/removed comment vs code per commit with `tools/comment-ratio.py`;
  a commit adding more comment than code is going the wrong way. **But do not use the ratio as the
  target** — the 2026-08-22 cut removed 102 comment lines and moved the surface 37% -> 36%, because
  removing comment lowers numerator and denominator together. Count SITES PER FACT instead; the
  duplicate-phrase scan in the round-7 plan is the tool for it (135 -> 34 over that pass). The strong comment reduction is **item 6**, which is held — items
  1-4 came out at comment +4 / code −51, and the +4 is three constraints that had nowhere else to live
  (tick wraparound, don't-reuse-`is_wiping`, the widget copies its string so the early free is safe).
  Naming a value is often what makes the wrong refactor look attractive, so that is exactly where the
  constraint has to be written down.
- **A test that has never been observed to FAIL is not evidence, and neither is a build system whose
  dependency tracking has never been observed to fire.** Mutation-test anything new: break the fix the
  test covers, watch the test go red, restore. On 2026-08-18 two `write_identity` tests passed against
  deliberately broken source -- not because the tests were weak, but because `make` re-ran a stale
  binary. `tools/hosttest`'s depfile rule compiled the test and its fake in ONE `cc` invocation sharing a
  single `-MF`, so the fake's dependency list overwrote the test's and no depfile ever named
  `iso15693_poller.c`. It had carried a comment asserting the opposite for weeks. Nothing was masked --
  a from-scratch run of all 83 passed -- but several "green" claims made that day were worth less than
  they looked. Fixed by compiling each TU separately; verify with the two greps in the Makefile.
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
- **Use exact-match, asserted string replacements when editing. Never a regex sweep over an
  identifier.** Index-based slicing broke a file mid-edit, an unasserted replacement silently
  no-matched and cost a build cycle, and on 2026-08-18 a `\bover_capacity\b` substitution also
  rewrote `instance->iso15693_result.over_capacity` into
  `instance->iso15693_result.reason == ...`, producing a file that had to be thrown away. A local and
  a struct member can share a name; `\b` does not know the difference. Match the whole expression,
  assert the count is 1.
- **Commit the baseline BEFORE mutation-testing it.** The restore step is `git checkout -- <file>`,
  which does not distinguish the mutation from the uncommitted work underneath it. On 2026-08-18 that
  destroyed a finished, passing refactor of the write-fail render chain -- recoverable only because the
  whole transformation had been scripted rather than hand-edited. Commit (or `git stash`) first, then
  break things. A corollary: if a mutation appears NOT to be caught, suspect a stale binary or a
  reverted baseline before concluding the test is weak -- both have happened, one on each side.
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

**The device is clean** as of 2026-08-20 — re-flashed from another project, carrying the app with it, so
the lowered-budget build is gone and nothing is owed there.

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
- **Scene coverage beyond the two result screens** — the write scene's routing (including this round's
  mode-gate, which decides whether a cut CLONE lands on the wipe-specific screen), the gen1 opt-in, and
  the confirm screens. Dev-only work, no card needed, and the recorders already exist.
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

## The host test harness — READ THIS BEFORE TOUCHING THE POLLER OR THE RESULT SCREENS

`tools/hosttest`, **106 tests, dev-only**. Full detail in [../tools/hosttest/README.md](../tools/hosttest/README.md).

```bash
cd tools/hosttest && make
```

Shipped code is compiled **verbatim**: the test files `#include` the `.c` so file-statics are reachable,
and the firmware calls resolve to fakes via `-Ifakes`. No seam, no `#ifdef TEST`, nothing added to the
app. `application.fam` excludes `tools/`, so none of it can ship.

Three groups, eight files. The third (`PLAIN_TESTS`) needs neither radio nor GUI, but still links
`fake_scene.o`, because that is where the real FuriString implementation lives:

| file | cases | drives |
|---|---|---|
| `test_wipe_sweep.c` | 15 | the wipe sweep, against a fake TAG |
| `test_clone_blocks.c` | 14 | the clone loop |
| `test_outcome.c` | 17 | the terminal-outcome contract |
| `test_write_step.c` | 15 | the write state machine, via the real poller callback |
| `test_write_identity.c` | 10 | the AFI/DSFID write-and-verify retry loop |
| `test_write_fail_scene.c` | 15 | the two result screens, against fake GUI RECORDERS |
| `test_write_scene.c` | 15 | the write scene's ROUTING, including the round-5 mode gate |
| `test_uid_format.c` | 5 | the shared UID formatter's two policies AND their widths |

**Four things that matter more than the test count:**

1. **Run it after ANY change to the poller or those scenes, and before claiming anything about
   coverage.** It found a real defect on its first run over the clone loop — the clock-cut "Card too
   small" claim, now on the PR.
2. **If a review asks for changes here, update the tests in the same commit.** A test that still passes
   because it was never updated is worse than no test — it reads as coverage and isn't.
3. **The fakes' semantics were verified against firmware source, with citations in the README**, and the
   GUI enum lists are copied from the firmware rather than invented. If you change a fake, re-check them;
   a wrong fake asserts wrong behaviour confidently.
4. **Mutation-test anything you add.** Break the fix the test covers and watch it fail. This is not
   ceremony: it is how the depfile bug below was found, and two tests that looked fine were proving
   nothing.

Where the previously reasoned-only behaviours now stand:

| behaviour | now |
|---|---|
| tail-drop fix's positive case | tested |
| capacity gate's discriminating case | tested |
| clock-cut clone, card present | tested — and it was wrong; that became a fix on the PR |
| truncated sweep reporting Partial | tested at the poller **and now at the screens** |
| `uid_verified` false | tested at the poller and on the screen |
| the gen1 path | **modelled, not settled** — the latch behaviour is our inference from proxmark's send order, so the tests show the app is right *given the model*, not that the model is. Needs a gen1 card. |

Still uncovered: the scenes' LAYOUT as opposed to their content (the recorders capture x/y/font, but
nothing asserts that N lines of FontSecondary fit above the button box — that arithmetic was measured by
the reviewer, not by a test); the other scenes (the write scene's routing including this round's
mode-gate, the gen1 opt-in, the confirm screens); and the radio layer below the SDK.
[test-bench-idea.md](test-bench-idea.md) has the state of the simulator idea — Option B is what got
built; Option A's better lead is the firmware's own listener, not the proxmark.
