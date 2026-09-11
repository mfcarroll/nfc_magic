# Round 8 — assessment and triage

**Received 2026-09-11T13:42:29Z**, review `5179298075`, state `COMMENTED`, body 4,283 chars,
**16 threads, all new** (none is a reply to ours). Verbatim in
[received/review-body.md](received/review-body.md) and [received/threads.md](received/threads.md).

He opened with "all 20 addressed, and most of them exactly" and closed with "Nothing blocking beyond
the regression, which is a one-token fix."

`reviewDecision` still reads `CHANGES_REQUESTED` — that is **stale**, from `2026-08-16`. His last four
reviews are all `COMMENTED` (08-20, 09-07, 09-11). GitHub keeps the last blocking state until it is
dismissed or superseded by an approval, so do not read it as a new objection.

## Triage — 16 threads

| # | site | class | fix |
|---|---|---|---|
| 01 | `write_confirm.c:30` | **CODE — regression** | gate the title on `iso15693_wipe`, not `is_wipe` |
| 02 | `iso15693_poller.c:494` | B, load-bearing | "every **data-block** write"; gen1 sequence needs its own mention |
| 03 | `write_fail.c:276` | A + B | contradicts `poller.c:1238-1244`; and neither string mentions the UID |
| 04 | `iso15693_poller.h:166` | B | the `<= 7` bound is at/above the claim only; re-scope to the bullet |
| 05 | `write_fail.c:180` | B (consolidation) | header `:133` states the tally reading the pointer disclaims; also `.h:255`, `.c:1080` |
| 06 | `iso15693_poller.c:260` | B | three exceptions not one; `:279` already says it right. Plus a 35-char orphan at `:262` |
| 07 | `iso15693_poller.c:855` | B | "nothing ever clears commit" — this sweep does; the argument needs the narrower claim |
| 08 | `CHANGELOG.md:105` | B, **user-facing** | no write goes out on `advertised==0`; scope to the card that answers everywhere |
| 09 | `iso15693_poller.c:1240` | B | same overstatement in source |
| 10 | `iso15693_poller.c:1236` | **A** | duplicated at `partial_details.c:134-140`; poller should own it; dispositions drifted |
| 11 | `nfc_magic_app_i.h:145` | B | "that screen" binds to the skipped screen |
| 12 | `write_confirm.c:53` | B | two warnings now, not one; "the only warning that names the cost" |
| 13 | `iso15693_poller.c:1188` | mechanical | extraction split `write_step`'s doc comment; move `:1188` to `:1211` |
| 14 | `partial_details.c:94` | scope | the test citation is dangling in-tree |
| 15 | `iso15693_poller.c:1491` | B | 27, not 28; and naming `target_uid`/`original_uid` beats any count |
| 16 | `write_fail.c:4` | mechanical ×2 | scope line overshoots; `BITMAP_SIZE * 8` spelled four times → **a decision** |

**Shape: 10 B, 2 A, 2 mechanical, 1 code, 1 scope.** Five are a comment contradicted by another comment
**in the same tree**, four of those inside the round-7 delta. He named that as "the pattern this round".

## What we owe him that he got wrong

**#14's sub-claim.** He speculates the citation "may be off against your own harness too", because the
commit names a test *function* rather than the file. It is not off:
`test_cut_at_the_claim_reads_as_past_it` is at `tools/hosttest/test_write_fail_scene.c:349`, and its
runner call is at `:543`. The file is right; only its absence from the repo is the problem.

**It is also the only such citation in shipped code.** Swept `scenes magic views helpers *.c *.h
CHANGELOG.md` for `test_*.c` and `hosttest` — exactly one hit, `partial_details.c:94`. So the cheap fix
is one line, and contributing the harness is genuinely optional rather than forced repair.

## The harness — sizing, which decides where it can go

| | files | lines |
|---|---|---|
| the PR today | 27 changed (13 added, 14 modified) | +4,123 −45 |
| `tools/hosttest` | **49** | **4,898** |

Contributing it **doubles the PR on both axes**. It cannot ride in as part of the round-8 fix delta,
and he reviews commit by commit. It is a parallel track: its own delta at minimum, more likely its own
PR against `base_pack`.

And it is a repo-shape change we do not own — this app has zero tests today and `application.fam`
excludes `tools/`. mishamyte asking for it is the reviewer half. **xMasterX has commented once on this
PR in six weeks** (2026-08-15, "@mishamyte Check latest changes pls :))"), so he has delegated and will
not weigh in unprompted. Name that risk; do not wait on it.

## Outward artifacts this round implies

- **#251 needs a comment.** Thread #02 is not a wording fix — it says the blast radius is larger than
  documented: an opt-in gen1 run sends **unaddressed** WRITE BLOCKs at 56/57/62/63, which on a bystander
  tag is four blocks of user data. `ISO15693_MAGIC_FLAGS` is `0x02`, unaddressed, per `:21` of the same
  file. Precedent is the #255 follow-up, [issue-255-followup.md](../pr-round-7/issue-255-followup.md).
