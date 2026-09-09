# The comment brevity pass — inventory and plan

**Status: NOT STARTED.** Written 2026-09-08, revised the same day after two findings that changed it
(see "Two corrections"). Nothing here is committed; this is the work list.

## The four operations, and why they are four

| | operation | risk | verification | state |
|---|---|---|---|---|
| **A** | is it duplicated? | none | repeated-phrase count | done once — **and re-run every round** |
| **B** | is it factually correct? | none | claim vs code | done once — **and re-run every round** |
| **C** | does it need to be there? | **none** — deletion only | code bytes unchanged | next |
| **D** | can it be correctly simplified? | **high** — rewords live claims | re-verify each rewrite | after C |

**A and B are invariants, not completed rounds.** They regress whenever comments are written. Evidence
from inside the round that supposedly closed them: `iso15693_poller.c:1236` and `partial_details.c:129`
state the same `wiped == 0` argument at length, in two places, both added by the Round 7 corrections
(an A-class defect), and `iso15693_poller.c:1188` carries a dangling half-sentence (B-class — a fragment
cannot be correct). C is safe from this by construction, since pure deletion introduces no claims.
**D is not**, so D must carry a B-check on every sentence it rewrites, inside D rather than as a fifth pass.

### Why C and D cannot be one pass

1. **Opposite risk profiles.** Round 7 measured it: **2 errors across 214 relocated comment lines, 4
   errors across ~10 newly-written claims.** Deletion cannot make a fact wrong; rewording is our
   highest-error activity by a wide margin. Fused, the whole pass carries D's error rate while appearing
   to carry C's.
2. **C changes D's input.** Several targets below exist only *because* a block is 46 lines. Once the
   narration is gone, 24 lines for a subtle wall-clock bound may need no compression at all. Deciding D
   against pre-C text is deciding about sentences that are losing half their neighbours.
3. **Different checks.** C's check is a diff a reviewer reads. D's is re-verifying each rewritten claim
   against the code — the expensive Round-7 pass. Bundled, the cheap check gets applied to the work that
   needs the expensive one.
4. **The reduction is not where it looks.** Reading the split off the descriptions below, of the -333 in
   the big blocks C is worth roughly **-150** and D roughly **-180**. D is the *larger* half. Fused, the
   "deletion cannot make a fact wrong" guarantee silently covers under half the work.

## Two corrections that changed this plan

### 1. THE REPO SQUASH-MERGES — but the message is THE COMMIT MESSAGES, not the PR body

**An earlier version of this file got this wrong in both directions. Corrected 2026-09-08 after the
owner challenged it.** What is actually verified:

- It squash-merges. Every commit on `dev` has **one parent**; #258's four commits are not ancestors of
  it (`compare/4d04165a2...dev` -> `diverged`). That part was right.
- **The squash body is GitHub's `COMMIT_MESSAGES` default**: `<title> (#N)`, then `* <headline>` plus
  that commit's body, per commit, joined by GitHub's own `---------` separator. Diffed against #238,
  #236 and #244 — the only differences were blank-line placement, the `---------` GitHub inserts, and a
  truncation artefact in the `messageHeadline` API field. **Not the PR body**: #258's body is 9949 bytes
  and its squash message is 799.
- **So commit messages DO survive a squash, concatenated.** The test this plan first used — "would the
  sentence be at home in the commit message? then it belongs only there" — was correct, and the
  replacement written here on 2026-09-08 was based on a wrong reading.

**The real hazard is different, and it is still real.** #258 proves the merger overrides the default:
mishamyte hand-wrote an 799-byte purpose-built message for a 4-commit PR whose concatenation would have
been 4064. **Our 20 fork-bound commits concatenate to ~728 lines.** Nobody wants that on `dev`, so an
override is the likely outcome — and then the messages ARE lost, as first claimed, but for a different
reason and with a different fix.

**The fix is to write the squash message ourselves and offer it to him**, which is the house pattern
#258 demonstrates — not to rewrite the PR description. See [../pr-description.md](../pr-description.md),
which is being repurposed for that.

**So both tests are live, with different strengths:**
1. *Would a maintainer editing this line, offline, need it?* -> the code. Unchanged, and the stronger test.
2. *Would it be at home in the commit message?* -> valid again, **but a weak home.** It survives only if
   nobody overrides the default, and on a 20-commit PR someone will. Anything genuinely load-bearing
   belongs in the code or in the proposed squash message, not in a commit message alone.

### 2. COMMENT VOLUME IS NOT HIS OBJECTION. It is ours.

