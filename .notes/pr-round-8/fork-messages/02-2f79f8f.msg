NFC Magic ISO15693: #251 is every write this app sends, not just the data blocks

"Every block write in this file funnels through here" was false, and you are
right that it is the load-bearing kind: that paragraph is where a reader sizes
#251. The gen1 sequence sends four WRITE BLOCKs straight through
iso15693_3_poller_send_frame at 56/57/62/63, never touching write_block_retried,
and builds them with ISO15693_MAGIC_FLAGS -- 0x02, unaddressed, per line 21.

Checking your finding turned up two more senders you did not name, so the scope is
wider again. Every unaddressed site, by what it reaches:

  - data blocks, through the SDK. Re-verified rather than taken on report:
    iso15693_3_poller_write_block appends SUBCARRIER_1 | DATA_RATE_HI and nothing
    else (iso15693_3_poller_i.c:237). A wipe zeroes the bystander too.
  - WRITE AFI (0x27) and WRITE DSFID (0x29) from the clone's identity pass. These
    are STANDARD ISO15693 commands, so unlike a block write at index 56 they do
    not need the bystander to be a particular size -- any compliant tag in the
    field applies them. A changed AFI can also drop a tag out of a selective
    inventory, so the damage is not confined to one field's value.
  - the gen1 backdoor, plain 0x21 at 56/57/62/63: four blocks of ordinary user
    data on a tag that big. This is your finding.
  - the gen2 backdoor, 0xE0, which is proprietary -- a conforming tag should
    reject it, which makes it the only one of the four with a floor under it.

Your suggestion was "every DATA-block write" here plus a separate mention of the
gen1 sequence. The fix goes further and in the other direction: the scope belongs
on ISO15693_MAGIC_FLAGS, which is the define that MAKES a frame unaddressed and
the place a reader of any of these four sites already lands. So one owner gains
the full statement and this comment shrinks to two lines pointing at it, rather
than a second site growing a partial copy that can drift from the first.

Net +7 comment lines for a hazard that got three categories wider. Both firmwares
build clean.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
