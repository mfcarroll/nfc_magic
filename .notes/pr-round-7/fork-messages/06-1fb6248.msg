NFC Magic ISO15693: the scenes stop restating the poller's contract

The same failure as the private struct, one layer up. A scene branch would
open by re-explaining the flag it had just branched on -- uid_unexpected,
uid_unverifiable, gen1_attempted, used_gen1, blocks_advertised, pass_truncated
-- and only then get to what the scene actually decides, which is the wording
the user sees. The header owns the flag; the scene owns the string.

So each branch keeps its own argument and drops the preamble:

  - uid_unexpected keeps "saying it isn't magic would be exactly backwards"
  - uid_unverifiable keeps the reason it earns a screen at all: the editor
    pre-seeds from the last Info read, so this is two menu taps away rather
    than a typo
  - gen1_failed keeps the consent argument -- the user accepted the risk and
    still needs to be told it was spent, and on which blocks
  - wipe_uid_changed keeps "print what it answers to now, or the card is lost",
    and hands the gen1-armed mechanism to #255
  - partial_details keeps why the bound applies in both modes even though it
    is inert in one, and hands the two-groups mechanism to pass_truncated

Two one-line comments went entirely: both said what the string beside them
already says ("gen1: 56/57/62/63 differ").

The line-budget arithmetic now has one owner, per the plan. It was derived
twice, in two branches, and it governs all twelve bodies in the file -- so it
is stated once at the top and the two sites point at it. That is why this
commit's file header GAINS four lines.

What this commit does NOT do is fold the twelve widget_add_string_multiline_element
calls into a table. Layout is the one thing the host tests do not cover, so that
change would be unverifiable here, and it is a code change in a delta that
promised to be a comment cut.

Comment -15, code +0 across five files, each byte-identical with comments
stripped. 101 host tests before and after.

