# Next session — ROUND 14 IS PUSHED AND POSTED. The blocker is the addressing work, not him.

**Pushed `749f10e6..dbc11980`, a fast-forward of 7, all signed.** PR head confirmed via the API
before posting. **Posted:** the main reply as
[issuecomment-5824709850](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-5824709850),
and all seven thread replies, each verified by `in_reply_to` AND by the root's file.

**He took the branch himself this round** -- `a27d187d..749f10e6`, twelve commits, four behavioural.
All twelve adopted into dev; three of our round-13 commits were moot because he reached the same
conclusions independently. We added seven on top.

**THE BENCH IS DONE.** His `0c5a7d69`, `62b60e2b` and `4d20f06e` all pass on hardware; `749f10e6` was
settled against the firmware source. Our own `dbc11980` consent fix is confirmed end to end by the
two-card Retry.

**⚠️ gen-2-card is left advertising 256 blocks against 64 physical**, from the CFG clamp fixture.
Re-clone any normal 64-block source to restore it -- the CFG frame sets the geometry either way.

## IN FLIGHT: addressed writes — measured, not yet implemented

**All five chips accept addressed WRITE BLOCK.** Measured 2026-09-24, full transcripts and frames in
[pr-round-15/addressed-writes-measured.md](pr-round-15/addressed-writes-measured.md). That file is
the input to the implementation; read it before writing code.

The design is settled by measurement rather than by argument:

- always-addressed is safe -- `gen-2-card` was the one that could have blocked it and does not
- the WIPE must re-inventory and re-address after a write to 56 or 57 lands, because the UID moves
  immediately and the stale address gets silence. Confirmed on NXP SLIX and ST LRi2K, the latter
  being the chip the armed-gen1 hazard was reproduced on
- the gen1 backdoor sequence stays UNADDRESSED -- measured to work that way on all five
- the SDK cannot do it: `iso15693_3_poller_write_block` hardcodes the flags and
  `iso15693_3_write_block_response_parse` is internal, so we need our own builder and response check

**One frame outstanding**, and it is a gap in my test design rather than a card behaviour:
`SL2S5302` was never sent a mis-addressed write, so "it enforces the address" is untested there.

    hf 15 raw -ackw -d 2221F8350003500204E10855667788    expect no answer

**Two controls also passed** before this, in
[pr-round-15/controls-2026-09-24.md](pr-round-15/controls-2026-09-24.md): EM-Marin still wipes 64/64,
and gen1 clears 28/28 ordinary data blocks unaddressed. Both were recorded as unmeasured prerequisites
in the round-10 finding.

**#251 is three defects and this closes one.** The 1-slot inventory and the missing STAY QUIET are
untouched, and the issue's worst consequence -- the post-wipe UID check answered by a bystander --
cannot be fixed by addressing at all. Do not report this as closing #251.

## WHAT IS ACTUALLY BLOCKING NOW

**Addressed writes, and the gen1 caveat gate with them.** The reply no longer offers him a scoping
choice: shipping ISO15693 support that cannot write a TI Tag-it is not a later problem, so this
belongs in this PR. Both are recorded in
[pr-round-10/unaddressed-write-finding.md](pr-round-10/unaddressed-write-finding.md).

Raised with him and awaiting his call: the protocol menus keeping their cursor across a fresh scan.
App-wide, one dispatch point in `magic_info.c`, six lines. We offered to implement it.

## ⚠️ TWO RULE FILES, READ THE RELEVANT ONE FIRST

[WRITING-RULES.md](WRITING-RULES.md) before anything goes outward.
[BENCH-RULES.md](BENCH-RULES.md) before anything is measured -- extracted 2026-09-24 after shipping a
five-card addressed-write measurement in which one card never received its control, so for that chip
"it enforces the address" and "it ignores the flag entirely" were indistinguishable.

### WRITING-RULES.md

Built this round because the rules existed and we broke them anyway -- they were 264 lines of prose
in this file, ordered by when we learned them. They are now ordered by WHICH ARTIFACT you are
writing, with a gate (`tools/check-writing.py`, wired into `replay-to-fork.sh`, which refuses to
replay on a finding) and, more importantly, **eight read-through questions the gate cannot replace**.

Every defect this round was caught by a human read, not a checker: comparisons against work he cannot
see, a section he could not act on, a conclusion that did not follow, a question that was no longer
open, restating a lesson he taught us, a whole reply section duplicating its own thread, and -- after
fixing that -- narration about where we had moved it. A fourth stale heading got through a checker
written to catch stale headings.

## Where things stood before

**mishamyte pushed `a27d187d..749f10e6` to the PR branch himself** — five prose fixes answering
round 11, a six-commit sweep of the whole PR, and an rx-buffer fix. **Four are behavioural**, which
is new; he had only pushed prose before. He invites rebasing, rewording or dropping any of them.

**ALL TWELVE ARE ADOPTED into dev** (`7bd719d`), since dev is what sync-to-fork overlays from and the
fork is no longer downstream of it. Our round-13 gen3 text, twin-site fixes and "copied exactly" came
off with them as moot — he reached the same conclusions independently.

**⚠️ THREE BEHAVIOURAL COMMITS ARE UNBENCHED.** `0c5a7d69` (gen1_attempted set before the frames go
out), `4d20f06e` (CFG geometry clamp), `62b60e2b` (menu cursor). He has no ISO15693 hardware and says
so. mfcarroll is at the bench 2026-09-21. The fourth, `749f10e6`, needed no bench and was verified
against the firmware source here: bit_buffer_copy's furi_check, the 64-byte poller buffer, and all
five in-tree callers passing instance->rx_buffer as both source and destination.

**Drafts in [pr-round-14/](pr-round-14/)** — reply and seven thread replies, checker-clean, every
cited SHA verified against the branch. NOT posted, NOT reviewed by mfcarroll.

