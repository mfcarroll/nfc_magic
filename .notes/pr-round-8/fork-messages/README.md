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
- **All sixteen bodies moved to second person.** Every one referred to the reviewer in the third
  person, since the dev message is written for us. "mishamyte points out" → "You point out",
  "He is right" → "You are right", "his suggestion" → "your suggestion".
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
