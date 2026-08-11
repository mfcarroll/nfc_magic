# NFC Magic ISO15693: writes and inventory are unaddressed, so a second tag in the field is written too

> Field-by-field for the pack's bug-report form. The H1 above is the **Title** field.
>
> Two things to flag before filing. This concerns the **ISO15693 support added in #250**, which is not
> in a released version yet — so "App version" is the unreleased 2.1, not 2.0. And the reproduction
> below is **expected from the frame construction, not staged on hardware**: it needs two ISO15693 tags
> in the field at once and would destroy data on the second one.

### App

NFC Magic

### App version

2.1 (unreleased — the ISO15693 support in #250)

### Describe the bug

The ISO15693 wipe sends its `WRITE BLOCK` frames **unaddressed**, so every ISO15693 tag inside the
reader field receives them — not just the one the user selected. A second tag in range has its blocks
zeroed too, with nothing in the UI indicating another tag was ever there.

Both of the helpers involved are unaddressed broadcasts, and nothing suppresses the other tags:

- `iso15693_3_poller_write_block()` builds its request flags as
  `ISO15693_3_REQ_FLAG_SUBCARRIER_1 | ISO15693_3_REQ_FLAG_DATA_RATE_HI` — no `ADDRESSED` flag and no
  UID in the frame.
- `iso15693_3_poller_inventory()` sends a **1-slot** INVENTORY (`INVENTORY_T5 | T5_N_SLOTS_1`), so with
  two tags present it returns whichever wins the slot rather than detecting the collision.
- The poller never sends STAY QUIET, so nothing puts a bystander tag out of scope.

The same flaw then undermines the report. The wipe re-reads the UID afterwards to catch a card whose
identity it moved — on a gen1 card the zeros land in blocks 56/57, which *are* the UID registers, so the
card carries on working but stops answering to the identity its owner recorded, and the screen printing
the UID it answers to now is the only route back to it. If that post-wipe inventory is answered by the
**bystander** instead, the screen prints a UID belonging to a different card entirely: the owner records
the wrong identity for their own card, and the real one is never shown. Strictly worse than printing
nothing.

Needs two ISO15693 tags within the field simultaneously, which is uncommon but hardly exotic — a badge
holder or wallet does it.

### Reproduction

**Not staged on hardware** — expected from the frame construction above. It would need two ISO15693 tags
and would destroy data on the second, so it has not been run:

1. Place two ISO15693 tags within the reader field at once.
2. NFC Magic → the ISO15693 menu → **Wipe**.
3. Expected: both tags are zeroed, though only one was presented for the operation; and the post-wipe
   UID check may report against whichever tag answers the inventory.

### Firmware version

Momentum `mntm-012-308-g8ed809fba`, not Unleashed. Local build carrying unrelated LF RFID changes; none of them touch `lib/nfc`, `furi_hal_nfc` or `applications/main/nfc`, so the NFC stack is stock Momentum.

### Anything else?

**Possible directions.** Not prescribing an approach, but for discussion:

- use the **addressed** form (`ADDRESSED` flag + UID) for `write_block` once activation has established
  a UID — the wipe already knows the target's UID before it writes anything;
- and/or send **STAY QUIET** to any non-target tag after inventory;
- and/or run a **16-slot** inventory first and refuse to write when more than one tag answers, which is
  the cheapest option and fails safe.

The last one alone would close the destructive half.

**On which tracker this belongs to.** The two helpers are firmware SDK code
(`lib/nfc/protocols/iso15693_3/iso15693_3_poller_i.c`), not app code — but the destructive behaviour is
the app's wipe, and every fix above is available app-side, since the app already builds its own raw
frames for the magic commands. Filing here for that reason; a firmware-side report may be warranted
too, as any caller of those helpers has the same exposure.

**Where.** Reached from `base_pack/nfc_magic/magic/protocols/iso15693/iso15693_poller.c`
(`iso15693_poller_wipe_blocks`, and the UID verify in `Iso15693WriteStateVerifyWipe`).

Raised by @mishamyte during review of #250 and split out at his request for future work.
