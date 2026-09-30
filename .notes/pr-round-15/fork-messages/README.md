# Round 15 — the behavioural sync points, SLIX saves as a source, the gen2 measurement and the release notes, two comment passes, and two hardening fixes

The addressing round. 01-07 each change behaviour, and 08 makes a SLIX save from the stock NFC app a
clone source. 09 changes none: it is the whole round's release notes plus the one comment that had to
move with them, because a note stopped arguing a case and started stating a measurement. 10 and 11
change no behaviour either: they correct comments from earlier rounds that the whole-PR review of
2026-09-28 ([app-review.md](../app-review.md)) found false (10) or telling their history (11). Round
15's own lines were fixed in the sync points that wrote them. 12 and 13 are hardening from round 10's
self-review, never put to him until now: the failure bitmap checks its index (12), and the write-fail
reason switches list every reason so -Wswitch sees them (13). Neither changes what a card sees. 13
changes one behaviour: a write-fail screen entered without a reason now crashes instead of saying
"Not a magic tag".

**THE REVIEW-3 FOLD, 2026-09-29**, mfcarroll's calls on final-review-3.md, went into the sync points
that own each change, by the same index-filter method as the folds before it: the gen1 Partial rule
became a question about data at 03 (the note for a file that reached the registers with nothing there
arrives with the other clone notes at 06); the clone's UID re-read, its closing progress frame and the
accounting of the write the UID followed at 05 (which moves the register-as-capacity fix out of 06);
the survey's presence check at 06; the gen1 verify's report of a half-moved UID at 07; the S2, S3 and
S6-S10 simplifications where their lines were written; every text fix at the commit that wrote the
line. The host test for a repair that lost a frame sits at 07 rather than 05, because the fake can drop
only an addressed frame and the repair's frames are addressed from 07.

**AND ONE MORE FOLD, the same evening**, mfcarroll's calls on what the verification of the review-3
fold found. S11 and S12, first a closing commit on the premise that they touched only earlier-round
code, went where their lines were written instead: that premise was wrong, since it rewrote ten lines
01 and 06 wrote this round. S12 is at 01, with a comment that names no verify, because the gen1 one
compares against the presented UID only from 07; S11 is at 06, where the label is written. 05's
progress-frame comment no longer names the survey, which arrives at 06 and adds itself there. The
OPTION sentence in the per-block cost comment moved from 11, whose message says nothing there is
false, to 10 beside its twin in the data pass, and 10's message lists both. And two wording nits:
05's verify-state comment, and a comma splice at 07. The closing commit is gone. (Numbers here are
today's; fold 9, below, inserted 08.)

**AND S5, decided by the bench** ([review3-bench.md](../review3-bench.md)): the block-count flag compares
the count, not the block size, from 06's first commit, where the size term was written. An 8-byte-block
file onto a gen2 card that took the geometry and then refused every write, and onto a gen1 SLIX, never
reached a success; and where the counts agree, the size term's only effect is a note printing two
equal numbers.
06's message says so; a host test pins it.

**AND 08, SLIX SAVES AS A SOURCE** (fold 9), mfcarroll's call after the bench: the stock NFC app saves
NXP ICODE tags as SLIX, and file select refused them as the wrong card. `676842c`, a notes-only commit
sitting directly on 07's anchor, now carries the one-condition change and its host test
(`test_file_select_scene.c`, mutation-checked), so fork commit 08 is exactly that change; the
release-notes sentence went in with the other release notes, at 09. 07's message no longer says the
release notes are "in the next commit". The old 08-12 are 09-13.

| # | sync at | one decision |
|---|---|---|
| 01 | `d09f499` | data-block writes carry the card's address |
| 02 | `4a3e382` | the OPTION flag, and the read-back it costs |
| 03 | `e7663ff` | the gen1 loss claim is gated on there being a loss |
| 04 | `53faffb` | the identity writes are addressed and take the flag |
| 05 | `efa16d7` | a clone that lands in a gen1 card's UID repairs it |
| 06 | `fb9c2fd` | what a clone leaves behind, and what it says about it |
| 07 | `22b7101` | the gen1 registers, addressed and no longer mis-scoped |
| 08 | `62dc264` | a SLIX save from the stock NFC app is a clone source |
| 09 | `9bf1e3e` | gen2 frames cannot be addressed, and the 2.3 release notes |
| 10 | `376d67d` | comments from earlier rounds that said something false |
| 11 | `c7826ed` | comments from earlier rounds that told their history |
| 12 | `712bd06` | the failure bitmap checks the index it is given |
| 13 | `0412314` | the write-fail reason switches name every reason |

