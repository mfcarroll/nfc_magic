# The comment brevity pass — inventory and plan

**Status: NOT STARTED. Planned for Round 8 of #250, after mishamyte responds to the cut.**
Written 2026-09-08. Nothing here is committed; this is the work list.

## Why Round 8 — not this round, and not after merge

- **Not this round.** The 20 commits in front of him are built, verified and tree-checked, and every
  Round 7 finding they answer was written against PRE-CUT code. Adding a ~1200-line deletion makes him
  review a moving target, which is the specific thing Round 7 punished.
- **Not after merge.** This is a first implementation of a new feature. At merge these lines become the
  shipped documentation of the feature in a third-party plugin pack, and a follow-up PR deleting 1200 of
  them invites the obvious question. Worse: comment volume is a live reviewer objection across multiple
  rounds. The cut answered the DUPLICATION half (repeated phrases 135 -> 34); the VOLUME half is
  unresolved, and merging over it then fixing it afterwards is the wrong order.
- **Round 8, because that is his first sight of the cut.** His reaction sets the scope instead of us
  guessing at it — which is exactly what went wrong when the cut proceeded on option 1 without waiting.
- Flagged in this round's reply (`reply.md`, "The ratio") so it is pre-committed rather than extracted.

## The measurement

Tree-based, not blame-based — the tree was reformatted early, so `git blame` credits us with the whole app.

| | code | comment | surface | per 100 code | blocks 8+ lines | median / max |
|---|---|---|---|---|---|---|
| app at the branch point (`0de9fe5`, pure upstream) | 6696 | 180 | 3pct | 3 | **1 of 156** | 1 / 10 |
| our ISO15693-named files, HEAD | 1881 | 1242 | 40pct | 66 | **46 of 255** | 3 / 46 |

Plus ~526 comment lines we added to shared files. **~1770 lines of ours in total.**

Per file, the sharpest pair: `iso15693_poller.c` is 90 comment lines per 100 code; `gen2_poller.c`, the
app's most-documented file, is 11. That is the "roughly 9x" figure in the reply, and it checks out.

**The distribution is the finding, not the ratio.** The entire pre-existing app contains ONE comment
block longer than 7 lines — 10 lines, `gen2_poller_i.c:461`. We shipped 46 in the ISO15693 files, plus
two more in shared files. 54pct of our comment lines sit inside those blocks.

## The two tests

1. **Would the sentence be equally at home in the commit message? Then it belongs only there.**
   Mechanical, and it needs no judgement about "value". The file is the right home for constraints; the
   commit is the right home for reasoning — and the reasoning is already written there.
2. **For whatever survives: name the wrong edit it prevents.** Cannot name one -> delete.

## Method: deletion, not compression

Round 7 measured this: **2 errors across 214 relocated comment lines, 4 errors across ~10 newly-written
claims.** Rewording a verified sentence is our highest-error activity by a wide margin. So the pass is
almost entirely deletion of whole claims, with rewording confined to a short explicit list signed off
individually. Deleting a sentence cannot make anything wrong.

## The guarantee that makes it reviewable

**Not one byte of code changes.** Verified by stripping comments from both trees and diffing — build this
check FIRST, before touching anything. An empty diff reduces the entire risk surface to "did we delete
something load-bearing": no functional risk at all.

## Inventory — all 48 blocks of 8+ lines

`K` = keep, already at or near minimum. `T` = trim; the core is load-bearing and the rest is narration.
**No block is a pure delete** — every one has something worth keeping, which is itself worth knowing.

