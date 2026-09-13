NFC Magic ISO15693: what the four gen1 backdoor registers do, measured

The gen1 sequence writes 62, 63, 56 and 57. What each does was read out of
proxmark's send order, and the register block said so with more confidence than that
deserved. It now separates what a card on the bench established from what is still
an inference.

MEASURED: the sequence sets the UID, it reads back, and the original restores
byte-identically. The UID changes immediately -- an inventory in the same field
session as the write already returns the new one -- so there is no power-up latch on
this chip. On a card that is already armed, writes to 62/63 are refused with error
0x10 while 56/57 are accepted, so the UID moves on 56/57 alone and the refusals do
not stop it.

STILL INFERENCE: what 0x3E and 0x3F actually do. proxmark sends them first and names
neither, and doc/magic_cards_notes.md's ISO15693-magic section is a TODO, so
"unlock" and "arms" remain our reading. Neither register has been observed to ACCEPT
a write, and no write tried so far clears an arm.

Two claims go with this. "The backdoor registers accept writes WITHOUT
acknowledging" was a parse, not a measurement: 62/63 answer with error 0x10 and the
proxmark client renders that as `( fail )`. And the header's reason for the
power-cycle before each read-back was the latch, which is false -- the reset stays
because it costs nothing and re-activates the card for a clean read, and the header
now says that instead.

Comment-only.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
