# Review 2 of round 15 — comments, fork messages, reply (2026-09-27)

Single-threaded, no fan-out. **Nothing edited**; every proposal is mfcarroll's to take or leave.
Line numbers are dev HEAD `233f01c`. They match fork `f61285bc` line for line. **NN** is the fork
sync point that wrote the text, from `git blame` on the fork.

## What was checked

- Dev's shipped tree equals `f61285bc` file for file, and no shipped commit sits above 08's anchor.
  Nothing shipped has changed since that build was verified, so build, host tests and clang-format
  were not re-run.
- Gates: comments 0 findings (14 long-block warnings, four of them new or grown this round), forkmsg
  0, headings 0, drafts 0. **Every finding below passed all of them.**
- Intra-push churn, re-measured by multiset against the tip: **69 lines**. By sync point: 01 22,
  02 22, 03 14, 04 2, 05 2, 06 0, 07 7, 08 0. The 7 at 07 are the first text flagged.
- Each sync point's comments were read in their own tree. "…that a bystander could act on…" exists
  only at 07.

## 1. The "what remains unaddressed" list is still held three times, and it churns

WRITING-RULES names this as the cause of the churn, and states the rule: "each frame builder should
state what IT does; nothing should hold the list." The tip still holds that list three times:

- `iso15693_poller.c:24-28` (08): "ONLY THE GEN2 SEQUENCE IS UNADDRESSED and this byte is now its
  alone". The word "now" is history, and the reason is only a pointer to the builder.
- `iso15693_poller.c:86-88` (08): "WHAT REMAINS UNADDRESSED is the gen2 backdoor alone … Every other
  frame this app sends carries a UID."
- `iso15693_poller.c:888-894` (07): the write helper's "The gen2 backdoor sequence builds its own
  frames and stays unaddressed, for the reason at ISO15693_MAGIC_FLAGS; the identity pass and the
  gen1 backdoor sequence build their own too and are addressed … which is where the evidence that
  addressing those addresses works came from in the first place." ISO15693_MAGIC_FLAGS no longer
  holds the reason: it points on to build_gen2_frame, so this is now a pointer to a pointer.

**"Every other frame."** At 07, "Every frame this app sends that a bystander could act on now carries
a UID" is consistent only with the conforming-tag argument before it. 08's own bench shows that
argument covers the wrong population, because a gen2 magic bystander does act on those frames. So
yes, it should say "every other". But 08's version, "Every other frame this app sends carries a UID",
is false the other way. The SDK sends READ SINGLE BLOCK and GET SYSTEM INFO unaddressed: flags
SUBCARRIER_1 | DATA_RATE_HI, no UID, at `lib/nfc/protocols/iso15693_3/iso15693_3_poller_i.c:199-202`
and `:169-170`. The inventories are unaddressed too. The survey, the OPTION read-back and the sweep's
re-probe all read through those calls. **The scope that holds is writes.**

**The churn cycle.** The base had a four-item list. 01 deletes it, 02 re-adds three items, and 04
deletes them again. 07 then re-adds one item in new words carrying the conforming-tag argument, and
08 retracts that argument. So 07→08 is also an **error-then-fix pair inside one push**: 07's new text
asserts what 08's bench refutes. Message 07 does the same thing ("These were the last frames in the
feature a bystander could take"), and 08 answers it ("the reason given for that has been replaced").

**Proposed:**
- **07:** add neither paragraph, and leave the write helper's note as it was at 01-06 ("Every
  DATA-block write … like any other block."). The define keeps its one-line comment until 08. Line
  90 stays "Addressing does not CLOSE #251", as at 04-06, which takes 04's 2 churned lines as well.
- **08:** replace 24-28 with a note about the byte alone, and add nothing at 86:

      // Used only by the gen2 sequence (see iso15693_poller_build_gen2_frame), and LITERALLY rather than
      // through iso15693_poller_write_flags(): with OPTION set a gen2 card applies the write and answers
      // nothing -- on all four measured, including two that do not want the flag for ordinary writes.

