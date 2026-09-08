COMMENTED

## Round 7 - all 22 addressed, and the blocking one is properly fixed

I traced the guard rather than taking it on report. A cut clone that accepted zero blocks now
runs `success_or_partial` -> Partial -> `nfc_magic_scene_write.c:441` -> **"Clone partial /
Cloned 0/N blocks / Not written: N / Timed out at block K"**, with Retry (`is_retryable`:
Partial && pass_truncated) and Details (`has_details`: pass_truncated), and the Details list
bounded at the cut so it names only blocks the card actually refused. That is the right screen
with the right buttons, and the sentence commit `8841a28c` exists to delete is now gone from
both places it lived.

The title refactor is exact. I diffed all 12 strings against `64326417`: every render-chain
branch is a bare `reason ==` test, the switch mirrors them one for one, and `default` covers the
same `{NotMagic, CardLost}` the old final `else` did. No branch gained or lost a title, and the
z-order change in the final `else` is invisible (title at y=0, icon at y=22).

Claims I checked against code and found accurate: the 15ms figure (`WRITE_ATTEMPTS` 3 x
`VERIFY_RETRY_MS` 5), the payload clamp, one read per persistent failure in both passes, the
dead `+ over_capacity` term, "the poller never asks whether a bit is set", "a cut wipe has no
back-fill", the six reachable wipe reason codes, Classic always prompting, `WipeUidChanged`
withholding Retry while the cut still reaches Details, and `#252` -> `#253` (right: #253 is the
swallowed-Back issue). #255 exists and covers both hazards it is cited for. Build clean at API
88.4, `ufbt format` clean, no source touched.

## Two corrections I owe you

**The `<` -> `<=` change: my round-6 finding was wrong. Please revert it.** `blocks_advertised`
is a count and `cut_block` is an index, so at equality the claimed blocks are 0..N-1 and the cut
sits at index N - the first block *past* the claim. The wording I sent you away from ("at block
N, past the N this card claims. Every claimed block was attempted") was the correct one; the one
I sent you to ("at block N **of the N** this card claims") names an index that is not among the N,
and reads as a completed fraction on the one boundary where the sweep was in fact cut. I also
handed you the justification now sitting at `:96-98`, and it is a non-sequitur - block N being
unattempted is equally true of both branches. Sorry for the round trip.

**A comment in this delta cost me a false finding.** I read "the wipe short-circuits to
NothingWiped before any truncation reporting", believed it, and worked up a finding that a cut
wipe which cleared nothing loses the cut entirely. It does not - `write_fail.c:274-278` prints
"Timed out at block %u" right there. I withdrew it before posting. Flagging that because it is
the strongest argument for the fix: the sentence is wrong in the specific way that misleads
someone checking carefully, and you wrote the accurate version of the same fact 90 lines earlier
at `:181-183`. The CHANGELOG sentence I gave you last round is fine as it stands - CardLost really
is the only route that drops the cut.

## This round

Nothing blocking. Twelve claim-accuracy items, five simplifications, two long lines.

The one I would fix first is `iso15693_poller.c:41` - the `COUNT_OF` rationale is inverted, and
it is the comment whose entire job is to stop someone reverting the refactor. Widening the
element type makes a `sizeof`-bounded loop run **more** iterations than the array has, not fewer.

`CHANGELOG.md:142` is the user-facing one: a gen3 card does not report "not a magic tag", it
lands on the gen1 opt-in screen titled "Not gen2 magic card". Your own enum doc says so at
`nfc_magic_app_i.h:107-109`.

The rest are the usual pattern - a claim in the new prose that the code next to it contradicts,
including three that contradict another comment in this same delta. `iso15693_poller.h:173` is
the notable one: it re-introduces the round-6 error in the framing sentence directly above the
correction.

Held open from before: the `:752` budget thread and `PASS_MAX_MS`, both of which this round's
findings continue.

