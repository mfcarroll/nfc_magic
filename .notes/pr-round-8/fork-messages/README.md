# Fork commit messages for the round-8 replay — PHASE 1 OUTPUT. Nothing committed or pushed.

One file per fork commit, in replay order. `NN-<dev sha>.msg` — the dev sha names the commit to sync
from; the file is the message to commit on the fork.

## Prerequisites, verified

- fork HEAD **== `origin/nfc-magic-iso15693` == `dbc6e4fa`**, working tree clean. Never replay onto an
  older base: that turns the next push into a force-push over live review threads.
- **no deletions or renames** this round, only modifications, so no manual `git rm` is needed.
  `git diff --name-status -M c338092..HEAD -- magic scenes views helpers assets '*.c' '*.h' CHANGELOG.md | grep -v '^M'`
  returns nothing.

## What is NOT replayed

**Eight `notes:` commits**, by convention. Of the 24 dev commits in `c338092..HEAD`, **16 touch the
pack** and every one of them replays — checked per commit against `magic scenes views helpers assets
nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h CHANGELOG.md application.fam`, so there are no empty
commits in this round and no `tools:`-only commits either.

## Phase 1 adaptations, per the conventions at NEXT-SESSION.md

- **Prefix STRIPPED, not stacked.** `iso15693:` and `nfc_magic:` removed, then `NFC Magic ISO15693: `
  prefixed.
- **Subjects NOT trimmed.** They run 70–90 columns. Existing fork subjects reach 121, so nothing
  requires 72 and the round-7 note says so explicitly — do not re-inherit it as a rule.
- **NO SECOND PERSON, AND THIS REVERSES THE ROUND-7 CONVENTION.** That round's README said to move
  bodies from third person to second, on the reasoning that the reviewer reads them. **That is
  wrong, and the data says so:** only 11 of the 143 existing fork commits use "you" at all, seven of
  them from round 7 — i.e. ours — and the pre-existing ones are mostly quoted UI strings. The older
  norm is third person or no attribution ("Split out at mishamyte's request").

  The reason is the artifact, not the audience. **A commit message outlives the review.** It is read
  by whoever runs `git log` on `dev` in two years, and "You are right that the conclusion survives"
  is meaningless to them — it reads as half of a conversation they cannot see. Second person belongs
  in the PR reply, which is a conversation; the commit states the defect and the fix.

  So: state it flat. "Flagged in review" or "Suggested in review" where the provenance carries
  information, nothing where it does not. Dropped with it: dialogue narration ("You found it, and
  found the tell with it"), self-assessment ("which is better than a corrected count"), and
  descriptions of the diff the reader has in front of them.
- **Three host-harness passages removed** — `02`, `15` lost "108 host tests pass" and `15`'s
  verification line was rewrapped around it. `tools/hosttest` is not in the pack, so a message citing
  it describes something the reader cannot see. **`16` KEEPS its references** on purpose: that commit
  removes a citation to `test_write_fail_scene.c`, so naming the file is quoting text we deleted,
  which is the standing exception.
- **Two dev SHAs replaced with the FORK counterpart, not the subject.** `55b17ae` (in `01` and `09`)
  is the gen3-warning commit, which is `dbc6e4fa` on the fork — and that is the hash he used for it
  himself in the round-8 review, so it is the reference he will recognise.
- **The six SHAs quoted from his review are left alone** (`e71dce03`, `9c14213d`, `9520e177`,
  `44617f89`, `5bc041d4`, `235b5e0e`, `e191411b`). They are fork hashes; they were dangling in dev and
  resolve correctly here, which is the whole point of doing the substitution in this direction.

Verified: **all eight SHAs cited across the sixteen messages resolve on the fork** (checked with
`git -C ../all-the-plugins cat-file -e`), and `python3 tools/check-drafts.py
.notes/pr-round-8/fork-messages/*.msg` is clean.

## Phase 2 — DONE 2026-09-11, NOT PUSHED

`tools/replay-to-fork.sh .notes/pr-round-8/fork-messages ../all-the-plugins`. **16 commits**, and
every gate green:

- fork tree **== a single full sync from dev HEAD** — the per-commit replay lost nothing
- **`origin/nfc-magic-iso15693` is still an ancestor** — the push fast-forwards, no force
- **16 of 16 signed**
- **his work intact**: key_cache refs 5 / 3 / 2 / 8 in `magic/nfc_magic_scanner.c`,
  `nfc_magic_app.c`, `nfc_magic_app_i.h`, `scenes/nfc_magic_scene_mf_classic_dict_attack.c`;
  `helpers/mfc_key_cache.c` present at 5,523 bytes; `fap_version` still 2.3
- **none of the 16 touches a file of his** — the whole range touches seven files, all ISO15693
- the PACK builds, which is what he actually compiles: `applications_user/nfc_magic` ->
  `all-the-plugins/base_pack/nfc_magic`, `./fbt fap_nfc_magic` on Unleashed, **170,428 bytes, zero
  app warnings** (170,404 last round, so +24 for the one code change). Symlink removed afterwards.

The push would be **`dbc6e4fa..<fork HEAD>`**, a fast-forward of 16.

### One thing to carry forward

**A second-person substitution re-wraps the paragraph.** Two of these merged lines without
reflowing and left 137 and 91 columns. The norm, measured rather than assumed: existing fork message
bodies run median 73, p95 80, **max 84, and zero lines over 90** across 2,194 lines — so the 81–83s
a one-word substitution produces are inside the distribution and need no action, and anything past
~84 does. Check with:

```bash
for f in .notes/pr-round-*/fork-messages/*.msg; do awk -v F="$f" 'NR>1 && length>84 {print F": "NR" ("length")"}' "$f"; done
```
