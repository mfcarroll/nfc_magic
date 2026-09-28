# Whole-PR prose review — findings only

Run 2026-09-28 per `.notes/APP-REVIEW-PROMPT.md`, single-threaded, no fixes applied.

**Reviewed SHAs:** fork `7de7ce82` (round 15 tip, unpushed), dev `4d67f3e`. Scope = the 27 files of
`git diff $(merge-base HEAD upstream/dev) HEAD -- base_pack/nfc_magic`. The dev copies were compared
byte-for-byte against the fork tree: identical except the expected dev-app identity lines in
`application.fam` and the icons include in `nfc_magic_app_i.h` (same line count), so **every `file:line`
below is valid in both trees.**

**Provenance** is from `git blame` in the fork against round 14's pushed head `1d411dec`:
**r15 (NN)** = written by that round-15 sync point; **earlier** = pushed before round 15. Where an
earlier line went stale *because of* a round-15 addition, that is noted, because the fix then belongs
in that round-15 commit.

## Gates

- `tools/check-writing.py comments` over all 25 shipped `.c`/`.h`: **0 findings, 13 long-block
  warnings** (poller.c 53, 87, 203, 226, 249, 282, 946, 1141, 1603, 1820, 1998; write.c 533;
  write_confirm.c 46). It does read its input: a canary line (`// this used to be different,
  previously`) in a scratch copy produced one `history` finding.
- `tools/check-writing.py headings .notes/pr-round-15/reply.md`: 0 findings, 0 warnings.
- **What the gate cannot see.** It scans only lines that *start* with `//`, so trailing comments
  (33 in the poller, 20 in `nfc_magic_app_i.h`) and `CHANGELOG.md` are never read. Its HISTORY pattern
  has no `now`, `no longer`, `once`, `in the first place` or `previously` on its own — which is why
  every class-8 item below passed it.

---

## A. Comment and code disagree (class 2)

**A1. poller.h:133-135 — a gen1 clone is "never a clean Success". It is, below block 56.** *earlier*
> That is why a gen1 clone that took still reports Partial and never a clean Success

