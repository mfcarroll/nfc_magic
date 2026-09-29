# Final review 3 — the pre-push check of round 15, 2026-09-29

**Reviewed:** the fork's twelve unpushed commits `1d411dec..846a82eb` (01 `008aa06c` … 12 `846a82eb`),
and the two drafts, [reply.md](reply.md) and [issue255-comment.md](issue255-comment.md), at dev
`1235845`. Line numbers are at `846a82eb`, paths relative to `base_pack/nfc_magic/`.

**How:** five report-only agents, on mfcarroll's go-ahead: code correctness, silent failures, comments
and release notes, commit messages and drafts, simplification. They shared one briefing and changed
nothing. Every finding here was re-checked against the code, the records or GitHub by the main session,
except where it says **not re-checked**. Cost: about 2.14M tokens (413k, 382k, 467k, 503k and 374k),
roughly 2.5 times the estimate given before launch.

**Nothing is Critical.** No sync point is broken, and no finding damages a card on a path the bench ran.

## Decided

- **mfcarroll, 2026-09-29: the gen1 Partial becomes a question about DATA, not reach.**
  - A gen1 clone whose file reaches 56/57/62/63 but holds nothing there loses nothing. It ends as
    "Clone finished", with the success tone, Finish and a note, not "Clone partial". That is the
    same shape as an empty over-capacity tail.
  - Partial stays for a file that holds data there.
  - Today the gate is reach: `clone_gen1_blocks_skipped = skipped > 0` (poller.c:1136, and again at
    :1070).
  - Its own comment says otherwise: the caveat is "a claim about the SOURCE -- that it held data at
    those addresses" (poller.c:1132-1133). So do the notes: "reports only real data loss"
    (CHANGELOG:45).
  - The opt-in warning already gates on data (gen1_optin.c:58).
  - Worked failure: a 64-block file with 56..63 empty, cloned onto a gen1 LRi2K, gives "Clone
    partial / Cloned 60/60 blocks / Not written: 0 / gen1: 56/57/62/63 differ" with the error
    tone. With over-capacity on top (data only in 0-27, onto a 28-block SLIX), it gives "Cloned
    28/60 / Not written: 32" and "differ", while Details calls the same 32 "Didn't fit on the card".
  - **Language.** The Details note, "gen1: 56/57/62/63 are the UID / backdoor registers, not file
    data." (partial_details.c:155), is true in both cases and can stay. The summary line
    "gen1: 56/57/62/63 differ" (write_fail.c:360) is right only when the file held data there. The
    Complete case needs its own "Clone finished" note line.
  - **Where it lands.** Clone notes and CloneComplete arrive in 06. So 03 takes the gate (at 03 the
    empty case reaches the plain success screen, which is 03's own stated rule), and 06 adds the
    note. 08 updates CHANGELOG:45 and :57-60.
  - Pairs with S2, S7 and S8 below. Needs a host test for "reached but empty", and a bench of both
    cases on `lri2k-keychain`.

## Behaviour — for mfcarroll's call

### A. [Important] A clone that converts to gen1 never re-reads the UID (05, 07, 01) — two agents
- **The gap.** finish_conversion (poller.c:1076-1080) sends the gen1 sequence, whose every result
  is discarded (:781-805). It then sets `address_uid = target_uid` unconditionally. finish_write
  (:1786-1794) reports straight after the pass. The ordinary gen1 clone's report rests on
  VerifyGen1's read-back; this path has none.
- **Trigger 1.** The repair's 56 frame is lost, which is one shot against the data blocks' three.
  readdress then sees "unchanged", and the card keeps the file's bytes in uid[7..4] under a Partial
  screen with the gen1 caveat.
- **Trigger 2.** The inventory misses, or a bystander answers (#251), right after the pass's own
  write to 56 moves the UID. `uid_moved_by_write` stays false and nothing converts. 57 onward go to
  the old address, giving an ordinary Partial "Not written: N", which is 05's bug shape. On a
  57-block source the result is Success.
- **The texts that promise it.** CHANGELOG:116-119 (08), 05's message ("The end state is the one
  an ordinary gen1 clone produces -- … reported correctly"), and reply.md:135-136.
- **Fix, either:**
  - One `verify_inventory` after the pass, when a gen2-path pass reached block 56. A mismatch sets
    `uid_unexpected` and copies `uid_readback`, as VerifyGen2 does (:1997), and reports Fail. No
    answer reports CardLost. The UidUnexpected screen already prints the UID.
  - Or scope the three texts: "written back, not re-read".

### B. [Important-low] finish_conversion takes a failure back from the wrong count (05) — three agents
- **Mechanism.** The failure arm marks the bit, then buckets the failure (poller.c:1263-1267). A
  gen1 register answers no read, so an empty file block 56 whose moving write lost its ACK goes
  to `clone_over_capacity`. finish_conversion (:1063-1067) clears the bit and decrements
  `clone_failed_count` anyway.
- **What the user sees.** "Cloned 59/60 / Not written: 1" with no block named in Details, or
  "Card too small" dropped. Rare.
- **Fix.** `if(moved_uid) continue;` after `converted = true;`, since the move shows the write
  landed and 06's `!moved_uid` gate then goes away. Plus take-back from the bucket the pass chose.
  The simplification agent's version: `unmark_failed` returns whether the bit was set. Accounting
  only, so no frames change.

### D. [Suggestion] VerifyGen1 says "not a gen1 card" over a half-moved UID (pre-round; 07 widened it)
- **The path.** On the opt-in path, a readdress fallback between 56 and 57 sends 57 to an address
  the card no longer has. VerifyGen1 (poller.c:2039-2051) compares only against the target. It
  reports "UID didn't take, so not a gen1 card", and never shows the UID the card now answers to.
- **Fix.** Mirror VerifyGen2's changed-but-not-target branch: `uid_unexpected` plus `uid_readback`.

### E. [Suggestion] The survey never asks whether the card is still there (06)
- **The path.** survey_above_source breaks on an absent run (poller.c:985-990). It runs after the
  popup's last frame has already said 100% (:1291), so a card lifted at "Writing 28 / 28" loses its
  residue and size notes: Success instead of "Clone finished". Lifted mid-survey, the size note is
  understated.
- **Fix.** Call `card_still_present` when the survey stops on its absent run, or emit the last
  progress frame after the survey.

### S5. [Suggestion] `memory_differs` is decided on count OR size, but both screens print only the count (06)
- **The mismatch.** poller.c:1024-1027 against partial_details.c:204/208 and write_fail.c:285-287.
  Reaching a screen needs a card that accepts the file's block length while reporting another.
- **Fix, either:** drop the size term, or print the size. **Not re-checked.**

## Shipped text that is false or claims too much

| # | site (owner) | the problem | fix |
|---|---|---|---|
| T1 | poller.h:31-32 (pre-round, false since 02); write_fail.c:303-304 (09 kept it) | "block CONTENTS are never compared" / "not a byte-for-byte read-back": an OPTION card is read back and compared (poller.c:849-852). CHANGELOG:63-65 already says so | fix both at 02 |
| T2 | poller.c:416-418, 08 msg:22-23 (08); reply.md:245-247 | the addressed-bit-no-UID frame ran on three gen2 cards, not four: absent from v2-sticker-bench.md:311-325 | "…and the three asked also refuse the address bit with no UID" |
| T3 | CHANGELOG:15-17 (08); reply.md:197-200 | AFI/DSFID are listed as gen2-only; finish_write writes them on every clone, gen1 included (:1787-1788) | "writes the UID, all data blocks and the AFI/DSFID; on gen2 it also programs IC ref and geometry" |
| T4 | poller.c:105-106 and :770 (07); reply.md:33-35 and :116 | "never observed to accept a write, on any card tested": gen3-a took 62/63 as data (gen3-a-bench.md step 5), and so do gen2 cards | "on a gen1 card" |
| T5 | poller.c:54-57 (02); CHANGELOG:114 (08) | "0x03 … addressed or not" contradicts its own table 14 lines up (unaddressed gives 0x01), and hides that detection needs addressed writes; "one such card" / "the one measured", but two TI cards were | scope to addressed; say two cards |
| T6 | poller.c:345-346 (pre-round; stale via 05, 06) | "three exceptions": get_result also skips `uid_moved_by_write` and `clone_card_blocks_known`, unnoted. The maintainer corrected this count once already (round 8) | "five", noted at both fields |
| T7 | poller.h:236-237 (06) | card_blocks is "0 only if GET SYSTEM INFO did not answer"; it is also 0 when the answer carries no memory field (poller.c:377-381) | say both |
| T8 | poller.c:114-117 (pre-round; stale via 05) | "four places … One list, four callers": finish_conversion is a fifth | five, or drop the count |
| T9 | poller.c:245-246, 09 msg:38 (09); 08 msg:58; reply.md:186-187 | "one tested tag" answers at every address: gen3-a does too (gen3-a-bench.md, full sweep) | "two tested tags" |

**Suggestions, same class, from the comments agent, not re-checked line by line:**

- partial_details.c:196-197 (06): "no configuration write ever attempted". Every gen2 attempt sends one; say "ever taking".
- Registers said to "hold" the sequence's bytes: poller.c:2053-2054 (03), :2328-2329 (07), poller.h:133 (09). On gen1, 62/63 refuse the write and none of the four read back.
- write_fail.c:320-322 and :338-339 (pre-round) still state the gen1 Partial rule as it was before 03. Folds into the Decided fix.
- "with nothing sent before it": poller.c:1431-1432 and :2007 (07), CHANGELOG:137 (08), issue255-comment.md:14. The record says "no unlock or commit".
- poller.c:1045-1047 (05): moving the UID proves 56/57 are registers. That 62/63 are too rests on the three gen1 chips.
- The refusal-cost comments, poller.c:295-296 (pre-round) and :1241-1242 (09): "answers in band". Not under OPTION, where every refusal is a timeout plus a read.
- poller.c:983-984 and :960 (06): a timed-out survey keeps what it found; and "the pass above" is below.
- poller.c:379-380 (06): "over-claim" is used in the opposite sense to everywhere else.
- poller.c:279 (10): "Measured at 4", when 20 lines down the figures are an estimate. The maintainer's round-6 wording was "on your sample card".
- poller.c:244-245 (09): "took writes past its end". The tag was sent them.
- CHANGELOG:125 and app_i.h:121 (06, 08): "a geometry", when the note also fires on IC ref alone.

## Commit messages

- **07:83 [Important]** "The release notes carry the same correction in the last commit." That is 08, the next commit, not the last.
- **04:14-17 [Suggestion]** "The last rules out the remaining one: addressing alone is not enough." It is the second row, `22 29 <uid> 05`, that shows that.
- **07:66 [Suggestion]** "DSFID survived per card, so nothing was written outside the UID registers." DSFID changes only under WRITE DSFID, so this proves nothing about other blocks. Keep the first half.
- **02:56-60 [Suggestion]** The "only on silence" bullet implies it prevents a write-protected blank card from reporting "Wiped 64/64". Under OPTION every refusal is silent (the TI timed out on 64-69, addressed-writes-implemented.md bench 9), so on the only cards it applies to, it doesn't. Line 58 is also the one line in the twelve that opens with "--", indented.
- 05:23-25 depends on A. 08 carries T2 and T9, and 09 carries T9.
- **Checked:** 11 and 12 are accurate against their diffs and the SDK.

## The drafts (notes only, no replay)

- **R1 [Important] reply.md:216-217** "…under a Partial banner with a note saying nothing had been skipped". The pushed tree never had that note; `1d411dec` shows "gen1: 56/57/62/63 differ". It compares against a version the maintainer never saw.
- **R2 [Important] reply.md:266-268** "a source in which no two blocks are alike". The 70-block source has 9 distinct blocks and 62 all-zero ones. The distinct pattern was on the CARD, `wipeseed_64` first.
- **R3 [Important] reply.md:29, :80, :257** "seven cards" reads as the whole bench, which spans ten cards.
  - The seven the addressed writes were validated on, plus `slix-1k-50x28`, `slix-1k-coin18` and `v1-coin-green18`.
  - The double clone and the three-chip Write UID ran on `slix-1k-50x28`.
  - :100-103 leads with "five cards", but its block-56 result is one card per chip.
- **R4 [Important] reply.md:322** "Recorded on #255" comes before it is, against :329-331's "I will put…". Say "It goes on #255 as well", or post #255 first.
- **R5 [Important] reply.md:116-117** "ones neither of us has" is a claim about the maintainer's cards (attribution class). "ones I have never had" is safe. **The maintainer's quoted remarks not re-checked.**
- **R6** reply.md:135-136 depends on A.
- **R7 [Suggestion] reply.md:299-301** A screen entered without a reason crashes only if no reason has been set this session; on_exit keeps the state. **Not re-checked.**
- **R8 [Suggestion] reply.md:288** "the list I gave you in the comment-cut round". The list was in the "Round 9 addressed" reply (2026-09-13); the cut was in "Round 11 addressed" (2026-09-16).
- **R9 [Suggestion]** About 23% of the reply's words repeat the commit messages (06, 08, 05, 02). Structural, mfcarroll's call. **Not re-checked.**
- **I1 [Important] #255 draft** It leaves the issue body's "neither of these has been exercised on hardware" (body :89-92) standing for gen1. The LRi2K reproduced it (CHANGELOG:139-140, poller.c:1433-1434), and so did today's bench.
- **I2 [Suggestion] issue255-comment.md:16** "the addressed writes to 62/63 the sweep sends". The record is block 62, by hand. Say "writes to 62/63 are refused on all three chips".
- **I3 [Suggestion] issue255-comment.md:24** "the earlier variant" is undefined on #255, and the brick is the reporter's claim (CHANGELOG:148-150). Attribute it.

## Simplifications (report-only, mfcarroll's call; the agent's report has each diff and equivalence argument)

| # | proposal | owner | pairs with | card-facing |
|---|---|---|---|---|
| S1 | = B | 05 | A | no |
| S2 | `has_details(Partial)` tests bare `used_gen1`; 03 narrowed the other three sites (write_fail.c:82-83) | pre-round, stale via 03 | Decided | no |
| S3 | the LSB-first UID append is written twice (poller.c:520-522, :636-638); one helper beside its rationale | 01, 04 | — | byte-identical, host-pinned |
| S4 | the survey's eleven fields kept in four places; one struct | 06 | — | no; wide diff, later or skip |
| S5 | see above | 06 | — | no |
| S6 | the holds-more sentence written twice (partial_details.c:176-190) | 06 | — | no |
| S7 | the gen1 deduction count computed twice (poller.c:1124-1129, :1058-1062) | 03, 05 | Decided, B | no |
| S8 | the success set written twice, tone and button (write_fail.c:191, :529); 06's CloneComplete once missed both | 06 | Decided: the new case needs both | no |
| S9 | the note preamble written seven times (partial_details.c) | 06 | — | no |
| S10 | `!iso15693_wipe` in `clone_notes` is dead (scene_write.c:406) | 06 | — | no |
| S11 | `over_capacity ||` in partial_details' `empty_blocks` is dead (:15, :51) | pre-round | — | no |
| S12 | Start copies the starting identity in two branches (poller.c:1813-1817, :1883-1885) | pre-round, 01 | — | no |

Spot-checked: S2 and S8. The rest are **not re-checked**.

## Checked and not a defect

- **poller.h:286-289**, the all-zero-UID LRi2K "stayed reachable": today's bench answers the worry. The second wipe on the zeroed card sent writes addressed to 00..00, and they were answered, so the result was Success. The sentence stands.
- **Checked and found correct:**
  - OPTION stickiness and reset per run
  - the 0x03-then-retry path
  - read-back size and bounds
  - UID byte order in every frame
  - predict_uid's mapping
  - readdress's two honest answers
  - 11's bound and casts
  - 12's four entry points and exhaustive switches
  - every new field reset in start_internal
  - integer widths
  - the budget arithmetic
  - the SDK claims (one EOF, 1-slot inventory, flags hardcoded, accessor furi_check)
  - 117 to 157 lines, about half the file
  - both reference paths exist
  - the gen3-a paragraphs in both drafts
  - the [👤] paragraphs contradict no record

## Where the fixes would land

- **01:** S12 (optional).
- **02:** T1, T5, and 02's message.
- **03:** the Decided gate, S2, S7 and the "hold" wording at :2053.
- **04:** 04's message, and S3 (optional).
- **05:** A, B, S7, 05's message and the :1045 wording.
- **06:** the Decided note, E, S5, S6, S8, S9, S10, T7 and the survey and "over-claim" comments.
- **07:** 07's message, T4 and "nothing sent before it".
- **08:** T2, T3, T5's CHANGELOG half, the gen1 rule at CHANGELOG:45/:57-60, :125, :137, and 08's message.
- **09:** T9, :244 and poller.h:133.
- **10:** :279.
- **Pre-round lines this round made stale** (T6, T8, :295): fold at the commit that made them stale.

Behaviour changes (the Decided fix, A, B, D, E) want host tests and a bench on `lri2k-keychain`:

- a gen1 clone with 56..63 empty, and one with data there;
- the re-clone that converts.
