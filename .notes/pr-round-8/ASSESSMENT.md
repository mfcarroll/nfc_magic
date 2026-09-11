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
