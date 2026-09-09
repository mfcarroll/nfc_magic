NFC Magic ISO15693: two claims the cut invented that were not quite true

A verification pass over the cut's own prose -- the 214 comment lines it added
that are still in the tree, after Round 7 superseded ten of them. Two findings,
and both are claims the cut WROTE rather than relocated.

"get_result copies all of it into an Iso15693PollerResult, one assignment per
field" -- 18 of the 19 fields in the span it claims. progress_step is in there
and is never reported: it is internal bookkeeping for the progress bands. A
sweeping claim with one counterexample, which is the same shape as the four
Round 7 caught. Narrowed to what is true.

"two paths return from Start before the send" -- four NfcCommandStop returns
precede it. Two are the wipe path, where gen1_attempted is false anyway, so the
count is two only ON A GEN1 RUN, which is the case the comment is about. It names
both paths so a careful reader is not misled, but the count read as absolute.
Qualified.

WHAT THE PASS CONFIRMED, so these are not in doubt:

  "the one UID formatter"        the only remaining UID hex loop in the app IS
                                 iso15693_info_cat_uid itself
  "the one write both block       iso15693_3_poller_write_block has exactly one
   passes make"                   caller, and that caller has exactly two --
                                 the clone loop and the wipe sweep
  the pass_truncated trap         scene_write.c:433-434 really does mode-gate on
                                 pass_truncated && mode == Wipe, with its own
                                 comment saying why
  gen1_attempted's timing         set in start_internal, so it is already true at
                                 both early returns
  the line-budget figures         13/24/35/46, 11px, rows 52-63 were all present
                                 BEFORE the cut -- preserved, not invented
  the 2026-08-04 wipe measurement consistent with hardware-plan.md's 36-of-64

The pattern across both passes is worth recording: the cut's prose is mostly
sound where it RELOCATED already-reviewed text, and wrong where it compressed a
claim (the four Round 7 caught) or wrote a new sweeping one (these two). Moving a
verified sentence is safe; restating it in fewer words is not.

108 host tests unchanged; comment-only.

