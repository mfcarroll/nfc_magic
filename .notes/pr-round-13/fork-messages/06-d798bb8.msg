NFC Magic ISO15693: the gen3 claim was escalated and its evidence deleted together

The gen3 bullet came to say the gen1 opt-in writes "reach the same blocks" as the
registers a wipe bricks. They are not the same blocks. A gen3 card keeps its UID
at 0x10/0x11 and its configuration signature at 0x14/0x15; the opt-in writes go
to 56/57/62/63, which on a card that big are ordinary user data. The wipe can
brick a gen3 card because it sweeps blocks 16/17 and 20/21 along with everything
else; the opt-in cannot, because it never addresses them. It is damage from the
opt-in and bricking from the wipe, and both sets are named rather than referred
to by position -- 16/17/20/21 are numerically BELOW the four just listed, so
"the registers above" could be read either way.

The same edit had removed those block numbers, so nothing in the app recorded the
addresses that falsify the stronger claim. Restored.

Separately: the four backdoor addresses answer no read at any point, which is
what lets 56/57 sit past a card's advertised count. That is measured, it is what
reconciles "Wiped 58/58" with a card advertising 56, and it is what makes
overshoot past the claim a question at all. It is recorded in the poller with
both of its users named.

It is NOT stated as "the advertised count is capacity". That would contradict the
premise of the sweep itself, the 36-of-64 measurement recorded beside it, the
clone's reason for not capping at the claim, and blocks_advertised in the header
-- all of which exist because the claim and what a card holds differ.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
