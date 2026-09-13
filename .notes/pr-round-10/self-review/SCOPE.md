# Review scope — PR #250 (xMasterX/all-the-plugins), ISO15693 support for NFC Magic

## READ-ONLY. DO NOT EDIT ANY FILE.
This is code under active external review (9 rounds so far). A bug you find is a FINDING TO
REPORT with file:line and a concrete failure scenario — never a licence to change the diff the
reviewer is reading. Do not run `git commit`, do not edit, do not run formatters.

## Where the code is
Live working tree: `/Users/Shared/code/personal/rfid/nfc_magic_dev` (branch `iso15693-dev`, clean).
This is the source of truth. The PR lives in a fork of `all-the-plugins` under
`base_pack/nfc_magic/`; a sync script overlays dev -> fork. Content is identical except the icon
header name. **Review the dev repo.**

Precomputed final-state PR diff (base = PR merge-base, head = dev HEAD incl. 15 unsynced commits):
`/Users/Shared/code/personal/rfid/nfc_magic_dev/.notes/pr-round-10/self-review/pr-final-full.diff`  (27 files, +3922 -45)

## The 27 files in PR scope (dev-repo paths)
NEW (review whole file):
  magic/protocols/iso15693/iso15693_poller.c      (1716 lines)  <- the core
  magic/protocols/iso15693/iso15693_poller.h      (295)
  magic/protocols/iso15693/iso15693_info.c        (296)
  magic/protocols/iso15693/iso15693_info.h        (31)
  scenes/nfc_magic_scene_iso15693.c               (83)
  scenes/nfc_magic_scene_iso15693_gen1_optin.c    (106)
  scenes/nfc_magic_scene_iso15693_get_info.c      (99)
  scenes/nfc_magic_scene_iso15693_info.c          (94)
  scenes/nfc_magic_scene_iso15693_partial_details.c (179)
  scenes/nfc_magic_scene_iso15693_write_fail.c    (498)
  scenes/nfc_magic_scene_iso15693_write_input.c   (54)
  scenes/nfc_magic_scene_partial_details_common.c/.h (26/21)
MODIFIED (review the hunks, read surrounding context):
  CHANGELOG.md, application.fam, magic/nfc_magic_scanner.c,
  magic/protocols/nfc_magic_protocols.c/.h, nfc_magic_app.c, nfc_magic_app_i.h,
  scenes/nfc_magic_scene_config.h, scenes/nfc_magic_scene_file_select.c,
  scenes/nfc_magic_scene_gen2_wipe_partial_details.c, scenes/nfc_magic_scene_magic_info.c,
  scenes/nfc_magic_scene_uscuid_ul_partial_details.c, scenes/nfc_magic_scene_write.c (602),
  scenes/nfc_magic_scene_write_confirm.c

OUT OF SCOPE (dev-repo-only, never reaches the PR): `.notes/`, `tools/`, `.vscode/`,
`ONBOARDING.md`. EXCEPT: `tools/hosttest/` is the host test harness — in scope for a
test-coverage review only, but note it does NOT ship in this PR.

## Domain facts worth not re-deriving (verified on hardware)
- ISO15693 magic cards come in "gen1" (backdoor register writes to blocks 56/57/62/63 change the
  UID) and "gen2" (a CFG frame). Gen1 is destructive and sits behind an explicit opt-in screen.
- Two UID address spaces exist; gen1's is the block space, gen2's is a register space.
- Capacity is detected by RETRY/WRITE, not by read: a read-based measurement is a LOWER BOUND.
  Blocks past the real capacity FAIL to read (they do not read as zero).
- Cards can advertise a block count different from their physical geometry (a card physically 64
  blocks can advertise 70, or 28/56, depending on what the last clone's CFG frame programmed).
- A gen1 card that has been "armed" (registers written) latches its new UID IMMEDIATELY — there is
  no power-up latch. A wipe zeroes blocks 56/57, which destroys the UID entirely (all zeros is not
  a valid ISO15693 UID, which must start 0xE0). The app mitigates by re-reading the UID after a
  wipe. No blind write ordering can de-arm the card.
- The backdoor registers may accept writes WITHOUT acknowledging, so a refused/no-ACK write does
  NOT mean the write did not land. 62/63 answer with error 0x10 on the observed card.
- `ISO15693_POLLER_PASS_MAX_MS` is a wall-clock budget, not a position — a consistently slow card
  is cut in the same place every retry; only a transient clears on a retry.
- Back-button handling: ISO15693 is the ONLY poller here that can emit a terminal event, so
  "Back-swallowing" during a pass is an ISO15693-specific concern.

## Review history — the recurring defect classes (weight these)
Rounds 6-9 found, repeatedly:
1. **Comment claims contradicted by the code** — the single largest class. Round 9: nine of eleven
   findings were defects that round 8's OWN fixes introduced.
2. **"The correction overshot"** — an unconditional claim replaced by a narrower one that is false
   at the edge.
3. **"Fixed one copy, left the twin"** — the same fact stated in N places; the fix landed in one.
4. **Stale cross-references** — a comment pointing at a line/function/commit that moved or no
   longer says what is claimed.
5. **Numbers in comments that no longer match** ("sixteen fields short", "three WRITE BLOCKs",
   block counts, line budgets, y-coordinates) — figures that outlived what anchored them.
6. **User-facing text (CHANGELOG, screen strings) making claims the code does not honour.**

## What the maintainer has said
- He has NEVER asked for less comment — only for ACCURATE comment. Volume is not the objection.
- His last five reviews are `COMMENTED`, nothing blocking. The standing `CHANGES_REQUESTED` badge
  is stale (2026-08-16).
- He explicitly asked for this self-review pass ("/pr-review-toolkit:review-pr and /simplify").

## Output contract
Report findings as a ranked list, most severe first. For each: `file:line`, one-sentence defect
statement, and a CONCRETE failure scenario (inputs/state -> wrong output, or: comment says X, code
at line N does Y). Separate CONFIRMED (you traced it in the code) from PLAUSIBLE (you suspect but
could not verify). Say explicitly if you found nothing in an area — a clean result is a real signal
here. Do not pad with praise; a short "Strengths" section at most.
