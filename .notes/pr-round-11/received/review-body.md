## Round 10 - you got gen1 hardware, and the measurement landed almost everywhere

This is the round I have been waiting nine rounds for. The gen1 path stopped being a faithful
port and became a measured one, and the three premises the measurement falsified are genuinely
gone. I swept the whole app directory rather than the delta, because a stale premise in an
untouched file is the failure mode here:

- **the power-up latch.** Every surviving hit is a corrected one. `CHANGELOG.md:150` "takes
  effect immediately, not on the next power-up"; `iso15693_poller.h:82-85` replaces the latch
  with the real reason the power-cycle stays.
- **"accepts writes without acknowledging"** - gone. `:367-370` now says 62/63 come back
  REFUSED with error 0x10 and the UID moves anyway, and the commit is explicit that the old
  claim was a parse of proxmark's `( fail )` rather than a measurement. That is the right
  instinct and I would rather see it written down than quietly corrected.
- **"not hardware-tested" / "no gen1 card"** - no surviving hit in the ISO15693 code.

And it does not over-claim in the other direction. `iso15693_poller.c:36-49` keeps MEASURED
and STILL INFERENCE apart, names the sample, and says unlock/commit is still your reading, with
"no write tried since has cleared the arm, the wipe's own zero write to 63 included" - which
closes out the round-9 thread about that register.

I also had every proxmark citation in the ISO15693 code checked against the real proxmark
source, since `ec5f9cbe` was fixing one that did not exist. They all resolve: `getTagInfo_15`
genuinely is not a proxmark symbol, `printTagInfo_15` and `uidmapping` are, `CmdHF15Wipe`'s
loop matches what the comment says it does, and the ST25TV `0x08` collision is real.

The behavioural change is correct end to end. A wipe that loses the card now reaches Details
with `list_upto = 0`, so it says the identity check never ran without printing a block list
whose counts a departing card makes meaningless. It cannot claim the check ran and cannot
report a UID change it never observed.

Build clean on API 88.6, `ufbt format` clean.

## Two findings worth your time

**`iso15693_poller.h:83`** is the one I would fix first. The no-latch result is stated there as
a property of "gen1 silicon"; it was measured on one armed ST LRi2K, and the `.c` line it
cross-references says "that chip". The three-chip result cannot support the broader claim -
set/read-back/restore all runs through a power-cycle, so it is silent on latching. Same commit
wrote both. It matters because this is the header contract: the next person meeting a different
gen1 chip reads it as settled and drops the power-cycle.

**`CHANGELOG.md:112`** publishes a rule the poller itself refutes. `iso15693_poller.c:1303-1308`
has the complete version, carve-out included - a card that refuses every write but still serves
a read "walks past 56/57 whatever it claims". The changelog kept the number and dropped the
exception, and in doing so also lost the old sentence's correct identification of that card.
You had this exactly right in the source.

## The rest

A commit message promising a regression-guard note that was never written, on a consent screen;
a cost footnote whose "since" does not hold; a dead conjunct; and a bundle of four pre-existing
mangled comment wraps.

Nothing blocking, and nothing behavioural outside the CardLost fix, which is right.