## What we added on top of his twelve

| dev | |
|---|---|
| `26df4dd` | P1 the clock cut |
| `a58de0b` | P3 the clamp -- his 4d20f06e made it three callers, not two |
| `155675a` | P2 the succeeded-blocks figure |
| `be1795e` | the sweep's attempted-count invariant |
| `ada1d98` | the layout note's x convention, which answers the P4 wrapper question |
| `9b68f3e` | the reach rule's third gate -- offered as a fix TO his 5c97e16f |

Two of the old ten folded away: the naming rationale now lives in P1's helper doc, and 07 became a
correction to his commit rather than a parallel rewrite.

## ⚠️ HIS -Wswitch COMMIT BROKE THE HOST HARNESS

`e32e6242` added `furi_crash`, which was not in `tools/hosttest/fakes/furi.h`, so the poller stopped
compiling there. Fixed in the adoption commit. **Nothing on his side could have shown this** — the
harness is dev-only and never syncs. It is now the concrete argument for releasing it, and the reply
says we will put it up as its own PR rather than adding it here.

## Still unsigned

1Password was locked through this work. Fork commits are signed at replay time; re-run
`replay-to-fork.sh` with it unlocked before any push and confirm the count.

## Where things stood before

**HIS ROUND 12 ARRIVED 2026-09-17** and is fully addressed: seven findings, all verified against the
tree before fixing, all fixed. Round 13 combines those with the simplification pass, since his review
landed before P1-P3 went out. Draft reply, thread replies and fork messages in
[pr-round-13/](pr-round-13/); his review is cached in `pr-round-13/received/`.

**NOTHING HAS BEEN PUSHED OR POSTED, AND NONE OF IT HAS HAD MFCARROLL'S READ.** That read has caught
more than any other check on this project, so it is the gate. Seven fork sync points, no reorder
needed, and **zero intra-round churn** -- the first round with none.

**⚠️ THE ROUND-13 DEV COMMITS ARE UNSIGNED.** 1Password was locked while he was away. Fork commits
are signed at replay time, so re-running `replay-to-fork.sh` with it unlocked gives a signed chain --
check for `7 of 7`. Do NOT re-sign the dev commits: it rewrites history and renames every
`NN-<sha>.msg`.

## What round 12 found, and the pattern under it

All seven were correct and all were verified here rather than taken on trust. Two of them are the
shape worth remembering:

- **The trim escalated a claim and deleted its evidence in the same edit.** The gen3 bullet came to
  say the gen1 opt-in writes "reach the same blocks" as the registers a wipe bricks -- false, since
  gen3 keeps its UID at 0x10/0x11 and its signature at 0x14/0x15 -- while the same commit removed the
  only record of those addresses. Unfalsifiable and wrong arrived together.
- **Three of the seven were one defect: a claim corrected in one place and left in its twin.** The
  consent screen fixed and the release notes not; the in-band error code scoped at its definition and
  not at its two users. Worse than uniform wrongness, because the reader has to work out which half
  is current. **When a claim appears in N places, fix all N or none.**

The reach rule had been wrong in four consecutive rounds, each time in a new corner, so it is now a
closed form stated once in the poller (`L = max(A + 7, claim - 1)`; 56 reached iff `A >= 49` or
`claim >= 57`) with the CHANGELOG carrying only the qualitative half. His catch that a landing READ
resets the run -- not just a landing write -- is one this session would have missed.

## Where things stood before that

**Pushed `536e0d4f..a27d187d`, a fast-forward of 7.** PR head confirmed via the API, comment-only
outside the CHANGELOG, `fap_version` still 2.3, nine files, all seven signed.

