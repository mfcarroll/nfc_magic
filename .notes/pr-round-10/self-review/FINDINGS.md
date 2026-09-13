# Self-review of PR #250 — consolidated findings

Run 2026-09-12 at mishamyte's request (`/pr-review-toolkit:review-pr` + `/simplify`).
Seven agents over the **final-state** PR diff — **27 files, +3922 −45**, which includes the 15
shipped commits not yet pushed (PR head is `09778b6d`; dev is 15 ahead).

Diff rebuilt by replaying `sync-to-fork.sh` onto the PR merge-base, so it is what the PR *will*
show, not what it currently shows. Every finding below was re-verified by hand against the code.

**Nothing here changes what the app does on the happy path.** One finding is a real user-facing
safety gap on the destructive path; the rest are comment/text defects and latent contract holes.

---

# THE SPINE: one broken tool produced eight stale gen1 sites

This is the finding to lead with, because it explains the other eight and because it is the same
mechanism that produced rounds 8 and 9.

`tools/gen1-staleness.py` is the scanner built to find comments the 2026-09-08/09-11 hardware
sessions invalidated. The gen1 B-round used it as its site list. **It catches none of the eight
sites that survived:**

| # | site | text | user-facing |
|---|---|---|---|
| S1 | `scenes/nfc_magic_scene_iso15693_gen1_optin.c:43` | `"Gen1 is not hardware-tested."` | **YES — consent screen** |
| S2 | `scenes/nfc_magic_scene_iso15693_gen1_optin.c:49` | `"Gen1 is not hardware-tested."` | **YES — consent screen** |
| S3 | `scenes/nfc_magic_scene_iso15693_gen1_optin.c:5` | `NOT-hardware-tested gen1 fallback` | no |
| S4 | `magic/protocols/iso15693/iso15693_poller.c:1382` | `destructive and not hardware-tested` | no |
| S5 | `CHANGELOG.md:78` | `the gen1 path is not hardware-tested` | **YES — release notes** |
| S6 | `magic/protocols/iso15693/iso15693_poller.c:882` | `the card latched on the next power-up` | no |
| S7 | `magic/protocols/iso15693/iso15693_poller.c:1399` | `on the only hardware this PR has` | no |
| S8 | `CHANGELOG.md:29` | `an open question pending a gen1 card to test against` | **YES — release notes** |

Against the tree's own statements: gen1 **is** hardware-validated (`CHANGELOG.md:149`,
`poller.h:103`, `poller.h:129`, `poller.c:38-45`), there **is** no power-up latch (`poller.c:43`,
`poller.h:80`, `CHANGELOG.md:152`), and the de-arm question is **no longer pending a card**
(`poller.c:876-890` records "Reproduced 2026-09-08 on an armed LRi2K").

## Why the scanner missed all eight — two vocabulary gaps

1. **`latch(es)?\b` does not match `latched` or `latching`** (`\b` blocks the inflection).
   `latch` appears at exactly three sites in shipped code. The scanner flags the two that are
   CORRECT and is blind to the one that is WRONG.
2. **The validation pattern matches "validated", never "tested".** Commit `2edb202`
   ("gen1 is hardware-validated, so the three NOT-validated notes go") removed exactly three sites,
   and all three said **"NOT hardware-validated"**. The four surviving sites say
   **"not hardware-tested"** — a phrasing the scanner has never been able to see.

So the round found and fixed precisely the set its tool could see, and reported the job done.

Suggested fixes (dev-only, `tools/`, does not touch the PR):
- widen to `latch(es|ed|ing)?\b`
- add `not hardware.?tested|NOT-hardware-tested` to the validation pattern
- **make it exit non-zero when given no file arguments.** Today a bare
  `python3 tools/gen1-staleness.py` prints nothing and exits 0 — indistinguishable from a clean
  pass. That is the `grep -q` false-pass shape the project has been bitten by twice.

Also `tools/hosttest/test_write_step.c:292` carries the latch model in a test comment (dev-only).

### The severity ordering inside the spine

S1/S2 are the ones that matter. They are the consent screen in front of the destructive path, and
they now tell a user that the only path that works on their gen1 card is untested. S5 and S8 are
release notes. The rest are maintainer-facing.