The code: `gen1_clone = clone && instance->clone_used_gen1 && instance->clone_gen1_blocks_skipped`
(poller.c:1696-1697). `gen1_blocks_skipped` is false on a source that ends before block 56, so a
clean gen1 clone of a 28- or 40-block file is **Success**. The same header says so at 211-215
("on a source below block 56 those addresses are not in the file at all"), and so does the
CHANGELOG (57-58: "A source below block 56 has none of them, so they cost it nothing and do not make
the clone Partial"). The code is right.
Knock-ons in the same doc block: the emits list at 135-137 ("Emits CardDetected, then Partial,
Fail … or CardLost") omits Success, and 38-39 lists "fell back to gen1" as a Partial cause
without the qualifier.
**Proposed:** "…skipping the gen1 registers, which now hold the UID and so can never match the
source. A source that reaches them therefore reports Partial; one that ends before block 56 can be a
clean Success. Either way a card that cannot do gen1 loses at most those four blocks. Emits
CardDetected, then Success, Partial, Fail (…) or CardLost." At 38-39: "fell back to gen1 over a
source that reaches 56/57/62/63".

**A2. write_fail.c:169-175 and 508-513 — CloneComplete plays the error tone and gets "Back".** *comment
earlier; CloneComplete added by r15 (06)*
> Over-capacity and a wipe that ran to the card's top are clean successes -> success tone.
> Everything else did not deliver what was asked for -> error tone

> over-capacity / partial / a completed wipe are (qualified) successes -> "Finish"; not-magic is a
> failure -> "Back".

CloneComplete is not in either list, so it gets `sequence_error` (175) and a "Back" button (513),
under a body that opens "All data written." The comment's own test ("did not deliver what was asked
for") is false for it, and `nfc_magic_app_i.h:115-119` and CHANGELOG:124 both call it a
success ("None is a failure and none makes the clone Partial"). **The comment's rule and the release
notes are right; the code is the outlier** — 06 added the reason without extending either
expression. That is a behaviour change, so it is mfcarroll's call whether it rides in 06 or goes in
a follow-up; the host test covers only the cut-sweep tone (`test_write_fail_scene.c:596`).
If the code stays, the comment must say CloneComplete takes the error tone and why.

**A3. partial_details.c:69-71 — the wipe does not deduct the backdoor blocks.** *earlier*
> the wipe and gen1 paths reduce blocks_total to a logical count that excludes the skipped backdoor
> blocks (56/57/62/63)

A wipe clears 56/57/62/63 (poller.c:1384-1386) and reports `highest_present + 1` (poller.c:1652,
poller.h:150-154). Only the gen1 clone path deducts. The conclusion (bound by `list_upto`) survives
because a wipe's bits can also sit above its range (the tail-drop keep branch).
**Proposed:** "The bound is list_upto, never blocks_total: a gen1 clone's blocks_total excludes the
skipped backdoor blocks and a wipe's stops at the highest block proven present, while failures are
recorded at their TRUE index, which can exceed either."

**A4. poller.c:1831 — "TWO gates", then three.** *earlier*
> THE REACH RULE, and 49 is not a threshold about the claim CONTAINING 56. TWO gates must
> both pass

Three bullets follow, the third is introduced as "That third gate". **Proposed:** "THREE gates must
all pass".

**A5. poller.c:1566-1570 — the pointer names the wrong file.** *earlier*
> the worst case iso15693_poller.h states beside the budget

The header states no re-probe worst case (its only "re-probe" is the cut-gap bound at
poller.h:184). The statement is poller.c:266-267, beside `ISO15693_POLLER_PASS_MAX_MS`.
**Proposed:** "the worst case stated beside ISO15693_POLLER_PASS_MAX_MS".

**A6. Four counts left stale by round-15 additions.** *text earlier; made stale by r15 (05)/(06)*

| site | says | is | added by |
|---|---|---|---|
| poller.c:119 | "the three loops below" | four `COUNT_OF` loops: 131, 1056, 1123, 2330 | 1056, r15 (05) |
| poller.c:910 | "Three callers" (lists clone, gen2 CFG, wipe) | four: 972 (survey), 1099, 1417, 1949 | 972, r15 (06) |
| poller.c:890 | "both clears are the else-arm of a two-way decision" | three `unmark_failed` calls: 1063, 1591, 1648; 1063 (the repair) is not an else-arm | 1063, r15 (05) |
| write_fail.c:10-15 | "The two bodies at (4, 20)", "the two branches here that use it", "all twelve", "the twelve calls" | three at (4,20): 282, 301, 350; thirteen bodies | 282, r15 (06) |
| write_fail.c:110-111 | "Only four of the twelve bodies are static; seven are built" | 4 + 7 = 11 of a claimed 12, of an actual 13 | r15 (06) |

Also poller.c:344, *earlier*: get_result copies one field per assignment "with three exceptions,
each noted at its own field". Four fields are not one-to-one (`uid_moved_by_write` and
`clone_card_blocks_known` are not copied; `clone_afi_failed|clone_dsfid_failed` are ORed;
`progress_step` is not copied), and only two carry the note.
**Proposed:** fix each count, or say what the count stood for instead of the number
("`COUNT_OF` at every loop over this list"; "every caller"). The write_fail pair wants its (4, 20)
rationale re-derived: CloneComplete's body is not one that "mirrors those screens".

**A7. partial_details.c:84-89 — the worked example disagrees with the header's.** *earlier*
> above it, a card claiming 200 while holding 10 read "time limit at block 10" about 170 blocks that
> were attempted and answered nothing.

The header's version of the same card (poller.h:188-189) cuts at block 150, which gives a gap of
**140**, and it is the *below-the-claim* case, so "above it" is the wrong label. "read" also reads as
a past output.
**Proposed:** "…and a card claiming 200 while holding 10, cut at block 150, would say 'time limit at
block 10' about the 140 blocks between that were attempted and answered nothing (the header's
example at cut_block)."

**A8. write.c:199-200 — the Fail handler's comment is two routes out of seven.** *earlier*
> Backdoor write not accepted (not a magic tag), or an empty-source clone.

