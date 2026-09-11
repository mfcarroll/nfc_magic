# #251 follow-up comment

**Status: DRAFT. Not posted.** Goes on issue #251.

~~~~
Scope correction. @mishamyte spotted in review of #250 that the gen1 backdoor sequence bypasses the
retry helper this issue was written around; checking that turned up two more senders. **Every write
this app transmits is unaddressed, not only the wipe's.**

The original report covers `iso15693_3_poller_write_block` and the 1-slot inventory. The full list,
ordered by what a bystander actually loses:

| path | frame | reaches a bystander |
|---|---|---|
| data blocks | SDK `write_block`, `SUBCARRIER_1 \| DATA_RATE_HI` | any tag — a wipe zeroes it outright |
| clone identity | `WRITE AFI` (0x27), `WRITE DSFID` (0x29) | **any tag, any size** |
| gen1 backdoor | `WRITE BLOCK` (0x21) at 56/57/62/63 | a tag of 57 blocks or more: four blocks of user data |
| gen2 backdoor | `0xE0` + sub-command | should be rejected — proprietary |

The three the original report did not name are all built by the app rather than by the SDK, with the
flags byte `0x02` — high data rate, no `ADDRESSED` flag, no UID.

**The AFI/DSFID pair is the one that widens the blast radius**, and it is on the CLONE path rather
than the wipe, so it sits outside how this issue is currently framed. Two reasons it is worse than the
gen1 case:

- No size floor. A stray write at index 56 needs the bystander to hold 57 blocks; AFI and DSFID
  exist on every compliant tag that reports them.
- AFI is the field a selective inventory filters on, so a bystander whose AFI is overwritten can stop
  answering the inventories its owner's reader uses. That presents as the tag having failed rather
  than as anything having been written to it.

Both are sent inside a retry loop, so a bystander can receive each of them up to three times
(`ISO15693_POLLER_WRITE_ATTEMPTS`).

The directions already suggested still hold — a 16-slot pre-flight inventory closes the destructive
half for all four paths at once. What changes is the sizing. This is not a wipe-only hazard, and
**"no magic tag nearby" is not a sufficient condition for safety**: three of the four paths send
ordinary ISO15693 commands (`0x21`, `0x27`, `0x29`), so an entirely unremarkable bystander is exposed.
Only the gen2 backdoor's `0xE0` needs the neighbour to be magic.

The app now carries the full list on `ISO15693_MAGIC_FLAGS`, the define that makes a frame
unaddressed, rather than on the block-write helper where it was scoped before.

Still not staged on hardware, for the same reason as the original report: it needs two tags and would
destroy data on the second.
~~~~