Note S5 is accurate *as a description of the current screen* — so it must be fixed together with
S1/S2, not separately, or the CHANGELOG becomes wrong the moment the screen is fixed.

---

# BEHAVIOURAL — the one finding with a real user cost

## B1. A wipe cut short by card removal never says the identity check did not run
`poller.c:1259-1262` → `write_fail.c:65` → `partial_details.c:133`

Traced end to end:
1. Armed gen1 card. The sweep reaches 56/57 — the UID registers — and an armed card **accepts**
   writes there (`poller.c:44-45`), so `wiped > 0` and the UID is now all zeros, which is not a
   valid ISO15693 identity.
2. User lifts the card. `card_lost` trips.
3. `poller.c:1259-1262` reports `CardLost` and returns `NfcCommandStop` — **before** the
   `VerifyWipe` state is set at `:1286`. So `uid_verified` stays false and the check never runs.
4. `has_details()` sends `CardLost` to `default: return false` (`write_fail.c:65`) — no Details
   button.
5. The "UID not re-checked" sentence (`partial_details.c:144-147`) is reachable only through
   Details. So it cannot be shown.

The user is told only: `"Card removed before the write could finish."`

The existing comment dismisses this path — *"CardLost has no Details, but the card left, so it is
moot"* (`partial_details.c:133`). **The reason it is not moot is ten lines below it in the same
comment block** (`:140-142`): on an armed gen1 card the UID registers are overwritten at index
56/57, "long before any plausible cut." The comment conflates *we could not check* with *there is
nothing to tell*.

**B1a, the follow-on:** `is_retryable` includes `CardLost` (`write_fail.c:29`), and every wipe run
re-reads `original_uid` from the card at `poller.c:1255`. So a Retry baselines against the
already-destroyed UID, finds no change, and reports **`WipeComplete`** — success tone, Finish
button. The one screen designed to catch this is disarmed by the button the previous screen
offered. (`uid_changed` being positive-observation-only is correct and well argued; it simply has
no memory across runs. A cross-run baseline is a design change, not a review edit — but the
interaction is written down nowhere.)

Smallest fix that does not reshape the diff: have `has_details` return true for `CardLost` when
`wipe_mode && !uid_verified`, which makes the existing sentence reachable.

---

# CONTRACT / LATENT — verified, nothing renders them today

## C1. `blocks_total` carries the card's unverified claim into a terminal event
`poller.c:861` sets `clone_blocks_total = advertised` as a live progress denominator. The early
return at `:869` (`advertised == 0 || block_size == 0`) fires **before** the measured overwrite at
`:1110`. That path reaches `NothingWiped`, a terminal event.

This breaks the header's stated rule (`poller.h:146-147`): *"Terminal events therefore always
report the measured figure; only WriteProgress can see the advertised one."* And the comment at
`:857-860` asserting it is "Overwritten at the end … **before any terminal event**" is false on
this path. No screen renders it today. One-line fix: move the assignment below the geometry guard.

## C2. `poller.h:41-42` — Partial's contract is false on the exact card the bound exists for
Header: a run "cut short by the wall-clock bound" is Partial. But the `wiped == 0` short-circuit
(`poller.c:1279-1282`) returns `Fail` first, so a card that refuses every write and answers every
read — the card the backstop was written for — reports **Fail**, not Partial. The scene layer holds
the correct rule (`write_fail.c:189-191`). Header is the wrong copy of the twin.

## C3. `poller.c:1177-1179` — "a cut run is Partial because claimed blocks were never attempted"
False when the cut lands **above** the advertised count: every claimed block was attempted. The
correct justification already exists at `poller.h:154-156`. Header right, `.c` overshot.

## C4. `CHANGELOG.md:36-38` promises a Retry the code withholds
"A wipe stopped by that limit is reported as **partial** — it names where it stopped and offers a
retry." Unqualified, and false at the `wiped == 0` edge: title is `"Wipe failed"`, `is_retryable`
excludes `NothingWiped`, `has_details` is false — so **no Retry and no second button at all**.

