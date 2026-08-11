# Round 3 — state as of the end of Passes A and B

Read [PLAN.md](PLAN.md) for the analysis and [received/](received/) for his review verbatim.

## Where things are

- Dev `iso15693-dev`: **16 code commits** on `a3e13a3a`'s content, plus `.notes` commits. Clean.
- Fork `nfc-magic-iso15693`: **NOT re-synced yet** — still at `a3e13a3a` (round 2). Do the replay before
  drafting reply SHAs.
- Builds clean at Momentum 87.15 and Unleashed 88.2, zero warnings, clang-format applied.
- Nothing posted for this round.

## Coverage against his 24 inline comments

All addressed except three he explicitly left to the simplification pass.

| his comment | commit |
|---|---|
| `:859` **blocking**, tail-drop vs floor | `940e62f` |
| `write_fail.c:344` details gate (asked for now) | `727036c` |
| `:978`, `write_fail.c:126`, `write_fail.c:196` truncation on every screen | `3e240a6` |
| `:1125`, `:1194`, `write_fail.c:307` uid_verified + retry | `8f42d79` |
| `:532` clone data-pass clock | `e6688cf` |
| `:62`, `:137`, `write_fail.c:70` the three figures | `c036f29` |
| `:260`, `:695`, `poller.h:24`, `:209`, `app_i.h:129` | `ddcc3f6` + `78ea645` |
| `:853` tail-drop premise, `:866` "every exit" | `faa81f2` |
| `CHANGELOG:39` | `4714d3b` |
| `scene_write.c:519` Back comment's wrong claims | `3818a42` |
| `:741` three copies of the rule (asked for now) | `cce73be` |
| **`:711` / `:799`** hoist ticks, name the off-by-one | **deferred, Pass C** |
| **`scene_write.c:396`** two locals | **deferred, Pass C** |

Plus, not from his inline list: truncation became `Partial` rather than a qualified Success
(`78ea645`), and the round-1 leftovers — capacity claim on evidence (`a2fb582`), block-size clamp and
re-probe by content (`88dd51a`), four remaining drift claims (`73e3c3a`).

Two bugs of ours caught by hardware during the round: the off-by-one in the attempted count
(`14ff079`), and the card-lost exits skipping the timing line (`faa81f2`).

## Verified on hardware

- 70/64 clone → Partial, 6 blocks named, "Card too small" — correct, the card genuinely is too small.
- 70/64 wipe → "Wipe complete / Cleared 64 blocks. / Card claims 70.", no spurious UID caveat.
- Clone with the card lifted → "Card removed before the write could finish."
- Wipe with the card lifted → timing line now emitted on **both** card-lost exits (above and below the
  advertised count), attempted − cleared == 8 on all runs.

## NOT verified — reasoned only

Needs a card that refuses a write while still answering a read, which nobody has:

- the blocking fix's positive case (a dead stretch inside the claimed range)
- every truncated-sweep screen, and truncation reporting as Partial
- `uid_verified` false
- the capacity gate's discriminating case (a failed block that answers a read)
- a clock-cut clone with the card still present

Say this plainly in the reply. See [../test-bench-idea.md](../test-bench-idea.md).

## Next

1. Re-sync the fork (one fork commit per dev commit, `NFC Magic ISO15693: <subject>`).
2. Draft the reply + thread replies into `.notes/pr-round-4/`, same shape as round 2:
   `pr-reply-body.md`, `pr-inline/<comment-id>.md`, `check-shas.py`, `assemble-review.py`,
   `post-pr-replies.sh`. Thread ids are in `received/inline.json`.
3. Push, then post.
4. Pass C: the two deferred simplifications above, plus the round-1 queue — the duplicated retry loop
   (he called it non-optional), the `{reason, title, body}` render table, the duplicated confirm scene,
   and the comment cut. File is ~42% comment against ~10% for the other pollers.

## Conventions carried forward

Same three as round 2 (see `../pr-round-2/README.md`), plus: **don't leak process into artifacts that
describe the present** — no "no longer" in a CHANGELOG for an unshipped feature, no "this used to be
duplicated" in a comment. The rule belongs in the comment, the incident in the commit message.
