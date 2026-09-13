NFC Magic ISO15693: nine claims in the poller that the code beside them contradicts

Each checked against the code it describes, not against the intent behind it.

  the re-probe "both fail closed". Only one half does. A run member that answers is
  not past the card's top, and nor is anything below it. Whether it CLEARED is
  decided by CONTENT twenty lines later, and a block that answers and reads empty is
  UNMARKED. Restoring mark_failed there, as the summary invites, would manufacture
  "Not cleared: N" for blocks that were cleared.

  the progress bound, wrong in two of its three copies. step takes every value in
  [0, STEPS] and progress_step is seeded UINT8_MAX, so a pass emits STEPS + 1 events
  -- nine, not eight. Harmless against a queue of 16, but this is the comment the
  file calls a hard safety bound. The worst case is now stated as the number it is:
  11 events.

  `done` labelled the denominator. It is the numerator; `total` is the denominator,
  and the wipe's twin uses the word correctly. Swapping the arguments to match
  renders "Cloning 64 / 3".

  "what runs after the loop costs no airtime". The card-present check beside the
  tail-drop is an inventory gated on failed_count > 0, so on the card the paragraph
  is costing -- one that fails every write -- it runs.

  "only reachable when the card is still present". The back-fill runs after every
  break and the card-present check is below it. The sentence's own second half said
  so. It is "only meaningful".

  the sweep's "only when" missed a second path: the geometry guard returns before
  the first write, so 56/57 can be missed because nothing was asked rather than
  because nothing answered.

  "blocks the card claims were never attempted" is false when the cut lands ABOVE
  the advertised count -- every claimed block was attempted, and the run is still
  Partial. blocks_total's doc already carries the justification that holds.

  "clamped ONCE, here, and never reassigned" reads as a whole-run guarantee. A clone
  that fails before the data pass reports an unclamped count.

  a citation to a probe suite that lives outside this repo, so neither name resolves
  here. The measurement stands on its own now.

Also: a forward declaration sat inside write_source_blocks' doc block with no blank
line, so "Defined below, next to the inventory helper it wraps" read as the last
line of that function's contract. And three comments described history rather than
the present.

Comment-only apart from the moved declaration.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
