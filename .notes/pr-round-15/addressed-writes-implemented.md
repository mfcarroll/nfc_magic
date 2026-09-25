# Addressed writes — implemented and benched

Four shipped commits: `db3d8ac` the addressed data-block writes, `4c1844b` the OPTION flag and the
read-back it costs, `844de55` the gen1 loss claim, `721dd30` the identity writes. 145 host tests, 0
failed. Both firmwares build warning-free, clang-format clean, writing gate clean.

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

Fixed in `4c1844b`, keyed on the tag's own 0x03 answer rather than on the UID. **Not** on the UID,
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

## Two firmware gaps came out of this

Both in `lib/nfc`, neither fixable from an app, and together they are why a TI Tag-it could not be
written by any FAP. Written up for an upstream report in [../firmware-gaps.md](../firmware-gaps.md),
which also says why the workaround here stays whatever happens upstream.

## THE BENCH

**1. TI Tag-it wipe — PASSES.** `white-coin`, with `DE AD BE EF` / `CA FE BA BE` / `12 34 56 78` /
`FE ED FA CE` at blocks 0, 8, 32 and 63, all verified present first. "Wipe complete / Cleared 64
blocks. Card claims 64.", and a full dump afterwards is all zeros. An earlier attempt at this ran
against an already-blank card and could not have failed.

**2. TI Tag-it clone — PASSES, byte for byte.** `edgedata_64` onto the same card: all 64 blocks match
the source exactly, including `DE AD BE EF` / `CA FE BA BE` at 62/63, which is what proves the data
pass reached the top of the range rather than stopping short. `identity_64` likewise, with its zeros
at 62/63. Both initially reported Partial for AFI/DSFID, which is what led to `721dd30`.

**3. Armed gen1 LRi2K wipe — PASSES, unchanged as predicted.** `lri2k-keychain` dirtied at 0/20/55.
"Wiped 58/58", UID moved to zeros, all 56 blocks clear afterwards. 1030ms. The log carries the
mechanism:

```
[W][Iso15693Poller] the card's UID moved mid-pass; re-addressing
[W][Iso15693Poller] the card's UID moved mid-pass; re-addressing
[I][Iso15693Poller] wipe: 66 blocks attempted, 58 cleared, 1030ms (advertised 56)
[E][Iso15693Poller] wipe: the UID CHANGED
```

Two re-address lines, one after block 56 and one after 57. Unchanged from before the round is the
pass here: the re-address is what stops addressing from breaking this path, not an improvement on it.

**4. gen2 regression — PASSES.** `wipeseed_64` onto `gen-2-card`, whose every block is a distinct
`5A <blk> A5 <blk>` so a partial write shows anywhere. All 64 match, UID `E0 04 01 10 5E ED 00 01`.
Reported as a bare **Success** popup with no counts, which is correct: `nfc_magic_scene_write.c` sends
a clean clone there and reserves the counted screen for a wipe or an empty over-capacity tail, the two
successes that carry something the popup cannot hold.

A first `hf 15 dump` aborted at block 11 with `iso15693 command failed`, and pm3 prints its zeroed
buffer for every row below an abort -- so that table looked like a clone that stopped at 11 and was
nothing of the kind. A re-read was clean. **Do not read a pm3 dump past its abort line.**

**5. gen1 regression and the caveat gate — PASSES.** `slix_28` onto `slix-1k-50x28`, via the gen1
opt-in. Screen: "Cloned 28/28 blocks / Not written: 0 / **gen1: UID in 56/57/62/63**", and Details:
"the UID was set through 56/57/62/63. The file has no blocks that high, so nothing in it was skipped."
Both are the `844de55` wording; the old build claimed those blocks differed from a file that has none.
UID `E0 04 01 10 A1 A2 A3 A4`, data matching the source. The card reports its own 28 blocks and IC ref
0x01 afterwards, which is right -- gen1 has no geometry block to program.

**6. gen1 wipe — PASSES, and it is the reach rule's other side.** Same card: "Cleared 28 blocks. Card
claims 28.", Success, **UID unchanged**. Claiming 28, the sweep trips out around block 35 and never
reaches its own UID registers -- the contrast with the LRi2K, which claims 56 and does. Two cards now
bracket that rule from either side.

**7. The outcome change — PASSES.** Re-cloning `slix_28` after `844de55` gives the plain **Success**
popup: no counts, no gen1 line. Source data present.

**8. Restores — both clean, and each confirms something.** `white-coin` back to
`E0 07 80 3D E2 E7 3A 29` with **IC ref 0x8B and 64 blocks**, which is the magic CFG default being
TI's own identity rather than a coincidence worth re-deriving; then wiped to all zeros.
`lri2k-keychain` back to `E0 02 22 24 50 00 83 03` with **IC ref 0x22 and 56 blocks**, its own
silicon, since gen1 programs no geometry.

**9. Over-capacity on TI — PASSES, and it is the read-back rescue's hardest case.**
`oversize_edgedata_70` onto `black-tag`, from a card first filled with `wipeseed_64` so every block
was distinct and a pass that did nothing would show. The log has the whole mechanism:

```
the card wants the OPTION flag on writes; setting it for this run
   ~64 FWT Timeouts -- one per block, 0..63, each rescued by the read-back
clone: block 64 refused (err 6)      <- three attempts, then given up
clone: block 65..69 refused (err 6)
```

`err 6` is Timeout. Blocks 64-69 do not exist, so each burned its full retry budget and its read-back
failed every time -- **the rescue does not pass writes to blocks that are not there.** Screen: "Cloned
64/70 / Not written: 6 / Card too small". A `wipeseed_64` clone beforehand wrote all 64 distinct
blocks correctly on the same card, one timeout each.

⚠️ **Most of this run was spent chasing a defect that did not exist.** Blocks 0/1 came back holding
neither the previous content nor what the local source file said, which reads exactly like a silent
two-block loss reported as success. The file ON THE FLIPPER was an older generation of a generated
fixture. See the rule added to [../BENCH-RULES.md](../BENCH-RULES.md). All nine device sources have
since been regenerated and pushed, and four of them were stale.

**WAS WORTH ONE RUN, NOW DONE: over-capacity on TI.** No source larger than its card has been written since
the read-back rescue went in, and that is the one mechanism that could turn a real failure into a
false success. It fires only on cards wanting the OPTION flag -- TI -- and a past-capacity block is
exactly where "the write timed out" and "the write landed" must be separated by a read that has to
fail. `test_the_read_back_is_compared_not_just_attempted` covers the shape; hardware does not.
`oversize_edgedata_70` onto `black-tag` closes it and covers the second TI card at the same time.
Expect Partial, "Cloned 64/70 / Not written: 6 / Card too small"; a Success or a count of 70 is the
failure.

## SETTLED — a gen1 clone that lost nothing is a clean Success (`844de55`)

Raised by mfcarroll on seeing run 5: "Cloned 28/28 / Not written: 0" under a **Partial** banner, with a
note underneath saying nothing was skipped.

`iso15693_poller.c:1495` puts `gen1_clone` -- `clone && used_gen1` -- unconditionally in the Partial
list, on the rationale that the four backdoor blocks differ from the source. That is the same claim
`844de55` just gated, left standing one level up.

And it can never be a claim about the CARD: on all three gen1 chips those four addresses answer no
read at any point, so they are write-only registers outside the memory map and a gen1 UID write
displaces nothing. Only SOURCE data at those indices can be lost, which is what gen1_blocks_skipped
records.

**Done:** the conjunct, so a gen1 clone is Partial only when blocks were skipped, and the outcome
agrees with the counts above it. Such a run reaches the plain Success popup and says nothing about
56/57/62/63 -- the user opted into gen1 and gen1 worked.

One thing that cost a mutant: the Write-UID case now sets `gen1_blocks_skipped`, a combination
production cannot reach, because the `clone &&` guard is what that test exists to hold and with the
field left false the guard could be deleted with every test still green.

**Considered and not proposed:** a Success routed to the counted screen with a note that the gen1
sequence sent unlock+commit, so a later wipe may move this card's UID. Real -- run 3 is that hazard --
but it depends on the card's claim reaching 49, and run 6 is a gen1 card that never will. Scoping that
warning is a separate question.

⚠️ **`white-coin` and `lri2k-keychain` are both wearing the wrong UID** -- `E0 04 01 10 1D 1D 1D 1D`
and all-zeros respectively, with DSFID 05 / AFI 27 on the TI. Restore before citing either as stock.

## WRITE AFI / WRITE DSFID — measured and done

Was the strongest remaining piece of #251's blast radius, and is now addressed and OPTION-carrying
like the data blocks (`721dd30`). Three frames settled it on `white-coin`:

| frame | result |
|---|---|
| `hf 15 info` | SYSINFO flags `0x0F` -- DSFID and AFI both advertised, so the verify had something to read |
| `hf 15 raw -ackw -d 422905` | unaddressed + OPTION, accepted, DSFID reads back `05` |
| `hf 15 raw -ackw -d 62271D1D1D1D100104E027` | addressed + OPTION, accepted, AFI reads back `27` |
| `hf 15 raw -ackw -d 22291D1D1D1D100104E005` | addressed, NO option -- refused, `01 03`, DSFID unchanged |

The first rules out the alternative -- that the card simply does not report those fields, in which
case no flag would have helped and "AFI/DSFID not set" would have been the honest answer.

The last rules out the other one. It was run AFTER the app already passed this case, because a pass is
consistent with two stories: the card complaining about the flag and the retry carrying it, or the
addressed frame being accepted outright with the flag never involved. Under the second, addressing
would have been the fix and OPTION irrelevant here -- the same two-bits-at-once confusion that made
the original TI finding wrong. It is the first: **the OPTION flag is required for the identity writes
as well, and addressing alone is not sufficient.**

**Bench: PASSES.** With the fields reset to `00` first, so the run could fail, an `identity_64` clone
onto `white-coin` reports **Success** and `hf 15 info` reads `DSFID 0x05` / `AFI 0x27`.

**What is left unaddressed is the gen1 and gen2 backdoor sequences**, measured to work that way on all
five cards, with nothing measured about addressing them.