The handler picks among seven reasons (write.c:468-510), and "(not a magic tag)" is the one reading
the same file calls "only the defensive fallback … nothing routed here is known to be an ordinary tag"
(write.c:474-477; also `nfc_magic_app_i.h:110-112`).
**Proposed:** "Every Fail carries its reason in the result; stash it so the fail handler can pick
the screen."

**A9. poller.h:49-55 — the Fail enum doc omits the all-rejected clone.** *earlier*
The list names five causes; the clone whose UID took and every data block was refused
(poller.c:1715-1718, `NothingCloned`) is missing, though the same header names it at 123.
**Proposed:** add ", a clone whose UID took but no data block did".

**A10. poller.c:1971-1974 — "in practice" the tag is the source's own. The repair says otherwise.**
*earlier; stale since r15 (05)*
> In practice that tag is the one the source was read from, so the payload it then receives is the
> data it already holds.

Round 15's repair exists because it need not be: a gen1 card already wearing the target UID reaches
this branch and has the file fed into 56/57 (poller.c:1199-1204), and a gen2 card re-cloned from a
*different* image sharing its UID does too (poller.c:1159-1163).
**Proposed:** "That tag may be gen1 silicon, which the data pass detects at 56/57 and repairs (see
write_source_blocks), or a card re-cloned from a different image with the same UID; either way the
payload is the file."