An earlier version of this file said volume was "a live reviewer objection across multiple rounds."
**It is not.** Checked against all 86 review events and every inline comment on #250: mishamyte has never
asked for less comment. His comment findings are uniformly about ACCURACY — "this comment is inaccurate
against the SDK this repo actually builds with", "the comment names the wrong event", "this comment
invents a constraint that doesn't apply" — and in Round 4, on the Back-scoping block, he wrote *"since
that comment is what a future maintainer will widen or narrow this from, they're worth correcting."*
He treats these as load-bearing and corrects them rather than asking for their removal.

The 42%-vs-10% figure is OURS (`pr-round-3/STATUS.md`). The cut was *"decided on our side 2026-08-20,
independently of his answer."*

**Do not let a future session re-inherit the wrong premise.** What is true:

- It IS an outstanding **public commitment**. The Round 6 comment says *"the comment cut is no longer a
  queued nicety... I am not treating it as optional"* and asks him to scope it. A promise we made, not a
  demand he made — so **the scope is ours**, and there is no need to gate D on his reaction.
- We publicly reframed it AWAY from volume: *"That is not a density problem, it is a duplication
  problem."* C reverses that framing. That is fine and it is defensible, but it has to be said out loud
  rather than slid past. `reply.md`'s "The ratio" section now does so.
- The standing justification is the simplest one and needs no metric to win: **this is the owner's
  feature, and matching the house style of the pack it contributes to is a valid preference.** The
  ratio argument had to win a debate about metrics and lost it on its own numbers. This one does not.
- Corollary: do not delete the comments he specifically asked to have stated — `write.c:521` (Back
  scoping), `write_fail.c:394` (the three-way button rule), `partial_details.c:87` (count vs index).
  All three are on the do-not-cut list, now for a second and better reason.

## STEP 0 — the PR description, and the merge race

**The description is the permanent record, and it is currently 48 lines of feature description.** No
queue-hang argument, no 2026-08-04 wipe measurement, no de-arming reasoning. As it stands, C would delete
reasoning into nothing. With the description built up first, C deletes *from the file into the permanent
record*, which is a different and defensible operation.

⚠️ **THIS IS A RACE.** Round 7 came back `COMMENTED` with nothing blocking and all 22 items addressed.
He can approve and merge at any point, and the squash message would then be the thin body as it stands.
**Flag it to him in advance** — one line in the round-7 reply is enough: the squash message is the
permanent record, a description rewrite is coming, do not merge on the current body. Cheapest possible
insurance and it costs nothing if he was not about to merge.

Order of work: **description first, then C, then D.**

## The measurement

Tree-based, not blame-based — the tree was reformatted early, so `git blame` credits us with the whole app.

| | code | comment | surface | per 100 code | blocks 8+ lines | median / max |
|---|---|---|---|---|---|---|
| app at the branch point (`0de9fe5`, pure upstream) | 6696 | 180 | 3pct | 3 | **1 of 156** | 1 / 10 |
| our ISO15693-named files, HEAD | 1881 | 1242 | 40pct | 66 | **46 of 255** | 3 / 46 |

Plus ~526 comment lines we added to shared files. **~1770 lines of ours in total.**

`iso15693_poller.c` is 90 comment lines per 100 code; `gen2_poller.c`, the app's most-documented file, is
11. That is the "roughly 9x" figure in the reply, and it checks out.

**The distribution is the finding, not the ratio.** The entire pre-existing app contains ONE comment block
longer than 7 lines — 10 lines, `gen2_poller_i.c:461`. We shipped 46 in the ISO15693 files plus two more
in shared files, and 54pct of our comment lines sit inside them.

## The two tests

1. **Would a maintainer editing this line, offline, need it to avoid a wrong edit?** No -> the PR
   description carries it, or nothing does.
2. **For whatever survives: name the wrong edit it prevents.** Cannot name one -> delete.

**C should record test 2's answer per surviving block** — one line each, in these notes, not in the code.
That is the output of the test anyway, and it hands D a list of clauses it must not compress away instead
of making D re-derive what is load-bearing.

## Inventory — all 48 blocks of 8+ lines

`K` = keep, already at or near minimum. `T` = trim. **No block is a pure delete** — every one has
something worth keeping, which is itself worth knowing.

**Read the columns as the two passes.** *C deletes* is C's whole instruction: remove that text, touch
nothing else, no line target. *D target* is where the block should end up AFTER C, and it is D's business
only — several of these will need revisiting once C has run, per reason 2 above.

