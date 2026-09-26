NFC Magic ISO15693: data-block writes carry the card's address

Every ISO15693 data-block write now goes out as an addressed WRITE BLOCK --
flags 0x22, the UID in the frame, least significant byte first. The clone's
payload and the wipe's zeros funnel through one helper, so both are covered by
the one change.

This is what #251 asks for. An unaddressed write reaches every tag in the
field, and on ISO15693 a bystander does not have to be touching the antenna --
a wallet or a badge holder is enough. A clone's payload or a wipe's zeros
landing on someone else's card is the harm the issue was filed about.

MEASURED, on five CARDS: all five accept an addressed WRITE BLOCK, and four
answer nothing at all to a UID one byte wrong. Four of them name a chip -- TI
Tag-it HF-I Plus, NXP ICODE SLIX, NXP ICODE SLIX-S, ST LRi2K. The fifth has only
ever worn a cloned UID, so the type a reader prints for it belongs to what was
copied onto it rather than to its silicon, and counting it as a chip would be
the error this round corrects elsewhere. No card here REQUIRES an address, so
this buys safety rather than compatibility.

The wipe has to retake its address as it goes. Blocks 56/57 are the gen1 UID
registers, so the sweep's own zero-write moves the card's identity -- at once,
with no power-cycle, measured on two chips. An address taken at activation is
wrong from block 56 on: every later frame goes unanswered, the absent run
trips, and the sweep reports a card shorter than the one in the field, on
exactly the armed-gen1 path the UID check exists for. The re-address runs on
the ATTEMPT rather than on a success, because a tag can apply a write without
answering and this file declines that inference everywhere else. At most twice
a sweep; 62/63 carry no UID and cost nothing.

It lives in the write helper rather than in the sweep, so the rule is stated
once and a clone whose source reaches block 56 on a card that is gen1 magic
underneath is covered by the same line.

The SDK cannot send any of this. iso15693_3_poller_write_block hardcodes
SUBCARRIER_1 | DATA_RATE_HI with no flags parameter and no UID, and
iso15693_3_write_block_response_parse is internal to lib/nfc. So the frame and
the response decode are both built here, the decode matching the SDK's error
mapping value for value.

The UID goes out least significant byte first, which is the reverse of the
order every screen, every .nfc file and `hf 15 reader` print it in. Getting it
backwards does not fail loudly: it addresses a card that is not in the field,
so every write meets silence and the card reads as one that refuses
everything.

Left unaddressed, and said so at the define: the gen1 and gen2 backdoor
sequences, and WRITE AFI / WRITE DSFID. The backdoors are measured to work
unaddressed on all five cards and nothing has been measured about addressing
them.

This does not CLOSE #251. The 1-slot inventory and the missing STAY QUIET are
untouched, and the issue's worst consequence -- the post-wipe UID re-read
answered by a bystander -- cannot be fixed this way at all, since that read
exists to discover whether the UID changed and so cannot be aimed at a UID
already in doubt.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
