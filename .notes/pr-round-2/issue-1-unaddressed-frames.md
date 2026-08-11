# NFC Magic ISO15693: writes and inventory are unaddressed, so a second tag in the field is written too

**Pre-existing, not introduced by #250** — raised there and split out at the reviewer's request.

## What

Both SDK helpers the ISO15693 magic poller relies on are unaddressed broadcasts, and nothing ever
suppresses the other tags in the field:

- `iso15693_3_poller_write_block()` builds its request flags as
  `ISO15693_3_REQ_FLAG_SUBCARRIER_1 | ISO15693_3_REQ_FLAG_DATA_RATE_HI` — no `ADDRESSED` flag and no
  UID in the frame. Every tag in the field that isn't in Quiet state acts on it.
- `iso15693_3_poller_inventory()` sends a **1-slot** INVENTORY (`INVENTORY_T5 | T5_N_SLOTS_1`), so with
  two tags present it returns whichever one wins the slot rather than detecting the collision.
- The poller never sends STAY QUIET, so nothing puts a bystander tag out of scope.

## Why it matters

For a read this is a wrong answer. For NFC Magic's **wipe** it is data loss on a card the user never
selected: a second ISO15693 tag inside the reader field receives the same `WRITE BLOCK` frames and has
its blocks zeroed, with nothing in the UI indicating a second tag was ever there.

The same flaw then undermines the report. The wipe re-reads the UID afterwards to catch a card whose
identity it moved — on a gen1 card the zeros land in blocks 56/57, which *are* the UID registers, so the
card carries on working but stops answering to the identity its owner recorded. The screen printing the
UID it answers to now is the only route back to that card.

If the post-wipe inventory is answered by the *bystander* instead, that screen prints a UID belonging to
a different card entirely. The owner records the wrong identity for their own card and the real one is
never shown — strictly worse than printing nothing.

Needs two ISO15693 tags within the field simultaneously, which is uncommon but hardly exotic — a badge
holder or wallet does it.

## Possible directions

Not prescribing an approach, but for discussion:

- use the **addressed** form (`ADDRESSED` flag + UID) for `write_block` once activation has established
  a UID — the wipe already knows the target's UID before it writes anything;
- and/or send **STAY QUIET** to any non-target tag after inventory;
- and/or run a **16-slot** inventory first and refuse to write when more than one tag answers, which is
  the cheapest option and fails safe.

The last one alone would close the destructive half.

## Where

`lib/nfc/protocols/iso15693_3/iso15693_3_poller_i.c` in the firmware SDK (`iso15693_3_poller_inventory`
~line 132, `iso15693_3_poller_write_block` ~line 237), reached from
`base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c`.
