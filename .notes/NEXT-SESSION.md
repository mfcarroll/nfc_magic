# Next session — ROUND 7 IS PUSHED AND POSTED. Waiting on his Round 8.

## Where things stand

PR #250, `nfc_magic_dev` on branch `iso15693-dev`.

**ROUND 7 SHIPPED 2026-09-11.** Fork pushed `d31f5162..dbc6e4fa`, a fast-forward of **19 commits**, all
signed. Main reply at **issuecomment-5628507918**; **all 20 thread replies posted**. The delta is three
reviewable groups: the cut (8), the corrections (10), the gen3 warning (1). His upstream key-cache work
was verified untouched by all 19 before the push, and `fap_version` 2.3 — his own conflict resolution —
is preserved.

What the reply commits us to, so a later session does not contradict it: the volume objection is
**conceded with numbers**, the C and D passes are **proposed and argued**, the CHANGELOG is **named as in
scope**, and he is **asked not to merge on the current PR body** and to let us propose the squash message.

**#255 is left as it stands — decided 2026-09-11.** Its text still carries the softer "moved UID"
wording the CHANGELOG no longer does, but Brian's comment is on the issue and says the stronger thing,
so a reader gets the real cost. Editing it would mean posting for little gain. Do not reopen.

Round 6 for reference: reply recorded at 2026-08-22T18:08Z, so use that date, not the 08-20 some of
these notes carry.

**Every physical tag is in [tag-inventory.md](tag-inventory.md)** — what it is, what it measured before
anything wrote to it, and whether it has ever been written to. Read it before touching hardware, and
`python3 tools/iso15693_magic_probe.py --identify` to find out which tag is actually on the antenna.
Labels live on paper, UIDs live on silicon, and three of the tags are physically identical.

**The comment cut is BUILT: sixteen commits on dev, on top of `04d5f8a`, all signed, nothing pushed.**
Eight of them are the cut itself; the rest are the corrections and citations found afterwards, plus notes. Results and the full argument are in
[pr-round-7/comment-cut-plan.md](pr-round-7/comment-cut-plan.md) under "EXECUTED". Headline: comment
**−95**, code **+11**, seven of eight commits comment-only and proven so, zero intra-batch churn, both
firmwares warning-free, host tests **101 → 106**.

