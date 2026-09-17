NFC Magic ISO15693: the gen3 claim was escalated and its evidence deleted together

Two facts went out of the gen1/gen3 hazard text that should have stayed.

The gen3 bullet ended up saying the gen1 opt-in writes "reach the same blocks"
as the registers a wipe bricks. They are not the same blocks. A gen3 card keeps
its UID at 0x10/0x11 and its configuration signature at 0x14/0x15; the opt-in
writes go to 56/57/62/63, which on a card that big are ordinary user data. The
wipe genuinely can brick a gen3 card, because it sweeps 16/17 and 20/21 along
with everything else. The opt-in cannot, because it never addresses them -- it
is damage, which is what the bullet said before. Those block numbers are back
too: without them nothing in the app recorded the addresses that falsify the
stronger claim.

Separately, and gone from one place to zero: the four backdoor addresses are
NOT memory. They answer no read at any point, so a card's advertised block
count is its capacity and 56/57/62/63 sit outside it. That is measured hardware
behaviour, it is why an LRi2K advertising 56 blocks reports "Wiped 58/58", and
it is what makes overshoot past the claim a question at all rather than a matter
of geometry. Restored in the poller, where both things that depend on it are.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
