NFC Magic ISO15693: eight claim-accuracy corrections from Round 7

All eight are prose contradicted by the code beside it. Threads 3948188938,
3948188955, 3948188969, 3948188977, 3948188987, 3948189002, 3948189009,
3948189016.

CHANGELOG (the user-facing one) said a clone or Write UID "reports 'not a magic
tag'" on a gen3 card. It does not: gen3 ignores the gen2 backdoor, so the UID
reads back unchanged, which is NotGen2 and lands on the "Not gen2 magic card"
opt-in screen. Our own enum doc had it right at app_i.h:107-109. Added the
omission you asked for too, and it is the sharper half: accepting that opt-in
sends four ordinary WRITE BLOCKs into 56/57/62/63, so the CLONE path can damage
a gen3 card, not only the wipe.

app_i.h:140 carried the same "before any truncation reporting" overreach as
write_fail.c, fixed the same way -- it is the WipeStopped reporting that is
skipped, and that screen states the cut itself.

poller.c:180 credited the wrong pass. "The sweep reads on every refused block
too, not only the empty ones" is not a property the wipe could ever have had:
its read is what DETERMINES emptiness. The pass that changed is the CLONE, which
this file documents at write_source_blocks. Last round's wording had it the right
way round and the rewrite moved the attribute.

poller.c:184 claimed the wipe pays more "on the same geometry". Both of its extra
costs are driven by absences, and absent_run increments only on the read-failure
path -- so on the very card the bound exists for, which refuses every write and
answers a read everywhere, it accumulates ZERO absences and the two passes cost
the same. Also: the re-probe fires INSIDE the loop, not after it; what runs after
the loop is the tail-drop.

write_fail.c:167 described only the below-the-claim case. Contradicted eleven
lines down by text this round added, and by a whole branch in partial_details.c
printing "Every claimed block was attempted."

write_fail.c:241 listed three qualifiers where there are four, and the omitted
one ranks FIRST -- pass_truncated, which the line below says outranks the rest.
"A cut sweep is not among them" is true of a cut WIPE and false of the cut CLONE
that is the only cut run reaching this branch.

poller.c:511 pointed the proximity argument at the wrong neighbours. What sits
three and five lines from the clears is clone_failed_count++, not a set; the
nearest set is 69 and 133 lines away. The real hazard is that each clear is in an
two-way decision about a block's CONTENT, whose other arm counts the failure and
leaves the provisional bit standing -- so a slip is mark_failed where
unmark_failed belongs. Described that instead.

Your cost-comparison thread had a third point this also carries: the card-present
inventory is gated on !claimed_range_attempted, so it runs only BELOW the
advertised count -- which strengthens your argument rather than qualifying it,
since the card the bound exists for sits above the claim by definition.

write_fail.c:177 and partial_details.c:81 gave two disagreeing descriptions of
blocks_total, neither exact. highest_present + 1 is the SIZE of the range to the
highest proven block, not a tally of proven blocks, because interior absences
fold in. Both now defer to the header's unqualified "a COUNT", which is the
description that survives.

107 host tests unchanged; all comment-only except the CHANGELOG prose.

