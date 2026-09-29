# #255 — the gen1-half correction (DRAFT, NOT POSTED)

Goes on issue #255 itself, after the round-15 push and reply, which both say it will. Post between
the `~~~~` markers. #255 is mfcarroll's issue; nothing here is `[👤]`, and he reads it before it goes.

It corrects the issue BODY, which still says both things below. The shipped text it must agree with
is the wipe's note in `iso15693_poller_wipe_blocks` and the Validation bullets in the 2.3 notes.

~~~~
## Correction: the gen1 hazard is wider than an armed card, and the registers do not latch

Two claims in this issue's gen1 half are wrong, both measured in the latest round of #250. The gen3 half is unchanged by those — but a gen3 card has since turned up, and what it showed under this app's paths is at the end.

**The hazard is not limited to a card left armed.** On all three gen1 chips here — ST LRi2K, NXP ICODE SLIX and NXP ICODE SLIX-S — blocks 56/57 took a write with nothing sent before it: five cards, one of them a card this app had never written, and whether any had been armed first cannot be told. So any gen1 card whose sweep reaches 56/57 can come back wearing a different UID. What bounds that is the reach rule — how far the sweep runs, given what the card claims and where it stops answering reads — not the card's history, which nobody can know. The table's "armed gen1" row should read "gen1".

**The registers do not latch on the next power-up.** A written UID takes effect immediately: an inventory in the same field session already returns it, on all three chips. The conclusion drawn from the latch still stands, for other reasons: the addressed writes to 62/63 the sweep sends are refused on all three chips, so there is nothing to clear on the way past, and 56/57 move the UID the moment they are written.

What stands: no pre-flight check is possible for gen1, and the post-wipe UID re-read remains the mitigation. Its power-cycle is not what makes the change visible, since that is immediate; it gives the read a cleanly re-activated card.

## The gen3 half: a card is now in hand

Since this issue was filed, a genuine gen3 card came in the same batch — un-finalized, its 0x14/0x15 configuration signature an exact match to proxmark's V3 config mode. Put to #250's paths: the gen2 backdoor leaves its UID unchanged (the UID lives at 0x10/0x11), so a clone or Write UID lands on the gen1 opt-in, and accepting that writes 56/57/62/63 as ordinary data — no identity move, the configuration signature untouched, restored byte-identical. That is the cost the release notes describe, now measured rather than reasoned.

The brick is deliberately not tested. Zeroing 0x14/0x15 on an un-finalized card is irreversible, and the pre-flight probe this issue asks for is the fix, not something to confirm by destroying a card. proxmark would accept cfinalize on this one, so the signature mismatch that makes the earlier variant safe does not protect it. The wipe hazard and the probe both stand.
~~~~