**Posted:** the main reply as [issuecomment-5703486508](https://github.com/xMasterX/all-the-plugins/pull/250#issuecomment-5703486508),
and all seven thread replies, each verified by `in_reply_to` AND by the root's file matching what
the reply discusses.

**THE SIMPLIFICATION PASS IS BUILT AND UNPUSHED** — P1-P3, four shipped-file commits' worth of work
in three, plus two dev-only. Draft reply and fork messages in [pr-round-12/](pr-round-12/). **Not
pushed, not posted, and not reviewed by mfcarroll** — drafted while he was away, and his read is the
check this project has relied on most. Nothing goes out until he has had it.

**The reply says this should NOT merge yet**, and why: TI Tag-it HF-I Plus refuses unaddressed
WRITE BLOCK with error 0x01, and every ISO15693 write this app sends is unaddressed. Not a
regression — the write path is unchanged since the passing August run and the EM-Marin control still
passes. It reframes #251 from a bystander hazard into a compatibility limit.

## THE PLAN HE HAS BEEN GIVEN, in the order stated in the reply

1. ~~**The comment cut, WITH the release-notes trim**~~ — **DONE, pushed as round 11.** 84 comment
   lines out, code `+0 −0`, and the 2.3 section 177 → 115. Numbers in "Where things stand" below.
2. ~~**The simplification pass**~~ — **BUILT, UNPUSHED.** P1-P3 from
   [pr-round-10/self-review/FINDINGS.md](pr-round-10/self-review/FINDINGS.md); P4 stays DECLINED and
   he has been told so. Three shipped commits, each with a test, each mutation-checked. Draft reply
   and fork messages in [pr-round-12/](pr-round-12/). See "The simplification pass" below.
3. **Addressed writes**, and the gen1 caveat gate with them — both recorded in
   [pr-round-10/unaddressed-write-finding.md](pr-round-10/unaddressed-write-finding.md).
4. **Re-test on device**, including the TI cards.
5. **The squash message** at merge — draft in [squash-message.md](squash-message.md), condense to
   60-100 lines first.

mfcarroll leans toward keeping the addressing work inside this PR and said so in the reply; the
scoping decision is with mishamyte.


## Where things stand

**Everything through round 11 is PUSHED and POSTED** — fork head `a27d187d`. **The PR is waiting on
him, not on us.**

Round 11 was his review of the round-10 push: seven findings, nothing blocking. Two needed no fix
(the cut had already removed the cost footnote and the "claim of 49" sentence). The rest became the
seven fork commits:

| | |
|---|---|
| `9b80e348` | the comments state the constraints and stop arguing for them |
| `ab647375` | the write-fail card-lost term is defensive, and says so |
| `7350fbd3` | three mangled comment wraps, and a note naming one mode of three |
| `0bd82c19` | the no-latch result covers three chips, not gen1 silicon |
| `8d419a09` | the gen1 consent screen says why it has no validation line |
| `44a2a0c1` | the sweep's reach depends on reads, not on the card's claim |
| `a27d187d` | block 57 needs one more silent block than 56, and the clock is a third exit |

**The cut, measured on the tree he has:** 84 comment lines out, code `+0 −0` (preprocessed output
byte-identical for every shipped `.c`/`.h`). Surface 1,544 → 1,460, block mass 740 → 610 across
40 → 36 blocks, 2.3 section 177 → 115.

**The estimate published in round 10 was 250–370 and the answer was 84.** Fourth time a comment-pass
estimate has missed in the same direction. Do not quote a projected line count to him; quote the
measurement after the fact, taken against the tree he can open.

### The simplification pass — BUILT, UNPUSHED

Item 2, and the first code change since the comment cut, so none of it has the "code bytes unchanged"
safety net. Each landed with a test and each was mutation-checked from the committed baseline.

| dev | |
|---|---|
| `a5070a1` | **P1** — the clock cut decided in one place, with both halves of the reason |
| `5ea5a5b` | **P2** — the three succeeded-blocks subtractions say why they saturate |
| `998e4b4` | **P3** — one clamp for block size, against the macro rather than a buffer |
| `d6036be` | dev-only: the fake tag modelled a latch the hardware does not have |
| `6ac73fa` | dev-only: the fake-flash analogy leaves the dev tooling too |

**All three were the same shape**: one rule spelled in two places, each copy carrying part of the
reason and neither carrying all of it. That is worth knowing before P-items are picked for a later
pass — it is a better predictor of value than line count.

**Sync points need no reordering this time**: dev order already matches fork order, three shipped
commits, one decision each. [pr-round-12/fork-messages/README.md](pr-round-12/fork-messages/README.md)
has the mapping and, more importantly, what those messages must NOT claim — every test is in
`tools/hosttest/`, which does not sync, so a message citing them would describe a change absent from
its own diff. That is the round-11 defect, avoided by putting the test evidence in the reply instead.

**DECIDED, and do not re-open without him: P1-P3 is NOT published as a side branch.** It was
considered while he was away. A loose branch cannot be merged, needs a comment to explain it (which
lands in the reviewer's queue exactly as a push would), and splits his attention while he is mid-review
on round 11 — the same objection as pushing, moved sideways. Nothing degrades by waiting.

**Mutation-testing gotcha, cost real confusion:** a scripted revert can land in the same second as the
object file, and make then treats the target as up to date indefinitely — a stale binary reporting the
MUTANT's result against restored source. `make clean` belongs in the mutation loop. The harness's own
dependency tracking is fine; this is a make property, not a Makefile defect.

### What round 11 cost us, and why

Three things went wrong and all three are the same shape — a claim that was true when written and
went stale when the tree moved under it.

- **The cut deleted the carve-out he quoted as correct** (`a card that refuses every write but still
  serves a read never accumulates a run`). Restored before it shipped, so he never saw it.
- **The cut dropped a negation**: `A cut wipe records nothing above its cut` became `records above
  its cut`, which inverts the claim. The sense pass missed it; the review workflow caught it.
- **Three fork messages claimed more than the final tree supported**, including one asserting an
  armed 56-block LRi2K "satisfies neither" conjunct in a commit whose point is that the rule IS a
  conjunction. Caught by re-checking every claim against the tree before the replay.

**The rule that comes out of it:** re-verify every number and every claim against the tree at the
moment of publishing, not against the round the sentence was drafted for.

### Churn control — how the seven were chosen

`replay-to-fork.sh` syncs the TREE at a named dev commit, so sync points decide what he watches
happen. Fifteen dev commits touched shipped files; seven became fork commits. **Dev history was
REORDERED** so the restorations sit immediately after the cut — otherwise 01 would have shipped the
dropped negation and a later commit would have put it back. Reordering was verified by tree hash:
identical before and after.

Residual churn is 11 lines, all one paragraph written in 01 and rewritten later for a DIFFERENT
reason (the latch paragraph, the carve-out, one CHANGELOG line). The harmful kind — delete-then-
restore — is absent. See [pr-round-11/fork-messages/README.md](pr-round-11/fork-messages/README.md).

### Prose rules he made us learn, round 11 edition

**No internal process in anything posted.** Not when something was cut, not that an earlier commit
in the same push already removed it, not that we broke and restored something he never saw broken.
He reads the delta between the round he reviewed and the next one. "Cut" is a complete answer.

This applies to COMMIT MESSAGES too: `replay-to-fork.sh` takes its message from the `.msg` file, and
round 10's were reflowed copies of the dev messages — which is how he came to quote one. Round 11's
dev messages were NOT copyable and the fork messages were written fresh.

### THE BENCH RUN IS DONE — 4/4 gen1, and it corrected one of round 9's own fixes

All four pm3-marked candidates are **gen1**: `slix-1k-50x28`, `slix-1k-coin18`, `slix-1k-50mm` (NXP
SLIX, 28 blocks) and `SL2S5302` (NXP SLIX-S, 40). With the LRi2K that is five cards across three
chips, and the CHANGELOG says so now.

**Two corrections came out of it, and both were standing facts here.**

1. **The backdoor addresses are NOT memory.** "Writable memory above the advertised count" was the
   wrong reading of the LRi2K, by analogy with a gen2 card's over-claim. With `--capacity-max 70`
   searching past them on all four new cards, every one stops at its advertised count and
   56/57/62/63 answer **no read, ever**. Write-only registers outside the memory map. A 28-block
   gen1 card has 28 blocks.
2. **The wipe reaches the UID registers from a claim of 49, not 57.** The sweep keeps writing past
   the claim until `ABSENT_RUN` blocks answer nothing, so a card silent from A is attempted through
   A+7, and a landing write resets the run — so 56 and 57 both go at 49. Measured 44→52, pinned by
   tests at 48/49. **The 57 figure was his, from round 9 thread 01, adopted without checking.** The
   band it wrongly called safe is 49–56.

   It runs toward safety: only the 56-block LRi2K reaches its own UID registers under a wipe. The
   other four trip out seven blocks past their claim.

**And a harness bug worth not repeating.** `pm3_exec` decoded pm3 stdout as strict UTF-8. `hf 15
rdbl` prints an ASCII rendering of the block, so block 0 of any **NDEF-formatted** tag — which opens
`E1`, the CC magic number — made the decode throw, and the caught exception was recorded as a FAILED
READ. Three bench sessions went looking for an RF cause for `slix-1k-50mm`. The tell was in the
first capture: every failure named the same byte offset (472/474), and RF does not fail
deterministically to the byte. Fixed with `errors="replace"`; the hex column the parser uses is
ASCII and survives. Same class as the `gcc -fpreprocessed` and `grep -q` false passes.

### The cut, as its own round — DONE 2026-09-15, see "Where things stand". Kept for its reasoning.

**The consent screen no longer reports gen1's validation status at all** — that was folded into the
first round-10 commit rather than shipped as a round-trip, since correcting it and then removing it
inside one batch is the intra-batch churn he has flagged twice. What the user consents to is the
write and its blast radius; the sample size is a fact about the project and lives in the release
notes.

**The bench run is now about the CHANGELOG claim, not the screen.** "validated on hardware, on one
ST LRi2K card" is a claim about evidence, and that is where more cards change the text.

Four `pm3`-marked tags have **never had a write probe run**, and the seller said those are the
proxmark-writable ones:

| tag | blocks | state | gen1 probe |
|---|---|---|---|
| `SL2S5302` | 40 adv / 40 phys | blank, restorable | safe — 56/57/62/63 above measured capacity |
| `slix-1k-50mm` | 28 adv | `[0,1]` unread | safe |
| `slix-1k-50x28` | 28 / 28 | blank | safe |
| `slix-1k-coin18` | 28 / 28 | blank | safe |

**The probe is a real test, not a foregone negative.** gen1 needs 56/57/62/63 to exist, and on all
four those sit above the *measured* capacity — but a read-based capacity figure is only a LOWER
BOUND, and `lri2k-keychain` is exactly the case that proves it: 56 advertised, and it took writes at
56/57/62/63 anyway. So the probe discovers whether these answer the gen1 backdoor at all. (RESULT:
all four do, and none of them has memory there — see the corrected finding below.)

The four unmarked tags (`slix-black-38x25` 28 blocks, `slix2-gold-30mm` 79/82, `ti-2k-silver-1/2`
64 each) are expected to refuse — the seller said they need a custom application. Note the last
three have the backdoor blocks genuinely IN range, so the probe is destructive there; all are blank
or restorable, but run them last and restore from baseline.

**Then the cut**, scoped in [pr-round-10/comment-cut-measurement.md](pr-round-10/comment-cut-measurement.md).
Its projection was ~250-370 lines out of `iso15693_poller.c` alone; **the shipped figure was −84
across the surface**. The advice that held: do not go after the header, whose 64% is per-field
contract and the least compressible comment here. The advice that did not: the opportunity in the
poller was a fraction of what was projected.

**ROUND 10 = the self-review mishamyte asked for**, run 2026-09-12 with
`/pr-review-toolkit:review-pr` (seven agents) over the reconstructed FINAL-STATE PR diff — 27
files, +3922 −45, built by replaying `sync-to-fork.sh` onto the PR merge-base so it covers the
unpushed commits rather than only what he can see. Everything is in
[pr-round-10/self-review/](pr-round-10/self-review/): `SCOPE.md`, `FINDINGS.md`, the diff, and the
session's own mechanical results.

### The finding to lead the reply with — it explains rounds 8, 9 AND 10

**`tools/gen1-staleness.py` could not see a single one of the eight stale gen1 sites that survived
the B-round**, including two consent-screen strings that told users gen1 was untested. Two
vocabulary gaps, both the same mistake: the patterns match the phrasing that was *already fixed*.

- `latch(es)?\b` misses `latched` — the word is at three shipped sites, and the scanner flagged the
  two that are CORRECT.
- the validation pattern matches "validated", never "tested". `2edb202` removed exactly three sites,
  all spelled "NOT hardware-**validated**"; the survivors all spelled it "not hardware-**tested**".

A pattern list written by reading the sites you just fixed encodes their vocabulary and is
systematically blind to the ones you missed — and then reports the job done. Fixed, plus a SELFTEST
list of known-stale phrasings asserted to match (runs on every invocation, refuses to scan if it
fails), plus a loud exit when given no files, since a bare run printed nothing and exited 0.

### The one behavioural fix, and it is a real user-facing gap

**A wipe that loses the card never said the identity check had not run.** It returns before
`VerifyWipe` is entered, `has_details` sent CardLost to its default arm, and "UID not re-checked"
is reachable only through Details. By then the sweep has written 56/57 — which on an armed gen1
card ARE the UID — so the card could be gone and its identity with it, and the user was told only
that they had removed it. `has_details` now covers CardLost when `wipe_mode && !uid_verified`; the
note's wording is chosen by route (a card-lost wipe never reached a field reset); and the block
list is SUPPRESSED there, because the sweep's own comment says a lifted card "can surface as a pile
of blocks that wouldn't clear". Four tests, each mutation-checked to kill exactly one.

Second code change: the wipe's live progress denominator moved BELOW the geometry guard, so a card
reporting `block_size == 0` can no longer carry its own unverified claim into a terminal event.

### What the review did NOT find, which is worth saying in the reply

A full correctness pass found **no functional defect** — memory, buffers, tick wraparound,
tail-drop arithmetic, state-machine completeness, Back handling and the poller/scene thread
boundary all verified, the "widget copies its string" constraint checked in the SDK rather than
assumed. Nothing treats a refused backdoor write as proof it did not land. All 13 reason codes
reach a titled screen. Every numeric figure outside two off-by-ones matched.

### Still OPEN from round 10 — deliberately not done

These were in the review but NOT in the five items that were actioned. Decide before the reply:

- **Y1** `mark_failed`/`unmark_failed` write the bitmap with no bound; the invariant is held by four
  call sites. A `furi_check` would make it structural.
- **Y2** the reason enum has three silent `default:` absorbers and `NotMagic == 0`, which is also
  the scene manager's default state. `…Unset = 0` plus dropping two defaults gets `-Wswitch`.
- **Y3** `Iso15693PollerResult` has no `mode` field though six of sixteen fields are mode-scoped.
- **Y4** `ISO15693_MAGIC_BLK_*` (gen1 blocks) and `ISO15693_MAGIC_V2_BLK_*` (gen2 registers) are
  both `uint8_t` and both spelled `BLK`; a gen2 ref into `build_gen1_frame` destroys four blocks.
- **X1** two implicit `uint16_t -> uint8_t` narrowings, the only two in the file that lack a cast.
- **P1-P3** three simplifications worth doing (the duplicated pass-cut is the real one: two write
  points for `pass_truncated`/`pass_cut_block`, each holding half the rationale).
- **P4** the twelve `widget_add_string_multiline_element` calls — a wrapper works and the y
  rationale is already hoisted to the file header, so the old objection is weaker than the notes
  said. **Declined on diff cost at round 10**, not on principle. Tell him, so it is not re-derived.
- **T3** zero coverage on the reason-code selection ladder and on the gen1 consent screen — the
  highest-consequence routing in the feature and the only screen where a wrong button destroys a
  card. Both are ~60 lines of test against harnesses that already exist.

### Verified on the current tree, 2026-09-12

| check | result |
|---|---|
| host tests, from `make clean` | **114 run, 0 failed** (108 -> 114) |
| Momentum `dev` @ slix, forced rebuild | **zero nfc_magic warnings** |
| Unleashed `unl092-base` | **zero nfc_magic warnings** |
| clang-format, shipped | **95 files, 0 need formatting** |
| intra-batch churn, all 13 round-10 commits | **none** |
| comment-only | 3 comment-only, 7 code/CHANGELOG, 2 dev-only tools |

⚠️ **`git checkout -- <file>` ate an uncommitted fix during mutation testing this session**, exactly
as the rule below says it would. And GNU Make 3.81's whole-second timestamps reported a wrong test
red and then a dirty baseline. **Mutation-test only from a committed baseline, and always
`make clean`.**

## Round 8 and earlier — where things stood

PR #250, `nfc_magic_dev` on branch `iso15693-dev`.

**THE GEN1 B-ROUND IS BUILT — eight shipped commits on dev, on top of `56e8f49`, NOTHING PUSHED.**
Seven are comment-only (proven with `tools/comment-only.py`) and one is the CHANGELOG. Zero C code
changed. Both firmwares warning-free, 108 host tests, clang-format clean, all signed.

**IT WAS BUILT TWICE.** The first build assumed the latch was unmeasurable; a second bench session
measured it, and four of those seven commits stated the superseded model. The round was reset and
rebuilt rather than patched, because a series that asserts a thing and retracts it two commits later
is the intra-batch churn he has flagged twice. Nothing was pushed, so it cost nothing.

⚠️ **AND THE RESET SILENTLY TOOK THE BENCH FINDINGS WITH IT.** `git reset --hard` to before the round
also dropped the notes commit holding Findings 5-7 — the only record of the session. Recovered from
the safety branch, which is the whole reason to make one before a reset. **Never reset a round without
`git branch wip-<name>` first, and check what notes commits sit inside the range.**

**THE TWO REVERSALS, both in [gen1-hardware-findings.md](gen1-hardware-findings.md) as Findings 5-7:**

1. **There is no power-up latch.** A written UID takes effect IMMEDIATELY — an INVENTORY in the same
   field session already returns it. The app was built on the opposite premise and four comments plus
   two CHANGELOG entries said so. **The `NfcCommandReset` STAYS** — it is free, it re-activates for a
   clean read, and gen2's UID is in a register space this was never tested against — but it is
   belt-and-braces now, not load-bearing, and the comments say which.
2. **"The backdoor registers accept writes WITHOUT acknowledging" was a PARSE, not a measurement.**
   62/63 answer with error 0x10; `hf 15 wrbl --ua` renders that as `( fail )`, which session 1 read as
   an absent ACK. Do not reinstate it. What replaced it is stronger and does not need silence: on an
   armed card BOTH register writes are refused and the UID moves anyway, so a poller acting on those
   returns would abort a run that worked.

**THE SURFACE ESTIMATE WAS WRONG, and in the wrong direction.** The findings doc predicted "20-40
lines out before C starts". It came out **positive**. A measurement is LONGER than the hedge it
replaces — it carries the card, the date and the observation. Do not budget a B-round as a reduction;
C is the reduction.

**#255 still carries the softer "moved UID" wording**, which this round makes wrong rather than merely
soft. The earlier do-not-reopen decision is worth revisiting: "left with no valid identity" is a
different warning from "the UID moved".

**ROUND 8 SHIPPED 2026-09-11.** Fork pushed `dbc6e4fa..09778b6d`, a fast-forward of **16 commits**,
all signed. Main reply at **issuecomment-5641879307**, **all 16 thread replies posted**, and the #251
scope correction at **issue 251 issuecomment-5641885668**. Everything verified byte-identical against
the drafts after posting. PR now reads **108 commits, 27 files, +4,123 −45**.

His round 8 was `COMMENTED` and closed "nothing blocking beyond the regression". **The standing
`CHANGES_REQUESTED` is stale — 2026-08-16 — and his last five reviews are all `COMMENTED`.** Do not
read the badge as an objection.

**13 of the 16 were comment-only**, proven with `tools/comment-only.py`. The three that were not: the
`write_confirm` title regression, `ISO15693_POLLER_MAX_BLOCKS` across three files, and the CHANGELOG.

What the reply commits us to, so a later session does not contradict it:

- **The case for finishing before the merge is made, and made as HIS interest** — the PR squashes, so
  tidying inside it costs `dev` nothing, while the same work afterwards is a second PR and a diff of
  pure comment churn against a released app.
- **The volume arithmetic is conceded in public**: the cut removed 95 comment lines, round 7's own
  corrections put 85 back, this round another 25. Fifteen lines worse off than before the cut.
- **The harness is promised as its own PR**, with the sizing that rules it out of this one.
- **The gen1 round is put back in front of him**, on the grounds that the card now exists.
- **A CHANGELOG correction is offered separately** if he merges before the gen1 round —
  `CHANGELOG.md:145` still tells users the gen1 path was never tested against hardware, which is now
  outright false rather than cautious.

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

**The round-7 comment cut was BUILT: sixteen commits on dev, on top of `04d5f8a`, all signed.** (It
shipped long ago — this paragraph is the round-7 state as it stood. NOT the 2026-09-15 cut, which is
a separate, later pass that shipped as round 11; see "Where things stand" at the top. Two different
deltas have been called "the comment cut" and BOTH are now pushed.)
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
- ~~the **compact-UID formatter's four copies**~~ — **DONE, struck 2026-09-12.** The helper
  `iso15693_info_cat_uid` exists (`iso15693_info.c:283`, declared `iso15693_info.h:27`) and those
  four sites are now call sites of it. No duplication remains; do not re-plan it.
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
- **The PR body is NOT being rewritten — settled 2026-09-12.** It is what a reader of the PR page
  sees and it never enters git history. Step 2 of the pathway used to say "worth doing on its own
  merits", which contradicted this; that step is struck. The squash message is the only one of the
  two worth writing.
- Consequence for the comment work: BOTH tests are live. "Would a maintainer editing this line, offline,
  need it?" is the strong one. "Would it be at home in the commit message?" is valid again but a WEAK
  home — it survives only if nobody overrides.

**Do not re-inherit the wrong premise:** comment VOLUME is not his objection. Checked against all 86
review events — he has never asked for less comment, only for accurate comment. The 42%-vs-10% figure and
the cut itself are ours. It IS an outstanding public commitment (round 6: "not treating it as optional"),
which means **the scope is ours, not his.**

## THE PATHWAY TO RELEASE, in order — settled 2026-09-08, step 3 added 09-09

1. **Push + post Round 7** (built, verified, awaiting a go-ahead).
2. ~~**Improve the PR body**~~ — **STRUCK 2026-09-12. Decided: we are NOT rewriting the PR body.**
   Only the squash message. The body never enters git history (#258: a 9949-byte body against a
   799-byte squash message), so it is the one artifact where effort does not compound. This step
   contradicted the "optional polish, not a priority" line in the section above, which is the
   reasoning that holds; the contradiction stood for three rounds and was acted on once. Do not
   re-open.
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
5. ~~**C — does it need to be there?**~~ **DONE, shipped as round 11**: 84 comment lines out, code
   `+0 −0`, surface 1,544 → 1,460. The 1,576 / 787 / 43 / 250-370 figures this step used to carry
   were measured against a tree that never shipped and the estimate missed by 3x; both are recorded
   in [pr-round-10/comment-cut-measurement.md](pr-round-10/comment-cut-measurement.md).
6. **D — can it be correctly simplified?** SMALLER than C and separate from it. The live items are
   the self-review's P1-P3: the duplicated pass-cut (two write points for the two fields that gate
   Retry, half the rationale at each), the three saturating `total - bad` subtractions whose shared
   constraint is explained 600 lines away, and the block-size clamp spelled two ways. **P4 —
   wrapping the twelve `widget_add_string_multiline_element` calls — is DECLINED on diff cost**, and
   the old objection recorded against it (that the per-site y values carry the line budget) does NOT
   hold, since that rationale is hoisted to the file header. Say so rather than let it be re-derived.
7. **THE RELEASE NOTES — added 2026-09-13, previously untracked. RUNS WITH STEP 5, not after it**
   (decided 2026-09-13). The cut deletes the code-side copies of reasoning the release notes also
   carry, so splitting them leaves the two artifacts briefly disagreeing and gives him two tightening
   deltas to read instead of one. The 2.3 section is **178 lines
   against 14 and 19** for the two entries before it, i.e. half of CHANGELOG.md for one feature.
   Same discipline as C: what a user needs about the card in their hand stays, the reasoning goes.
   This was discussed across several rounds and never written down anywhere, which is why it kept
   being re-raised.

Nothing in 3-6 is a merge blocker, but there IS a reason to want all of it BEFORE the merge, and it
is mechanical rather than a taste for polish: **the PR squashes, so everything inside it collapses to
one commit and the tidying costs `dev` nothing.** The same work afterwards is a second PR, a second
review, and a diff of pure comment churn against a released app — more expensive for him than for us.
State it that way round; "nothing blocks a merge, and I am not asking you to hold" gives away an
argument that is actually in his interest.

**Do not over-claim what he said about gen1.** The line is "on the tags: worth having, but nothing
here waits on them. None of the three blocking items below needs a gen1 card" — that is about the
BLOCKERS being reachable on gen2, said while the tags had not arrived. It is not "merge without the
gen1 round", and earlier notes here paraphrased it that way. The card exists now, which makes that
round bounded work rather than an open wait, so it is worth putting back in front of him rather than
treating his old answer as settled.

The order is still a preference. Running C before 4 would have C protect text the gen1 round is about
to replace.

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

### Surfaced by the round-14 self-review, UNFILED and not in this round — 2026-09-17

Both are pre-existing, neither is touched by round 14, and neither was on any list before now, which
is the only reason they are written down here. Checked against the notes first: the third item the
review raised -- `NothingWiped` having no `uid_verified` route -- is ALREADY settled above and in
[#255](https://github.com/xMasterX/all-the-plugins/issues/255), so it is not repeated.

1. **The gen2 CFG frame programs an UNCLAMPED block size.** `cfg_blocksize = (uint8_t)(sys->block_size - 1)`
   takes the source's raw value while every data write goes through `iso15693_poller_clamp_block_size`
   and is capped at 32. Reachable only through a hand-edited `.nfc`: the SDK's loader checks
   `block_size > 0` and nothing else, and that is byte-identical in Momentum, Unleashed, RogueMaster,
   Xero and official, so 1..255 can arrive. A source claiming 200 programs 199 into the card's
   geometry register while writing 32 bytes a block, leaving a clone whose reported geometry its own
   contents do not match. Not a safety issue -- the write path is clamped -- but the two should agree
   on one number, and the clamp's new note now makes the asymmetry easy to see.

2. **The unusable-geometry return blames the card for a refusal it was never asked for.**
   `iso15693_poller_wipe_blocks` returns 0 on `advertised == 0 || block_size == 0` before a single
   frame goes out, and that lands on the same `wiped == 0` branch as a card that refused everything --
   where the screen reads "No blocks could be cleared: the card accepted no zero-write." It accepted
   nothing because nothing was sent. Nothing is logged at that return either, so a card reporting no
   geometry is indistinguishable from a card refusing writes, both on screen and in the log. It also
   carries `wipe_advertised` (set above the guard) against `blocks_total` 0 (set below it), so the
   screen can show a claim of 64 beside a total of 0 with nothing explaining the gap. A third branch
   in the `nothing_wiped` body and one `FURI_LOG_E` would close both halves.

## Open, waiting on him

- **The scoping call on addressed writes** — whether they land inside this PR or as a follow-up.
  mfcarroll leans toward inside and has said so twice; the decision is his. This is what "should not
  merge yet" rests on.
- **The comment cut's scope** — asked at the end of the round-10 reply, and round 11 did not answer
  it either way. The cut shipped regardless; the open part is whether he wants more taken.
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

  **THE RULE COVERS THE MAIN REPLY TOO, AND THE 7-GRAM SCAN WILL NOT FIND MOST OF THE PADDING.**
  Learned again in round 8, where the rule was already written down and twelve of sixteen thread
  replies plus the main reply broke it anyway. The overlap scan flagged exactly ONE (16.7%); the other
  twelve measured **0-5% and were still padded**, because paraphrasing his reasoning in fresh words
  shares no 7-grams. So the scan catches quotation, not restatement. **The only check that works is
  per-paragraph, by hand: does this tell him something he does not already know, and can he act on
  it?** Run it over every paragraph of every reply including the main one.

  Three shapes to delete on sight, none of which the earlier version of this rule named:

  - **Describing the diff he is about to read.** "Two comment lines say why the title is not taken
    from `is_wipe`", "The line now says why the derivation is written out". He reads the diff commit
    by commit; a prose summary of it is strictly worse than the diff.
  - **Justifying our own action to him.** "It is new information about the hazard's size, not a
    restatement of what is already filed." Nobody asked. Say what was done.
  - **Explaining why we are telling him something.** "Laying it out because you are close to ready and
    I would rather you know what is still coming than discover it." State the thing.

  And the tell that started the round-8 sweep, which the user caught: any sentence opening **"Worth
  noting"**, **"It is worth"**, or **"One thing to note"** is almost always about to narrate our own
  process. Grep for them before posting.

  What the trim is worth: thread replies 202 -> 161 payload lines, main reply ~1,200 -> 1,030 words,
  and nothing of substance lost — every cut was his own reasoning, a description of a diff, or
  process narration.

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

- **"WE CANNOT TEST THAT" GOES STALE THE MOMENT HARDWARE ARRIVES, AND NOTHING CHECKS IT.** Caught by
  the user 2026-09-11 in the #251 draft, which repeated the issue's original "it would need two
  ISO15693 tags and would destroy data on the second". That was true on 2026-08-11, when there was one
  tag. **There are fourteen now, every one with a captured baseline and restorable from it, and most
  of them blank** — see [tag-inventory.md](tag-inventory.md). So the two-tag test destroys nothing
  permanent and is available today.

  This is the same class as the gen1 hedging, but `tools/gen1-staleness.py` does not catch it: that
  scans for claims about the gen1 PATH, not for claims about what the bench can do. **Before repeating
  any "not staged / needs a card we do not have / would destroy" line, open the inventory and check
  the count and the restorable column.** The phrases to grep for are `not staged`, `no card`, `needs a
  card`, `would destroy`, `cannot test`, `nobody on the PR has`.

  And when correcting one of these in a comment on our own earlier report, **say the reason expired
  rather than quietly dropping it** — the old text stays visible above the new comment, so an
  unexplained reversal reads as inconsistency.

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
- **`gcc` ON THIS MACHINE IS CLANG, AND CLANG HAS NO `-fpreprocessed`.** Caught 2026-09-11 when the
  user asked why the reply claimed "every commit touches shipped code" for a round that is mostly
  comments. The comment-only check run inline as
  `gcc -fpreprocessed -dD -E -P <file> | md5` **errors out and emits NOTHING**, so comparing two
  empty outputs reported "identical" for every pair — including a commit that changes a ternary and
  adds a line. A false PASS of exactly the `grep -q` shape, and it was quoted in a commit message as
  "proven so rather than asserted".

  **Use `tools/comment-only.py`**, which finds a real GCC in
  `../Momentum-Firmware*/toolchain/arm64-darwin/bin/arm-none-eabi-gcc`, and **exits non-zero if the
  preprocessor emits nothing** rather than treating emptiness as a match. `--range A..B` classifies
  a whole round. This is the check the C deletion pass promises ("code bytes unchanged"), so it has
  to be the thing that cannot pass by accident.

  The round-8 answer it gives: **13 of 16 comment-only**, the exceptions being the title regression,
  `ISO15693_POLLER_MAX_BLOCKS` across three files, and the CHANGELOG.

  **And the reply bullet it replaced was wrong for a second reason:** "none is notes-only" names a
  category the fork does not have. `.notes/` and `tools/` exist only in the dev repo, so on the PR
  every commit is in the pack by construction and the claim is both invisible and, read as English,
  the opposite of true. **Shipped-vs-dev-only is OUR bookkeeping; never put it in a reply.** What he
  wants there is code-vs-comment, which tells him where to spend attention.

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
  2. ~~**Writable memory above the advertised count.**~~ **CORRECTED 2026-09-12 — it is not memory.**
     It advertises 56 blocks and took writes at 56/57/62/63, and the first reading of that was extra
     memory above the claim, by analogy with the gen2 card holding blocks above its own. The four-card
     run settles it the other way: on all five gen1 cards those addresses answer **no read at any
     point**, with `--capacity-max 70` searching straight past them. They are WRITE-ONLY BACKDOOR
     REGISTERS that the magic silicon decodes as a UID-set command — not capacity, and not part of the
     memory map. A 28-block gen1 card has 28 blocks. The gen2 card's over-claim is a genuinely
     different phenomenon and the analogy was the thing that misled.
  **NOT settled: the latch.** `SetTag15693Uid` ends in `switch_off()`, so every read-back sits behind a
  field power-cycle and cannot distinguish "latches on power-up" from "changes immediately". The app's
  `NfcCommandReset`-before-verify is still justified by the model, not by measurement. Isolating it needs
  a read in the SAME field session as the write.
  **THE ARMED-CARD WIPE HAS BEEN RUN — an earlier version of this bullet called it the next test and
  it was done in the same session.** The wipe zeroed 56/57, the armed card latched on power-up, the
  identity moved, and the post-power-cycle re-read caught it: `uid_changed`, reported Partial. The
  mitigation this PR ships works on real gen1 silicon.
  **And the outcome is WORSE than either #255 or the poller says.** The UID did not move to another
  valid identity — it moved to **all zeros**, and an ISO15693 UID must begin with `0xE0`, so the card
  is left with no valid identity at all. "Moved" and "changed" both understate it. Recovery is
  byte-identical via `hf 15 csetuid`, and only possible because the app prints the original UID, which
  is the argument for that screen.
  **The card stays armed after a wipe**, so it is a reusable fixture: restore, test, restore.
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
- **A `changelog:`-prefixed commit flows through the sync correctly — checked 2026-09-11.**
  `sync-to-fork.sh` does not filter by subject AT ALL; it overlays by path, and `CHANGELOG.md` is in
  `APP_PATHS`. Commit SELECTION is done by hand when generating `fork-messages/`, using the same path
  list, so a `changelog:` commit is picked up there too, and the subject-strip regex covers
  `changelog` alongside `iso15693|nfc_magic|scene_write`.
  **But `application.fam` is NOT in `APP_PATHS`** — the script only seds `fap_version` across
  (`sync-to-fork.sh:52-53`). So a commit changing anything ELSE in the fam (the sources list, a new
  asset, the stack size) is selected for replay by the path filter and then silently not transferred.
  No such commit has existed yet. Check for one before trusting a replay.
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
- Also adapt the BODY for publication: drop mentions of `tools/hosttest`, whose files are not in the
  pack. **But do NOT move it to second person — that instruction used to live here and it was wrong,
  corrected 2026-09-11 after the user caught it.** A commit message outlives the review: it is read by
  whoever runs `git log` on `dev` in two years, and "you are right that..." reads to them as half of a
  conversation they cannot see. The data agrees — only **11 of 143** existing fork commits use "you",
  **seven of them ours from round 7**, and the older norm is third person or no attribution at all
  ("Split out at mishamyte's request"). Second person belongs in the PR reply, which IS a
  conversation. In the commit, state the defect and the fix flat: "Flagged in review" / "Suggested in
  review" where provenance carries information, nothing where it does not.

  Three things go out with the pronoun, all of them the commit-message form of the reply padding:
  **dialogue narration** ("You found it, and found the tell with it"), **self-assessment** ("which is
  better than a corrected count"), and **descriptions of the diff** the reader already has open.

  Length is NOT the problem and should not be cut for its own sake — measured 2026-09-11, existing
  NFC-Magic fork messages run **median 243 words, mean 263, p90 375**. The round-8 sixteen average 226
  after the rewrite, already under the median. The commit test still governs what goes in: does it
  tell a reader of the diff something the diff cannot show?
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
