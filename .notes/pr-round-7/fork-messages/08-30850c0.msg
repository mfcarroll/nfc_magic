NFC Magic ISO15693: give the retry-is-not-a-promise fact an owner

Found by running the duplicate-phrase detector over the surface AFTER the cut,
which is the check the whole round argues for. Three pairs left, all
pre-existing:

  - "a Write-UID has no payload to follow, so a verified UID is a clean
    Success" was in start_write_uid_gen1's doc and again at its implementation.
  - uid_changed's positive-observation rule was in its doc and again at
    Iso15693WriteStateVerifyWipe. That site now states only what the doc
    cannot: the cost of the rule, which is that a card bricked past
    inventorying goes unreported.
  - the wall-clock consequence -- a consistently slow card is cut in the same
    place every time, so only a transient clears on a retry -- was DERIVED
    independently in is_retryable and in the Details note. That one matters
    most: it came out of hardware testing (a retried wipe stopped at the same
    block), it corrected a false promise on a user-facing screen, and it had
    no owner at all. It is now part of pass_truncated, which is the flag it is
    a fact about, and both screens point there.

Comment +0, code +0. A net-zero commit in a delta about cutting, which is worth
saying plainly: pass_truncated GAINS five lines and the two screens lose five
between them. The line count is not the point and never was -- three facts went
from two homes to one, and the third of them is the kind that drifts, because
two screens were each reasoning their way to it separately.

Also left alone, deliberately, and reported rather than quietly skipped: the
detector still finds ~40 repeated phrases. They are parallel structure in the
header (the two gen1 entry points describe themselves the same way, which a
caller reading one of them needs), cross-references that name the same owner
twice, and short stretches of ordinary English like "contiguous run at the top
of the card". Cutting those would cost clarity and buy nothing.

101 -> 106 host tests unchanged by this commit.