| block | now | -> | | keep | cut |
|---|---|---|---|---|---|
| `iso15693_poller.c:34` | 9 | 5 | T | the set exists once because four sites ask; COUNT_OF not sizeof | the four-lines-past-the-end worked example |
| `iso15693_poller.c:85` | 12 | 5 | T | why this budget differs (no wait-for-presentation phase); why not the minimum; what to raise if a bench wipe logs the timeout | the four-second freeze arithmetic; the ~1.5s justification |
| `iso15693_poller.c:111` | 19 | 10 | T | the queue-hang deadlock: QUEUE_LEN 16, FuriWaitForever, Back -> thread_join stops draining; per-block needs a cursor yield | the 5-line "four of these names are not greppable" provenance paragraph -- written for a reviewer, not a maintainer |
| `iso15693_poller.c:132` | 21 | 10 | T | the advertised count is programmed not physical; the 2026-08-04 measurement (36 of 64 survived a Success) verbatim; only a write settles existence | the proxmark-precedent paragraph down to one clause; the edgepages phantom-write paragraph to two lines |
| `iso15693_poller.c:155` | 46 | 12 | T | why a wall clock at all (refuse-write/answer-read card walks 256); the asymmetric errors; worst case is bound + one re-probe; "4 is the sample card" | the ENTIRE wipe-vs-clone cost derivation, ~22 lines. This is his :752 thread |
| `iso15693_poller.c:203` | 32 | 12 | T | one absence is not evidence; the coupling-wobble hazard vs the 200ms hand; what stays open (a run that answers neither, even on re-probe) | the per-block cost derivation to one line; the "not the only guard" recap to two |
| `iso15693_poller.c:410` | 11 | 6 | T | an in-band refusal returns None from send_frame, so read-back is mandatory; GET SYSTEM INFO returns both fields | the SDK parenthetical; the retry recap |
| `iso15693_poller.c:490` | 8 | 7 | K | why it retries; the SDK builds these frames UNADDRESSED -> #251 | one line of framing |
| `iso15693_poller.c:512` | 10 | 4 | T | a named pair because both clears are the else-arm of a content decision, and |= vs &= ~ gives no signal | "the point is not the six lines"; "the whole reason this was on the list" |
| `iso15693_poller.c:537` | 8 | 8 | K | function contract; why skip_backdoor exists on gen1 and not gen2 | -- |
| `iso15693_poller.c:565` | 8 | 5 | T | block number is uint8_t on the wire; clamped once so both loops bound on source_count | "restated this in two more places" -- reviewer-facing |
| `iso15693_poller.c:593` | 15 | 9 | T | attempt every block (verified: writes succeed past the advertised count); do NOT skip source-locked blocks | the middle paragraph's classification derivation |
| `iso15693_poller.c:613` | 10 | 6 | T | Back is swallowed, so this loop is time the user cannot escape; it can fire with the card present | the 19s arithmetic |
| `iso15693_poller.c:662` | 19 | 9 | T | past-capacity blocks refuse READS too (measured), so a block that answers exists; without this a transient fabricates "holds 60/64" | the "paid on EVERY persistent failure" paragraph to two lines |
| `iso15693_poller.c:717` | 10 | 6 | T | the two halves answer different questions; tracked as a bool, not an index compare, because gen1 skips 56/57/62/63 | the hole-in-the-middle restatement |
| `iso15693_poller.c:736` | 24 | 10 | T | position AND evidence AND actually asked; the KNOWN OVERLAP limitation, 3 lines | the 7-line "direction of the error" decision rationale to two |
| `iso15693_poller.c:803` | 22 | 14 | T | the three-way read-back classification table -- genuinely a table, and load-bearing | the gen1-registers paragraph, which restates the header's |
| `iso15693_poller.c:852` | 17 | 14 | K | **highest-value block in the feature.** "Do NOT try to de-arm by pre-writing the commit block" prevents a specific destructive edit, and says why the conclusion survives the premise being wrong | light trim only |
| `iso15693_poller.c:913` | 13 | 8 | T | filter_error maps Timeout to None, so the activation cache can be a truncated prefix; pvPortMalloc's memset makes "zeroed" a fact | the "load-bearing for the keep branch" forward reference |
| `iso15693_poller.c:947` | 13 | 7 | T | below the advertised count a long run is not a capacity signal; the once-per-run inventory | the 64-block worked example |
| `iso15693_poller.c:980` | 13 | 8 | T | the two ways dropping the run would be wrong (dropout vs real-lowest-members) | the bounded-by-run-length note |
| `iso15693_poller.c:1025` | 42 | 16 | T | the activation cache is a PREFIX -- the block itself says this is load-bearing twice and "worth not re-deriving". Keep that, and the Not-closed disposition | the 4-line "it is NOT what makes an unread entry read as empty" disambiguation of two of our own comments; the "no i < advertised guard" note; most of the disagree-with-note_present walkthrough |
| `iso15693_poller.c:1107` | 14 | 10 | T | the terminal-outcome table; the clone-is-not-identical caveat | the parenthetical asides |
| `iso15693_poller.c:1126` | 15 | 8 | T | why a cut run must be excluded: the back-fill makes failed_count reach blocks_total either way | the "same fabrication, one branch earlier" framing |
| `iso15693_poller.c:1158` | 10 | 8 | K | the SDK inventory is 1-SLOT, so a bystander can answer and the screen would print the wrong card's UID -> #251 | one line |
| `iso15693_poller.c:1188` | 8 | 3 | T | what skip_backdoor is | "the call sites sit ~80 lines apart"; "that seam is the reason, not the nine lines". **Also carries a dangling half-sentence -- see defects below** |
| `iso15693_poller.c:1236` | 9 | 9 | K | the wiped==0 path can still have moved the UID; the one inference this file declines to draw. Added THIS round, deliberately | -- |
| `iso15693_poller.c:1260` | 8 | 8 | K | a read-back only proves magic when the target UID is one the card did not already have | -- |
| `iso15693_poller.c:1364` | 20 | 12 | T | why it is a state of its own behind the power-cycle; why uid_changed is positive-observation only | the gen2-cannot-do-this recap |
| `iso15693_poller.h:71` | 10 | 10 | K | public API contract. Appropriate for a header | -- |
| `iso15693_poller.h:87` | 8 | 8 | K | public API contract; gen1 is destructive on any writable tag; NOT hardware-validated | -- |
| `iso15693_poller.h:113` | 8 | 8 | K | public API contract; why a gen1 clone can never be a clean Success | -- |
| `iso15693_poller.h:142` | 15 | 9 | T | what the flag saves differs by mode, and only the clone has the problem; Retry may be offered, never promised | the wipe-has-no-back-fill derivation |
| `iso15693_poller.h:158` | 19 | 10 | T | cut_block is an INDEX and not derivable from blocks_total; which side of the claim the cut lands on | the two-card case analysis and the <= 7 bound derivation |
| `iso15693_poller.h:239` | 18 | 12 | T | public contract; it does not stop at the advertised count and why; the gen1 registers are cleared | the proxmark comparison to one clause |
| `nfc_magic_scene_iso15693_gen1_optin.c:4` | 9 | 9 | K | scene contract; the two flows consent to different things | -- |
| `nfc_magic_scene_iso15693_partial_details.c:26` | 13 | 7 | T | a partial can reach here with no failed blocks, so the title cannot say "blocks not written" | the 5-line "no + over_capacity term: it was dead" note -- explains a REMOVAL, belongs in the commit |
| `nfc_magic_scene_iso15693_partial_details.c:87` | 8 | 6 | T | STRICT <, because blocks_advertised is a COUNT and cut_block an INDEX; the test that pins it | "both of us have had this backwards once" -- reviewer-facing |
| `nfc_magic_scene_iso15693_partial_details.c:129` | 17 | 8 | T | NothingWiped has no Details route and uid_verified is false by construction | the second half restates iso15693_poller.c:1236 at length -- one owner, cross-reference the other |
| `nfc_magic_scene_iso15693_write_fail.c:67` | 9 | 4 | T | the title is the uniform part, so the title is what gets the table | the "deliberately NOT a {reason,title,body} table" rejected alternative to one line |
| `nfc_magic_scene_iso15693_write_fail.c:147` | 8 | 5 | T | both figures, no verdict: blocks_total < advertised is a normal card | the popup comparison |
| `nfc_magic_scene_iso15693_write_fail.c:178` | 21 | 8 | T | the figure is the CUT not blocks_total; "of %u" parses as a fraction and had to go | this is a changelog of the comment's own two corrections -- the history goes to the commit |
| `nfc_magic_scene_iso15693_write_fail.c:244` | 8 | 8 | K | the four qualifiers in priority order, and that a dropped one is still reachable via Details | -- |
| `nfc_magic_scene_iso15693_write_fail.c:275` | 8 | 5 | T | the UID was never touched, so the generic message would be wrong | the short-circuit walkthrough |
| `nfc_magic_scene_iso15693_write_fail.c:309` | 8 | 5 | T | both flags can hold, so lead with counts rather than "Data cleared" | the line-budget arithmetic |
| `nfc_magic_scene_iso15693_write_fail.c:394` | 16 | 13 | K | the three-way button rule, and the regression it actually caused when unstated. Names the edit it prevents | light trim |
| `nfc_magic_scene_write_confirm.c:47` | 31 | 9 | T | why 56/57/62/63 are not spared; FontSecondary is ASCII-only so no glyph; the hard breaks; text_height 38 is a clipping fix | the 6-line pixel-width-vs-character-count paragraph and the y-coordinate derivation |
| `nfc_magic_scene_write.c:521` | 42 | 16 | T | Back discards rather than aborts; the clone's Reset gap; why NOT extended to the other four protocols; the 88-second measurement -> #252/#253 | the 14-line per-protocol enumeration, incl. gen4_poller.c:281/:358/:460 line numbers -- the part most likely to go stale |