**A11. "advertises more blocks than it physically holds" is gen2-only.** *earlier* — PLAUSIBLE, not
benched
Sites: `nfc_magic_app_i.h:122-123`, write_fail.c:285-287, write.c:400-401, CHANGELOG:46-47.
OverCapacity is reachable on a gen1 clone: a sub-57-block source onto a smaller gen1 card has
`gen1_blocks_skipped` false, so `success_or_partial` returns Success with `over_capacity > 0`. But
gen1 has no CFG register (poller.c:1009-1011), so that card reports its **own** count, not the
file's. The on-screen string ("Holds %u/%u blocks") is fine; the prose around it, and the local
variable `advertised` at write_fail.c:289 (it holds the file's block count), are not.
**Proposed:** "the file is larger than the card, and the blocks past the card's end were empty".

---

## B. A measurement stated as a law, or an absolute that does not survive (classes 3, 5, 6)

**B1. "Past physical capacity a block refuses reads" — unscoped at two sites, hedged at a third,
scoped at the fourth.** *957: r15 (06); 1235, 1684: earlier*
- poller.c:956-958: "past physical capacity a block refuses reads outright" — no scope.
- poller.c:1235-1236: "a block past physical capacity refuses reads as well as writes (measured: …)".
- poller.c:1683-1684: "(measured on hardware; it may differ by card)".
- poller.c:242-244: scoped, to "the gen2 test card and a plain NXP SLI".

`slix2-gold-30mm` has 128 cells mirrored across the 8-bit space (NEXT-SESSION "WHAT TONIGHT
SETTLED"), so it serves reads everywhere; `v2-sticker-50x28` supports the claim (hard edge at 64).
NEXT-SESSION's gold-tag section already flags "three unscoped shipped comments" of this claim, and
two remain. This is the one discriminator both the survey and the capacity test rest on, so the
scope matters.
**Proposed:** 242-246 is the home (see also B8 for its bench wording): "Measured on N cards: past
physical capacity, writes are refused and reads fail outright. An aliasing card serves reads at
every address; there the survey over-reports holds_more and the capacity test reports nothing." The
other three say "(see ISO15693_POLLER_WIPE_MAX_BLOCKS for the sample)".

**B2. "A block that fails every write is past capacity" — stated as a law twice, contradicted by the
code's own read-probe.** *earlier*
- poller.c:197-200: "On these cards writes are gated by physical memory, so a block that fails
  EVERY attempt is past the card's real capacity".
- poller.c:1146-1147: "Magic cards ignore their own lock bits, so a block that survives the retries
  is past physical capacity."
- poller.c:1142: "(measured: writes succeed well past it)" — no sample.

Contradicted by: the read-probe 90 lines later, which exists because a failed write can be "a block
the card genuinely refuses" (1237-1238); the wipe's own classification "read OK, block non-zero ->
write-protected" (1374); the twin at 1371, which hedges ("a magic card **often** ignores its own
lock bits"); 62/63, refused inside the block space on every gen1 chip measured; and a card that
wants OPTION, which never acknowledges.
**Proposed:** 197-200: "Retrying rides out a transient RF error. What a block that fails every
attempt means is decided by the read-probe in write_source_blocks, not here." 1146-1147: "A block
that survives the retries is a capacity candidate, and only that; the read-probe below decides."
1142: scope it or cut it.

**B3. poller.c:774 — "62/63 are REFUSED on every card tested".** *r15 (07)*
poller.c:90 says "On one card of each chip", and 1429-1430 "on every gen1 chip measured". The
record: `gen1-hardware-findings.md` Finding 6 (the LRi2K's `01 10 1E 06`), and 139-142 there ("the
in-band-refusal result stays at one chip"; the other two reported a client-level "command failed").
**Proposed:** "62/63 are REFUSED on one card of each gen1 chip tested".

**B4. poller.c:43-44 — "and no card refuses it".** *r15 (02)* **Proposed:** "and no card measured
refuses it".

**B5. CHANGELOG:104 — the gen2 backdoor "cannot be addressed at all".** *r15 (08)*
The notes' own Validation bullet scopes it: "On all four gen2 cards tested" (149-151), as does
poller.c:415. A release note may not claim more than its evidence section.
**Proposed:** "The gen2 backdoor is the exception: on every gen2 card tested it refuses the
addressed form — see the gen2 bullet under Validation."

**B6. CHANGELOG:137-138 — the LRi2K's *claim* does not take the sweep to 56.** *r15 (08)*
> It was reproduced on the LRi2K, the one card tested whose claim takes the sweep that far.

By the poller's closed form (poller.c:1845-1847), 56 is reached from `A >= 49 OR claim >= 57`. The
LRi2K claims 56, so the claim term is false; it reaches 56 because it answers reads through 55
(A = 56). The SLIX (28) and SLIX-S (40) stop short on the A term. This contradicts the bullet it
sits under (CHANGELOG:78-82: reach "depends on both where the card stops answering *reads* and
what it claims").
**Proposed:** "…the one card tested that answers reads far enough to take the sweep there."

**B7. "The one result that proves the card is magic."** *earlier*
- CHANGELOG:94-95: "is the one result that *proves* the card is magic"
- `nfc_magic_app_i.h:129`: "this is the only outcome that proves it"
- poller.c:1987-1988: "This is the ONE branch that proves the card is magic"

A verified Write-UID Success proves it too, because the card's own UID is refused before anything is
sent (poller.c:1879-1892), so a match means the backdoor moved it. So do the gen1 verify and the
repair's trigger. Within *failures* it is nearly true (NothingCloned's UID also moved).
**Proposed:** "the one failure that proves the card is magic".

**B8. CHANGELOG:23-24 — "It cannot report the absence of one."** *earlier* — PLAUSIBLE
When the check runs and the UID matches, the WipeComplete screen omits "UID not re-checked", which
is reporting the absence of a move; CHANGELOG:75-76 draws exactly that contrast. What is meant is at
write_confirm.c:49-51: a card that does not answer afterwards goes unreported.
**Proposed:** "…reports a move when it sees one, and says so when the check could not run."

**B9. CHANGELOG:46-47 — "every block that fits is written and acknowledged".** *earlier*
The OPTION bullet (108-113) is a card that acknowledges no write; its blocks are read back instead.
**Proposed:** "written and confirmed".

**B10. poller.c:1617-1620 — a cut wipe "is reported as Partial".** *earlier*
One that cleared nothing is Fail (poller.h:43-45; the NothingWiped screen states the cut).
**Proposed:** "…reported as Partial, or as Fail if nothing cleared, and either names where the sweep
stopped".

**B11. poller.c:239-240 — "It is a ceiling, not a cost".** *earlier*
> the sweep stops at the card's real top plus ISO15693_POLLER_WIPE_ABSENT_RUN probes and one
> re-probe of the run.

False for the two cards the next constants exist for: one that refuses writes and serves reads walks
all 256 (251-254), and below the claim the sweep never stops on absence (303, 1567-1570), so an
over-claimer runs to its claim and re-probes the whole range.
**Proposed:** "It is a ceiling: a card that stops answering ends the sweep ABSENT_RUN blocks past
its top or at its claim, whichever is later. The pass budget below covers the card that never stops
answering."

**B12. poller.c:730-732 — "exactly two honest outcomes".** *r15 (01)*
> The UID is UNCHANGED, which means those addresses are ordinary memory here: gen2 or a plain tag

An unchanged UID also follows a register write that did not land. The code (keep the address) is
right either way; the inference is stronger than the evidence.
**Proposed:** "UNCHANGED: the write did not move it, whether because those addresses are ordinary
memory or because it did not land, and the address stands."

**B13. poller.c:1321 against 1316-1317.** *earlier*
"Under-claiming leaves a correct report and a Retry to act on" comes five lines after "Retrying is
cut in the same place". **Proposed:** "…and a Retry, which clears only a transient".

**B14. poller.c:26 — "on all four measured".** *r15 (08)* Four cards. **Proposed:** "on all four
cards measured".

---

## C. The same fact in more than one place (classes 1, 10)

**C1. The UID moves immediately / no power-up latch — five statements.** *mostly r15 (01)/(07)*
poller.h:86-89; poller.c:87-91 (**home**); poller.c:779-781; poller.c:861-863 (names the three
chips again); poller.c:1435-1437. The header already points at the home ("see
ISO15693_MAGIC_BLK_UNLOCK"). **Proposed:** keep 87-91; each other site keeps its own consequence
and points ("immediate — see ISO15693_MAGIC_BLK_UNLOCK").

**C2. "Every gen1 card measured took a write to 56/57 with nothing sent before it" — four
statements.** *r15 (07)*
poller.c:105-106 (**home**, with the sample); 1424-1425 (already points); 2000-2001;
`nfc_magic_app_i.h:138-139`. **Proposed:** 2000-2001 and the app header point at the home.

**C3. gen1_attempted is set at the send — three statements, one of them history.** *earlier*
poller.h:272-273; poller.c:395-404 (**home**); poller.c:1919-1922, which ends "so at start the
flag claimed spent gen1 registers for a card the field never saw" (class 8: it describes the flag's
old placement). **Proposed:** 1919-1922 becomes one line: "Set here, at the send; see
gen1_attempted."

**C4. The 40-70ms figure — three statements, once as fact.** *earlier*
poller.c:253 ("at the 40-70ms a refused-write-plus-read costs"), 293-297 (**home**: "treat
40-70ms as an estimate, not a measurement"), 1314 ("an estimated 40-70ms"). **Proposed:** 253: "at
an estimated 40-70ms (see ISO15693_POLLER_WIPE_ABSENT_RUN)".

**C5. pass_truncated covers the clone — a note about write.c in the poller, and history in
write.c.** *earlier*
poller.c:353-355 describes write.c's gate and what a reader of write.c might do (class 9);
write.c:449: "Mode-gated because pass_truncated **now** covers a clone's data pass too" (class 8).
**Proposed:** write.c: "Mode-gated because a clone's data pass sets pass_truncated too"; the
poller's field keeps "Set by the wipe's sweep and by the clone's data pass" and drops the rest.

---

## D. The comment tells the code's history (class 8)

All *earlier* unless marked. Each keeps its constraint; only the history goes.

| site | text | proposed |
|---|---|---|
| `nfc_magic_app_i.h:160-161` | "Replaces the old is-wipe bool, now that Write-UID runs there too rather than in a scene of its own." | "Clone, wipe and Write-UID all run in the shared write scene." |
| partial_details_common.h:8-9 | "(previously each re-implemented the same loop)" | cut |
| partial_details_common.h:19-20 | "the only raw bit arithmetic that had leaked into the scene layer" | cut the clause |
| partial_details.c:35-36 | "which is now structural rather than a promise in a comment" | "structurally: both go through list_upto" |
| partial_details.c:98-99 | "Both of us have had this backwards once, which is why the derivation is written out rather than left as a bare operator." | cut — history, and about the authors |
| partial_details.c:117-118 | "it is the same answer in both modes and was wrong to phrase as a clone-specific promise" | "it is the same answer in both modes" |
| gen1_optin.c:36-37 | "Both once ended \"Gen1 is not hardware-tested\", which stopped being true the moment gen1 was tested, and a corrected version would be no better:" | keep 35 and 38-40 |
| write_fail.c:106-108 | "That was twelve identical widget_add_string_element calls differing only in the string" | "The title is the one element every screen shares, so it is named here and drawn once." (and see A6) |
| write_fail.c:480-483 | "While is_retryable and has_details shared no reason, … putting one reason in both made a control labelled Exit open the Details scroll view." | "Add a reason to either predicate and re-read this." suffices |
| write_fail.c:548-549 | "which is the assumption that put Details under an \"Exit\" label" | cut |
| write_fail.c:38-42 | "and they have disagreed … this is a promise about what reaches the user if it recurs" | "The tail-drop's keep branch is guarded against it; this is what reaches the user if the guard fails." |
| poller.c:347-348 | "The `clone_` prefix is historical" | "The `clone_` fields serve a wipe too" |
| poller.c:1862, *r15 (07)* | "The short-circuit predates this feature and is left as it is." | "The short-circuit is left as it is." (The wipe is this PR's, so "predates" is not true of anything a reader can see.) |
| poller.c:1944-1945 | "Unclamped, a source claiming 257 blocks wrapped the cast to 0 and left the card permanently advertising a single block" | "…would wrap the cast to 0 and program the card to advertise a single block" — also drops "permanently", since the next CFG write reprograms it |
| write.c:536-540 | "It never aborted a write in the first place … So the write happens either way; Back only discarded the report." | "Back cannot abort a write: leaving runs on_exit -> … -> furi_thread_join, which waits for the worker. The write happens either way; leaving would only discard the report." |
| poller.c:1766-1768 | "It exists because the two were byte-identical … That seam is the reason, not the nine lines." | "One tail for both verifies, so a change to one reaches the other." |

---

## E. Shipped text that knows where the bench is, or carries evidence (classes 7, 12)

**E1. poller.c:242-246.** *earlier*
"on this silicon", "the gen2 test card", and the parenthetical: "(The probe that measured it lives in
the dev repo, not in this tree, so the observation is stated rather than cited -- a bare tool name
would resolve to nothing here.)" — a reader has no dev repo, and the parenthetical is about how the
comment was written. Also: check "a plain NXP SLI" against the record. The inventory calls the
28-block tags "SLI" only in the seller-label note, while the one measured is SLIX (IC ref 0x01).
**Proposed:** "measured on two cards — a gen2 magic card and an NXP SL{I,IX} [whichever the record
says] — phantom writes are rejected, …" and cut the parenthetical. Fold with B1.

**E2. poller.c:276-277 "4 is the sample card."; 296 "Two bench runs disagree by about 2x";
187-188 "If a bench wipe logs …".** *earlier* The first two are bench leaks ("4 is the block size
the figures were taken at"; "Two runs disagree by about 2x"). The third is an instruction to a
developer and reads fine as one.

**E3. poller.h:288 — "Recovery was byte-identical, and needed the original recorded."** *earlier*
(285 is r15 (07)). A bench narrative. The constraint is already in the sentence before ("which is
why uid_readback is printed"). **Proposed:** cut.

**E4. CHANGELOG:74-75 — "Observed on a gen1 card whose sweep reached those blocks: it went to
all zeros".** *r15 (08)* This is evidence; what a user can act on is the consequence.
**Proposed:** "The new UID can be all zeros, which is not a valid ISO15693 identity at all."

**E5. CHANGELOG:79 — "and the closed form is stated in the poller beside that branch".** *earlier*
A user cannot act on a pointer into the source. **Proposed:** cut the clause; the sentence after it
already says what a user needs.

**E6. CHANGELOG:105-106 — "The identity writes are the ones that matter most to a bystander".**
*r15 (08)* A ranking with no support: a stray data-block write destroys a bystander's data outright.
**Proposed:** "The identity writes matter to a bystander too: …"

---

## F. A comment that keeps a list of what other code does (class 9)

**F1. file_select.c:111-115.** *earlier* "Gen2 reaches the write unprompted too … NOT Classic, whose
check sets uid_locked unconditionally … Gen1/Gen4/USCUID-UL show the static confirm below
regardless of the card." That is four other protocols' routing, restated. **Proposed:** keep what
this branch does and why (up to "becomes real"), and the wipe sentence; cut the rest.

**F2. write.c:548-565.** *earlier* Four paragraphs on gen1a, the USCUID-UL backdoor engine,
gen2/Classic, USCUID-direct and gen4 pollers. It is also a count problem: "the other four magic
protocols" and then five engines are named. The constraint is one sentence: swallowing Back is safe
only where the write is guaranteed to report an outcome, and ISO15693 is (the activation-error
budget). **Proposed:** keep that and the ISO15693 paragraph; point at #252/#253 for the rest.

**F3. write_fail.c:10-13 "that pair appears in exactly three files in the repo"; write.c:172-173
"mirrors the other magic pollers, which all emit a card-detected event".** *earlier* Low; the first
also goes with A6.

---

## G. User-facing strings (class 13)

**G1. gen1_optin.c:52 — "Gen1 writes the UID to blocks 56/57/62/63 first".** *earlier*
The UID goes to 56/57. 62/63 get the sequence's other two frames, whose purpose is inference
(poller.c:102-108, 137-138). The Write-UID body at 46 already has the accurate form ("sets the UID by
writing blocks 56/57/62/63"). Two comment twins: gen1_optin.c:10-11 and write.c:521-522 ("the four
UID registers").
**Proposed string:** "Gen1 writes blocks 56/57/62/63 first to set the UID, then the rest of the data
only if that UID takes." (A scroll view, so the length is fine.) The comments become "the four gen1
registers".

**G2. CHANGELOG:27 — 'Live "Writing X / N" progress during a clone or wipe'.** *earlier* A wipe
shows "Wiping X / N" (write.c `nfc_magic_scene_write_is_wiping`). **Proposed:** '"Writing X / N"
(or "Wiping") progress'.

Every other string checked against the code and the notes: the wipe confirm (character counts
29/23/27 verified), the result bodies, the Details notes, the info screen, the magic-info
candidate text. Nothing else found.

---

## H. Mechanical (class 15)

- `nfc_magic_app_i.h:142-144` breaks early ("so it can report the" / "range it measured"), and
  153-155 breaks after "A" ("that states the cut.) A" / "partial outcome"). *earlier*
- scanner.c:197 "treated as a ISO15693 *candidate*" -> "an". *earlier*
- poller.c:918 "Do not drop the one that looks redundant; it is not the one that is." A flourish,
  and it does not parse (class 11). **Proposed:** "The wipe's clamp cannot fire; the other two can."
- poller.c:119-122: four lines of `sizeof` arithmetic for a one-line constraint (class 11). Goes
  with A6.
- The 13 long blocks the gate warns about. The three longest carry findings above (1820: A4; 533
  in write.c: D and F2; 1603: B10). Once those are fixed, re-run the gate to see which still want a
  reason.

---

## J. Judgement calls, not defects

**J1. The TI chip is identified by behaviour.** poller.c:30-31 and CHANGELOG:128 count "four
identified chips — TI Tag-it HF-I Plus, …"; poller.c:36-56 and 506-507 and CHANGELOG:112 name it.
The two cards behind it, `white-coin` and `black-tag`, are gen2 magic cards. Their "Texas
Instrument" type line decodes from UID byte `0x07`, and their IC ref `0x8B` is the gen2 CFG default
(`tag-inventory.json`), which is the same pattern the round corrected for `gen-2-card` and
`v2-sticker-50x28`. The identification actually rests on the `0x03` OPTION refusal, which
final-review.md:176 and gen2-addressed-bench.md:42-50 record as a deliberate call. The shipped text
does not say it is behavioural. That is a candidate for class 4 (inference stated as fact) if
mfcarroll wants it, e.g. "a chip that behaves as TI Tag-it HF-I Plus". Not reopened here.

**J2.** CHANGELOG:112-113 generalises from one card ("it takes no data block without the flag and
acknowledges none with it"; the measurement is "on one such card", poller.c:55). It stands or falls
with J1.

**Out of scope, noticed:** poller.c:542-543/591-592 allocate and free `tx`/`rx` in
`write_identity`, and nothing uses them (the frames go through `instance->frame_tx/rx`). This is
code, not prose, and no comment claims otherwise.

---

## Tables

### By class

| class | findings |
|---|---|
| 2 comment vs code | A1-A11 (A6 is five sites) |
| 3/5/6 law, absolute, count | B1-B14 |
| 1/10 twin, say-it-once | C1-C5, plus B1, B2, B3 |
| 8 history | D (16 sites) |
| 7/12 bench, evidence in notes | E1-E6 |
| 9 lists of other code | F1-F3, C5 |
| 13 strings | G1-G2 |
| 11/15 flourish, mechanical | H |
| 4/14 inference, chip | J1-J2 (judgement) |

### By file

| file | findings |
|---|---|
| iso15693_poller.c | A4 A5 A6 A10 B1 B2 B3 B4 B10 B11 B12 B13 B14 C1 C2 C3 C4 C5 D×5 E1 E2 H×2 J1 |
| iso15693_poller.h | A1 A9 C1 C3 E3 |
| nfc_magic_app_i.h | A11 B7 C2 D H |
| scene_iso15693_write_fail.c | A2 A6 A11 D×4 F3 |
| scene_iso15693_partial_details.c | A3 A7 D×3 |
| scene_write.c | A8 A11 C5 D F2 F3 G1 |
| scene_iso15693_gen1_optin.c | D G1 |
| partial_details_common.h | D×2 |
| scene_file_select.c | F1 |
| nfc_magic_scanner.c | H |
| CHANGELOG.md | A1 A11 B5 B6 B7 B8 B9 E4 E5 E6 G2 J1 J2 |

### By provenance — where the fix folds

| round-15 sync point | findings |
|---|---|
| 01 data-block addressing | B12, C1 (part) |
| 02 OPTION | B4 |
| 05 repair | A6 (COUNT_OF, unmark), A10 |
| 06 survey | A2 (CloneComplete), A6 (clamp count, write_fail counts), B1 (957) |
| 07 gen1 registers | B3, C1 (part), C2, D (1862), E3 (285 only) |
| 08 gen2 + notes | B5, B6, B14, E4, E6, J2 |
| **earlier** — a new commit | everything else |

## The ones that matter most

1. **A1** — the header contract says a gen1 clone never gets a clean Success. The code, the same
   header's field doc and the release notes all say it does below block 56. This is the doc a
   reader of the API trusts first.
2. **A2** — CloneComplete ("All data written.") plays the error tone and offers "Back". The comment's
   rule and the release notes say success. This is the one finding that may want a code change, so
   it is mfcarroll's call where it lands.
3. **B6** — a release note explains the LRi2K's exposure by its claim, which the poller's own
   closed form shows cannot reach 56. The user-facing mechanism is stated backwards.
4. **B1 + B2** — two capacity "laws" the survey and the capacity test rest on. Both are unscoped,
   and a card on the shelf (B1) or the code's own read-probe (B2) contradicts them. NEXT-SESSION
   already flagged B1 as three sites; two remain.
5. **A6** — five stale counts. Every one was made stale by a round-15 addition (the survey, the
   repair, CloneComplete), so they fold into 05 and 06 rather than a new commit.
6. **B5, B7** — release notes claiming more than their own Validation section ("cannot be addressed
   at all"; "the one result that proves the card is magic").
7. **A3, A10** — comments describing the wipe's total and the VerifyGen2 match in ways the code
   (A3) or round 15's own repair (A10) contradicts.