| block | now | | C deletes | D target | keep (never cut) |
|---|---|---|---|---|---|
| `iso15693_poller.c:34` | 9 | T | the four-lines-past-the-end worked example | 5 | the set exists once because four sites ask; COUNT_OF not sizeof |
| `iso15693_poller.c:85` | 12 | T | the four-second freeze arithmetic; the ~1.5s justification | 5 | why this budget differs (no wait-for-presentation phase); why not the minimum; what to raise if a bench wipe logs the timeout |
| `iso15693_poller.c:111` | 19 | T | the 5-line "four of these names are not greppable" provenance paragraph -- written for a reviewer, not a maintainer | 10 | the queue-hang deadlock: QUEUE_LEN 16, FuriWaitForever, Back -> thread_join stops draining; per-block needs a cursor yield |
| `iso15693_poller.c:132` | 21 | T | the proxmark-precedent paragraph down to one clause; the edgepages phantom-write paragraph to two lines | 10 | the advertised count is programmed not physical; the 2026-08-04 measurement (36 of 64 survived a Success) verbatim; only a write settles existence |
| `iso15693_poller.c:155` | 46 | T | the ENTIRE wipe-vs-clone cost derivation, ~22 lines. This is his :752 thread | 12 | why a wall clock at all (refuse-write/answer-read card walks 256); the asymmetric errors; worst case is bound + one re-probe; "4 is the sample card" |
| `iso15693_poller.c:203` | 32 | T | the per-block cost derivation to one line; the "not the only guard" recap to two | 12 | one absence is not evidence; the coupling-wobble hazard vs the 200ms hand; what stays open (a run that answers neither, even on re-probe) |
| `iso15693_poller.c:410` | 11 | T | the SDK parenthetical; the retry recap | 6 | an in-band refusal returns None from send_frame, so read-back is mandatory; GET SYSTEM INFO returns both fields |
| `iso15693_poller.c:490` | 8 | K | one line of framing | 7 | why it retries; the SDK builds these frames UNADDRESSED -> #251 |
| `iso15693_poller.c:512` | 10 | T | "the point is not the six lines"; "the whole reason this was on the list" | 4 | a named pair because both clears are the else-arm of a content decision, and |= vs &= ~ gives no signal |
| `iso15693_poller.c:537` | 8 | K | -- | 8 | function contract; why skip_backdoor exists on gen1 and not gen2 |
| `iso15693_poller.c:565` | 8 | T | "restated this in two more places" -- reviewer-facing | 5 | block number is uint8_t on the wire; clamped once so both loops bound on source_count |
| `iso15693_poller.c:593` | 15 | T | the middle paragraph's classification derivation | 9 | attempt every block (verified: writes succeed past the advertised count); do NOT skip source-locked blocks |
| `iso15693_poller.c:613` | 10 | T | the 19s arithmetic | 6 | Back is swallowed, so this loop is time the user cannot escape; it can fire with the card present |
| `iso15693_poller.c:662` | 19 | T | the "paid on EVERY persistent failure" paragraph to two lines | 9 | past-capacity blocks refuse READS too (measured), so a block that answers exists; without this a transient fabricates "holds 60/64" |
| `iso15693_poller.c:717` | 10 | T | the hole-in-the-middle restatement | 6 | the two halves answer different questions; tracked as a bool, not an index compare, because gen1 skips 56/57/62/63 |
| `iso15693_poller.c:736` | 24 | T | the 7-line "direction of the error" decision rationale to two | 10 | position AND evidence AND actually asked; the KNOWN OVERLAP limitation, 3 lines |
| `iso15693_poller.c:803` | 22 | T | the gen1-registers paragraph, which restates the header's | 14 | the three-way read-back classification table -- genuinely a table, and load-bearing |
| `iso15693_poller.c:852` | 17 | K | light trim only | 14 | **highest-value block in the feature.** "Do NOT try to de-arm by pre-writing the commit block" prevents a specific destructive edit, and says why the conclusion survives the premise being wrong |
| `iso15693_poller.c:913` | 13 | T | the "load-bearing for the keep branch" forward reference | 8 | filter_error maps Timeout to None, so the activation cache can be a truncated prefix; pvPortMalloc's memset makes "zeroed" a fact |
| `iso15693_poller.c:947` | 13 | T | the 64-block worked example | 7 | below the advertised count a long run is not a capacity signal; the once-per-run inventory |
| `iso15693_poller.c:980` | 13 | T | the bounded-by-run-length note | 8 | the two ways dropping the run would be wrong (dropout vs real-lowest-members) |
| `iso15693_poller.c:1025` | 42 | T | the 4-line "it is NOT what makes an unread entry read as empty" disambiguation of two of our own comments; the "no i < advertised guard" note; most of the disagree-with-note_present walkthrough | 16 | the activation cache is a PREFIX -- the block itself says this is load-bearing twice and "worth not re-deriving". Keep that, and the Not-closed disposition |
| `iso15693_poller.c:1107` | 14 | T | the parenthetical asides | 10 | the terminal-outcome table; the clone-is-not-identical caveat |
| `iso15693_poller.c:1126` | 15 | T | the "same fabrication, one branch earlier" framing | 8 | why a cut run must be excluded: the back-fill makes failed_count reach blocks_total either way |
| `iso15693_poller.c:1158` | 10 | K | one line | 8 | the SDK inventory is 1-SLOT, so a bystander can answer and the screen would print the wrong card's UID -> #251 |
| `iso15693_poller.c:1188` | 8 | T | "the call sites sit ~80 lines apart"; "that seam is the reason, not the nine lines". **Also carries a dangling half-sentence -- see defects below** | 3 | what skip_backdoor is |
| `iso15693_poller.c:1236` | 9 | K | -- | 9 | the wiped==0 path can still have moved the UID; the one inference this file declines to draw. Added THIS round, deliberately |
| `iso15693_poller.c:1260` | 8 | K | -- | 8 | a read-back only proves magic when the target UID is one the card did not already have |
| `iso15693_poller.c:1364` | 20 | T | the gen2-cannot-do-this recap | 12 | why it is a state of its own behind the power-cycle; why uid_changed is positive-observation only |
| `iso15693_poller.h:71` | 10 | K | -- | 10 | public API contract. Appropriate for a header |
| `iso15693_poller.h:87` | 8 | K | -- | 8 | public API contract; gen1 is destructive on any writable tag; NOT hardware-validated |
| `iso15693_poller.h:113` | 8 | K | -- | 8 | public API contract; why a gen1 clone can never be a clean Success |
| `iso15693_poller.h:142` | 15 | T | the wipe-has-no-back-fill derivation | 9 | what the flag saves differs by mode, and only the clone has the problem; Retry may be offered, never promised |
| `iso15693_poller.h:158` | 19 | T | the two-card case analysis and the <= 7 bound derivation | 10 | cut_block is an INDEX and not derivable from blocks_total; which side of the claim the cut lands on |
| `iso15693_poller.h:239` | 18 | T | the proxmark comparison to one clause | 12 | public contract; it does not stop at the advertised count and why; the gen1 registers are cleared |
| `nfc_magic_scene_iso15693_gen1_optin.c:4` | 9 | K | -- | 9 | scene contract; the two flows consent to different things |
| `nfc_magic_scene_iso15693_partial_details.c:26` | 13 | T | the 5-line "no + over_capacity term: it was dead" note -- explains a REMOVAL, belongs in the commit | 7 | a partial can reach here with no failed blocks, so the title cannot say "blocks not written" |
| `nfc_magic_scene_iso15693_partial_details.c:87` | 8 | T | "both of us have had this backwards once" -- reviewer-facing | 6 | STRICT <, because blocks_advertised is a COUNT and cut_block an INDEX; the test that pins it |
| `nfc_magic_scene_iso15693_partial_details.c:129` | 17 | T | the second half restates iso15693_poller.c:1236 at length -- one owner, cross-reference the other | 8 | NothingWiped has no Details route and uid_verified is false by construction |
| `nfc_magic_scene_iso15693_write_fail.c:67` | 9 | T | the "deliberately NOT a {reason,title,body} table" rejected alternative to one line | 4 | the title is the uniform part, so the title is what gets the table |
| `nfc_magic_scene_iso15693_write_fail.c:147` | 8 | T | the popup comparison | 5 | both figures, no verdict: blocks_total < advertised is a normal card |
| `nfc_magic_scene_iso15693_write_fail.c:178` | 21 | T | this is a changelog of the comment's own two corrections -- the history goes to the commit | 8 | the figure is the CUT not blocks_total; "of %u" parses as a fraction and had to go |
| `nfc_magic_scene_iso15693_write_fail.c:244` | 8 | K | -- | 8 | the four qualifiers in priority order, and that a dropped one is still reachable via Details |
| `nfc_magic_scene_iso15693_write_fail.c:275` | 8 | T | the short-circuit walkthrough | 5 | the UID was never touched, so the generic message would be wrong |
| `nfc_magic_scene_iso15693_write_fail.c:309` | 8 | T | the line-budget arithmetic | 5 | both flags can hold, so lead with counts rather than "Data cleared" |
| `nfc_magic_scene_iso15693_write_fail.c:394` | 16 | K | light trim | 13 | the three-way button rule, and the regression it actually caused when unstated. Names the edit it prevents |
| `nfc_magic_scene_write_confirm.c:47` | 31 | T | the 6-line pixel-width-vs-character-count paragraph and the y-coordinate derivation | 9 | why 56/57/62/63 are not spared; FontSecondary is ASCII-only so no glyph; the hard breaks; text_height 38 is a clipping fix |
| `nfc_magic_scene_write.c:521` | 42 | T | the 14-line per-protocol enumeration, incl. gen4_poller.c:281/:358/:460 line numbers -- the part most likely to go stale | 16 | Back discards rather than aborts; the clone's Reset gap; why NOT extended to the other four protocols; the 88-second measurement -> #252/#253 |

