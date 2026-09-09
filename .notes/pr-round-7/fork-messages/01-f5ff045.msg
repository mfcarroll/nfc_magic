NFC Magic ISO15693: the header states each fact once

The header measured 65% comment against 44% for the .c it declares -- and a
header is the worst place for a duplicated fact, because the copy sits away
from the code that would contradict it.

One owner per fact:

  Iso15693PollerEvent   the outcome contract, including CardLost covering a
                        card lifted mid-pass, which both block loops restated
  each result field     its own semantics -- blocks_advertised owns the
                        programmable-count fact, failed_bitmap owns its
                        true-index rule, and blocks_total says "a count, never
                        an index" where that claim belongs
  each start_* doc      only what is specific to that entry point: what it
                        writes, in what order, and its own ordering-safety
                        property
  ISO15693_MAGIC_BLK_*  which register is which block

Sites per fact, before -> after: 56/57/62/63 10 -> 1, "first activation" 6 -> 1,
the gen2 CFG frame 2 -> 1. Mentions of uid_verified and blocks_total barely
move, and that is the point -- they are now cross-references to one statement
rather than three statements of their own.

Two removals were not on the plan's list. start_clone_gen1 said "the gen1
registers now hold the UID, so they cannot match the source" twice inside one
comment. And the header explained why has_details(WipeStopped) is unconditional,
which the scene already says at its own has_details site; the scene owns it, so
the header keeps only its own conclusion.

Also dropped "which overreached twice" from the Success contract: that is the
history of the comment, not a fact about the code.

Comment -27, code +0. With comments stripped the declarations are
byte-identical. 101 host tests before and after.

The figures are measured rather than asserted, which Round 6 argued for: that
round was +117 comment against +28 code with every individual edit correct.

