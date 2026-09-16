NFC Magic ISO15693: the sweep's reach depends on reads, not on the card's claim

The release notes described how far a wipe sweep reaches with a threshold on the
advertised block count. No threshold is general. absent_run increments only where
the write and the read both failed, so a card that answers a read at every
address never accumulates a run of silence and walks past 56/57 whatever it
claims.

Naming that card alone would be the same mistake in the other direction, since
it is not the only one that gets there. The rule is a conjunction: a card stays
short of 56/57 only if it BOTH claims fewer than 49 blocks AND stops answering
above that claim. An armed LRi2K advertising 56, the chip the hazard was
reproduced on, fails the first conjunct outright.

So the bullet states the safe case, which is the one that can be stated
completely, instead of a number that holds for one shape of card.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