## C5. `nfc_magic_app_i.h:189-190` — fields documented `USCUID-UL:` that ISO15693 drives
`write_progress_current` / `_total` are written from `iso15693_result` at `scene_write.c:167-168`,
and `:165` says so. (`write_failed_count` / `_bitmap` really are USCUID-UL-only — those are right.)

## C6. `scene_write.c:368` — the surviving twin of the bug the helper above it fixed
`// Live "Writing X/N" while the USCUID-UL poller advances page by page.` The next statement is the
`is_wiping` ternary, and that helper exists (`:208-216`) *because* this handler is no longer
USCUID-UL-only. Comment one line from its own post-mortem.

---

# FACTUAL ERRORS IN COMMENTS — confirmed by checking the source cited

## D1. `iso15693_info.c:254-255` cites a proxmark function that does not exist
`// Provenance: proxmark3 cmdhf15.c getTagInfo_15 (masked UID table).` — **0 hits** in proxmark3
HEAD (5ca15e45). The real name is `printTagInfo_15` (`cmdhf15.c:321`) over table `uidmapping`
(`:97`). The *other* provenance note in the same file (`:166`, "from proxmark uidmapping") is
correct — one real name, one invented.

## D2. `iso15693_info.c:166-167` says an entry is omitted that is present three lines below
Comment says the `0x08 / 0x23`-C variants are omitted; `:168` is
`{0x02, 0x23, 0xFF, "ST25TV02K / ST25TV512"}`. In proxmark the -C variants are at IC id **0x08
only**. A maintainer reconciling the two could delete `:168` and break the decode.

## D3. `poller.c:1018-1019` — "both fail closed" describes the opposite of the code
The re-probe unmarks a block that answers and is empty (`:1043`), and its own inline comment at
`:1036-1039` states the correct rule. Only the blocks *below* fail closed. A maintainer trusting
`:1018` and "restoring" `mark_failed` would manufacture "Not cleared: N" for cleared blocks.

## D4. `poller.c:133` / `:497` vs `:506` — progress bound stated three times, two off by one
`step = done * STEPS / total` takes `STEPS + 1` values over `done ∈ [0, total]`, so a pass emits
**nine** events, not eight. `:506` is the correct copy. Harmless (9 < queue length 16), but this is
a comment the file calls "a hard safety bound, not a tuning knob."

## D5. `poller.c:636` — `done` labelled the denominator; it is the numerator
`uint16_t done = 0; // blocks attempted, the denominator the progress popup shows`. It is passed as
the position; `total` is the denominator. The wipe's twin (`:857`) uses the word correctly. A
maintainer "fixing the transposed arguments" would render "Cloning 64 / 3".

## D6. `write_confirm.c:84` — title and body of one screen contradict
`title = "Wipe? (gen1/gen2 only)"` over a body warning `"This can brick a gen3 card!"`. The
comment at `:52-54` states the truth: *"The wipe performs NO magic detection -- menu, confirm,
sweep"*, and `CHANGELOG.md:160-161` agrees. The title reads as an enforced scope the app does not
enforce. (The round-9 rationale for setting the title *in this branch* is sound — the defect is the
string, not the placement.)

## D7. `CHANGELOG.md:124` names a `"Card removed"` screen that has no such title
Every other name in that sentence is an exact title string. `CardLost` falls to `default:` and
renders **`"Write failed"`**; "Card removed" appears only in the body.

## D8. Smaller, all confirmed
- `poller.c:213` "what runs after the loop costs no airtime" — `:1131` also runs after the loop and
  issues a retried inventory, on exactly the card the paragraph is about.
- `poller.c:723-724` "only reachable when the card is still present" — the back-fill runs
  unconditionally; the card test is at `:758`. The sentence's own second half says so.
- `poller.c:1272-1274` "the sweep misses 56/57 **only when**…" — `:869` returns with no block
  attempted at all, a second path.
- `poller.h:222-223` `blocks_done` "converges on blocks_total" — deliberately false for a cut
  clone, argued at `poller.c:735-738`.
- `poller.h:213-216` `capacity_confirmed` doc omits the `!pass_truncated` conjunct the `.c` spends
  20 lines on (`:768-785`).