**mishamyte has NOT replied.** He was asked to pick the cut's scope and whether he wants the gen3
pre-flight probe (#255) as its own PR. The cut was done on the stated preference (option 1) rather than
waiting, which was the plan. If he asks for the narrow version, this delta is wider than he wanted —
that is the known risk and it was taken deliberately.

**ROUND 7 LANDED 2026-09-07 — `COMMENTED`, 20 threads, nothing blocking.** Read
[pr-round-7/ASSESSMENT.md](pr-round-7/ASSESSMENT.md) first: it is against the PRE-CUT code, our cut
closes exactly ONE of his threads (and not in the shape he proposed), and it PRESERVED FOUR claims he
has now flagged — it shortened comments without re-checking them. He also merged upstream
`dev` into the PR branch (`d31f5162`) as housekeeping.

**The WHEN decision is resolved: he has replied, so the cut can go up framed as a partial answer to
Round 7** rather than as an unprompted round. That was the better option and it is now available.

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

## ⚠️ THE SQUASH MESSAGE IS THE COMMIT MESSAGES — corrected 2026-09-08

**A previous version of this section said the PR body becomes the squash message. That is WRONG.**
Verified by diffing #238 / #236 / #244: the squash body is GitHub's `COMMIT_MESSAGES` default —
`<title> (#N)`, then `* <headline>` + body per commit, joined by GitHub's `---------`. #258's body is
9949 bytes; its squash message is 799. So **commit messages survive a squash, concatenated.**

The pack does squash-merge (every `dev` commit has one parent; #258's four commits are not ancestors).
That part holds.

**The live hazard: the merger overrides the default.** mishamyte hand-wrote #258's 799-byte message
rather than take the 4064-byte concatenation. **Our 20 fork-bound commits concatenate to ~728 lines** —
so an override is likely, and then the messages are lost.

- **ON THE LIST: write the proposed squash commit message and POST IT AS A PR COMMENT when merge nears.**
  That is the whole mechanism, and two earlier readings of it here were wrong. The PR DESCRIPTION is
  never used — #258's body is 9949 chars against a 799-char squash message. The default is the
  concatenated commit messages, and he overrode it on #258 by writing his own in the merge box. So the
  only lever is to hand him one: he is the person clicking merge, and a comment is how he gets it.
  Draft ready in **[squash-message.md](squash-message.md)** — 62 lines, house style verified by
  measurement rather than assumed (#258's longest line is 79 columns, not the 72 an earlier note here
  claimed). Post the payload between the `~~~~` markers, as a comment, when merge nears.
- **Improving the PR body is therefore optional polish, not a priority.** It is what a reader of the PR
  page sees and it never enters git history. Do not confuse the two again.
- Consequence for the comment work: BOTH tests are live. "Would a maintainer editing this line, offline,
  need it?" is the strong one. "Would it be at home in the commit message?" is valid again but a WEAK
  home — it survives only if nobody overrides.

**Do not re-inherit the wrong premise:** comment VOLUME is not his objection. Checked against all 86
review events — he has never asked for less comment, only for accurate comment. The 42%-vs-10% figure and
the cut itself are ours. It IS an outstanding public commitment (round 6: "not treating it as optional"),
which means **the scope is ours, not his.**

## THE PATHWAY TO RELEASE, in order — settled 2026-09-08, step 3 added 09-09

1. **Push + post Round 7** (built, verified, awaiting a go-ahead).
2. **Improve the PR body** — worth doing on its own merits even though it is NOT the permanent record
   (see the section above). Material is in [pr-description.md](pr-description.md).
3. **Offer him the squash message** — same file. **Timing: not yet.** The trigger is the PR nearing
   merge — an approval, or him asking whether it is done — because the message has to describe the final
   state and the C/D passes will change it. **DECIDED: it stays in `.notes/` and gets pasted as a PR
   comment.** Never a tracked file: "remove before finalization" is a step that gets forgotten, and this
   way we keep what a comment cannot give — a diff across rounds. Do not re-open this. Condense first:
   ~190 lines now, #258's comparable is ~20, so 60-100 is the target.
4. **The gen1 B-round** — [gen1-hardware-findings.md](gen1-hardware-findings.md). Eight of its nine
   items are comment corrections, so it is a **B pass, and B precedes C/D.** It also SHRINKS the surface
   (inference -> measurement removes the hedging), so it makes the comment passes easier rather than
   harder. The card is a reusable fixture — restore, test, restore — so it is not one-shot.
5. **C — does it need to be there?** Deletion only. Verifiable by code bytes unchanged.
6. **D — can it be correctly simplified?** Rewords live claims, so it carries a B-check inside it.

Nothing in 3-6 is a merge blocker; mishamyte said explicitly not to hold for gen1. The order is a
preference. Running C before 4 would have C protect text the gen1 round is about to replace.

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

- **THE COMMIT TEST IS NOT THE COMMENT TEST, and reasoning is not "process narration" wherever it
  appears.** The two artifacts answer different questions for different readers. A comment answers
  *what must I not break when I edit this line?* — which is why "measured on device rather than
  counted" fails there: nobody can act on it. A commit answers *why did this change?* — so the test
  is **does it tell a reader of the diff something the diff cannot show?**

  Two categories pass that test while failing the comment test, and they are the same two things a
  diff structurally cannot show:

  - **Removals.** A diff shows what left, never whether it went somewhere else. `02-c0aa63a`'s note
    that "NOT hardware-validated" was dropped *because the caveat already lives on both gen1 entry
    points, and what went was a dangling `see the PR note`* is evidence about the current state of
    the code. Without it a reviewer correctly reads a vanished safety caveat as a quiet deletion.
  - **Rejected alternatives.** `17-26602eb` says the rewrap does not reopen the cut's
    do-not-rewrap decision, and where the boundary falls: churn-avoidance holds at 104 columns, not
    at 189. That forestalls "you said you weren't rewrapping."

  **Do not use "it gets squashed anyway" as the licence** — that has it backwards. Squashing makes
  commit messages LESS durable. GitHub's `COMMIT_MESSAGES` default would concatenate all of ours into
  the squash body, but they run ~728 lines, so whoever merges will more likely write their own summary
  (as he did for #258) and they leave git entirely, surviving only on the PR page. The licence is that
  explaining the change IS the commit's job. It is also why load-bearing facts cannot live only there:
  the queue-hang argument and the 2026-08-04 measurement stay in the code, and the PR description
  rewrite is on the list.

- **VERIFY WHICH TREE A BUILD CLAIM IS ABOUT.** Two ways this went wrong on 2026-09-09, both silent.
  `Momentum-Firmware` is a working tree on branch `t5577-deep-read`, **1287 commits ahead of
  `origin/dev`** with unrelated LF-RFID work — a green build there says nothing about stock. Stock
  Momentum is **`Momentum-Firmware-slix`** (branch `dev`, clean, API 87.1). And
  `unleashed-firmware/applications_user/nfc_magic_dev` had been a real **directory**, not a symlink, so
  it silently went three commits stale and a "both trees clean" claim covered a copy with the gen3
  warning missing from it. **Now a symlink** (`../../nfc_magic_dev`, matching Momentum's), verified by
  rebuilding through it: same 165,952-byte FAP, zero warnings, `tools/` still excluded. `applications_user`
  is gitignored in Unleashed, so the symlink is not a tracked change.

  Stock trees to build against, and what to report:

  | | tree | branch | API |
  |---|---|---|---|
  | Unleashed | `unleashed-firmware` | `unl092-base` @ `3c9be0fd` | **88.4** — the SDK he builds with |
  | Momentum | `Momentum-Firmware-slix` | `dev` @ `8ed809f` | **87.1** |

  And **`clang-format` is not on PATH** — the toolchain's is at
  `Momentum-Firmware*/toolchain/arm64-darwin/bin/clang-format` (18.1.8), used with
  `--style=file:<firmware>/.clang-format`. Running the bare command makes every file report as needing
  format, which is the false-FAIL twin of a `grep -q` false pass and just as uninformative.

- **A REPLY TO A FINDING NEEDS AT MOST THREE THINGS: the disposition, anything HE got wrong, and
  anything WE found while doing it.** Everything else is padding to the person who wrote the finding.
  Round 7's drafts broke this in two opposite directions, nine replies between them, and both come from
  the same instinct — writing to show we engaged rather than to say what he does not already know.

  **Restating his reasoning back at him.** The absent-run reply spent four lines explaining his own
  finding to him. Eight more did it at 9–20% overlap. Detect it by measuring shared 7-grams between his
  comment and the reply, **excluding code spans** — shared identifiers are legitimate, shared prose is
  not. Legitimate exceptions exist and the measure will flag them: quoting text we *added* or *deleted*,
  and confirming reasoning on a thread where he wrote "my error, please revert" — there, showing we
  followed the argument rather than the instruction is the point.

  **Claiming his correction as ours.** Worse, because it reads as taking credit. *"Fixed. Four
  qualifiers, not three"* — he wrote "There are four qualifiers, not three". Detect it by pulling every
  correction-shaped phrase (`not two|three|N`, `rather than the N`, `your figure`, `actually`,
  `miscount`) and checking each against his actual text. Round 7 had one of these, three legitimate
  corrections stated once too often, and one correcting a claim he never made at all — "there are two of
  those preambles, not three", when he had said two.

  When a phantom correction has real substance under it, **reframe, do not delete**: that one became a
  note that three sites share the condition line and only two share the behaviour, which is worth
  knowing and was never his claim to be wrong about.

- **NEVER CITE A DEV-REPO SHA IN ANYTHING HE WILL READ.** Caught 2026-09-09 in the round-7 reply, which
  cited `3199bb9`→`7883953` and `58de6ba`→`b312deb` for its churn figure. Both halves were unresolvable
  for him, for two independent reasons: `7883953` and `b312deb` had been rewritten by the fold rebase and
  are unreachable, and even the LIVE dev hashes never appear on the fork — `sync-to-fork.sh` overlays its
  own commits, so the branch he reads has entirely different hashes. **Cite commit SUBJECTS instead**;
  they survive the sync, and he greps for them anyway. This is a whole error class the dev-repo/fork split
  creates and nothing in the workflow catches it — no build, test or format check looks at prose. Sweep
  every draft before posting:

  ```bash
  grep -oE '\b[0-9a-f]{7,9}\b' .notes/pr-round-*/reply.md .notes/pr-round-*/thread-replies.md | sort -u
  ```

  Same trap in reverse: a fork SHA is meaningless in a dev-repo commit message. And any SHA-bearing
  figure goes stale the moment a commit is added — the same bullet claimed "6 lines, both instances" when
  a later commit had made it 9 lines in three. **Re-measure every number in a draft immediately before
  posting**, not when it is written.

- **A read-based capacity measurement is a LOWER BOUND, not a capacity, and so is a search that hits its
  own ceiling.** Proven twice on 2026-09-08. `tools/iso15693_magic_probe.py`'s capacity probe reported
  "82 real blocks" on a card advertising 79 when every block up to its search bound answered — 82 was
  `advertised + 2 + 1`, the probe's own limit. And on the gen1 card it measured 56/56 while the app's
  write-based wipe found 58, under-detecting by two. `ISO15693_POLLER_WIPE_MAX_BLOCKS` says exactly why:
  only a WRITE settles whether a block exists. The tool now says "lower bound" in both cases.
- **A no-ACK is not a refusal on the gen1 backdoor registers.** They accept writes without answering —
  observed 2026-09-08, previously an inference from proxmark's source. So no write-based probe of those
  registers can have a meaningful negative, and a gate built on one will confidently mislead: that cost
  two wrong answers on the same card before it was understood. Only the full sequence plus a
  power-cycled UID re-read is conclusive.

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

**Out-of-range writes do not alias, on either silicon we have — 2026-08-24.** Previously this was known
only for the gen2 card, from the probe suite's `edgepages` test (phantom writes rejected, phantom reads
failing, block 0 unchanged across four runs). It now also holds for plain NXP SLI: a full gen1 UID
attempt against a 28-block white-tag sends ordinary WRITE BLOCKs at 56/57/62/63, all past the end, and
block 27 still read its factory `57 5F 4F 4B` afterwards. This is what makes the gen1 opt-in test safe
on a small tag, and it is the assumption that had to hold for that to be true.

## Backlog — needs cards we do not have yet

- **`source_uses_gen1_blocks`** — the one backdoor-predicate site the gen2 card cannot reach, since it
  sits behind the gen1 opt-in and so needs a card that FAILS gen2. **Any ordinary ISO15693/NfcV tag does
  it**. **THE TAGS ARRIVED 2026-08-24 and this is now UNBLOCKED** — three plain NXP SLI, 28 blocks,
  fully unlocked, classified non-magic on hardware. See [tag-inventory.md](tag-inventory.md).
  Select a source with data in 56/57/62/63, present a white-tag, reach the gen1 opt-in, confirm the
  extra warning renders.
  **And then you may ACCEPT, which the old version of this note said not to do.** The correction:
  those tags are 28 blocks, so 56/57/62/63 do not exist on them, and the gen1 writes go out of range.
  That was an assumption until it was tested — an out-of-range write could in principle alias onto a
  real block — so it was checked: after a full gen1 UID attempt against white-tag-1, block 27 still
  read `57 5F 4F 4B`, its untouched factory value. **No aliasing on SLI silicon.** So accepting is
  safe HERE and exercises the whole gen1-failure path end to end (Fail, gen1_attempted, and the
  "56/57/62/63 may be overwritten" screen) rather than stopping at the warning.
  Note what does NOT generalise: on a plain tag of 64 blocks or more those four blocks are real, and
  accepting there destroys them. The safety comes from the tag being SMALL, not from gen1 being gentle.
- **Other gen2 magic silicon** — **ARRIVED 2026-08-24: two samples, both confirmed gen2 on hardware.**
  white-coin and black-tag, TI Tag-it HF-I Plus presentation, IC ref 0x8B, 64x4, physical 64, in
  proxmark's default CFG state — so the gen2 probe was geometry-neutral on both. Different silicon
  from the original test card, which presents EM-Marin at IC ref 0x0F and advertises 66 against 64
  physical. **Re-run the regression five on one of them**; that is the outstanding piece.
- **GEN1 IS CONFIRMED ON HARDWARE, 2026-09-08 — `lri2k-keychain`.** The four-frame sequence set
  `E0F1E2D3C4B5A697`, it read back exactly, and the original UID restored via gen1. Campaign
  `iso15_20260908_025701`. **This is the first gen1 card the project has ever had**, and it unblocks the
  gen1 row of the harness table, which has read "modelled, not settled" since the beginning.
  **Two findings beyond gen1 itself, both citable:**
  1. **The backdoor registers accept writes WITHOUT acknowledging.** The gate got no ACK on an
     unaddressed zero write to block 62, then the full sequence worked — so that write was accepted
     silently. `iso15693_poller.h` justifies discarding these frames' return values as something that
     "must" be done "on a card that may not answer". That was an inference from proxmark's source.
     **It is now a measurement.**
  2. **Writable memory above the advertised count, on a third kind of silicon.** It advertises 56 blocks
     and took writes at 56/57/62/63 — the same principle as the gen2 card holding blocks above its own
     claim, now shown on gen1.
  **NOT settled: the latch.** `SetTag15693Uid` ends in `switch_off()`, so every read-back sits behind a
  field power-cycle and cannot distinguish "latches on power-up" from "changes immediately". The app's
  `NfcCommandReset`-before-verify is still justified by the model, not by measurement. Isolating it needs
  a read in the SAME field session as the write.
  **THE CARD IS NOW ARMED, deliberately.** `0x6996` went into block 63 twice and nothing clears it, so
  its UID can move on any later write to 56/57 — including the app's own wipe. That is the state #255's
  armed-gen1 case describes, and it has never been reproducible before. **The highest-value hardware
  test now available: run the app's WIPE on it.** It should zero 56/57, move the UID, and report
  `uid_changed` as Partial — the mitigation this PR ships and has never exercised on real gen1 silicon.
  **Everything from that session, plus the work list it implies, is in
  [gen1-hardware-findings.md](gen1-hardware-findings.md).** Read that rather than this bullet; the
  corrections it lists belong in a delta AFTER the comment cut, which was promised as one decision with
  nothing else in it.
- **The earlier gen1 candidate note, superseded:** The listing it was ordered from
  is titled "15693 UID Changeable + **Lua Script by Iceman** Compatible ST LRi 2K (0-55 block)", and
  `proxmark3/client/luascripts/hf_15_magic.lua` sends `02213E00000000`, `02213F69960000`,
  `022138<uid hi>`, `022139<uid lo>` — WRITE BLOCK (`0x21`) at 62, 63, 56, 57 with 0, `0x6996` and the
  UID halves. That is **byte-for-byte** `SetTag15693Uid` in `armsrc/iso15693.c:3166`, i.e. the gen1
  sequence `hf 15 csetuid` sends with no flag. So the Lua method IS gen1, our probe already covers it,
  and a Lua-writable product is a gen1 product.
  It reads 56 blocks, and **that does not rule gen1 out** — `ISO15693_POLLER_WIPE_MAX_BLOCKS` says only
  a WRITE settles whether a block exists, so 56-63 failing to read is not evidence they are absent.
  They may be backdoor registers outside the user range, as our gen2 card holds blocks above its own
  advertised count. Which also makes the probe safe for user data: writing them cannot touch 0-55.
  **Next action: `--probes magictype --destructive` on `lri2k-keychain`.** Its baseline is taken.
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
- **Do NOT resolve his review threads.** Established from the data 2026-09-08: of 100 threads, the only
  24 resolved are all from round 5 (2026-08-16), all opened by him, and the two he left open from that
  round are exactly the two he named in round 7 as "held open from before" (the `:752` budget thread and
  `PASS_MAX_MS`). So resolution is HIS record of having traced a fix himself -- "I traced the guard
  rather than taking it on report" -- and marking a thread resolved asserts that verification on his
  behalf. Other rounds sitting unresolved is his housekeeping, not a gap to tidy. `viewerCanResolve`
  comes back mixed, so this is a choice rather than a permission wall.
- **The user pushes and posts. Never push the PR branch without an explicit go-ahead** -- and when told
  to "post the reply", confirm the VENUE before sending. "Post it here" once meant this chat and was
  read as the PR thread, which put an unreviewed comment in front of the maintainer. An outward-facing
  send is not undoable by apology; ask if the target is not explicit.
- Hardware: **six tags as of 2026-08-24, all in [tag-inventory.md](tag-inventory.md)** — the original
  physically-64 gen2 card (advertised count is programmable, see the hardware section), two more
  confirmed gen2 (white-coin, black-tag), and three plain NXP SLI at 28 blocks. **Still no gen1 card
  confirmed on either side.** A further order was expected to include gen1 candidates at other
  capacities; anything new gets an inventory entry before it gets written to.
  **Do not identify a tag by its label alone.** The three white-tags are physically indistinguishable
  and are NOT interchangeable -- white-tag-1 has been write-probed, white-tag-3 is the untouched
  control. `--identify` reads the tag and names it; `--identify --card <label>` asserts it and exits
  non-zero on the wrong one, which is the check to run before any destructive probe.

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
| the gen1 path | **partly settled on hardware 2026-09-08** — the four-frame sequence works, the backdoor registers accept writes without acknowledging, and the armed-card wipe hazard is reproduced. **The LATCH specifically is still the inference**: every read-back sits behind a field power-cycle, so "latches on power-up" and "changes immediately" remain indistinguishable. See [gen1-hardware-findings.md](gen1-hardware-findings.md). |

Still uncovered: the scenes' LAYOUT as opposed to their content (the recorders capture x/y/font, but
nothing asserts that N lines of FontSecondary fit above the button box — that arithmetic was measured by
the reviewer, not by a test); the other scenes (the write scene's routing including this round's
mode-gate, the gen1 opt-in, the confirm screens); and the radio layer below the SDK.
[test-bench-idea.md](test-bench-idea.md) has the state of the simulator idea — Option B is what got
built; Option A's better lead is the firmware's own listener, not the proxmark.