**Totals: 742 comment lines in these blocks -> 409. Deleted: 333.** 12 keep, 36 trim.

### What that actually projects to — and it is not what I first estimated

The other **1026** comment lines sit outside these blocks, as 1-3 line notes. Those are already the app's
own idiom and mostly fine, so the cut rate there should be LOW — call it 10-20pct, not the 45pct the big
blocks take. That gives:

| | comment lines | ISO-file surface |
|---|---|---|
| now | 1768 | 40pct |
| after (10pct outside) | ~1330 | ~33pct |
| after (20pct outside) | ~1230 | ~31pct |

**So this pass is worth about -450 to -550 lines, landing near 30pct — not the "~500 lines / 12-15pct" I
estimated before building the table.** That earlier figure came from extrapolating one hand-worked example
(`write_confirm.c:47`, 31 -> 9) across the whole surface, and the inventory does not support it: 12 of the
48 blocks are keeps at near-full size, and two thirds of the comment volume is outside the big blocks
entirely.

Getting to 12-15pct would need a stricter standard than block-by-block trimming — closer to the app's own
median of 1 line and max of 10, which would mean deleting most of the 1-3 line notes too. That is a scope
decision for the reviewer, not something to promise in advance.

## Three defects found while inventorying — worth fixing regardless of the pass

