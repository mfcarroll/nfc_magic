NFC Magic ISO15693: the post-wipe UID check has a case that never runs

Same correction as the previous commit, on the user-facing side. Two statements
described the mitigation as unconditional:

  :26  "re-reads the UID afterwards and reports a change"
  :96  adds the no-answer case -- the card did not come back, or would not
       inventory -- and says the wipe reports "UID not re-checked"

Neither covers a wipe that clears NOTHING. That path reports "Wipe failed" and
never attempts the check, so it does not even say the check was skipped. On an
armed gen1 card that is the bad case: the refused writes still went to blocks
56/57, and a tag can apply a write without answering.

So :26 now says "where it can", and the :96 bullet carries a Limit clause
naming the gap and pointing at #255, matching how the other known limits in
this section are written.

Worth being plain about what this is: a claim in shipped release notes that was
broader than the code, found by checking the notes against the code rather than
by review. It is the fourth of that kind across six rounds, which is the
argument for the ownership model this delta applies -- the mitigation is now
described in one place and qualified in the same place.

Rewrapped the Wipe bullet, which the edit left ragged.

