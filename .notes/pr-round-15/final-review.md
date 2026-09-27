# Final review of round 15, driven by REVIEW-PROMPT.md — 2026-09-26, at `f43802a`

Single-threaded, fresh context, nothing fixed except two Pass 1 lines in the instrument itself.
Every number below was re-derived at the time of reading. Findings are ordered by severity; each
names the site, the class, and what it should say instead. **Nothing here is decided** — the
remedies are proposals.

## Pass 1 — every mechanical claim re-derives

- **19 shipped commits** from `eaa1673`; exactly one touches `CHANGELOG.md` (08); nothing shipped
  above 08. The `.msg` names, the fork README table and NEXT-SESSION's table agree, and all eight
  anchors are ancestors of HEAD.
- **168 host tests, 0 failed**, from `make clean`. Five `-Wincompatible-pointer-types` in
  `nfc_magic_scene_write.c` are present at the round base too — the fakes type `NfcDeviceData` as a
  struct — so not a round defect.
- **95 files clang-format clean.** A zsh `for f in $files` loop hands them over as one filename and
  reports failure; use a while-read loop.
- **Gates**: comments 0 findings (14 advisory long-block warnings), forkmsg 0, drafts 0, headings 0.
- **FAP**: the first build compiled NOTHING — 0 CC lines, because SCons had it up to date. After
  deleting the object dir: 71 units, 0 warnings, API 87.47. REVIEW-PROMPT now says to do that.
- **Every sync point**: no conflict marker (all three marker forms, wider than the replay's
  `^<<<<<<< `), passes its own tests (132, 139, 142, 145, 149, 165, 168, 168), and compiles all 71
  units under `-Werror` with fbt's own `compile_commands.json` pointed at the anchor's tree. Every
  anchor imports the same 254 external symbols as the tip, which passes APPCHK, so every one loads.
- `comment-only.py`: 7 comment-only, 12 code; 08's poller change is comment-only.
- `check-stale-shas.py`: 0 stale in 23 files. No live citation depends on `refs/original`, which
  alone keeps 32 commits alive.
- **Replay** into a throwaway `--shared` clone: 8 commits, the tree equals a full sync of HEAD,
  fast-forward from `dbc11980`. The PR head on GitHub is `dbc11980`. Nothing from the maintainers
  since the round-14 post; #251 and #255 are both mfcarroll's.

## F1 — HIGH — the PR branch does not hold dev's round-14 tree, and fork commit 01 carries the gap

**The pushed tree is byte-identical to dev `868c598`, a commit no ref contains.** Round 14's `.msg`
files name `150505a`..`868c598`; dev's round-14 commits were re-committed at 09-24 16:15, and the
fork commits were built at 17:41 from those already-orphaned anchors — kept reachable now as
**`wip-round14-as-pushed`** (= `868c598`), so a `gc` cannot take the evidence. Dev's same-subject
commits (`ab95f19`..`ef2be2a`) differ by comment trims in the poller (the block-size clamp and clock-cut
helper docs) and in `nfc_magic_scene_write.c` (the CONSUME-the-grant block, 13 lines to 4). Then
`0c76458`, subject `tools+notes:`, edits four comments in three SHIPPED files. None of it reached
the PR.

**All of it lands inside fork commit 01**: 200+/73− across three files, where 01's own change is
171+/23− in one. The 79 lines are 29+/50− and PROVEN comment-only (`-fpreprocessed`, all three
files); three of `0c76458`'s four edits are to comments in his own commits. Message 01 describes
none of it.

**Why every check passed**: the "19 shipped commits" count starts at `eaa1673`, one commit too late;
the anchor check looks only above the LAST anchor; the replay's verification compares the end state
with dev HEAD, which agrees by construction; and that verification prints FAIL but exits 0. Class 10
and class 11, in the direction nothing looks: below the first anchor.

**Remedy, mfcarroll's call**: (a) a comment-only sync point 00 anchored at `0c76458`, the last
shipped commit before the round — its tree builds clean (71 units, `-Werror`) and passes 127/0; or
(b) disclose it in 01's message. Tooling either way: refuse an anchor that is not an ancestor of
HEAD, exit non-zero on a verification FAIL, and pre-check that `origin/<branch>`'s tree equals a
sync of the commit before the first anchor.

## F2 — MED — the 2.3 release notes narrate history no user had (classes 3 and 8)

Upstream is at 2.2, so 2.3 is the first release with ISO15693 at all. Round 14's 2.3 section had no
change narration — its two "now"s are "the UID the card now answers to". 08 adds eight phrasings in
five bullets: "now does"; "now gets it, and is no longer reported as having refused them"; "used to
write the file's blocks 56/57 into the UID registers … now ends with"; "A clone now carries notes";
"now reports plain success … every such clone was reported as Partial … are now a clean success".
And "with only the first a wipe zeroed the whole card" describes a dev-only intermediate: 02 never
ships the flag without the read-back. **Restate each as end-state behaviour**; the history already
has a home in message 08's WHAT IS NEW / WHAT IS CORRECTED.

## F3 — MED — "a source below block 57 has none of 56/57/62/63" is off by one (classes 5 and 1)

A 57-block source (blocks 0..56) stops below block 57 and holds block 56. The code is right — "the
first deduction needs a 57-block source". Wrong as naturally read: `iso15693_poller.c:1205`,
`iso15693_poller.c:1774`, `nfc_magic_scene_iso15693_write_fail.c:349`,
`nfc_magic_scene_iso15693_partial_details.c:154`, `CHANGELOG.md:129`, `reply.md:211`. Right only as
a count: `iso15693_poller.h:211`. All new this round (03, 06, 08); none on the PR. **State the
property: "a source that ends before block 56".** The same count-against-index confusion 06 fixed
in the size note.

