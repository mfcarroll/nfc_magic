# Round 8 — main reply

**Status: DRAFT. Not posted.** Numbers re-measured immediately before posting, per the rule.

~~~~
All sixteen are addressed, sixteen commits, one decision each, and the regression is the first of
them.

`write_confirm.c` — you are right that it was unintended, and the commit message you quote is the
evidence: it argues the title purely as an ISO15693 scope statement. The title now lives inside the
`iso15693_wipe` branch, which is the shape the file already uses for the write-UID variant, and
`is_wipe` goes back to "Wipe card?". I did not take the one-token version: a nested ternary on the
line whose job is the protocol-neutral default is how this happened once already.

## The one you would fix first is bigger than you found

You are right that the gen1 sequence bypasses `write_block_retried`, and right that it is
unaddressed. Checking that turned up two more senders, and one of them outranks gen1:

- **`WRITE AFI` (0x27) and `WRITE DSFID` (0x29)**, from the clone's identity pass. These are
  STANDARD ISO15693 commands. A block write at index 56 needs the bystander to be at least 57 blocks
  for it to land on anything; AFI and DSFID exist on every compliant tag that reports them, so there
  is no size floor at all. A changed AFI can also drop a tag out of a selective inventory, so the
  damage is not confined to the value of one field.
- **the gen2 backdoor**, `0xE0`, which is proprietary — a conforming tag should reject it. That is
  the only one of the four with a floor under it.

One path became four, and the two you did not name are the ends of the range.

Your suggested fix was "every **data-block** write" here, plus a separate mention of the gen1
sequence. I went the other way: the scope now lives on `ISO15693_MAGIC_FLAGS`, the define that makes
a frame unaddressed and the place a reader of any of the four sites already lands. This comment
shrank to two lines pointing at it. A second site growing a partial copy is how the first one got
stale.

I re-verified the SDK half rather than carrying it forward on report — `iso15693_3_poller_write_block`
appends `SUBCARRIER_1 | DATA_RATE_HI` and nothing else.

#251 gets a comment carrying the AFI/DSFID part. It is new information about the hazard's size, not
a restatement of what is already filed.

## One correction, and it is in your favour

On the test citation you suspect it may be wrong against my own harness too, because the commit named
a test function rather than the file. It is not: `test_cut_at_the_claim_reads_as_past_it` is in
`test_write_fail_scene.c`, with its runner call at the bottom of the same file. The filename was
right; only its absence from this repository was the problem.

I swept for others — `scenes`, `magic`, `views`, `helpers`, the root sources and the CHANGELOG, for
both `test_*.c` and `hosttest`. That was the only one, so the exposure was a single line. It now says
why the derivation is written out (this boundary has been got backwards twice, by both of us) and
names nothing invisible.

## The harness — yes, and as its own PR

I would rather contribute it than delete the citation, so: yes. But it should not ride in here, and
the reason is arithmetic. This PR is 27 files and +4,123 lines. `tools/hosttest` is **49 files and
4,898 lines** — eight test files at 2,843 lines, three fakes at 940, and 33 fake SDK headers.
Folding it in more than doubles the file count and roughly doubles the diff, at round eight, in a
review you read commit by commit. That is a worse deal for you than for me.

What it is, so you can judge it before it arrives:

- Shipped code compiles **verbatim** — the test files `#include` the `.c` so file-statics are
  reachable, and firmware calls resolve to fakes via `-Ifakes`. No seam, no `#ifdef TEST`, nothing
  added to the app. `application.fam` excludes `tools/`, so none of it can ship.
- 108 cases across eight files: the wipe sweep, the clone loop, the terminal-outcome contract, the
  write state machine, the AFI/DSFID retry loop, the two result screens against fake GUI recorders,
  the write scene's routing, and the UID formatter's two policies.
- The fakes' semantics are cited to firmware source in the README, and the GUI enum lists are copied
  from the firmware rather than invented.
- It has caught real defects rather than only confirming: the clock-cut "Card too small" claim on this
  PR came from its first run over the clone loop.

Two things you should know before saying yes. It is a **host** harness — it runs on a development
machine with `make`, not on the Flipper and not under `ufbt`, so it needs somewhere to live that CI
does not try to build as an app. And whether `base_pack` grows a test directory at all is a
repository-shape decision that is @xMasterX's as much as yours. I will open it as its own PR and let
the two of you decide; nothing in this one depends on the answer.

## The volume number, measured

This round is **+118 comment lines against −93**, so net **+25**, and across the ISO15693 surface the
standing comment count went 1,236 → 1,257. The ratio did not move: 38% before, 38% after.

That is the wrong direction and I am not going to dress it up. Worse, here is the arithmetic across
all three rounds since the cut:

| | comment |
|---|---|
| the cut itself, eight commits | **−95** |
| the corrections that followed it in the same push | **+85** |
| this round | **+25** |

So the cut removed 95 lines and correctness work has since put 110 back. We are fifteen lines worse
off than before it started. That is not an argument against correcting things — every one of those
lines went in because a claim was wrong — it is the mechanism: correcting a claim means writing the
sentence that makes it correct.

Which is the argument for doing the deletion pass **after** the correctness ones rather than before,
and it is what the plan below is shaped around.

## Where this goes from here

Nothing below blocks a merge, and I am not asking you to hold for any of it. Laying it out because
you are close to ready and I would rather you know what is still coming than discover it:

1. **This round.** Done, and it is the third consecutive round that was mostly factual.
2. **A gen1 hardware round.** I now have a confirmed gen1 card — the four-frame sequence works, the
   backdoor registers accept writes **without acknowledging** (that was an inference from proxmark's
   source and is now a measurement), and the card is deliberately left armed, which reproduces the
   #255 case for the first time. Eleven sites in shipped code currently hedge something that session
   settled, so this round mostly **removes** text rather than adding it. The latch specifically stays
   an inference: every read-back sits behind a field power-cycle, so "latches on power-up" and
   "changes immediately" are still indistinguishable.
3. **A deletion pass.** Does each comment need to be there at all — no rewording, so it is verifiable
   by the code bytes being unchanged. This is where the volume actually comes down.
4. **A simplification pass.** Can the survivors be said more briefly. This one rewrites live claims,
   so it carries its own re-check rather than being safe by construction.

The CHANGELOG is in scope for 3 and 4 as well. Its 2.3 section is 165 of the file's 344 lines for one
feature, against 50 for your 2.0 major release, 19 for 2.1 and 14 for 2.2. That is out of proportion
whatever the feature is worth.

Separately: the harness PR, and the #251 comment.

Four of your sixteen are a comment contradicted by another comment a few lines away, and one is a
duplication that a commit in the same push removed three other instances of. I do not have a mechanical
check for that class yet — the existing one compares comments against code, not against each other —
and building one is part of step 3, because deletion is what removes the substrate they live in.

## Verification

- Both firmwares rebuilt from a cleared object directory: stock Momentum `dev` and stock Unleashed
  `unl092-base`. Warning-free.
- `clang-format --dry-run -Werror` across every `.c`/`.h` outside `tools/`: no violations.
- 108 host tests pass.
- Zero intra-batch churn: each pair of the sixteen commits diffed at `--unified=0`, no later commit
  removes a line an earlier one added.
- Every commit touches shipped code; none is notes-only.
~~~~