- **The squash message will need re-measuring** — #02 changes what the "verified by read-back" and
  known-limits sections can claim about #251.

## The one decision hidden in the list

**#16's second half.** He proposes `ISO15693_POLLER_MAX_BLOCKS` in the header to own
`ISO15693_POLLER_BLOCK_BITMAP_SIZE * 8`, spelled out at `iso15693_poller.c:153`, `:573`, `:574` and
`partial_details.c:25`. He flags it himself as "mildly against the grain of `44617f89`" — our own
stated preference against naming values. That is a judgement call, not a correction, and it adds a
public symbol. Answer it explicitly rather than just doing it.

---

# EXECUTED — all 16 addressed, 16 commits on dev, nothing pushed

Built on `c338092`. Every commit is SHIPPED (nothing dev-only), all signed, **zero intra-batch churn**
(checked pairwise with `--unified=0`, longest-common-line overlap, 0 hits).

| thread | commit | delta |
|---|---|---|
| 01 regression | `e3f673e` the wipe title is ISO15693's | code: title moved into the `iso15693_wipe` branch |
| 02 #251 scope | `2f79f8f` every write this app sends | comment +11 −4 |
| 03 retracted UID claim | `1147320` drop the claim the poller retracted | comment −2 |
| 04 the `<= 7` bound | `c31036f` an above-the-claim fact | comment +3 |
| 06 get_result | `9cc7fcc` three exceptions, not one | comment ±0, code bytes identical |
| 07 clears commit | `6ea63e4` the argument needs ordering | comment +2 |
| 11 pronoun | `995322e` name the screen that states the cut | comment ±0 |
| 15 reset list | `f16cc7c` name the four fields, not the count | comment +5 |
| 12 only-warning | `b70f69f` say which one names the cost | comment +1 |
| 13 doc comment | `869bfcc` reunite write_step's doc comment | comment −1 |
| 10 duplication | `ee7f437` the poller owns the wiped == 0 argument | comment **−4** |
| 08+09 wipe reach | `33dbde4` only on a card that answers everywhere | CHANGELOG +6 −4, comment +3 |
| 05 blocks_total | `9bdb36e` a RANGE SIZE, and the header says so | header +9 −6, screen +4 −6 |
| 16a line budget | `872687d` covers both y values | comment +2 |
| 16b MAX_BLOCKS | `147b57d` name the block range once | **code**: new header define, 4 sites → 1 |
| 14 test citation | `7e39d81` stop citing a file this repo lacks | comment +1 |

Verified: both firmwares rebuilt from a cleared object dir and warning-free (stock
`Momentum-Firmware-slix` @ `dev`, stock `unleashed-firmware` @ `unl092-base`), **108 host tests pass**,
`clang-format --dry-run -Werror` reports **0 violations** across every `.c`/`.h` outside `tools/`.

## Two findings that go beyond what he raised

**#251 is wider than his correction.** He found the gen1 sequence bypassing `write_block_retried`.
Checking it turned up two more unaddressed senders he did not name — and one of them outranks gen1:
**WRITE AFI (`0x27`) and WRITE DSFID (`0x29`)** from the clone's identity pass are STANDARD ISO15693
commands, so unlike a block write at index 56 they do not need the bystander to be any particular size.
Any compliant tag in the field applies them, and a changed AFI can drop a tag out of a selective
inventory. The gen2 backdoor's `0xE0` is proprietary, so it is the only one of the four with a floor
under it. The scope now lives on `ISO15693_MAGIC_FLAGS`, the define that makes a frame unaddressed.

**His #14 sub-claim is wrong, in our favour.** He suspects the citation is also off against our own
harness because the commit named a test function. It is not: `test_cut_at_the_claim_reads_as_past_it`
is at `tools/hosttest/test_write_fail_scene.c:349`, runner call at `:543`.

## ⚠️ ADAPTATION REQUIRED BEFORE THESE REACH THE FORK

**`55b17ae` is a DEV sha and it is cited in two messages** — `e3f673e` and `b70f69f`. It will not
resolve for him. Replace with the subject, `iso15693: the gen3 wipe costs the card, and both warnings
now say so`, when writing `fork-messages/`.

The other seven SHAs in these messages (`e71dce03`, `9c14213d`, `9520e177`, `44617f89`, `5bc041d4`,
`235b5e0e`, `e191411b`) are FORK shas quoted from his own review. They are dangling in dev and correct
on the fork, which is where these messages get published — left deliberately, since echoing his own
reference is what makes the connection obvious to him.

## Still open after this delta

- **The harness.** Not started; it is 49 files / 4,898 lines against a PR of 27 / +4,123, so it cannot
  ride inside this delta. Ask him where he wants it.
- **#251 needs an issue comment** carrying the AFI/DSFID finding — it is new information, not a
  restatement.
- The squash message needs re-measuring: #02 changes what it can claim about #251.
