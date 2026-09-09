NFC Magic ISO15693: #255 owns the gen1 hazard argument

The OPEN QUESTION was 27 lines making a case that now has a filed home. #255
carries the mechanism, the gen3 sibling, the proxmark citations, the reason no
pre-flight check exists for gen1, and that neither case has been seen on
hardware. Repeating all of it at the loop means the two drift apart, and the
issue is the one a reader can subscribe to.

What stays is what belongs at the code:

  - the code fact, that this loop zeroes 56/57 before it reaches 62/63
  - "Do NOT try to de-arm by pre-writing the commit block", in full, with its
    reason. This is the one comment here that stops a plausible-looking fix,
    and a reader who deletes it writes the arm sequence one step deeper.
  - that the conclusion holds even if the unlock/commit reading is wrong,
    which is what makes the constraint safe to trust
  - the mitigation and where it lives: Iso15693WriteStateVerifyWipe, behind the
    power-cycle a gen1 latch needs to be visible at all

Dropped "nobody on this PR has one", which is about the PR rather than the
code, and is in #255 where it stays true after the PR closes.

Comment -10, code +0. 101 host tests before and after.

