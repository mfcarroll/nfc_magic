# Round 9 — assessment

**Received 2026-09-12T07:03:37Z**, review `5185694197`, `COMMENTED`, **11 threads**. Verbatim in
[received/](received/). Plus a process request as a separate issue comment, and mfcarroll's reply
accepting it.

He opens *"16 for 16, and three of them went past what I asked for"* and closes: *"Nothing blocking.
I have no outstanding concern about what this code does — only about what its comments claim, which
is a much better place to be than round 6."*

## The pattern, and it is ours

**Nine of the eleven are defects round 8's own fixes introduced.** He names two shapes; there are
four once they are sorted:

| shape | threads | |
|---|---|---|
| **the correction overshot** | 01, 02, 07, 11 | replaced an unconditional claim with a narrower one that is false at the edge |
| **fixed one copy, left another** | 03, 04, 06 | the replacement landed and the twin stayed |
| **a true fact went out with the false one** | 09 | the clause explaining why the branch exists |
| **code leftover** | 10 | `is_wipe`'s `\|\| iso15693_wipe` term is now dead at its only use site |
| **his wording** | 05 | round-8 phrasing of his that was too strong |

## The one to fix first, and it is worse than "overshot"

**01 / 02** — one fact in two places, as with round 8's 08/09. The round-8 fix replaced "the sweep
sent three WRITE BLOCKs each at 56 and 57" with "a card claiming under 57 blocks ends the sweep near
its own claim". **Verified in code, he is right and the new claim is false:** a card that refuses the
write but answers the read reaches `wipe_note_present` at `:956`, which sets `absent_run = 0`, then
`continue`s — so it never reaches `++absent_run` at `:971`. The run never trips and the sweep walks
to the 256 ceiling or the clock **whatever the card advertises**.

That is not merely an overcorrection. **Three comments already in the tree say the right thing** —
`poller.c:167-170`, `write_fail.c:276-278`, `poller.h:174-177` — so the round-8 fix introduced a
contradiction with the file's own existing text *while fixing a contradiction*. And in the CHANGELOG
the two halves of one bullet now contradict each other four lines apart.

## Two other confirmations

**06.** `alloc` sets three state fields, not eleven — it has since round 7's `82ca85c`, so "sixteen
fields short" has been wrong for two rounds. 27 − 3 = 24. Round 8 deleted the "28" that was the only
figure a reader could check it against. His fix is right: drop the number, the named fields are the
useful part.

**05 is his, and he says so unprompted:** *"That is twice now my own phrasing has gone into the code
and needed correcting. Worth saying plainly rather than letting it look like your error."* The fix is
correct; only the rationale needs rewording, and the reason the title must not leak is **scope**, not
vocabulary.

## The gen1 round does not collide

It touches the same three files and **none of the eleven sites**. Checked each against the working
tree with the round applied: `nothing ever clears it` still at `:1395`, `sixteen fields short` at
`:1523`, `EVERY WRITE THIS APP SENDS` at `:23`, `Every block bound in this feature` in the header. So
it neither fixes nor conflicts — round 9's fixes land on top of it.

## The process request — and what is actually available here

> do self-review, using `/pr-review-toolkit:review-pr` and `/simplify` Claude Code plugins. Provide
> the whole diff + that PR as a context.

- **`/simplify`** — available, built in. Quality only; it does not hunt for bugs.
- **`/pr-review-toolkit:review-pr`** — **NOT installed.** `ListPlugins` and `SearchPlugins` both
  return nothing for it; it is a marketplace plugin and this session's catalogue has no match. The
  built-in **`/code-review`** covers the same ground (correctness plus reuse/simplification, with
  effort levels, and a `--comment` mode that posts inline).

Either install the plugin he named or run `/code-review` + `/simplify`. **This is the check that
would have caught most of round 9** — nine of eleven are diff-visible defects in our own changes.

---

# EXECUTED — all 11 addressed, 7 commits, nothing pushed

| threads | commit | |
|---|---|---|
| 01 + 02 | `b89bd06` | the sweep reaches 56/57 unless the card is BOTH silent and small |
| 03 | `d76f0b4` | the stale clears-commit claim, 522 lines from its replacement |
| 04 | `230d5d9` | #251 is every ISO15693 write, in the release notes too |
| 05 + 10 | `d5e457b` | the confirm title's rationale is scope; `is_wipe` stops pretending |
| 06 + 11 | `a6832b7` | two figures that outlived what anchored them |
| 07 + 08 | `efe3ec0` | scope the #251 note; stop the pointer over-claiming |
| 09 | `f946180` | restore why the NothingWiped branch exists |

Verified: both firmwares **0 app warnings**, **108 host tests**, clang-format **0 violations**,
**zero intra-batch churn**. Four commits comment-only, two CHANGELOG-only, **one real code change** —
removing the dead `|| iso15693_wipe` term, whose four cases were checked to produce identical titles.

## The discipline that was missing in round 8, applied here

Nine of round 9's eleven were defects round 8's own fixes introduced. The guard is a **twin check
before each edit, not after**:

- **01/02** — grepped for other descriptions of the refuses-writes-answers-reads card FIRST and found
  **four**, then used the backstop note's own wording and pointed at it rather than inventing a fifth
  phrasing. Inventing one is exactly what produced the contradiction.
- **03** — grepped every statement about clearing commit before editing: exactly two, one already
  correct. Fixed the stale one by removing the mechanism entirely and pointing at the owner, since two
  copies of a mechanism is what let them drift.
- **09** is a different failure mode worth naming separately: **a deletion that removes a true fact
  along with a false one.** Nothing in the result looks wrong, so nothing prompts a re-check. The
  guard is to read what a sentence does BESIDES carry the false claim, before cutting the sentence.

## Handoff for the self-review session

Branch `iso15693-dev`, working tree clean. **Fifteen shipped commits are unpushed**: the gen1 B-round
(8) then these round-9 fixes (7). The PR's pushed head is `09778b6d`.

Run over **the whole PR diff**, as he asked — not just these fifteen. `/pr-review-toolkit:review-pr`
is installed on disk (2026-09-12 08:02) but was not visible to the session that built this; a fresh
session picks it up. `/simplify` is built in.

**The comment-analyzer agent is the one to weight most** — its stated job is cross-referencing every
comment claim against the code, which is precisely the defect class of the last two rounds.
