# Fork commit messages for the round-7 replay — PHASE 1 OUTPUT. Nothing committed or pushed.

One file per fork commit, in replay order. `NN-<dev sha>.msg` — the dev sha names the commit to
sync from; the file is the message to commit on the fork.

## Phase 2 — DONE 2026-09-10, NOT PUSHED

`tools/replay-to-fork.sh .notes/pr-round-7/fork-messages ../all-the-plugins` did it. 19 commits on
the fork, and every gate green:

- fork tree **== a single full sync from dev HEAD** — the per-commit replay lost nothing
- **`origin/nfc-magic-iso15693` is still an ancestor** — the push will fast-forward, not force
- **19 of 19 signed**
- **his work intact**: key_cache refs 5 / 3 / 2 / 8 in `nfc_magic_scanner.c`, `nfc_magic_app.c`,
  `nfc_magic_app_i.h`, `mf_classic_dict_attack.c`; `mfc_key_cache.c` present at 5523 bytes;
  `fap_version` still 2.3
- **none of the 19 touches a file of his** — checked by name across the whole range
- the PACK builds, which is what he actually compiles: `applications_user/nfc_magic` ->
  `all-the-plugins/base_pack/nfc_magic`, `./fbt fap_nfc_magic` on Unleashed, **170,404 bytes, zero
  app warnings**. Symlink removed afterwards; it was a one-off gate.

Awaiting the go-ahead to push. Nothing posted.

## How phase 2 ran, for next round

Prerequisites, both already verified 2026-09-09:
- fork HEAD **== `origin/nfc-magic-iso15693`** (`d31f5162`). Never replay onto an older base — that
  turns the next push into a force-push over live review threads.
- **no deletions or renames** this round, only three additions, so no manual `git rm` is needed.
  The check is `git diff --name-status -M 04d5f8a..HEAD -- magic scenes views helpers assets '*.c' '*.h' CHANGELOG.md | grep -v '^M'`.

Then per file, in order: `SYNC_SRC=<dev sha> tools/sync-to-fork.sh ../all-the-plugins` from the dev
repo, then commit in the fork with that message. Finish by checking the fork tree matches a single
full sync from dev HEAD, and that `origin/nfc-magic-iso15693` is still an ancestor.

The fork's working tree currently holds the output of a full sync — regenerable, discard it first.

## What is NOT replayed, and why

- **`notes:` commits** — never reach the fork by convention.
- **12 `tools:` + 1 `hosttest:`** — touch nothing in the pack (`magic scenes views helpers assets
  nfc_magic_app.c nfc_magic_app.h nfc_magic_app_i.h CHANGELOG.md application.fam`), so they would
  produce empty commits.
- **`sync: mishamyte's upstream merge`** — the fork already has that content, so it replays as a
  verified no-op. That is what putting it at the BASE of the round bought: his 538 lines never
  appear in our series, in either direction.

## Phase 1 adaptations, per the conventions at NEXT-SESSION.md:532

- **Prefix STRIPPED, not stacked.** `iso15693:` / `changelog:` / `scenes:` removed, then
  `NFC Magic ISO15693: ` prefixed. Stacking it is what left 17 commits in August reading
  `NFC Magic ISO15693: iso15693: ...`.
- **Six subjects trimmed to 72 columns.** Worth being honest that **72 is not the house norm** — the
  pre-round-7 fork subjects run up to **121 characters**, so nothing required this. Shorter reads better
  in a review pane and the trims are pushed and fine, but do not carry "72" forward as a rule.
- **Seven bodies moved to second person** — the reviewer reads these, so "he is right" became
  "you are right". `19-55b17ae.msg` keeps its third person deliberately: that "his authority" is
  @0x6r1an0y, who implemented proxmark's V3 path, not the reviewer.
- **Two `tools/` and `hosttest` passages removed** — those files are not in the pack, so a message
  describing them describes something the reader cannot see.
- **Three dev SHAs replaced.** A dev sha resolves for nobody on the fork. `8c9f9b3` had a fork
  counterpart and became `ddd47988`; `27d939b` is commit 11 of this very replay so no fork sha
  exists yet and it is now "the previous commit"; `dbc9943` predates the round entirely, so the hash
  bought nothing and the sentence names the round instead.

Verified with `python3 tools/check-drafts.py .notes/pr-round-7/fork-messages/*.msg` — clean.
