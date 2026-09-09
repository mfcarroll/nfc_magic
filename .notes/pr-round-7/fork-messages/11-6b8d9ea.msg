NFC Magic ISO15693: cite #251 where its frames are actually sent

#251 was the only one of the four open issues with no pointer from the code.
#252 and #253 are cited from scene_write.c, #255 from the poller and the
CHANGELOG; the unaddressed-frame hazard was cited nowhere, which makes it look
like an oversight rather than a decision when it is the latter -- the issue
records that mishamyte asked for it to be split out for future work.

It is also the most reachable of the four. #255 needs a gen3 card nobody has;
this one needs two ordinary ISO15693 tags in the field, which a wallet does.

Cited at the two choke points rather than at the functions the issue names,
because these are where the frames are built and every caller passes through
them:

  write_block_retried    every block write in the file, and the SDK sets only
                         SUBCARRIER_1 | DATA_RATE_HI -- no ADDRESSED, no UID
  verify_inventory       every UID read-back, and the SDK's inventory is
                         1-slot, so with two tags either can answer

The second is the half worth spelling out. After a wipe, an inventory answered
by the bystander makes the "UID changed" screen print a UID belonging to a
different card as though it were this one -- worse than printing nothing, since
that screen exists to be the route back to a card whose identity moved.

Re-verified against the current SDK rather than trusting the Round 2 report:
iso15693_3_poller_i.c:237 write_block, :132 inventory. Both still as filed.

Comment +12, code +0. An addition in a delta about cutting, and worth it: this
is a citation the other three already have.