**742 comment lines in these blocks -> 409 after both passes. 12 keep, 36 trim.**

### What that projects to across the whole surface

The other **1026** comment lines sit outside these blocks as 1-3 line notes. Those are already the app's
own idiom and mostly fine, so the rate there should be low — 10-20pct, not the 45pct the big blocks take:

| | comment lines | ISO-file surface |
|---|---|---|
| now | 1768 | 40pct |
| after (10pct outside) | ~1330 | ~33pct |
| after (20pct outside) | ~1230 | ~31pct |

**So C+D together are worth about -450 to -550 lines, landing near 30pct — not the "~500 lines /
12-15pct" first estimated here.** That figure came from extrapolating one hand-worked example
(`write_confirm.c:47`, 31 -> 9) across the whole surface, and the inventory does not support it: 12 of
the 48 blocks are keeps at near-full size, and two thirds of the volume is outside the big blocks.

Getting to 12-15pct needs a stricter standard than block-by-block trimming — closer to the app's own
median of 1 and max of 10, which means deleting most of the short notes too. **That is the owner's call
and a legitimate one** (see correction 2); it is not a promise to make to the reviewer in advance.

## The guarantee that makes C reviewable

**Not one byte of code changes.** Verified by stripping comments from both trees and diffing. Build this
check FIRST, before touching anything. An empty diff reduces C's entire risk surface to "did we delete
something load-bearing" — no functional risk at all. D cannot use this check, which is the fourth reason
the two are separate passes.

