NFC Magic ISO15693: the gen1 loss claim is made only where there was a loss

A gen1 clone of a 28-block source onto a 28-block card reported:

    Clone partial / Cloned 28/28 blocks / Not written: 0
    gen1: 56/57/62/63 differ

Every block took, nothing failed, and the screen still said four blocks differ
from the source -- and called the run Partial for it. A 28-block file has no
blocks 56/57/62/63, so there is nothing for them to differ FROM. A file that
reaches them with nothing stored there has lost nothing either.

Nor can it be a claim about the CARD. Measured on all three gen1 chips here,
those four addresses answer no read at any point, so they are registers outside
the memory map rather than blocks holding something to displace. Only SOURCE
data at those indices can be lost.

The same fact was already derived correctly twenty lines away. The block-count
deduction gates on whether each backdoor block is below the source count, which
is why "Cloned 28/28" was right: nothing was deducted. Both result screens asked
whether gen1 had been used instead, and one carried a comment asserting the
opposite -- "Unconditional: those four blocks differ from the source whatever
the write results above say." So did the comment where the gen1 fallback records
itself, and it now says which clones are Partial. The comment above the
deduction had the boundary wrong too, saying two of the three chips measured sit
below it; all three do, and it now states the boundary instead: nothing is
deducted from a source that ends before block 56.

One fact, two derivations, disagreeing. So the poller records two flags, both
from the SAME count that produces blocks_total: gen1_blocks_skipped, that the
file reached those addresses, and gen1_data_lost, that it held data in one of
them. The deduction and the Details note follow the first. The outcome, the
"differ" wording and whether Details is offered for it follow the second. The
scenes could not have derived either themselves, which is why this is a poller
change rather than a comment fix -- on a small source the deduction is a no-op,
so blocks_total looks identical either way.

What the user sees: a gen1 clone that lost nothing -- its file ending before
block 56, or reaching those blocks with nothing there -- and has nothing else to
report reaches the plain success screen, with no counts and no mention of
56/57/62/63. They opted into gen1 and gen1 worked; the registers are not their
problem. A run that is Partial for some other reason leaves them off its
summary too, and its Details notes them only where the file reached them. Where
the file DID hold data there nothing changes -- still Partial, still the
"differ" wording, and Details still names what is missing.

The Write-UID asymmetry is unchanged: a bare Write-UID has no source data to
disturb, so gen1 there is a clean success either way, and the clone-only guard
that holds that is untouched.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
