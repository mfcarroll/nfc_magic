# Round 8 — main reply

**Status: DRAFT. Not posted.** Numbers re-measured immediately before posting, per the rule.

~~~~
All sixteen are addressed, sixteen commits, one decision each. The regression is the first of them,
and it is not the one-token fix — detail on that thread.

## The one you would fix first is bigger than you found

Checking your finding turned up two more senders, and one of them outranks gen1:

- **`WRITE AFI` (0x27) and `WRITE DSFID` (0x29)**, from the clone's identity pass. These are
  STANDARD ISO15693 commands. A block write at index 56 needs the bystander to be at least 57 blocks
  for it to land on anything; AFI and DSFID exist on every compliant tag that reports them, so there
  is no size floor at all. A changed AFI can also drop a tag out of a selective inventory, so the
  damage is not confined to the value of one field.
- **the gen2 backdoor**, `0xE0`, which is proprietary — a conforming tag should reject it. That is
  the only one of the four with a floor under it.

Your suggested fix was "every **data-block** write" here, plus a separate mention of the gen1
sequence. I went the other way: the scope now lives on `ISO15693_MAGIC_FLAGS`, the define that makes
a frame unaddressed and the place a reader of any of the four sites already lands. This comment
shrank to two lines pointing at it. A second site growing a partial copy is how the first one got
stale.

I re-verified the SDK half rather than carrying it forward on report — `iso15693_3_poller_write_block`
appends `SUBCARRIER_1 | DATA_RATE_HI` and nothing else.

#251 gets a comment carrying the AFI/DSFID part.

## One correction, and it is in your favour

The test citation is not also wrong against my own harness.
`test_cut_at_the_claim_reads_as_past_it` is in `test_write_fail_scene.c`, with its runner call at the
bottom of the same file. Only its absence from this repository was the problem.

I swept `scenes`, `magic`, `views`, `helpers`, the root sources and the CHANGELOG for both `test_*.c`
and `hosttest`. That was the only one, so the exposure was a single line.

## The harness — yes, and as its own PR

I would rather contribute it than delete the citation, so: yes. Not in here, though, and the reason
is arithmetic — this PR is 27 files and +4,123 lines, and `tools/hosttest` is 49 files and 4,898. It
would more than double what you are reading, at round eight, in a review you take commit by commit.

I will open it separately and put the detail there. Two things worth knowing before then: it is a
**host** harness, run with `make` on a development machine rather than under `ufbt`, so it needs
somewhere to live that CI will not try to build as an app — and whether `base_pack` grows a test
directory at all is a repository-shape call that is @xMasterX's as much as yours. Nothing in this PR
depends on the answer.

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
off than before it started. Every one of those lines went in because a claim was wrong, which is the
mechanism rather than an excuse: correcting a claim means writing the sentence that makes it correct.

It is also the argument for running the deletion pass after the correctness rounds rather than
before.

## Where this goes from here

Nothing below blocks a merge and I am not asking you to hold for any of it — but you are close to
ready, so you should have it in front of you rather than discover it:

1. **This round.** Done.
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
5. **A squash message, whenever you are ready to merge.** I will post a proposed one as a comment.
   The default here would be GitHub's concatenation of 92 commit messages — 2,748 lines — so you
   would end up writing your own in the merge box, as you did on #258. Better you have a draft to cut
   than a blank box — it is the only prose from this PR that ends up in git rather than on GitHub.

The CHANGELOG is in scope for 3 and 4 as well. Its 2.3 section is 165 of the file's 344 lines for one
feature, against 50 for your 2.0 major release, 19 for 2.1 and 14 for 2.2. That is out of proportion
whatever the feature is worth.

Separately: the harness PR, and the #251 comment.

On the pattern you named: I have no standing check for it. What I run compares comments against the
code, and the duplicate-phrase scan I used during the cut was a one-off rather than a tool — so
nothing in the loop compares two comments to each other. Building that is part of step 3, since
deletion is what removes the substrate these live in.

## Verification

- Both firmwares rebuilt from a cleared object directory: stock Momentum `dev` and stock Unleashed
  `unl092-base`. Warning-free.
- `clang-format --dry-run -Werror` across every `.c`/`.h` outside `tools/`: no violations.
- 108 host tests pass.
- Zero intra-batch churn: each pair of the sixteen commits diffed at `--unified=0`, no later commit
  removes a line an earlier one added.
- Every commit touches shipped code; none is notes-only.
~~~~
