# Does a write to block 16 move a gen3 UID at once? — predictions, written before the first frame

**Card: `gen3-a` only.** Genuine V3, un-finalized, config mode (baseline 2026-09-29,
[../pr-round-15/gen3-a-bench.md](../pr-round-15/gen3-a-bench.md)).

## Why it matters

A gen3 card's UID is driven by blocks 16/17 (0x10/0x11), and its configuration signature sits in
blocks 20/21 (0x14/0x15), which per 0x6r1an0y bricks an un-finalized card if overwritten. Every
write the app sends is addressed to the card's UID, and the app re-takes that address only after a
write to 56/57 (`iso15693_poller_readdress`, called for `iso15693_poller_is_uid_block` only). So
after the app writes block 16:

- **If the UID moves AT ONCE** (as on gen1, measured 2026-09-15): every later write -- 17, 18 ... 20,
  21 -- is addressed to the OLD UID, and an addressing card ignores it. A wipe, or a clone of a file
  whose UID matches the card, would change the UID's tail and **never reach 20/21**. No brick by
  this app; the card ends on a moved UID, which the wipe's re-read reports.
- **If it moves only after the field drops**: the later writes still match, 20/21 are written, and
  the brick follows. The wipe warning stands as written, and a matching-UID clone is a second route.

Both the wipe and the clone pass through the same block-16 write, so this one measurement decides
both. The bench never measured it: the 2026-09-29 run saw the move with `hf 15 reader` afterwards,
which is a new field session.

## The three things never to do (unchanged)

1. **Write blocks 20/21** by any route. Nothing below writes them; step 1 and step 6 READ them.
2. **Send finalize**, `hf 15 cfinalize` or anything like it.
3. **Run the app's Wipe** on this card.

Only block 16 is written, and only with the marker and then the original. Both were written on
2026-09-29 and restored byte-identically.

## Values (from the 2026-09-29 baseline)

| | printed | on the wire (LSB first) |
|---|---|---|
| original UID | `E0 48 03 00 12 85 5F 76` | `76 5F 85 12 00 03 48 E0` |
| UID after the marker | `E0 48 03 00 DD CC BB AA` | `AA BB CC DD 00 03 48 E0` |

Block 16 original `76 5F 85 12`, marker `AA BB CC DD`. Block 17 `00 03 48 E0`. Block 20
`A5 2B 44 2C`, block 21 `21 AE 93 00`.

An inventory answer is the flags byte, the card's DSFID, then the UID on the wire. Frame shapes,
as on the gen1 benches: `22` = addressed, high data rate; `21` WRITE BLOCK, `20` READ
BLOCK; inventory `26 01 00` (1 slot). `-a` activates, `-c` appends CRC, `-k` keeps the field up,
`-w` waits longer for a write's answer. A write's OK is `00` + CRC, three bytes.

## Step 1 — baseline, read only. STOP if anything differs

    hf 15 reader                    -> E0 48 03 00 12 85 5F 76
    hf 15 rdbl -b 16                -> 76 5F 85 12
    hf 15 rdbl -b 17                -> 00 03 48 E0
    hf 15 rdbl -b 20                -> A5 2B 44 2C
    hf 15 rdbl -b 21                -> 21 AE 93 00

## Step 2 — the measurement, all in ONE field session

Each line keeps the field up (`-k`) except the last, which drops it.

    hf 15 raw -ackw -d 2221765F8512000348E010AABBCCDD   (a) addressed write, block 16 <- marker
    hf 15 raw -ckw  -d 260100                           (b) inventory, same session
    hf 15 raw -ckw  -d 2220765F8512000348E011           (c) addressed READ blk 17, OLD uid
    hf 15 raw -c    -d 2220AABBCCDD000348E011           (d) addressed READ blk 17, NEW uid; field drops

(c) and (d) are reads, not writes, and they are each other's control (BENCH-RULE 1): exactly one of
them should answer. They show what the app's NEXT addressed frame would meet, without sending one.

**Predictions:**

| | moves at once | moves only after the field drops |
|---|---|---|
| (a) | `00` + CRC | `00` + CRC |
| (b) | `00 <dsfid> AA BB CC DD 00 03 48 E0` + CRC | `00 <dsfid> 76 5F 85 12 00 03 48 E0` + CRC |
| (c) | silence ("command failed") | `00 00 03 48 E0` + CRC |
| (d) | `00 00 03 48 E0` + CRC | silence |

**If (a) is refused or silent:** gen3 does not take an addressed write at block 16 at all. Then the
app's writes there do nothing, the UID never moves, and the later writes DO reach 20/21 -- the brick
route is open. Skip to step 4 (nothing to restore beyond checking), record it, and stop.

**If (b) and (c)/(d) disagree** (e.g. inventory shows the new UID but the old address still
answers): record it verbatim. The app's behaviour follows (c)/(d), since what matters is whether
the next addressed frame is matched.

## Step 3 — after the field drops

    hf 15 reader                    -> E0 48 03 00 DD CC BB AA   (both cases, as on 2026-09-29)

## Step 4 — restore block 16, addressed to the UID the card now answers to

    hf 15 raw -ackw -d 2221AABBCCDD000348E010765F8512   restore
    hf 15 reader                    -> E0 48 03 00 12 85 5F 76

If the restore is silent: `hf 15 wrbl --ua -b 16 -d 765F8512`, the unaddressed form the 2026-09-29
restore used, then `hf 15 reader` again.

## Step 5 — confirm nothing else moved

    hf 15 rdbl -b 16                -> 76 5F 85 12
    hf 15 rdbl -b 17                -> 00 03 48 E0
    hf 15 rdbl -b 20                -> A5 2B 44 2C
    hf 15 rdbl -b 21                -> 21 AE 93 00

## What each result does to ISO15693.md

- **Moves at once:** the app cannot reach 20/21 through block 16's write. The wipe's gen3 risk
  becomes a moved UID (the tail zeroed) rather than a brick by this app, and the clone note is not
  needed. The gen3 section's "zeroes the UID and configuration blocks" has to change: the wipe would
  zero block 16 and miss the rest. The brick warning stays as the reason never to wipe a gen3 card,
  scoped to what was measured.
- **Moves only after the field drops:** the wipe warning stands as written, and the matching-UID
  clone is a second route to the brick that the doc should name.
- **(a) refused:** as the row above -- the brick route is open, and both warnings stand.

## RESULTS

(to be filled in, verbatim, after the run)
