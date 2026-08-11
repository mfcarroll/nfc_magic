Twelve commits, one per decision. The blocking item is fixed with your discriminator, the three you'd
bundle are in, both simplifications you asked to take alongside them are in, and so are the nine
worth-fixing and drift items still outstanding from the round before.

Details in the threads; three things need saying here.

## Where the contract landed, which is not where you suggested

You asked the Success enum to document that a wipe Success no longer implies full coverage. I've removed
the divergence instead: **a sweep the clock cuts short now reports `Partial`.**

The poller was saying Success while the screen played the error tone, titled itself "Wipe stopped" and
offered Retry. Documenting that would have left every future consumer needing to remember the footnote
to get it right — which is exactly how one screen out of four came to read `sweep_truncated` at all.

The line I drew is whether the operation's own job is left undone. A cut sweep never attempted blocks
the card claims, so the wipe did not do what a wipe is for. The UID check failing to reach an answer is
different: every block the sweep reached is clear, the wipe finished, and only a best-effort identity
check did not run — making *that* Partial would downgrade every wipe where the user lifts the card as it
completes, which is the case you were protecting when you said not raising an error was the right call.

The contract got **shorter** as a result, which I take as the tell that it had been stated in the wrong
place.

## Your fix for the blocking item was one step short

The activation-cache discriminator is exactly right and I've used it as written. But "keep the bit and
count it" alone renders **"Wiped 0/20 blocks, not cleared: 44"**: the partial screen derives its cleared
figure by subtracting failures from the total, so 44 kept failures against a `blocks_total` of 20 breaks
the `failed_count < blocks_total` invariant you verified by hand last round.

A kept block has to update `highest_present` too. That follows from your own argument — if the cache
proves the block exists, it belongs in the count of blocks proven to exist — but it isn't in the sketch,
and the failure is silent.

## What is verified, and what is not

Hardware, on the gen2 card: the fake-flash regression the blocking fix could have broken (70 advertised
against 64 physical still reports a clean 64), a clean wipe unchanged by the truncation rework, a clone
with the card lifted, and both new card-lost exits in the sweep — above and below the advertised count —
now emitting the timing line they used to skip.

Not verified, and I'd rather say so than let it read as covered:

- the blocking fix's **positive** case — a dead stretch inside the claimed range
- every truncated-sweep screen, and truncation reporting as Partial
- `uid_verified` false
- the capacity gate's discriminating case — a failed block that answers a read
- a clock-cut clone with the card still present

All five need a card that refuses a write while still answering a read, which neither of us has. The
capacity gate is the one where that's least satisfying: on our 70-block non-empty-tail source it still
reports "Card too small", correctly — the card genuinely is too small, and the six blocks refuse reads
as well as writes. What changed is that the claim is now *earned* rather than inferred from the shape of
the failure run. The case where it would newly stay silent is the one we can't stage.

I've written up what a bench for this would take — host-side tests of the sweep's decision logic look
more tractable than tag simulation, since the arithmetic is pure over the results of two calls and every
geometry you've traced by hand becomes table-driven. Not started.

## Still to come

The simplification pass: your two remaining minor items (hoisting the tick conversion out of the sweep
loop, naming the `block + 1 >= advertised` off-by-one), plus the queue from the round before — the
duplicated retry loop you called non-optional, the `{reason, title, body}` table, and the duplicated
confirm scene. The render-branch count went up again this round, to twelve.

The comment density is the part I'm least happy with. It went 39.4% to 42.4% before I started measuring
per-commit; the last ten commits are net +73 comment against +91 code, which is the right side of the
line but not by much, against ~10% for `gen2_poller.c` and `uscuid_ul_poller.c`. The cut belongs in that
same pass, under your ownership model.

## One confirmation

Yes, deliberate: a clean ISO15693 wipe no longer auto-dismisses through the shared success popup. It
matches the over-capacity clone route, and it is the only way to show the measured range beside the
card's claim.
