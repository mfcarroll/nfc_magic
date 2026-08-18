# Round 5 follow-up comment — DRAFT for review

Replaces the prematurely-posted #issuecomment-5322865570 by editing it, once `3c88cf1` is pushed.

---

One more push, out of the hardware run the last comment promised — the local build with
`ISO15693_POLLER_PASS_MAX_MS` dropped to 200ms, which is what makes the truncation screens reachable on a
healthy card. It found two things, both ours.

**The "Wipe stopped" screen named where the sweep stopped and never said why**, while offering a Retry
button — so the user was asked to retry against a cause the screen withheld. Details had it; the summary
did not.

Folded into the line that was already there rather than added as a fourth, on your own Round 4
measurement: at `y=13` a fourth line's bottom rows land inside the button box (tops at 13/24/35/46/57
against a box at rows 52-63), and this body already reaches three lines when blocks failed. So
`Stopped at block 23.` is now `Timed out at block 23.` — one character wider than the string I watched
render, which is the only caveat on it. Same fix on the partial screen's qualifier and on the
nothing-wiped screen's third line.

**And the worse one: "Running the clone again writes them" was false.** The bound is a wall clock, not a
position, so re-running repeats the same work against the same budget — a consistently slow card is cut
in the same place every time. Only a transient, marginal coupling forcing per-block retries, clears on a
second pass. Observed rather than reasoned: a retried wipe stopped at the same block.

Same dead end this delta documents at the capacity guard, where I reasoned it through and then wrote the
opposite claim two files away.

**It applies to the wipe as much as the clone**, which is the part worth your attention. The wipe's Retry
button exists on your Round 4 argument that re-running is the correct action when data may sit above the
cut — which I accepted, extended to the clone, and never tested. On the wipe the promise was implicit in
the button, with nothing qualifying it anywhere.

One clause now covers both modes:

> Retrying may get further, but the limit is a time budget rather than a position: if it stops at the
> same block, the card is too slow to finish in one pass rather than refusing.

`is_retryable`'s comment carried the same false claim and now says re-running is the only thing that
*can* write those blocks — true, and a different statement.

**The button stays in both modes**, because the transient case is real and is why a truncation with the
card still present is reachable at all. But that transient is the whole of the argument, so if you would
rather Retry disappeared on a cut run, say so and it goes.

Also closing the offer from the last comment: that same run rendered all six screens it listed as
unreachable — Retry + Details on the cut sweep with Back as the exit, the cut-bounded block list, the
clone note, and the `uid_verified` route. The one truncation path still out of reach is the cut landing
*above* the advertised count, which needs a card that answers reads at every address.

Builds clean at Momentum 87.15 and Unleashed 88.2, 59 host tests green, `clang-format` clean.
