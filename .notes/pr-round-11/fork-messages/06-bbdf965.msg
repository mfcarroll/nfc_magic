NFC Magic ISO15693: the sweep's reach depends on reads, not on the card's claim

The release notes described how far a wipe sweep reaches with a threshold on the
advertised block count. No threshold is general. absent_run increments only where
the write and the read both failed, so a card that answers a read at every
address never accumulates a run of silence and walks past 56/57 whatever it
claims -- and that is the card the limit is about.

The bullet now states the rule the poller states and names that card, instead
of giving a number that holds for one shape of card.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