- **07, twin:** `iso15693_poller_verify_inventory` ends "Same root cause as the unaddressed writes
  above; #251" (`:1833`). From 07 on, the only unaddressed writes are the gen2 frames, so the line
  misleads. Proposed: "Addressing cannot fix it: this read exists to find out whether the UID changed,
  so it cannot be aimed at a UID already in doubt; #251."
- **Optional, larger:** lines 90-93 restate that same comment, so they could go entirely. That touches
  01, 02 and 04 and removes most of their churn with it.

## 2. Process narration in shipped comments

- `iso15693_poller.c:107-111` (07). "…and it is why "the registers answer" was one chip for as long as
  the frames went out unaddressed."
  - **Yes, narration.** "For as long as the frames went out unaddressed" is the code's history, and
    "the registers answer" quotes the maintainer's round-13/14 review.
  - **Not a reason either.** "The second reason the sequence is addressed" rests on NXP answering. The
    answer is read only for a 0x03, and neither NXP code is 0x03.
  - **Stated four times:** the 0x0F/0x10 result also appears at send_gen1_frame `806-809`, the
    sequence `835-838` and the OPEN QUESTION `1513-1515`.
  - **Proposed:** delete 107-111. Keep one statement, at send_gen1_frame: "NXP ICODE SLIX and SLIX-S
    answer this frame only when it is addressed -- 0x0F at block 62, silence unaddressed; ST LRi2K
    answers either form with 0x10." The other two sites say only "refused on every gen1 chip measured".
- `…partial_details.c:53-55` (06). "Naming it after the block list was right when the list was all
  there was; it now sits…" **Proposed:** "One title whatever the page holds: its notes are independent
  of one another, and each opens with its own label."
- `…partial_details.c:60-63` (06). "The list opens with its own words now that the page is not named
  after it." **Proposed:** "Empty top blocks are past the card's physical capacity and lost nothing,
  so they are labelled as not fitting rather than as failures. ("Card too small" is reserved for the
  partial screen, where data IS lost.)"
- **Twins the title change left stale.** These predate the round and became wrong at 06:
  - `…partial_details.c:10`, "its reason picks the title". The mode alone picks it now.
  - `:31-33`, "Titling an empty list "Blocks not written" would be wrong, so name the screen for what
    it actually shows". The title no longer depends on the list.

  Both should say the reason and the list decide the list's label.
- `iso15693_poller.c:1508-1510` (07). "So "a card someone previously ran a gen1 write on" is not the
  condition" argues against wording this tree never contains, and "keeps most of this shelf clear of
  it" refers to our bench. The rewrite is in 3.
- `iso15693_poller.c:532` (04). "these had the strongest claim to it of anything left" is inventory
  language. **Proposed:** "ADDRESSED: WRITE AFI and WRITE DSFID are STANDARD commands, so a bystander
  of any size takes them, and …"

The gate's HISTORY patterns match none of these. MUST-match selftest candidates: "was right when",
"now that the page", "for as long as the frames went out", "is now its alone", "in the first place".
This would be dev-only tooling.

## 3. The yo-yo: "take a write with nothing sent in front of them"