- `poller.c:590-596` clamp note — `:1325` sets `clone_blocks_total` unclamped for a pre-data-pass
  Fail.
- `iso15693_info.c:236-237` "a wider mask pins more chip-id bits" — the code compares masks
  numerically, which coincides with bit count only for contiguous masks; a non-contiguous 0x78 is
  used 30 lines later.
- `poller.c:171-174` cites `edgepages` / `aliased`, which exist only in `tools/` and so have no
  referent in the merged tree.
- `poller.c:561-569` — a forward declaration sits inside a function's doc block with no blank line.
- Process-not-state comments: `poller.c:545` ("the whole reason this was on the list"), `:978`
  ("the bounded loop this sweep replaced"), `:59` ("Each used to spell the set out").

---

# TESTS — 108 pass, and two of them are false passes

Harness verified: **108 tests, 0 failed** from `make clean`. Coverage measured with gcov:
`iso15693_poller.c` 73%, `write_fail.c` 94%, `scene_write.c` 19%. Mutation-tested: 10 mutants, 8
killed, 2 survived.

**The good news first, because it was the highest-value thing to check:** the stale-binary fix from
2026-08-18 is real and still works. Every TU compiles separately with its own `-MF`; the live
depfiles name the code under test; an end-to-end edit-and-rebuild on a copy went red on exactly the
right tests and green again on restore.

## T1. `test_gen1_skips_the_backdoor_blocks` cannot fail — CONFIRMED
`tools/hosttest/test_clone_blocks.c:193-209`. Mutating `is_backdoor_block` to always return false —
so a gen1 clone writes source data straight over the four backdoor registers — **leaves the test
green**. Two independent reasons:
- `fake_tag_init` fills the tag with `FAKE_MARKER` (0xA5) and `fake_data_init` fills the *source*
  with the same `FAKE_MARKER`, so `CHECK(content[56][0] != 0)` passes whether the block was skipped
  or overwritten.
- The `clone_blocks_total == 60` assertion routes through the deduction walk at `poller.c:603-607`,
  which iterates the array directly and never calls `is_backdoor_block`.

`test_gen1_partial_backdoor_overlap` (`:231-247`) has the identical defect. Fix is cheap: fill the
source with a different byte and assert `content[56][0] == FAKE_MARKER`.

## T2. The interior-failure guard on the capacity claim is unpinned — CONFIRMED
Dropping `!wrote_above_failure` from `poller.c:784` leaves the suite green.
`test_interior_failure_is_not_capacity` uses `FakeBlockLocked`, which **answers reads**, so
`any_failure_answered` produces the verdict and the guard never does any work. With
`FakeBlockAbsent` at interior block 10: real code `over_capacity=0, failed_count=1`; mutant
`over_capacity=1, failed_count=0` — i.e. a card with a mid-memory dropout reported "Clone finished
/ Holds 27 of 28", a fabricated verdict about the user's hardware.

## T3. Two zero-coverage gaps on the highest-consequence paths
- **The reason-code selection ladder** (`scene_write.c:452-506`) — gcov marks every branch `#####`.
  The chain is tested at both ends and unguarded in the middle. Reordering the `ModeWriteUid` test
  below the `blocks_total == 0` test makes every failed Write UID report "Nothing to clone", and
  the suite stays green. `:464` says "Order matters throughout"; nothing enforces it.
- **The gen1 consent screen** (`gen1_optin.c`) — no test at all. Swap the Left/Right button arms
  and all 108 tests pass while a user pressing "Back" writes 56/57/62/63 on an ordinary tag.
  The *lowest*-consequence routing in the feature is exhaustively tested; the highest is not.

## T4. Vacuity shapes and harness notes
- `test_write_fail_scene.c:162` — `if(label == NULL) continue;` executes zero assertions if every
  screen lost its right button.
- `:422` — `CHECK(strstr(scroll, "3") != NULL)`, a single-character search; live today, one wording
  change from passing unconditionally.
- `:135-149`, `:188-201` — hand-maintained 13-reason arrays with no sentinel; a 14th is silently
  uncovered. A `…ReasonCount` sentinel plus `_Static_assert` closes it.