## Three defects to fix regardless of the pass

1. **`iso15693_poller.c:1188` carries a dangling half-sentence** — it opens "Drives one write-mode step.
   Runs on the Nfc worker thread with the field active. Returns the" and then jumps to "The tail both UID
   verifies share". A merge artefact from the extraction. Broken text, not verbosity.
2. **`iso15693_poller.c:1236` and `partial_details.c:129` state the same `wiped == 0` argument at
   length**, both added in Round 7. One owner, cross-reference from the other.
3. **`ISO15693_POLLER_PASS_MAX_MS` is his open `:752` thread** and the worst block in the file: 46 lines
   for one `#define`, about half a cost derivation. Whatever else happens, that block answers a thread he
   is holding open.

## Do not cut

- `iso15693_poller.h` public contracts (`:71`, `:87`, `:113`) — a header documenting its API is the app's
  own idiom, and the "gen1 is NOT hardware-validated" warnings are the honest part of the PR.
- The `view_dispatcher` queue-hang argument (`poller.c:111`). Delete it and someone emits per block and
  hangs the device.
- The 2026-08-04 wipe measurement (`poller.c:132`) — verbatim, with the date and the block numbers.
- "Do NOT try to de-arm by pre-writing the commit block" (`poller.c:852`). Prevents a destructive edit.
- The activation-cache PREFIX property (`poller.c:1025`). Two rules depend on it.
- `write.c:521` Back scoping, `write_fail.c:394` the three-way button rule, `partial_details.c:87` count
  vs index — **he asked for all three specifically.**
- `write_fail.c:244`'s four-qualifier priority order.

## Related

- [reply.md](reply.md) — "The ratio" carries the concession, the plan, and the do-not-merge-yet flag.
- [comment-cut-plan.md](comment-cut-plan.md) — the A/B pass. A different defect: duplication, not volume.
- The operational rule the rounds produced: **moving a verified sentence is safe; restating it in fewer
  words is not.** C obeys it by construction. D is the pass that has to prove it.
