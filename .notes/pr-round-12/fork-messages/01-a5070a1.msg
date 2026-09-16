NFC Magic ISO15693: the clock cut is decided in one place, with both halves of the reason

The pass-cut was written out twice, once in the clone's write loop and once in
the wipe's sweep, and each copy carried half of why it is written the way it is.

The clone site said why the cut is recorded on the INSTANCE rather than returned
into a local: held local, it stopped the poller fabricating "Card too small" and
let every screen fabricate the same claim per block instead -- each unreached
index named under "Blocks not written", and Finish rather than Retry, about
blocks the card refused none of. The wipe site said why the comparison is
elapsed-against-budget, which is what survives a tick wraparound, and why it is
asked before the block is attempted, which is what keeps the recorded block the
exclusive end of the attempted range. Neither site said both.

iso15693_poller_pass_expired now decides and records it, with both reasons in
one place. The log lines stay at the call sites: they are the one thing that
genuinely differs. pass_truncated and pass_cut_block gate Retry in seven places
and now have a single write point outside the reset.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