**EVERY LINE IS WRITTEN IN ITS FINAL FORM AT THE FIRST SYNC POINT THAT HAS IT** -- review 2's fold,
2026-09-27, [final-review-2.md](../final-review-2.md). A measurement that widened during the round
is stated at its widest from the commit that first cites it, a helper a later commit shares is
defined where its first caller is, and no comment keeps a list of what other code does not yet do.
Intra-push churn is 5 lines (it was 69); see the residual section below.

**ALL RELEASE NOTES ARE ONE COMMIT, 09, and nothing before it touches `CHANGELOG.md`.** Written per
commit they get rewritten by later commits in the same push -- three bullets for what is one fact to
a user and then merged, a bullet reworded twice as the bench widened. A release note is a release
artifact, not a running log, and he reads the delta between rounds. The old CHANGELOG-only sync point
folded into 09 with the rest.

**Dev history WAS reordered, deliberately**: the repair moved ahead of the survey so it
could be its own sync point. See below. 06 collapses six dev commits and 07 collapses six, and
those are the two places this round does not get one decision per commit; the reason is churn.
(07's range holds a seventh dev commit, "TI enforces the address too", whose shipped change moved
to 01 and 02 in the review-2 fold, so it now touches notes only. 09's range holds four: the release
notes, two small fixes to its gen3 note, one of which also touches the comment behind the same
warning, and the whole-PR review's release-note fixes.)

The notes commits and the host-test fake are dev-only and are not sync points -- which is why 07's
message cites the cards rather than a test.

## Why 07 carries two decisions, and anchors where it does

It is the one frame set 01 left out, and it carries its own measurement: at block 62 the addressed
form is the only one NXP silicon answers, which is a fact about the frames rather than about the
safety argument. Folding it into 01 would put a claim about block 62 inside a commit whose subject is
data blocks, and would hide the re-address seam -- the one thing addressing this sequence costs --
inside a commit that already explains a different re-address for a different reason.

**It ANCHORS ABOVE THE SELF-REVIEW, not at the commit that introduced the addressing**, and that is
what makes it carry the arm-model correction as well. Five shipped dev commits land after the
addressing: the correction itself, and then this round's own self-review fixing text the round had
written, the last of them carrying review 2's fixes to the range. The message is split under `==`
headings so the two decisions stay separable by a reader.

The arm correction is comment-only here, its release-notes line being in 09; nothing about it
changes behaviour. **Verify
before replaying** that no shipped commit sits after the last anchor -- `replay-to-fork.sh` checks this
up front now, and the end-state diff catches it too. It is what would have caught the sync points
once reaching only as far as the addressing commit while six shipped commits sat above them.

## Why 01 and 02 are separate, and why 02 is not two commits

01 is the #251 safety fix and stands alone: it is correct on every card here and no card needs it to
be writable. 02 is what makes TI Tag-it writable at all, and is separable from 01 — worth keeping
distinct so he can weigh them separately if he wants to.

02 is NOT split into the flag and the read-back, because the flag alone is a broken intermediate: it
makes the card accept the write and simultaneously destroys the acknowledgement, so that commit on
its own is one where a wipe zeroes a card and reports that nothing was cleared. That is the error-
then-fix pairing he has flagged twice, wearing a different hat.

03 folds the caveat wording and the outcome for the same reason in reverse: they are one rule — the
gen1 loss claim is made only where there was a loss — applied at three sites. Split, the first
invites "why was the outcome not fixed in the same breath?"

## Why the repair was lifted OUT, and why the survey and the screens stay together

The repair is the round's most consequential behavioural fix -- it stops a clone writing a file's
bytes into a gen1 card's UID registers -- and it had no visible existence, buried inside a commit
whose subject is what a clone leaves behind. A reviewer scanning subjects would never have found it.

**It was first tried as a split of the existing order and reported impossible: "it does not apply
without the survey in the tree."** That was a fact about a PATCH, not about the code. The repair
references no survey symbol at all -- no `clone_residue_*`, no `clone_survey_top`, no
`survey_above_source`; it is entirely inside the poller while the survey reaches the whole scene
chain. They conflict because they edit neighbouring regions of one file and one struct, which is a
rebase problem. So dev history was reordered instead: repair first, then survey.

The reorder is content-preserving and that was checked rather than assumed -- `git diff` between the
pre-reorder tip and the rebuilt one is EMPTY across every shipped path, and across `tools/` and
`.notes/` as well. The code did not change; only its order did, which is what keeps the hardware
bench standing. The repair also builds and passes the host tests at its own commit, so it is a sync
point a reviewer can actually stop at.

**06 COLLAPSES SIX**: the survey, the register-as-capacity fix, the notes-page wording, the comment
explaining the repair, and the two size-note corrections. Published one per sync point he would see
the survey introduce a geometry note reading "The card reports 28 blocks and IC ref 01, not the
file's" -- which leads with a number that matched -- and then see it corrected twice. **19 of the 30
lines the survey adds to the details scene are gone or rewritten by the last of the six** --
re-derived 2026-09-26 and again after the review-2 fold, by matching every line the survey adds to
`nfc_magic_scene_iso15693_partial_details.c` against that file at 06's anchor, counting a duplicated
line once for each time it was added. It read 16 when the range was five commits; the figure moves
whenever the range does, so re-derive it rather than quote it -- and match multiset, not membership,
or repeated lines like a lone brace read as survivors and the figure comes out low. **Zero churn
won**, on the same grounds as 02: a sync point must not show him an error we then fix.

**07 collapses seven**, for the reason in its own section above.


## Residual churn — five lines, each a later commit needing what an earlier one lacked

Measured by multiset against the tip: every line a sync point adds that is gone at 13. **5 lines**:

- **01's frame builder call passes `ISO15693_POLLER_WRITE_FLAGS`; 02 passes
  `iso15693_poller_write_flags(instance)`.** The builder takes its flags byte from 01 so 02 changes
  only what the caller passes, and the helper cannot exist before the OPTION flag it reads.
- **05's repair calls `iso15693_poller_send_backdoor_uid_gen1(iso_poller, ...)`; 07 adds `instance`
  to that signature**, because the sequence then needs the card's address and flags.
- **05's progress-frame comment says the frame follows the re-read; 06 adds the survey, which it
  follows too**, so two of its three lines gain the survey there. Written in its 06 form at 05 it
  named code that did not exist yet.
- **10 puts the OPTION sentence into the per-block cost paragraph; 11 drops "bench" from the line
  it shares**, one of 11's history items. Two changes to one line, neither correcting the other.

None is error-then-fix. **Check it rather than believing it** -- the measurement is a few lines of
Python over `git diff -U0` between consecutive anchors, and `git log <round base>..<tip> --
CHANGELOG.md` must still name exactly one commit.

## What these messages must NOT claim

- **No tests.** Every test is in `tools/hosttest/`, which does not sync, so a message citing one
  describes a change absent from its own diff. The mutation results, the host-test counts and the
  fake tag's new block kinds all stay out. That evidence belongs in the reply.
- **No dev SHAs**, and no reference to the round having been rebuilt.
- **No bench narrative.** The measurements are stated as results, not as the sequence that found
  them — several of them corrected an earlier reading during this round, and he reads the delta,
  not the search.
- **`gen-2-card` has no known chip.** Do not call it an EM-Marin; that type line belongs to a
  credential cloned onto it. Four identified chips, plus two cards whose silicon was never captured.
- **The TI chip is identified by its behaviour, not its UID.** Both TI cards are gen2 magic, so their
  UID, IC ref and geometry are all settings; what identifies them is the OPTION refusal, which one
  gave while wearing an NXP UID. Where a message first counts it as identified, say so -- 01 does.