What is measured: five gen1 cards of unknown history. The `[👤]` paragraph in the reply says so: they
may have arrived armed, or been armed early on. The text has swung from too narrow ("a card left
armed") to too broad, a general statement about gen1 silicon.

- **Measured:** every gen1 card here took a write to 56/57 with nothing sent before it.
- **What follows for a user:** a card's history cannot be known, so the hazard covers any gen1 card
  the sweep reaches.
- **What does not follow:** that gen1 cards never need the frames.

| site | NN | says | proposed |
|---|---|---|---|
| `nfc_magic_app_i.h:138-139` | 07 | "take a write with nothing sent in front of them, so it needs no prior gen1 write on the card" | "…which ARE the UID registers; every gen1 card measured took that write with nothing sent before it." |
| `iso15693_poller.c:1506-1511` | 07 | "no history is needed -- those registers take a write with nothing in front of them … not the condition … this shelf" | "WHICH CARDS: any gen1 card the sweep reaches. Every gen1 card measured took a write to 56/57 with nothing sent before it (evidence at ISO15693_MAGIC_BLK_UNLOCK), and a card's history cannot be known, so the bound is the reach rule at the wiped == 0 branch. Reproduced end to end on an LRi2K: …" |
| `iso15693_poller.c:2089-2091` | 07 | "they take a write with nothing sent in front of them, so any gen1 card…" | "…ARE the UID registers, every gen1 card measured took a write there with nothing sent before it, and so a sweep that reaches them can leave the card wearing a different UID." |
| `iso15693_poller.c:126-134` | 07 | "neither is necessary on this shelf … It is an argument about what may be claimed for them, which is nothing." | "Neither has ever been observed accepted, on any card here, in any form. Five cards took the write to 56 with neither sent first, but their histories are unknown, so that says nothing about a card that needs them. They stay because proxmark sends them; nothing is claimed for what they do." |
| `CHANGELOG.md:136-139` | 08 | "on every one blocks 56/57 take a write … so the wipe hazard above needs no prior gen1 write on the card" | "…and on every one blocks 56/57 took a write with nothing sent before it. A card's history cannot be known, so treat the wipe hazard above as applying to any gen1 card whose sweep reaches those blocks. It was reproduced on the LRi2K, …" |
| message 07 :69-73, :98 | 07 | "What the same evidence does settle is that they are not required"; "Those registers are simply writable." | "None of the five cards here needed them -- each took the write to 56 with neither in front of it -- but their histories are unknown." Cut the second sentence. The message's own :109-114 ("Nothing is claimed in the other direction. There may well be a lock state…") is the right register, and currently contradicts :69-73. |
| reply :35, :38, :122 | — | "There is nothing under that qualifier"; "simply take a write"; "The same evidence says they are not needed" | see 7 |
| #255 :14 | — | "take a write with nothing sent in front of them … The table's "armed gen1" row is just "gen1"." | "…took a write with nothing sent before it … Nobody can tell whether a card has been armed, so any gen1 card whose sweep reaches 56/57 can come back wearing a different UID … The table's "armed gen1" row should read "gen1"." |
| `.notes/squash-message.md:116` | — | the same phrase | fix with the rest, before it is posted |

## 4. The other two quoted lines, and the same register elsewhere

- `iso15693_poller.c:1025-1028` (06), and message 06:40-43. "Eight reads on a card telling the truth
  is what not resting on that costs."
  - **Intended meaning:** the survey runs on the gen1 path too. gen1 cannot misstate its count, and on
    a card whose count is true the survey costs one absent run of reads.
  - **"Eight" is a number true of most cases.** It holds only when the card ends where the source
    does. A 40-block SLIX-S under a 28-block source reads 12 more first.
  - **Proposed:** "It runs on the gen1 path too: that gen1 cannot misstate its count is known only for
    the cards measured, and the check costs one absent run of reads past the card's top." Drop "this
    whole file exists because claims lie".
- `iso15693_poller.c:1074-1075` (06). "Both sides of this are claims, and that is the right subject:"
  - **Proposed:** "Does the card report the geometry and IC reference the source did? Claims on both
    sides, because a reader shows claims."
  - **Say it once.** The same point is also at `iso15693_poller.h:237`, reply :213 and message
    06:28-29. Keep the function's version and cut the rest. In the reply, the Emosyn example makes the
    point on its own.
- `iso15693_poller.c:1019-1021` (06). "a claim is a costume … a guess wearing the clothes of a
  measurement". The same appears at reply :186-187 and message 06:31-33. **Proposed:** "The advertised
  count cannot be the test: on a magic card it is whatever was last programmed."
- `iso15693_poller.c:908-911` (02). "and the reason is structural rather than a hunch about flaky
  radio … Ask the memory instead of the messenger."
  - **Duplicated.** The same rule is also at ISO15693_POLLER_OPTION_FLAG `78-83`.
  - **Proposed:** keep the rule here, beside the code, without the flourishes. Cut 80-83 down to
    "…see the timeout arm of iso15693_poller_write_block_retried".
- `iso15693_poller.c:768-773` (05). "It makes the answer checkable instead of merely believed, and it
  is what turns … which is a strong enough premise to act on." **Proposed:** "The prediction does not
  replace the inventory: it holds only if the card is gen1, which is what is being tested. It lets
  the answer be checked rather than believed. Not airtight: …"
- `iso15693_poller.c:801-802` (07). "What makes it magic is the ADDRESS it names, not the command,
  which is exactly why it must be addressed" uses "address" in two senses in one sentence: block
  number and UID. Message 07:85 has the same problem. Say "the block number".
- `…partial_details.c:176-184` (06). "is the tempting phrasing and it is wrong twice over … Only the
  SUMMARY makes them exclusive, and that is priority, not agreement." **Proposed:** "'The same as the
  file' only when the two numbers are equal. Not 'configured to match': that claims this app set the
  count, and only the gen2 CFG frame does, only on a magic card -- a tag that already wore the file's
  UID and a gen1 clone both reach here with a count nothing here wrote."
- `…partial_details.c:201-202` (06). "is what makes this actionable rather than a complaint". Cut the
  sentence.
- `…write_fail.c:257-265` (06). "the physical top is the surprise underneath it", plus a second copy
  of the who-set-the-count paragraph. Keep one copy, at partial_details, and point to it.
- `iso15693_poller.h:106-107` (07). "which is what a caller's consent text can promise and what it
  cannot" does not parse. **Proposed:** "ADDRESSED: a tag in the field with a different UID ignores
  it, so a caller's consent text need cover only the card in hand."
- **One rule, four statements.** The gen1-caveat rule is stated at the `poller.h` gen1_blocks_skipped
  field, `success_or_partial` `1775-1781`, `write_fail.c:348-349` and `partial_details.c:154-156`.
  The field is its home, and the two scene copies can go.
- **Our own vocabulary.** `iso15693_poller.c:129`, `:133` and `:1509` say "this shelf", and
  `CHANGELOG.md:150` says "this bench". Use "the cards measured" or "here", as elsewhere.
- `iso15693_poller.c:57`, `:78` (02) name `white-coin`, a bench label no reader of the PR can
  resolve. Use "a TI Tag-it HF-I Plus card".

## 5. Claims the code or the tree contradicts

- `iso15693_poller.h:238` (06): "card_blocks / card_ic_ref are what the CARD says, and are 0 while
  neither flag is set." **False.**
  - compare_reported_geometry sets both unconditionally once GET SYSTEM INFO answers
    (`iso15693_poller.c:1096-1098`).
  - The size note depends on that. In the gen2 residue case neither flag is set, yet
    `write_fail.c:269` and `partial_details.c:186-198` print card_blocks.
  - A reader who believed the comment and zeroed them would break the size note.
  - **Proposed:** "…what the CARD reported; 0 only if GET SYSTEM INFO did not answer."
- `iso15693_poller.c:510` (04): "Every write this app sends takes its flags from here". The same file
  says at line 26 that the gen2 byte deliberately does not. **Proposed:** "Every standard write…"
- `iso15693_poller.c:97` (07): "built by iso15693_poller_build_write_frame like every other write
  here". The identity writes build their own frame in send_identity_frame, and gen2 has its own
  builder. **Proposed:** "like the data-block writes". Message 07:10 has the same slip.
- `iso15693_poller.c:535` (04): "TI Tag-it, which is the only chip that refuses anything here".
  Every gen1 chip refuses 62/63. **Proposed:** "the one chip here that wants the OPTION flag".
- `iso15693_poller.c:1030-1031` (06): "A block above the source that EXISTS is a statement about the
  card's size". holds_more fires only above the count the card reports (`1061`). On gen1 that is the
  card's own size, not the source's. **Proposed:** "A block that answers above the count the card
  reports is a statement about its size".
- `iso15693_poller.c:1192-1195` (07): "so all three chips this was validated on are under the
  boundary". The deduction depends on the source, not the chip, and the round's own conversion bench
  exercised it: a 64-block source onto a gen1 SLIX, per message 05. **Proposed:** "Nothing is deducted
  from a source that ends before block 56." Drop the validation remark.
- `iso15693_poller.c:2296-2297` (01): "Derived from the card rather than supplied by the caller,
  unlike target_uid / original_uid". original_uid is set in write_step from the card too (`1964`).
  **Proposed:** "Set in write_step from the card's own answer, so it is reset here with the rest of
  the run state."
- `CHANGELOG.md:17` (08): "**gen1 has no geometry register**".
  - Message 06:65 says ""Geometry register" is imprecise and the wording goes with them", and the
    scene says "configuration register".
  - The phrase first appears in any tree at 08. So 06 describes a wording the maintainer has never
    seen, and the release notes then adopt it.
  - **Proposed:** in the CHANGELOG, "**gen1 has no configuration register**". Cut 06's sentence.
- `CHANGELOG.md:123-125` (08): "notes for three things above the file's last block". The geometry
  note is not above anything. **Proposed:** "notes for three things: data still above the file's last
  block, a card answering reads past the count it reports, and a card reporting a geometry the file
  did not."
- `CHANGELOG.md:149-151` (08): "the mechanism behind it is not, since one tag on this bench does keep
  a writable UID register at 0x10". A register at 0x10 shows the identity half, not the bricking,
  which is about 0x14/0x15. **Proposed:** "The bricking is their report, not observed here. One tag
  here does keep a writable UID register at `0x10`, inside any claim, where a sweep would zero it."
- `CHANGELOG.md:62-64` and `:109-114` (08): the read-back is stated three times across the two
  bullets. The OPTION bullet's last clause, "so its writes are confirmed by reading them back",
  repeats its own previous sentence. **Proposed:** cut that clause, and cut the first bullet's addition
  to "…except on a card that needs the OPTION flag (below), whose blocks are read back and compared."
- *Reasoned, not measured. Low priority.* `CHANGELOG.md:157-160` names only the gen2 frames and the
  post-wipe re-read as what a second tag can still disturb. The unaddressed reads feed the new survey
  and geometry notes too, so a bystander answering a read above the target's top could produce a
  false "holds more" note. "Keep one tag in the field" already covers it as advice. Either do not
  enumerate, or add "and the app's reads, which the firmware sends unaddressed".

## 6. The fork messages, against their own trees and the rules

- **07 describes changes absent from its diff.** 07 does not touch CHANGELOG.md; only 08 does. Two
  passages still describe release-notes edits anyway:
  - `07:116-119`: "One more release-notes line goes with it … The rule still holds and now carries
    its exception".
  - `07:92-93`: "at six places, three of them user-facing". 07 changes no user-facing string: every
    "armed" site it touches is a comment.

  Move 116-119 into 08's WHAT IS CORRECTED, which is missing that item. Reword 92-93 to name only the
  comment sites 07 carries.
- **07:12-18.**
  - "These were the last frames in the feature a bystander could take": see 1; this is true only of
    standard writes.
  - "the rest of the round already does it everywhere else; shipping the safety fix with this frame
    set exempt would have been the first thing a reviewer asked about": talks about the round and
    about the reviewer, in a message that outlives both.
  - **Proposed:** "These were the last standard writes in the feature to go out unaddressed, and the
    worst to leave open: … #251 asks for exactly this."
- **07:33-34.** "both because the bench ran again and not because either was wrong" is bench
  narrative, which the README rules out. **Proposed:** "Two figures in 01's comments widen here: …"
- **06:97-102.** "It no longer asserts gen1 either". The note is introduced in 06, so the version that
  asserted gen1 exists only in collapsed dev commits (question 1). Drop "no longer". The same applies
  to 06:65's "geometry register" sentence (section 5).
- **04:30-33.** "The file's own list of what remains unaddressed is corrected with them." 04 deletes
  the list outright, including the gen1 and gen2 entries, which stay unaddressed until 07/08. If
  section 1 is taken, the accurate word is "removed".
- **01:17-18.** "the error this round corrects elsewhere": "this round" means nothing in `git log`.
  **Proposed:** "…so it is counted as a card, not a chip."
- **01:27-28**, and the same wording in code at `iso15693_poller.c:933-934`. "this file declines that
  inference everywhere else" does not say what the inference is. **Proposed:** "a tag can apply a
  write without answering, so no answer is not taken to mean no write."
- **08:9.** "Every other frame set": make it "every other write" (section 1).
- **08:58-59.** "where the round ends up": make it "the end state".
- **05:46.** "CHECKS ITS ANSWER NOW. It took whatever…" is the exact phrasing WRITING-RULES lists as
  reply narration. It is defensible in a commit message, since 01's version is in the PR, but
  "The re-address checks its answer" loses nothing.

## 7. The reply

- **:34-41.** Rewritten below to agree with the `[👤]` paragraph, which currently reads as a rebuttal
  of the sentence just above it. The rewrite also fixes "The notes **and the screens** say so now".
  No user-facing string has ever mentioned arming, before or during this round, so the screens did not
  change.

  > **A second correction, smaller but user-facing.** The release notes scoped the wipe's identity
  > hazard to a gen1 card "left armed by an earlier UID write". Five gen1 cards here take a write to
  > block 56 with neither "unlock" nor "commit" sent first — one of them a card this app had never
  > written — and no card has accepted either frame. A user cannot know a card's history and nothing
  > can detect it, so the hazard is any gen1 card whose claim lets the sweep reach 56/57: the reach
  > rule, not the card's history. The release notes say so now, and I will put the same correction on
  > #255, which still carries the older wording.

- **:120-125.** Drop "The same evidence says they are not needed — …" and the second "including a card
  this app had never written". Keep "They stay anyway: proxmark sends them, and the cards that would
  prove them necessary are ones neither of us has." The `[👤]` paragraph carries the rest.
- **:99-102** does not parse ("…about the card in the user's hand, but risks accidentally modifying
  any other card in the vicinity"). **Proposed:** "…user data — and they went out behind an opt-in
  whose warning covers only the card in the user's hand."
- **Broken wraps.** GitHub renders a newline in a comment as a line break, so these will show as short
  lines: :105-106; :109, where "every card refuses," sits alone; and :147-148, "…is an inventory. The" /
  "SDK's is…". Re-wrap them.
- **:213.** Cut "Both sides of that comparison are claims, deliberately: …" (section 4).
- **:286.** "now on all seven": the maintainer never saw fewer. Drop "now".
- *Optional,* **:25-29.** This is the archaeology of how the error happened, and WRITING-RULES asks
  for the correction "in one sentence, without the archaeology". The `hf 15 wrbl` detail is worth
  keeping because the maintainer may rely on that tool: "Its control was `hf 15 wrbl`, which
  silently sets OPTION for TI's manufacturer byte, so it changed two bits, not one."

Re-derived and fine:
- the 2.3 notes grow from 120 lines to 159
- the V3 developer's card has a record (`tag-inventory.json`, "Gift from 0x6r1an0y")
- no `[👤]` paragraph is touched by any proposal here

## 8. Block lengths and wraps

- **Overlong line.** `iso15693_poller.h:107` (07) is **170 columns**: a wrap was never applied. These
  files wrap at about 104, and `ReflowComments: false` is why clang-format let it through.
- **Blocks over 20 lines, new or grown this round:**
  - OPTION_FLAG `55-84`: 29 lines, new (02).
  - the gen1 block `101-134`: 22 → 34 (07).
  - the survey header `1010-1034`: 25, new (06).
  - `1216-1237`: 13 → 22 (05). That growth is two unrelated comments merged by a missing separator;
    see the next bullet.

  The trims above bring the gen1 block to about 20 lines and the survey to about 17. OPTION_FLAG loses
  80-83, which duplicate the timeout arm.
- **Missing paragraph breaks.**
  - `iso15693_poller.c:1228→1229` (05): "…Attempt every block." runs straight into "skip_backdoor is
    the CALLER's decision", which is a different comment.
  - `:939→940`: "…to know it might." runs into "Only a full-width write".

  Each needs a `//` line between them.
- **Early breaks** (the next line's first word would fit): `iso15693_poller.c:841` (07), `:928` (07),
  `:1113` (05), `iso15693_poller.h:246` (06), `CHANGELOG.md:123` (08).

## 9. Code simplifications — code, not comments, so mfcarroll's call

Both are behaviour-preserving by reading. Neither is needed.

- **send_gen1_frame collapses to one call.** `iso15693_poller_send_gen1_frame` (`813`, 07) is exactly
  `(void)iso15693_poller_write_block_addressed(instance, iso_poller, data, block,
  ISO15693_MAGIC_REGISTER_SIZE)`: the same builder, flags, send and option check, and the parse in
  write_block_addressed has no side effects. That saves about ten lines, and its comment shrinks to "a
  WRITE BLOCK like any other; the result is discarded".
- **The conversion switch is made twice.** `write_source_blocks` makes it with the same log line in the
  success arm (`1273-1293`) and again after it (`1295-1304`). Instead:
  - compute `moved_uid = !skipping && instance->uid_moved_by_write` once, before
    `if(error == …None)`
  - make the switch there
  - gate `wrote_above_failure` on `!moved_uid`

  Same result on both paths, and the two comments become one.

Both touch benched code, at 05 and 07. The host tests cover the conversion and the gen1 sequence, but
the mutants would want re-running.

## Where the fixes land, and what the fold costs

| sync point | tree | message |
|---|---|---|
| 01 | 2296 (minor) | 17-18, 27-28 |
| 02 | 57/78 label; 80-83; 908-911 | — |
| 04 | 510; 532-535 | 30-33 |
| 05 | 768-773; 933-934; 939; 1113; 1228→1229 | 46 (optional) |
| 06 | 1019-1034; 1074; poller.h 237/238/246; partial_details 10, 31-33, 53, 60, 176, 201; write_fail 257-265, 348 | 28, 31-33, 40-43, 65, 97-102 |
| 07 | stop adding 24-28/86-88/888-894 text; 97; 107-111; 126-134; 801-809; 835-841; 928; 1192-1195; 1506-1515; 1833; 2089-2091; poller.h 107; app_i.h 138 | 10, 12-18, 33-34, 69-73, 85, 92-93, 98; 116-119 → 08 |
| 08 | 24-28 as the define note, nothing at 86-88; CHANGELOG 17, 62-64, 109-114, 123-125, 136-139, 149-151 | 9, 58-59, and the moved item |

- **Nature of the change:** all of it is comment and release-notes text unless section 9 is taken, so
  every bench stands.
- **Process:** the usual safety branch, then a tree-level fold, `comment-only.py` on each rewritten
  commit, and the per-sync-point gates.
- **Replay:** rebuilds all eight fork commits, because 01's message changes. The branch is still
  unpushed and still a fast-forward from `1d411dec`.
- **The reply, the #255 comment and the squash message** are notes-only and move independently.
