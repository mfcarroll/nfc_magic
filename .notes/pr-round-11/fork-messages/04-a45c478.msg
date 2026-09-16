NFC Magic ISO15693: the no-latch result covers three chips, not gen1 silicon

The header stated the absence of a power-up latch as a property of gen1 silicon.
It rested on a same-session inventory run on one armed ST LRi2K, and the
three-chip set/read-back/restore result cannot support it: that sequence runs
through a power-cycle, so it is silent on latching.

The measurement now covers all three. One card of each chip -- ST LRi2K, NXP
ICODE SLIX-S at IC ref 0x02, NXP ICODE SLIX at IC ref 0x01 -- with the same
frames, the same UID and the field held up. Every one returns the newly written
UID with no power-cycle, and both added cards restored byte-identically.

One claim is deliberately narrower than the other. On the LRi2K the 62/63
refusal is in band, error 0x10, block not available. On the other two the client
reported a failure that does not distinguish an error frame from silence, so the
refusal is three chips and "the registers answer" is one. The comment separates
them.

The power-cycle before each read-back stays regardless: it re-activates the card
for a clean read, and a gen2 UID lives in a register space still untested.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