- The two "verifying greps" at `Makefile:41-42` are **comments, not checks** — `make` never runs
  them, so a regression to one `cc` invocation would fail nothing. A `verify-deps` target that
  `run` depends on would close it.
- `make` here is GNU Make 3.81 with whole-second timestamps; scripted edit-then-build inside one
  second reuses stale objects. Any CI runner should `rm` objects or use `-B`.
- `fake_write.c:84` types `nfc_device_get_data` as returning `const NfcDeviceData*` where the SDK
  says `void`, producing five `-Wincompatible-pointer-types` warnings on every build of
  `scene_write.c`. Shipped code is fine; a permanently-warning build trains people to skim.

**The fakes are honest.** Checked specifically: refused AFI/DSFID writes that return `ErrorNone`
while swallowing the value, locked blocks that refuse writes but answer reads, past-capacity blocks
that fail both, advertised-vs-physical as independent knobs, a virtual clock for deterministic
wall-clock cuts, `furi_assert`/`furi_check` that fail the test rather than no-op. No fake returns
unconditional success where the code branches on failure.

**Highest-value missing tests:** (1) the `WorkerFail` reason ladder driven through
`nfc_magic_scene_write_on_event` — ~60 lines, pure data, no radio, and the harness already exists;
(2) the gen1 opt-in consent routing, which also gives `source_uses_gen1_blocks` its first coverage.

---

# TYPE DESIGN — ratings and the four cheap structural wins

ENCAPSULATION **7/10** · INVARIANT EXPRESSION **5/10** · USEFULNESS **9/10** · ENFORCEMENT **6/10**

The prose contract is exceptional and the ownership model really is where the header says it is.
The gap is that almost all of it lives in comments. Four changes, ~15 lines total, would convert
the most consequential prose invariants into compile- or crash-time ones:

- **Y1.** `mark_failed` / `unmark_failed` (`poller.c:546-552`) write `clone_failed_bitmap[block/8]`
  with **no bound**. The invariant is held entirely by four call sites' loop bounds; the fourth
  bounds on `source_count`, which arrives from a hand-editable `.nfc` and is clamped 50 lines away
  at `:597-598`. The clamp holds today. A `furi_check(block < ISO15693_POLLER_MAX_BLOCKS)` costs
  two instructions and converts silent struct corruption on a destructive path into a loud crash.
- **Y2.** The reason enum has three silent absorbers (`write_fail.c:65`, `:104`, `:386-392`) and
  `NotMagic == 0`, which is also the scene manager's default state. A 14th variant compiles clean
  and renders "Not a magic tag" with no Details and no Retry. Add `…Unset = 0`, drop the two
  `default:` labels so `-Wswitch` fires.
- **Y3.** `Iso15693PollerResult` has **no mode field**, though six of sixteen fields are
  mode-scoped and `blocks_total`'s *meaning* changes by mode. Every consumer re-derives the mode
  from a **different enum** on the app struct that the poller has never seen. One `mode` field
  copied in `get_result` makes every "Wipe only" comment in the header checkable.
- **Y4.** `ISO15693_MAGIC_BLK_*` (gen1 block numbers) and `ISO15693_MAGIC_V2_BLK_*` (gen2 register
  refs) are both `uint8_t`, both spelled `BLK`, both feed a `uint8_t` builder. The cost is
  asymmetric: a gen2 ref into `build_gen1_frame` emits an ordinary WRITE BLOCK at 64/65/71/82, and
  any writable tag accepts an ordinary write — four blocks of user data destroyed, no error
  surfaced. The reverse is a refused custom command. Renaming the gen2 family to
  `ISO15693_MAGIC_GEN2_REG_*` makes "block" and "register" different nouns at every call site.

Also noted, both inert today: the app-side `iso15693_result` copy is never invalidated
(`scene_write.c:253-255` resets three siblings but not it), and `iso15693_force_gen1` is reset on
two of three mode entries (`scenes/nfc_magic_scene_iso15693.c`).

**Sound, and stated explicitly:** constants (no bare literal duplicates any named constant outside
comments); `cut_block`/`pass_truncated` pairing; `uid_changed ⟹ uid_verified` structurally
guaranteed by adjacency; mode-scoping actually honoured by every consumer; the bitmap range
convention honoured by `partial_details.c:23-25`; genuine opaque handle with a by-value result copy
and `const` accessors.

