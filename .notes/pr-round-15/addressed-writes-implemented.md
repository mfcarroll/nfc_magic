# Addressed writes — implemented, awaiting the bench

Dev `c9a8431`, one shipped commit (`iso15693_poller.c`, `CHANGELOG.md`) plus the harness. 132 host
tests, 0 failed. Both firmwares build warning-free, clang-format clean.

The measurements this was built from are in
[addressed-writes-measured.md](addressed-writes-measured.md). Nothing here re-argues them.

## What went in

- `ISO15693_POLLER_WRITE_FLAGS` (0x22) and `iso15693_poller_build_write_frame` — the frame, with the
  UID reversed onto the wire.
- `iso15693_poller_parse_write_response` — the SDK's `iso15693_3_write_block_response_parse` written
  out, because it is internal to `lib/nfc`. Error mapping matched value for value.
- `iso15693_poller_write_block_retried` now takes the poller instance and addresses every data-block
  write to `instance->address_uid`. The clone's payload and the wipe's zeros both go through it.
- `iso15693_poller_readdress` — re-inventory and retake the address after a write to 56 or 57, on the
  attempt rather than on a success.
- `address_uid` is set from the card at `Start` and again in `finish_write`, where the UID has just
  been verified as the target.
- The SDK's `iso15693_3_poller_write_block` is no longer called from anywhere in the app.

The backdoor sequences and WRITE AFI / WRITE DSFID are unchanged and unaddressed.

## The one thing that is reasoned rather than measured

Where the re-address LIVES. The measurements say the wipe needs it; putting it in the write helper
rather than in the sweep also covers a clone whose source reaches block 56 on a card that is gen1
magic underneath. No such card is known, and the cost of covering it is two inventories on a clone of
a 58-block-or-larger source.

## Mutation-tested, from the committed baseline

Eight mutants, all killed. The three that matter are killed by the new tests alone:

| mutant | killed by |
|---|---|
| no re-address after a UID register | the sweep that moves its own UID; the inventory-count delta |
| `finish_write` keeps the pre-write address | the gen2 clone's data pass |
| 62/63 also re-address | the inventory-count delta |
| UID goes out MSB-first | 32 cases -- every write in the suite |
| the frame is unaddressed | 32 cases |
| every response reads as a write that took | 24 cases |
| an in-band refusal reads as success | 23 cases |
| the wipe never takes the card's address | 3 cases |

## The TI row was never measured with our frame — corrected 2026-09-24

The first device test failed: the wipe on `white-coin` still cleared nothing. The cause was not the
build.

`hf 15 wrbl` forces the OPTION flag on for any tag whose manufacturer byte is TI's 0x07
(proxmark `client/src/cmdhf15.c:3126`, and the same override in `restore` and `wipe`). The round-10
result behind "TI accepts addressed writes" was taken with that command, so it measured a frame two
bits different from the one this app sends. Measured properly, with our frame:

```
hf 15 reader                                      -> E0 07 80 3D E2 E7 3A 29
hf 15 raw -ackw -d 2221293AE7E23D8007E00811223344 -> 01 03 04 24   error 0x03, option not supported
hf 15 raw -ackw -d 6221293AE7E23D8007E00855667788 -> 00 78 F0      accepted
hf 15 rdbl -b 8                                   -> 55 66 77 88   the OPTION write landed
hf 15 reader                                      -> E0 07 80 3D E2 E7 3A 29
```

Fixed in `f7d6010`, keyed on the tag's own 0x03 answer rather than on the UID. **Not** on the UID,
although proxmark does it that way: a clone writes the source's UID onto the card, so a TI card cloned
from an NXP image stops looking like TI immediately before the data pass that needs the flag. The
wipe's block 57 and a card carrying someone else's UID are two more routes to the same error. That
question came from mfcarroll, before the pm3-style gate was written.

**What this does to the claim.** "TI refuses unaddressed WRITE BLOCK" is no longer established. The
evidence is an unaddressed write with no OPTION refused with 0x01 and an addressed one with no OPTION
refused with 0x03; no frame has carried one bit without the other. One frame would settle it:

    hf 15 raw -ackw -d 422108AABBCCDD      unaddressed, OPTION set

Accepted means the OPTION flag alone was the barrier and addressing is purely the #251 safety fix on
that chip. Refused means TI wants both. It changes nothing in the code and only what may be said.

## Addressing STAYS in this PR — settled 2026-09-24, do not re-open

The correction above removes the argument we had been making for it (that TI could not be written
without it). The decision does not change, and the reason is mfcarroll's rather than derivable from
anything in the tree:

**Not damaging things has been the maintainer's overriding concern across every round**, and on
ISO15693 the bystander risk is materially worse than on the protocols this app already handles. The
operating range means a second tag does not have to be on the antenna to take a clone's payload or a
wipe's zeros -- a wallet or a badge holder is enough. mfcarroll is being deliberately careful at the
bench for exactly this reason, with cards in view and one hand on the tray; an ordinary user has none
of that.

So the framing for the next reply is a correction, not a scoping question: **the OPTION flag is what
made the TI card writable, and addressing is the #251 safety fix.** Both are built. Do NOT offer him
a minimal merge with the addressing split out -- that presents a closed decision as open, which is
the round-14 defect, and it undersells the half that serves his stated priority.

What we DO owe him is the retraction, in a sentence: we told him TI refuses unaddressed WRITE BLOCK,
and it does not.

## WHAT THE BENCH STILL HAS TO SAY

Nothing has been run on hardware. Predictions first, so the run can falsify them:

1. **TI Tag-it HF-I Plus (`white-coin`, `black-tag`) — the acceptance test for the whole feature.**
   A wipe cleared nothing before this and reported "no blocks could be cleared"; a clone set the UID
   and failed every data block. Both should now work. If they do not, the feature does not ship.
2. **An armed gen1 ST LRi2K (`lri2k-keychain`) wipe — the re-address path.** Predicted UNCHANGED from
   today: "Wiped 58/58", Partial, the UID reported as moved. The re-address is not an improvement on
   the current behaviour, it is what stops addressing from breaking it, so an unchanged result is the
   pass and a shorter count is the failure.
3. **A regression pass on the cards that already worked** -- a gen2 clone on `gen-2-card`, a gen1
   clone and a wipe on an NXP SLIX. Nothing should change.

## Still unaddressed and not yet measured — WRITE AFI / WRITE DSFID

Two frames would settle it, on any card that reports the fields:

    hf 15 raw -ackw -d 2227<UID-LSB-first><afi>      addressed WRITE AFI
    hf 15 raw -ackw -d 2229<UID-LSB-first><dsfid>    addressed WRITE DSFID

They are standard commands and they reach a bystander of any size, so they have the strongest claim
of what is left. Not implemented on a spec reading alone: an addressed WRITE AFI a card refuses would
downgrade a clone that works today.
