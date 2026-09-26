# The gen1 backdoor sequence, addressed — what shipped and what it rests on

Dev `b312653` (shipped) and `757fa5a` (dev-only). Fork sync point 07.

## What changed

`iso15693_poller_send_backdoor_uid_gen1` builds its four frames with
`iso15693_poller_build_write_frame` -- the same builder, flags and OPTION handling as every other
write in the file -- instead of its own unaddressed `02 21 <block> ...`. `iso15693_poller_build_gen1_frame`
and `ISO15693_MAGIC_CMD_WRITE` are gone; `ISO15693_MAGIC_REGISTER_SIZE` (4) replaces the latter's
role of saying how wide a register write is.

The sequence is unchanged in order and in fire-and-forget behaviour, with one addition: after block
56 it calls `iso15693_poller_predict_uid` + `iso15693_poller_readdress`, the sweep's pair, before
sending 57.

`ISO15693_MAGIC_FLAGS` (0x02) now belongs to the gen2 sequence alone.

## Why the re-address is there

Block 56 carries uid[7..4] and takes effect immediately -- no power-cycle, measured on NXP ICODE SLIX
and ST LRi2K. So the frame after it would carry an address the card has already stopped answering to,
and the run would end with half the target UID and half the card's own. The unaddressed form had no
such seam; this is the one thing addressing the sequence costs.

`readdress` accepts exactly one new UID: the value `predict_uid` says this write implies. Anything
else keeps the old address, which on a card that is not gen1 is the ordinary case.

## The measurements it rests on

All in [dearm-probe-bench.md](dearm-probe-bench.md) and [v1-coin-bench.md](v1-coin-bench.md), with
the frames and responses pasted verbatim.

| chip | card | unaddressed at 62 | addressed at 62 | wrong address |
|---|---|---|---|---|
| ST LRi2K | `lri2k-keychain` | `01 10` | `01 10` | silence |
| NXP SLIX | `slix-1k-50x28` | silence | `01 0F` | silence |
| NXP SLIX | `slix-1k-coin18` | silence | `01 0F` | silence |
| NXP SLIX-S | `SL2S5302` | silence | `01 0F` | silence |
| NXP SLIX | `v1-coin-green18` | silence | `01 0F` | silence |

Three chips, five cards. The addressed frame is received, address-matched and PARSED at block 62 on
every one; a UID one byte wrong is answered by nothing, bracketed by a reader either side. On the
four NXP cards the addressed form is the only one that draws a response at all.

Already load-bearing before this change, which is what makes "addressing the magic sequence is
untested" wrong: the wipe's sweep zeroes 56/57/62/63 through the addressed path, and the clone's
conversion path writes 56/57 addressed and watches the UID follow.

## What CANNOT be validated, and is shipped anyway

**unlock (62) and commit (63) ACCEPTED addressed.** No card here has ever accepted either frame in
any form -- including `v1-coin-green18`, which this app had never written. There is no acceptance to
observe, so the addressed form cannot be shown to fare differently. They are addressed on the safety
argument alone.

The same evidence settles that they are not NEEDED: five cards take a bare write to 56 with no unlock
in front of it, and `0x10` is *block not available* rather than *refused*. They stay regardless --
proxmark sends them, and the cards that would prove them necessary are the ones nobody here owns.

## What the host tests pin, and what the mutants had to kill

`tools/hosttest/test_addressed_write.c`:

- the frame bytes, against the `lri2k-keychain` transcript
- the re-address between the two halves, on a **28-block** card, where the registers sit outside the
  memory map so neither half can be a data write that happens to land
- the sequence addressed one byte wrong moves nothing

The fake tag gained the gen1 registers (outside the memory map, so the block table cannot decide
them) and, in `757fa5a`, the behaviour of a real card under an UNADDRESSED backdoor write. Without
that second change the bystander test passed on a poller with the addressing taken back out -- the
UID stayed put because nothing was listening, not because the frame was aimed elsewhere. A control
that cannot fail (BENCH-RULES 2).

Four mutants, all killed:

| mutant | killed by |
|---|---|
| drop the re-address between 56 and 57 | 4 tests, incl. the dedicated one |
| address to `target_uid` instead of `address_uid` | 5 tests |
| flags reverted to 0x02, builder left addressed | 5 tests |
| the whole pre-change unaddressed frame restored | the frame bytes AND the bystander control |

The last is the honest revert and is the one that matters. Note the re-address test does NOT fail
under it, correctly: an unaddressed sequence needs no re-address because it never had the seam.

## The hardware run this still needs

The risk is that addressing breaks a sequence that works. The cards to catch it are the three gen1
ones: `lri2k-keychain`, `slix-1k-50x28`, `SL2S5302`. Build, install, and run a gen1 Write-UID on each,
checking the UID lands whole rather than half.
