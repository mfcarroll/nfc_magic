NFC Magic ISO15693: data-block writes carry the card's address

Every ISO15693 data-block write now goes out as an addressed WRITE BLOCK --
flags 0x22, the UID in the frame, least significant byte first. The clone's
payload and the wipe's zeros funnel through one helper, so both are covered by
the one change.

This is what #251 asks for. An unaddressed write reaches every tag in the field,
and on ISO15693 a bystander does not have to be touching the antenna -- a wallet
or a badge holder is enough. A clone's payload or a wipe's zeros landing on
someone else's card is the harm the issue was filed about.

MEASURED on seven CARDS over four identified chips -- TI Tag-it HF-I Plus (by
its behaviour, not its UID), NXP ICODE SLIX, NXP ICODE SLIX-S, ST LRi2K -- and
two whose own silicon was never captured, because each has only ever worn a
written UID: the type a reader prints for them describes what was put on, not
the chip underneath. Every one accepts an addressed WRITE BLOCK (TI Tag-it with
the OPTION flag set, which the next commit adds). On every one, a write carrying
a UID one byte wrong, aimed at a block holding something else, left that block
unchanged when read back, and the same frame with the right address changed
it -- so the silence is the address being filtered, not a write that landed
without answering. No card here REQUIRES an address, so this buys safety rather
than compatibility.

The wipe has to retake its address as it goes. Blocks 56/57 are the gen1 UID
registers, so the sweep's own zero-write moves the card's identity -- at once,
with no power-cycle, measured on three chips. An address taken at activation is
wrong from block 56 on: every later frame goes unanswered, the absent run trips,
and the sweep reports a card shorter than the one in the field, on exactly the
gen1 path the UID check exists for. The re-address runs on the ATTEMPT rather
than on a success, because a tag can apply a write without answering. At most
twice a sweep; 62/63 carry no UID and cost nothing.

THE RE-ADDRESS CHECKS ITS ANSWER. It takes the new address from the SDK's 1-slot
unaddressed inventory, so a second tag in the field can answer it (#251), and
re-addressing to a stranger would point every later frame at the wrong card.
Because we know what we wrote -- 56 carries uid[7..4], 57 carries uid[3..0] --
there are exactly two honest replies: unchanged, or what the write implies. A
third is refused and the address held. Only a full-width write is checked, since
a narrower block cannot carry a whole UID half. Not airtight, and unfixably so:
a bystander holding the predicted UID would pass, because magic cards make UIDs
non-unique and a 1-slot inventory cannot tell two cards apart.

It lives in the write helper rather than in the sweep, so the rule is stated
once and a clone whose source reaches block 56 on a card that is gen1 magic
underneath is covered by the same line.

The SDK cannot send any of this. iso15693_3_poller_write_block hardcodes
SUBCARRIER_1 | DATA_RATE_HI with no flags parameter and no UID, and
iso15693_3_write_block_response_parse is internal to lib/nfc. So the frame and
the response decode are both built here, the decode matching the SDK's error
mapping value for value.

The UID goes out least significant byte first, which is the reverse of the order
every screen, every .nfc file and `hf 15 reader` print it in. Getting it
backwards does not fail loudly: it addresses a card that is not in the field, so
every write meets silence and the card reads as one that refuses everything.

Not covered by this commit: the gen1 and gen2 backdoor sequences and the WRITE
AFI / WRITE DSFID identity writes, which build their own frames. The list of
unaddressed writes at the top of the poller goes rather than being kept current:
each sender states what it does.

This does not CLOSE #251. The 1-slot inventory and the missing STAY QUIET are
untouched, and the issue's worst consequence -- the post-wipe UID re-read
answered by a bystander -- cannot be fixed this way at all, since that read
exists to discover whether the UID changed and so cannot be aimed at a UID
already in doubt. The comment at that read says so now.

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