---

# SIMPLIFICATION — three worth doing, and the "leave it" answers

**First, a correction to the project's own notes.** The four-way compact-UID formatter duplication
recorded in `pr-round-7/comment-cut-plan.md` **no longer exists**. `iso15693_info_cat_uid` is
defined at `iso15693_info.c:283`, declared at `iso15693_info.h:27`, and the four sites are now four
*call sites* (`scene_iso15693_info.c:18`, `write_fail.c:326`, `:339`, `write_confirm.c:39`) plus
six in tests. Verified — that item should be struck from the notes.

- **P1 (do).** The pass-cut is written out twice (`poller.c:651-660`, `:914-920`) and both copies
  set `pass_truncated` + `pass_cut_block` — two fields read in seven places that gate Retry. Worse
  than the agent stated: the clone site carries a four-line rationale the wipe lacks, and the wipe
  site carries the tick-wraparound rationale the clone lacks. A shared helper gives both reasons
  one home. Nothing is lost; the two log lines differ and stay at the call sites.
- **P2 (do).** Three saturating `total - bad` subtractions (`write_fail.c:202`, `:235`, `:324`)
  with no rationale at any of them — the explanation is 600 lines away in the poller. A named
  helper is where that sentence finally gets to live next to the code that needs it.
- **P3 (consider).** The block-size clamp is spelled two ways (`poller.c:585-587` against the
  macro, `:872-873` against `sizeof` of a buffer). They agree only because the buffer is sized from
  the macro.
- **P4 (decline, and say why).** The twelve `widget_add_string_multiline_element` calls. A thin
  wrapper works and the per-site y rationale is already hoisted to the file header at `:4-10`, so
  the earlier objection is weaker than the notes claim. But it touches all twelve branch bodies,
  forcing a re-read of the most-churned part of the file for 25 lines of formatting. **Highest
  re-read cost, lowest benefit, at round 10.** Worth telling mishamyte the form exists and was
  declined on diff cost, so nobody re-derives it.
- **Deliberately left:** the if/else-if chain → `switch` (re-indents ~250 lines); collapsing
  `start_clone` / `start_clone_gen1` into a `bool gen1` parameter (the named pair buys exactly the
  consent safety the opt-in flow exists for — line count is the wrong metric); the gen1/gen2 frame
  builders (the header-length difference *is* the generation); the CardLost preamble in the verify
  states (two matching copies and one deliberate divergence — hoisting hides why the third differs);
  the seven per-branch `FuriString` alloc/free pairs (a 240-line lifetime and a heap allocation on
  five branches that never use it).

---

# VERIFICATION OF THE CURRENT TREE

| check | result |
|---|---|
| host tests, from `make clean` | **108 run, 0 failed** |
| clang-format (toolchain 18.1.8, slix `.clang-format`) | **95 files, 0 need formatting** |
| comment-only classification, 15 shipped commits | 11 comment-only, 3 CHANGELOG-only, **1 code** (`d5e457b`) |
| intra-batch churn, all 15 | one pair (`3a8b6ea`→`4ec5f8c`), 3 lines, **re-wrap only** |
| final-state PR diff | **27 files, +3922 −45** |

Firmware builds were verified by round 9 on this exact tree (both warning-free); the tree has not
changed since, so they were not re-run.

---

# CORRECTNESS — no confirmed functional defect

This is the headline negative result, and at round 10 it is worth stating plainly: a full
correctness pass over the ISO15693 surface found **nothing functionally wrong in the shipped
code**. Every area the brief named was traced and came back clean.

Three low-severity items only:

- **X1.** `poller.c:667` and `:1699` are the only two implicit `uint16_t → uint8_t` narrowings in
  the file. `iso15693_3_get_block_data` takes a `uint8_t`; every *other* index-passing site casts
  explicitly (`:669`, `:707`, `:804`, `:931`, `:957`, `:1027`). Both are provably in range today —
  `source_count` is clamped to 256 at `:597-599`, and `:1699`'s index comes from the backdoor array
  (max 63). Worth fixing for consistency: the comment at `:62-64` argues at length that a width
  change is the hazard to design against, and these two sites are exactly the ones that would
  silently truncate if a bound moved. clang's `-Wimplicit-int-conversion` flags both; ARM GCC's
  `-Wall -Wextra` does not, so the firmware build is unaffected.
