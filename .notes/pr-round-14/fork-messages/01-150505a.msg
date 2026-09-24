NFC Magic ISO15693: the clock cut is decided in one place

The pass-cut was written out twice, once in the clone's write loop and once in
the wipe's sweep, and each copy carried half of why it is written that way.

The clone site said why the cut is recorded on the INSTANCE rather than returned
into a local: held local, it stopped the poller fabricating "Card too small" and
let every screen fabricate the same claim per block instead. The wipe site said
why the comparison is elapsed-against-budget, which survives a tick wraparound,
and why it is asked before the block is attempted. Neither said both.

iso15693_poller_cut_pass_if_expired decides and records it, and is named as an
imperative because it WRITES. pass_cut_block is the EXCLUSIVE END of the range
the caller attempted, which the clone's back-fill and the wipe's tail-drop both
read -- an exclusive end, not a count, because the clone `continue`s over the
four backdoor blocks on a gen1 run without attempting them, so its cursor runs
ahead of what it tried. `done` is the clone's count.

The budget stays a parameter rather than being converted inside, because #253
contemplates a longer one for the clone alone and that should remain a change of
one argument.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