1. **`iso15693_poller.c:1188` carries a dangling half-sentence.** It opens "Drives one write-mode step.
   Runs on the Nfc worker thread with the field active. Returns the" and then jumps to "The tail both UID
   verifies share". A merge artefact from the extraction. Not verbosity — it is broken text.
2. **`iso15693_poller.c:1236` and `partial_details.c:129` state the same `wiped == 0` argument at
   length**, in two places, both added this round. One owner, cross-reference from the other. That is the
   defect the cut existed to remove, reintroduced by the corrections.
3. **`ISO15693_POLLER_PASS_MAX_MS` is his open `:752` thread**, and it is the worst block in the file:
   46 lines for one `#define`, about half of it a cost derivation. Whatever else the pass does, that
   block answers a thread he is holding open.

## What NOT to cut, so scope creep does not eat the load-bearing part

- `iso15693_poller.h` public contracts (`:71`, `:87`, `:113`) — a header documenting its API is the app's
  own idiom, and the "gen1 is NOT hardware-validated" warnings are the honest part of the PR.
- The `view_dispatcher` queue-hang argument (`poller.c:111`). Delete it and someone emits per block and
  hangs the device.
- The 2026-08-04 wipe measurement (`poller.c:132`) — verbatim, including the date and the block numbers.
- "Do NOT try to de-arm by pre-writing the commit block" (`poller.c:852`). Prevents a destructive edit.
- The activation-cache PREFIX property (`poller.c:1025`). Two rules depend on it.
- The three-way button rule (`write_fail.c:394`). It caused a real regression when unstated.
- `write_fail.c:244`'s four-qualifier priority order.

## Related

- [reply.md](reply.md) — "The ratio" carries the concession and the plan.
- [comment-cut-plan.md](comment-cut-plan.md) — the round-6/7 cut. A different defect: duplication, not volume.
- The operational rule the rounds produced: **moving a verified sentence is safe; restating it in fewer
  words is not.**