- **X2.** The wipe's run re-probe (`:1023-1045`) is not deadline-checked. On a card advertising 200
  and holding 10, `absent_run` grows to 190 unchecked below the claim, then the re-probe runs 190
  reads with no deadline test — with Back swallowed throughout. **Already documented** as a
  deliberate trade at `poller.h:194-196`. Recorded only because the in-code comment at `:1022`
  ("once, on a healthy card") reads as a much smaller number than the boundary case allows.
- **X3.** `success_or_partial`'s `clone &&` guard on `identity_failed` (`:1154-1155`) is provably
  redundant — the same shape as round 9's dead `|| iso15693_wipe`. **Not recommended for removal**:
  the twin at `get_result:1677` carries an explicit cross-mode-leak note and this one does not, so
  the fix is to justify it, not delete it.

**Verified clean, explicitly:**

- **Thread/lifetime across the poller/scene boundary.** Every result field is written on the Nfc
  worker thread and copied out by `get_result` *before* `view_dispatcher_send_custom_event`, in all
  five handlers. No scene reads a poller-owned pointer after the free.
- **The "widget copies its string" constraint was checked in the SDK, not assumed** —
  `widget_element_string.c:54`, `_multiline.c:55`, `_text_box.c:62`, `_text_scroll.c:235` all do
  `furi_string_alloc_set`. So every early free is safe. (`popup_set_header` does *not* copy, which
  is why it correctly gets the persistent `text_store`.)
- **Memory.** Every `furi_string_alloc` and `bit_buffer_alloc` has a matching free on every path,
  including `write_identity`'s early return. No double-free on Retry.
- **Buffer/index safety.** No card-advertised count ever indexes a constant-sized array —
  `advertised` is used only as a comparison. All fixed buffers are clamped to
  `ISO15693_MAX_BLOCK_SIZE`.
- **Tail-drop arithmetic.** `absent_run <= block` verified at all five loop exits. No underflow.
- **Integer issues, including the tick-wraparound claim** — the elapsed-against-budget form is
  wraparound-correct, as the project claims. `failed_count <= blocks_total` verified structurally.
- **State-machine completeness.** All 13 reason codes reach a titled screen with a button set; no
  branch shadows another; Back escapes every screen; the three-way right-slot rule is honoured in
  both `on_enter` and `on_event`.
- **Back handling**, including the race between a keypress and `CardDetected` — `on_exit` →
  `furi_thread_join` waits for the in-flight callback.
- **Progress-event deadlock bound** — worst case 11 events against a queue of 16, structural.

Note: there is no `CLAUDE.md` in this repo (`.claude/` is empty), so no project-guideline
violations could be checked against one.

---

# WHAT THE REVIEW DID NOT FIND

Worth stating, because a clean result is a real signal at round 10:

- **No memory-safety defect, no leak, no use-after-free** on any traced path, including the gen1
  opt-in detour that pushes a second write scene (verified against the SDK's scene manager: the
  stale scene never re-runs `on_exit` against the NULLed poller).
- **No silent failure of the sharpest shape the brief asked for.** Nothing anywhere treats a
  refused backdoor write as proof the write did not land. All ten discarded return values are
  correct; eight of ten are justified in comment. (The two gen2 sender discards are correct for a
  different reason than gen1's, and that reason is stated nowhere — worth one line.)
- **No reason code can be reached by omission.** All four navigations set the scene state
  immediately before `next_scene`.
- **The Back-swallow argument is correct in every particular** — each protocol's poller-start call
  was checked; ISO15693 really is the only one that can emit a terminal event.
- **Every numeric figure outside D4/D5 matches**, including the worked absent-run example, the
  activation budgets, the frame layouts, the four backdoor block numbers, and the line budgets.
- **Every cross-reference resolves** except D1 and the `tools/`-only citation in D8.
- **Proxmark provenance is accurate** apart from D1's invented function name.