## F4 — MED — reply: "Before this, a wipe zeroed all 64 blocks of a TI card …" (question 1)

The PR as pushed never sets OPTION, so on his version a TI wipe cleared nothing. The zeroed-but-
reported-nothing result is the dev build with the flag and no read-back, which he has never had.
Message 02 states it correctly, as a conditional: "With the flag and without the read-back, a wipe
zeroes all 64 blocks …". Use that.

## F5 — MED — 08's tree contradicts itself on the addressing evidence (classes 1 and 2)

`iso15693_poller.c:31-36`: "Measured on five CARDS … every one answers NOTHING to a UID one byte
wrong". `CHANGELOG.md` in the same tree: seven cards, read back, "filtered rather than merely
unanswered". The read-back result reached the notes at 08 and not the comment. Update the comment,
or cut its evidence down to the constraint.

## F6 — MED — the #255 comment the reply promises twice has no draft, and needs two corrections

#255's body carries the "already armed" scoping AND "the registers latch on the next power-up" —
the premise of "nor does write ordering help" — which the round measured false on all three gen1
chips. Draft it with both, and gate it, before the round is posted.

## F7 — LOW-MED — `iso15693_poller.c:1110`: "… and so ARMS the card" (class 6)

The define 980 lines up says what may be claimed for those frames "is nothing". Introduced at 05;
the arm sweep at 07 fixed six sites and missed this one. Fork message 05 already has it right:
"unlock and commit included".

## F8 — LOW-MED — reply's gen1 paragraph: "so a silence is a refusal and not an absence" (class 12)

The bracket excludes absence only. At block 56 on `SL2S5302` the one-byte-wrong frame carried
`AABBCCDD` into a block that already held it, so a landed write reads back identically to a refused
one. The claim is still true — the block-8 enforcement bench read `SL2S5302` back with distinct
data — but the reason given is the weaker control. Four paragraphs earlier the same reply says
"Silence alone would not have shown that", and message 07 states it correctly ("to show the card
was there throughout"). Say "not an absence" and leave the refusal to the read-back result.

## F9 — LOW-MED — reply's bench list: "… controls, to show nothing regressed" (class 12)

Both ran on 2026-09-24 on the round-14 build, before the first addressed commit existed. They
establish the premise "nothing here requires an addressed write", not the absence of a regression.
"The gen2 card" is also ambiguous now that there are four: it is `gen-2-card`.

## F10 — LOW — reply: "Nothing here requires an addressed write. Five cards …" (class 15)

The five exist (`white-coin`, `slix-1k-50mm`, `slix-1k-50x28`, `lri2k-keychain`, `SL2S5302`), but
the count leaves out `gen-2-card`, which WAS measured, and "nothing here" covers `black-tag` and the
v2 sticker, which have no recorded unaddressed WRITE BLOCK.

## F11 — LOW — reply: "the release-notes trim … are all in" (questions 4 and 6)

He was told 178 lines was the problem; the trim took it to 112; the PR holds 120; this push makes it
165. The trim happened and is two-thirds undone. Say so, or trim.

## F12 — LOW — squash message: "five cards over four identified chips: all five … enforce it"

Behind the CHANGELOG's seven, and it drops the two unidentified cards (class 15). Its TIMING
paragraph still waits on "the gen1 B-round plus the C/D passes", which are done.

## F13 — LOW — message 08's last paragraph narrates dev history he never saw (class 3)

"three bullets for what is one fact to a user, then merged; a bullet reworded twice" happened only
in dev history, and "pure cost to him" is WRITING-RULES pasted into his commit message in the third
person. `check-writing.py forkmsg` has no narration rule, so no gate sees it. What he can see is
that only 08 touches the CHANGELOG; the reason fits in a sentence.

## F14 — LOW — the gen2 bullet ends with the #251 inventory caveat (question 3)

"With two tags present the post-wipe UID re-read …" and "Keep one tag in the field" were round 14's
own bullet; now they sit under a heading about the gen2 backdoor, and that is the advice's only home.

## F15 — LOW — "Narrower than the gen1 hazard" (poller.c:451, message 08, reply)

It compares against the gen1 bystander hazard that 07 closed. In the tree, gen1 is addressed.

## Dev-only, low

- NEXT-SESSION's live head: "It has NOT been on hardware" beside the section saying it benched and
  passed; item 4 still says seven sync points, thirteen shipped commits, "06 collapses seven",
  "16 of the 30".
- fork README: "reordered, twice" describes one reorder; "the arm correction is comment and release
  notes only" — its notes half is in 08 now.
- `addressed-writes-measured.md`'s title says "measured on ONE chip" over a running total of seven
  cards; the record says transcripts are in `enforce-<card>.txt`, and the v2 sticker's is in its
  bench file.
- The chat summary's "79 orphaned SHAs" does not reproduce: `--all` finds 45 distinct unreachable
  SHAs, cited on 53 lines in rounds 5-14.

## Verified and not findings

Every reply claim about what he was told is on record: TI "refuses unaddressed WRITE BLOCK"
(5652071967, 5824709850); the six-item list with the squash message sixth and "it has to describe
the final state" (5652071967); the harness's size reason (5641879307); "left armed by an earlier UID
write", "advertises the same chip" and "0xE0 … a conforming tag should reject it" are all in the
pushed tree. GitHub attributes proxmark's V3 support (`2362cabcd`) to 0x6r1an0y. The SDK claims —
hardcoded write flags, the internal response parser, one EOF (`0x04`) per frame — hold against the
firmware source. The TI identification of `white-coin` and `black-tag` is behavioural (`0x03` to
OPTION clear), per `628d3fc`.
