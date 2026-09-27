# Loose-end bench 2 — the block-56 address filter, and two unaddressed writes

**Predictions committed BEFORE the run (BENCH-RULE 5).** Two independent things, neither blocking a
push. The device carries `73e9d76`, which is code-identical to the round tip; these are card facts,
not build facts.

## ⚠️ Card state going in

- **`lri2k-keychain` is ARMED** (`0x6996` in block 63, and nothing here clears it). Its UID moves on
  ANY write to 56/57. That is exactly the mechanism under test, so it is fine — but the restore has
  to re-address to the moved UID, which the generated frames already do. Identify it by its tape, not
  by what it answers, and put it back on `E0 02 22 24 50 00 83 03`.
- The three gen1 cards were restored to their own UIDs 2026-09-26. Re-read each with `hf 15 reader`
  before its frames and confirm it matches the sheet; if a card moved, regenerate from what it shows.
- One card on the antenna at a time.

## Item 1 — is block 56 filtered on the address, on each gen1 chip?

**The gap.** The only mis-addressed write ever sent to block 56 (on `SL2S5302`, 2026-09-26) carried
the value the register already held, so a write that LANDED would have read identically to one that
was refused. Enforcement at block 8 is read back on these chips, but block 56 — the register the
whole gen1 addressing exists to keep off a bystander — rests on that one inconclusive frame.

**The fix.** `tools/gen1-addressed-frames.py` now sends the mis-addressed control with DIFFERENT data
(`44 33 22 11`) from the value step 1 writes (`AA BB CC DD`), and reads it back. Block 56 IS the UID
register, so the readout is `hf 15 reader`: a landed wrong write moves the UID.

**PREDICTION, all three chips:** the mis-addressed frame is silent and the UID is UNCHANGED after it
(still `<uid[0:3]> DD CC BB AA`). If instead the UID reads `<uid[0:3]> 11 22 33 44`, the wrong write
landed and that chip does not filter block 56 — stop and record it; it would weaken the gen1-frame
safety claim for that chip, and belongs in the reply.

Run, one card at a time, pasting each transcript under its card:

    tools/gen1-addressed-frames.py E0 02 22 24 50 00 83 03    # lri2k-keychain  (LRi2K, ARMED)
    tools/gen1-addressed-frames.py E0 04 02 50 03 00 35 F8    # SL2S5302        (SLIX-S)
    tools/gen1-addressed-frames.py E0 04 01 50 20 26 06 8C    # slix-1k-50x28   (SLIX)

### lri2k-keychain — LRi2K — TRANSCRIPT

```
[usb] pm3 --> hf 15 info
[=] Using scan mode

[=] --- Tag Information ---------------------------
[+] UID....... E0 02 22 24 50 00 83 03
[+] TYPE MATCH ST Microelectronics SA France
[+] SYSINFO... 00 0F 03 83 00 50 24 22 02 E0 02 00 37 03 22 
[+] DSFID..... 0x02
[+] AFI....... 0x00
[+] IC ref.... 0x22
[+] Tag memory layout (vendor dependent)
[+]     4 ( or 3 ) bytes/blocks x 56 blocks
[+]     224 total bytes

[usb] pm3 --> hf 15 reader

[+] UID.... E0 02 22 24 50 00 83 03
[+] DSFID.. 02
[+] TYPE MATCH ST Microelectronics SA France

[usb] pm3 --> hf 15 raw -ackw -d 222103830050242202E038AABBCCDD
[+] (3) 00 78 F0 
[usb] pm3 --> hf 15 reader

[+] UID.... E0 02 22 24 DD CC BB AA
[+] DSFID.. 02
[+] TYPE MATCH ST Microelectronics SA France

[usb] pm3 --> hf 15 raw -ackw -d 2221AABBCCDD242202E13844332211
[!] ⚠️  command failed
[usb] pm3 --> hf 15 reader

[+] UID.... E0 02 22 24 DD CC BB AA
[+] DSFID.. 02
[+] TYPE MATCH ST Microelectronics SA France

[usb] pm3 --> hf 15 raw -ackw -d 2221AABBCCDD242202E03803830050
[+] (3) 00 78 F0 
[usb] pm3 --> hf 15 reader

[+] UID.... E0 02 22 24 50 00 83 03
[+] DSFID.. 02
[+] TYPE MATCH ST Microelectronics SA France
```

### SL2S5302 — SLIX-S — TRANSCRIPT

```
[usb] pm3 --> hf 15 info
[=] Using scan mode

[=] --- Tag Information ---------------------------
[+] UID....... E0 04 02 50 03 00 35 F8
[+] TYPE MATCH NXP (Philips); ICS5302/ICS5402 ( SLIX-S )
[+] SYSINFO... 00 0F F8 35 00 03 50 02 04 E0 00 00 27 03 02 
[+] DSFID..... 0x00
[+] AFI....... 0x00
[+] IC ref.... 0x02
[+] Tag memory layout (vendor dependent)
[+]     4 ( or 3 ) bytes/blocks x 40 blocks
[+]     160 total bytes
[usb] pm3 --> hf 15 reader

[+] UID.... E0 04 02 50 03 00 35 F8
[+] DSFID.. 00
[+] TYPE MATCH NXP (Philips); ICS5302/ICS5402 ( SLIX-S )

[usb] pm3 --> hf 15 raw -ackw -d 2221F8350003500204E038AABBCCDD
[+] (3) 00 78 F0 
[usb] pm3 --> hf 15 reader

[+] UID.... E0 04 02 50 DD CC BB AA
[+] DSFID.. 00
[+] TYPE MATCH NXP (Philips); ICS5302/ICS5402 ( SLIX-S )

[usb] pm3 --> hf 15 raw -ackw -d 2221AABBCCDD500204E13844332211
[!] ⚠️  command failed
[usb] pm3 --> hf 15 reader

[+] UID.... E0 04 02 50 DD CC BB AA
[+] DSFID.. 00
[+] TYPE MATCH NXP (Philips); ICS5302/ICS5402 ( SLIX-S )

[usb] pm3 --> hf 15 raw -ackw -d 2221AABBCCDD500204E038F8350003
[+] (3) 00 78 F0 
[usb] pm3 --> hf 15 reader

[+] UID.... E0 04 02 50 03 00 35 F8
[+] DSFID.. 00
[+] TYPE MATCH NXP (Philips); ICS5302/ICS5402 ( SLIX-S )
```

### slix-1k-50x28 — SLIX — TRANSCRIPT

```
[usb] pm3 --> hf 15 info
[=] Using scan mode

[=] --- Tag Information ---------------------------
[+] UID....... E0 04 01 50 20 26 06 8C
[+] TYPE MATCH NXP (Philips); IC SL2 ICS2002/ICS2102 ( SLIX )
[+] SYSINFO... 00 0F 8C 06 26 20 50 01 04 E0 00 00 1B 03 01 
[+] DSFID..... 0x00
[+] AFI....... 0x00
[+] IC ref.... 0x01
[+] Tag memory layout (vendor dependent)
[+]     4 ( or 3 ) bytes/blocks x 28 blocks
[+]     112 total bytes
[usb] pm3 --> hf 15 reader

[+] UID.... E0 04 01 50 20 26 06 8C
[+] DSFID.. 00
[+] TYPE MATCH NXP (Philips); IC SL2 ICS2002/ICS2102 ( SLIX )

[usb] pm3 --> hf 15 raw -ackw -d 22218C062620500104E038AABBCCDD
[+] (3) 00 78 F0 
[usb] pm3 --> hf 15 reader

[+] UID.... E0 04 01 50 DD CC BB AA
[+] DSFID.. 00
[+] TYPE MATCH NXP (Philips); IC SL2 ICS2002/ICS2102 ( SLIX )

[usb] pm3 --> hf 15 raw -ackw -d 2221AABBCCDD500104E13844332211
[!] ⚠️  command failed
[usb] pm3 --> hf 15 reader

[+] UID.... E0 04 01 50 DD CC BB AA
[+] DSFID.. 00
[+] TYPE MATCH NXP (Philips); IC SL2 ICS2002/ICS2102 ( SLIX )

[usb] pm3 --> hf 15 raw -ackw -d 2221AABBCCDD500104E0388C062620
[+] (3) 00 78 F0 
[usb] pm3 --> hf 15 reader

[+] UID.... E0 04 01 50 20 26 06 8C
[+] DSFID.. 00
[+] TYPE MATCH NXP (Philips); IC SL2 ICS2002/ICS2102 ( SLIX )
```

## Item 2 — do `black-tag` and the v2 sticker take an UNADDRESSED write?

**Why.** The reply says "nothing measured here requires an addressed write", measured on six of the
seven cards. These two are the gap. Closing it lets the sentence cover all seven directly rather than
by inference.

`hf 15 wrbl --ua` is the unaddressed write. It forces the OPTION flag on for a TI-manufacturer UID
(`black-tag`, `uid[1]=0x07`) and not otherwise (`v2-sticker`, `uid[1]=0x11`) — which is exactly the
flag each card wants, so one command fits both. Block 8 is ordinary memory and reads normally.

**PREDICTION, both cards:** block 8 reads `00 00 00 00` before (both are blank), the unaddressed
write is accepted, block 8 reads `AA BB CC DD` after, and `00 00 00 00` again after the restore. That
is the card taking an unaddressed write. A refusal (block unchanged at `00 00 00 00`) would be the
surprising result and belongs in the record.

    # black-tag  (E0 07 81 B8 AF 14 42 07)  and  v2-sticker-50x28  (E0 11 22 33 44 55 66 99)
    hf 15 reader                          expect the card's UID; brackets the writes
    hf 15 rdbl -b 8                        expect 00 00 00 00
    hf 15 wrbl --ua -b 8 -d AABBCCDD
    hf 15 rdbl -b 8                        PREDICTION: AA BB CC DD  (took the unaddressed write)
    hf 15 wrbl --ua -b 8 -d 00000000      restore
    hf 15 rdbl -b 8                        expect 00 00 00 00
    hf 15 reader                          brackets: the card was there throughout

### black-tag — TRANSCRIPT

```
[usb] pm3 --> hf 15 info
[=] Using scan mode

[=] --- Tag Information ---------------------------
[+] UID....... E0 07 81 B8 AF 14 42 07
[+] TYPE MATCH Texas Instrument; Tag-it HF-I Plus (RF-HDT-DVBB tag or Third Party Products)
[+] SYSINFO... 00 0F 07 42 14 AF B8 81 07 E0 02 00 3F 03 8B 
[+] DSFID..... 0x02
[+] AFI....... 0x00
[+] IC ref.... 0x8B
[+] Tag memory layout (vendor dependent)
[+]     4 ( or 3 ) bytes/blocks x 64 blocks
[+]     256 total bytes

[usb] pm3 --> hf 15 reader

[+] UID.... E0 07 81 B8 AF 14 42 07
[+] DSFID.. 02
[+] TYPE MATCH Texas Instrument; Tag-it HF-I Plus (RF-HDT-DVBB tag or Third Party Products)

[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] 00 00 00 00 | 0 | ....
[=] ------------+---+------

[usb] pm3 --> hf 15 wrbl --ua -b 8 -d AABBCCDD
[=] Using unaddressed mode
[!!] 🚨 iso15693 card returned error 1: The command is not supported
[-] ⛔ Writing to page 08 (0x08) | AA BB CC DD   ( fail )
[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] 00 00 00 00 | 0 | ....
[=] ------------+---+------

[usb] pm3 --> hf 15 wrbl --ua -b 8 -d 00000000
[=] Using unaddressed mode
[!!] 🚨 iso15693 card returned error 1: The command is not supported
[-] ⛔ Writing to page 08 (0x08) | 00 00 00 00   ( fail )
[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] 00 00 00 00 | 0 | ....
[=] ------------+---+------

[usb] pm3 --> hf 15 reader

[+] UID.... E0 07 81 B8 AF 14 42 07
[+] DSFID.. 02
[+] TYPE MATCH Texas Instrument; Tag-it HF-I Plus (RF-HDT-DVBB tag or Third Party Products)
```

Note: I thought that was because black-tag required the option set, which is what proxmark does silently. Perhaps I misunderstood the commands above, but this works:

```
[usb] pm3 --> hf 15 wrbl -b 8 -d AABBCCDD
[=] Using scan mode
[+] Writing to page 08 (0x08) | AA BB CC DD   ( ok )
[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] AA BB CC DD | 0 | ????
[=] ------------+---+------

[usb] pm3 --> hf 15 wrbl -b 8 -d 00000000
[=] Using scan mode
[+] Writing to page 08 (0x08) | 00 00 00 00   ( ok )
[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] 00 00 00 00 | 0 | ....
[=] ------------+---+------

[usb] pm3 --> hf 15 reader

[+] UID.... E0 07 81 B8 AF 14 42 07
[+] DSFID.. 02
[+] TYPE MATCH Texas Instrument; Tag-it HF-I Plus (RF-HDT-DVBB tag or Third Party Products)
```

Additional run to check the unaddressed+option frame, black-tag:

```
hf 15 reader                          confirm E0 07 81 B8 AF 14 42 07
hf 15 rdbl -b 8                       expect 00 00 00 00
hf 15 wrbl --ua -o -b 8 -d AABBCCDD   unaddressed + OPTION
hf 15 rdbl -b 8                       PREDICTION: AA BB CC DD
hf 15 wrbl --ua -o -b 8 -d 00000000   restore
hf 15 rdbl -b 8                       expect 00 00 00 00
hf 15 reader                          brackets: card present throughout
```

```
[usb] pm3 --> hf 15 info
[=] Using scan mode

[=] --- Tag Information ---------------------------
[+] UID....... E0 07 81 B8 AF 14 42 07
[+] TYPE MATCH Texas Instrument; Tag-it HF-I Plus (RF-HDT-DVBB tag or Third Party Products)
[+] SYSINFO... 00 0F 07 42 14 AF B8 81 07 E0 02 00 3F 03 8B 
[+] DSFID..... 0x02
[+] AFI....... 0x00
[+] IC ref.... 0x8B
[+] Tag memory layout (vendor dependent)
[+]     4 ( or 3 ) bytes/blocks x 64 blocks
[+]     256 total bytes

[usb] pm3 --> hf 15 reader

[+] UID.... E0 07 81 B8 AF 14 42 07
[+] DSFID.. 02
[+] TYPE MATCH Texas Instrument; Tag-it HF-I Plus (RF-HDT-DVBB tag or Third Party Products)

[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] 00 00 00 00 | 0 | ....
[=] ------------+---+------

[usb] pm3 --> hf 15 wrbl --ua -o -b 8 -d AABBCCDD
[=] Using unaddressed mode
[+] Writing to page 08 (0x08) | AA BB CC DD   ( ok )
[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] AA BB CC DD | 0 | ????
[=] ------------+---+------

[usb] pm3 --> hf 15 wrbl --ua -o -b 8 -d 00000000
[=] Using unaddressed mode
[+] Writing to page 08 (0x08) | 00 00 00 00   ( ok )
[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] 00 00 00 00 | 0 | ....
[=] ------------+---+------

[usb] pm3 --> hf 15 reader

[+] UID.... E0 07 81 B8 AF 14 42 07
[+] DSFID.. 02
[+] TYPE MATCH Texas Instrument; Tag-it HF-I Plus (RF-HDT-DVBB tag or Third Party Products)

[usb] pm3 --> 
```

### v2-sticker-50x28 — TRANSCRIPT

```
[usb] pm3 --> hf 15 info
[=] Using scan mode

[=] --- Tag Information ---------------------------
[+] UID....... E0 11 22 33 44 55 66 99
[+] TYPE MATCH Emosyn-EM Microelectronics USA
[+] SYSINFO... 00 0F 99 66 55 44 33 22 11 E0 00 00 3F 03 8B 
[+] DSFID..... 0x00
[+] AFI....... 0x00
[+] IC ref.... 0x8B
[+] Tag memory layout (vendor dependent)
[+]     4 ( or 3 ) bytes/blocks x 64 blocks
[+]     256 total bytes

[usb] pm3 --> hf 15 reader

[+] UID.... E0 11 22 33 44 55 66 99
[+] DSFID.. 00
[+] TYPE MATCH Emosyn-EM Microelectronics USA

[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] 00 00 00 00 | 0 | ....
[=] ------------+---+------

[usb] pm3 --> hf 15 wrbl --ua -b 8 -d AABBCCDD
[=] Using unaddressed mode
[+] Writing to page 08 (0x08) | AA BB CC DD   ( ok )
[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] AA BB CC DD | 0 | ????
[=] ------------+---+------

[usb] pm3 --> hf 15 wrbl --ua -b 8 -d 00000000
[=] Using unaddressed mode
[+] Writing to page 08 (0x08) | 00 00 00 00   ( ok )
[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] 00 00 00 00 | 0 | ....
[=] ------------+---+------

[usb] pm3 --> hf 15 wrbl -b 8 -d AABBCCDD
[=] Using scan mode
[+] Writing to page 08 (0x08) | AA BB CC DD   ( ok )
[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] AA BB CC DD | 0 | ????
[=] ------------+---+------

[usb] pm3 --> hf 15 wrbl -b 8 -d 00000000
[=] Using scan mode
[+] Writing to page 08 (0x08) | 00 00 00 00   ( ok )
[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] 00 00 00 00 | 0 | ....
[=] ------------+---+------

[usb] pm3 --> hf 15 reader

[+] UID.... E0 11 22 33 44 55 66 99
[+] DSFID.. 00
[+] TYPE MATCH Emosyn-EM Microelectronics USA
```

## Item 3 — housekeeping, while the cards are out

- **`lri2k-keychain` block 8** still holds probe residue from an earlier session. After item 1's
  restore, `hf 15 wrbl --ua -b 8 -d 00000000` and confirm, so the card is left uniformly blank.

Looks like lri2k-keychain was already blank:

```
[usb] pm3 --> hf 15 reader

[+] UID.... E0 02 22 24 50 00 83 03
[+] DSFID.. 02
[+] TYPE MATCH ST Microelectronics SA France

[usb] pm3 --> hf 15 rdbl -b 8
[=] Using scan mode

[=] #  8        |lck| ascii
[=] ------------+---+------
[=] 00 00 00 00 | 0 | ....
[=] ------------+---+------

[usb] pm3 --> hf 15 dump
[=] Using scan mode
[+] Reading memory
 🕗 blk  56
[=] --- Tag Memory -------------------

[=] -----+-------------+---+------
[=]  blk | data        |lck| ascii
[=] -----+-------------+---+------
[=]    0 | 00 00 00 00 | 0 | ....
[=]    1 | 00 00 00 00 | 0 | ....
[=]    2 | 00 00 00 00 | 0 | ....
[=]    3 | 00 00 00 00 | 0 | ....
[=]    4 | 00 00 00 00 | 0 | ....
[=]    5 | 00 00 00 00 | 0 | ....
[=]    6 | 00 00 00 00 | 0 | ....
[=]    7 | 00 00 00 00 | 0 | ....
[=]    8 | 00 00 00 00 | 0 | ....
[=]    9 | 00 00 00 00 | 0 | ....
[=]   10 | 00 00 00 00 | 0 | ....
[=]   11 | 00 00 00 00 | 0 | ....
[=]   12 | 00 00 00 00 | 0 | ....
[=]   13 | 00 00 00 00 | 0 | ....
[=]   14 | 00 00 00 00 | 0 | ....
[=]   15 | 00 00 00 00 | 0 | ....
[=]   16 | 00 00 00 00 | 0 | ....
[=]   17 | 00 00 00 00 | 0 | ....
[=]   18 | 00 00 00 00 | 0 | ....
[=]   19 | 00 00 00 00 | 0 | ....
[=]   20 | 00 00 00 00 | 0 | ....
[=]   21 | 00 00 00 00 | 0 | ....
[=]   22 | 00 00 00 00 | 0 | ....
[=]   23 | 00 00 00 00 | 0 | ....
[=]   24 | 00 00 00 00 | 0 | ....
[=]   25 | 00 00 00 00 | 0 | ....
[=]   26 | 00 00 00 00 | 0 | ....
[=]   27 | 00 00 00 00 | 0 | ....
[=]   28 | 00 00 00 00 | 0 | ....
[=]   29 | 00 00 00 00 | 0 | ....
[=]   30 | 00 00 00 00 | 0 | ....
[=]   31 | 00 00 00 00 | 0 | ....
[=]   32 | 00 00 00 00 | 0 | ....
[=]   33 | 00 00 00 00 | 0 | ....
[=]   34 | 00 00 00 00 | 0 | ....
[=]   35 | 00 00 00 00 | 0 | ....
[=]   36 | 00 00 00 00 | 0 | ....
[=]   37 | 00 00 00 00 | 0 | ....
[=]   38 | 00 00 00 00 | 0 | ....
[=]   39 | 00 00 00 00 | 0 | ....
[=]   40 | 00 00 00 00 | 0 | ....
[=]   41 | 00 00 00 00 | 0 | ....
[=]   42 | 00 00 00 00 | 0 | ....
[=]   43 | 00 00 00 00 | 0 | ....
[=]   44 | 00 00 00 00 | 0 | ....
[=]   45 | 00 00 00 00 | 0 | ....
[=]   46 | 00 00 00 00 | 0 | ....
[=]   47 | 00 00 00 00 | 0 | ....
[=]   48 | 00 00 00 00 | 0 | ....
[=]   49 | 00 00 00 00 | 0 | ....
[=]   50 | 00 00 00 00 | 0 | ....
[=]   51 | 00 00 00 00 | 0 | ....
[=]   52 | 00 00 00 00 | 0 | ....
[=]   53 | 00 00 00 00 | 0 | ....
[=]   54 | 00 00 00 00 | 0 | ....
[=]   55 | 00 00 00 00 | 0 | ....
[=] -----+-------------+---+------
```

- **`gen-2-card`** may still advertise 256 blocks against 64 physical, from the CFG clamp fixture.
  Re-clone any normal 64-block source to reset the geometry, then confirm `hf 15 reader` /
  `hf 15 info` shows 64. Record what it read before and after.

initial:

```
[usb] pm3 --> hf 15 reader

[+] UID.... E0 04 01 10 A1 A2 A3 A4
[+] DSFID.. 00
[+] TYPE MATCH NXP (Philips); IC SL2 ICS2002/ICS2102 ( SLIX )

[usb] pm3 --> hf 15 info
[=] Using scan mode

[=] --- Tag Information ---------------------------
[+] UID....... E0 04 01 10 A1 A2 A3 A4
[+] TYPE MATCH NXP (Philips); IC SL2 ICS2002/ICS2102 ( SLIX )
[+] SYSINFO... 00 0F A4 A3 A2 A1 10 01 04 E0 00 00 1B 03 03 
[+] DSFID..... 0x00
[+] AFI....... 0x00
[+] IC ref.... 0x03
[+] Tag memory layout (vendor dependent)
[+]     4 ( or 3 ) bytes/blocks x 28 blocks
[+]     112 total bytes
```

after writing a 64 image, wiping, and setting UID on flipper:

```
[usb] pm3 --> hf 15 info
[=] Using scan mode

[=] --- Tag Information ---------------------------
[+] UID....... E0 00 00 00 00 00 00 00
[+] TYPE...... no tag-info available
[+] SYSINFO... 00 0F 00 00 00 00 00 00 00 E0 00 00 3F 03 8B 
[+] DSFID..... 0x00
[+] AFI....... 0x00
[+] IC ref.... 0x8B
[+] Tag memory layout (vendor dependent)
[+]     4 ( or 3 ) bytes/blocks x 64 blocks
[+]     256 total bytes

[usb] pm3 --> hf 15 reader

[+] UID.... E0 00 00 00 00 00 00 00
[+] DSFID.. 00
[+] TYPE...... no tag-info available

[usb] pm3 --> hf 15 dump
[=] Using scan mode
[+] Reading memory
 🕛 blk  64
[=] --- Tag Memory -------------------

[=] -----+-------------+---+------
[=]  blk | data        |lck| ascii
[=] -----+-------------+---+------
[=]    0 | 00 00 00 00 | 0 | ....
[=]    1 | 00 00 00 00 | 0 | ....
[=]    2 | 00 00 00 00 | 0 | ....
[=]    3 | 00 00 00 00 | 0 | ....
[=]    4 | 00 00 00 00 | 0 | ....
[=]    5 | 00 00 00 00 | 0 | ....
[=]    6 | 00 00 00 00 | 0 | ....
[=]    7 | 00 00 00 00 | 0 | ....
[=]    8 | 00 00 00 00 | 0 | ....
[=]    9 | 00 00 00 00 | 0 | ....
[=]   10 | 00 00 00 00 | 0 | ....
[=]   11 | 00 00 00 00 | 0 | ....
[=]   12 | 00 00 00 00 | 0 | ....
[=]   13 | 00 00 00 00 | 0 | ....
[=]   14 | 00 00 00 00 | 0 | ....
[=]   15 | 00 00 00 00 | 0 | ....
[=]   16 | 00 00 00 00 | 0 | ....
[=]   17 | 00 00 00 00 | 0 | ....
[=]   18 | 00 00 00 00 | 0 | ....
[=]   19 | 00 00 00 00 | 0 | ....
[=]   20 | 00 00 00 00 | 0 | ....
[=]   21 | 00 00 00 00 | 0 | ....
[=]   22 | 00 00 00 00 | 0 | ....
[=]   23 | 00 00 00 00 | 0 | ....
[=]   24 | 00 00 00 00 | 0 | ....
[=]   25 | 00 00 00 00 | 0 | ....
[=]   26 | 00 00 00 00 | 0 | ....
[=]   27 | 00 00 00 00 | 0 | ....
[=]   28 | 00 00 00 00 | 0 | ....
[=]   29 | 00 00 00 00 | 0 | ....
[=]   30 | 00 00 00 00 | 0 | ....
[=]   31 | 00 00 00 00 | 0 | ....
[=]   32 | 00 00 00 00 | 0 | ....
[=]   33 | 00 00 00 00 | 0 | ....
[=]   34 | 00 00 00 00 | 0 | ....
[=]   35 | 00 00 00 00 | 0 | ....
[=]   36 | 00 00 00 00 | 0 | ....
[=]   37 | 00 00 00 00 | 0 | ....
[=]   38 | 00 00 00 00 | 0 | ....
[=]   39 | 00 00 00 00 | 0 | ....
[=]   40 | 00 00 00 00 | 0 | ....
[=]   41 | 00 00 00 00 | 0 | ....
[=]   42 | 00 00 00 00 | 0 | ....
[=]   43 | 00 00 00 00 | 0 | ....
[=]   44 | 00 00 00 00 | 0 | ....
[=]   45 | 00 00 00 00 | 0 | ....
[=]   46 | 00 00 00 00 | 0 | ....
[=]   47 | 00 00 00 00 | 0 | ....
[=]   48 | 00 00 00 00 | 0 | ....
[=]   49 | 00 00 00 00 | 0 | ....
[=]   50 | 00 00 00 00 | 0 | ....
[=]   51 | 00 00 00 00 | 0 | ....
[=]   52 | 00 00 00 00 | 0 | ....
[=]   53 | 00 00 00 00 | 0 | ....
[=]   54 | 00 00 00 00 | 0 | ....
[=]   55 | 00 00 00 00 | 0 | ....
[=]   56 | 00 00 00 00 | 0 | ....
[=]   57 | 00 00 00 00 | 0 | ....
[=]   58 | 00 00 00 00 | 0 | ....
[=]   59 | 00 00 00 00 | 0 | ....
[=]   60 | 00 00 00 00 | 0 | ....
[=]   61 | 00 00 00 00 | 0 | ....
[=]   62 | 00 00 00 00 | 0 | ....
[=]   63 | 00 00 00 00 | 0 | ....
[=] -----+-------------+---+------

[=] Using UID as filename
[+] Saved to json file /Users/matthew.carroll.personal/hf-15-E000000000000000-dump.json
[usb] pm3 --> 
```

- Update `tools/tag-inventory.json` for anything a run leaves changed (BENCH-RULE 9).

No updates have been made yet.

## After the run

If every prediction holds: the block-56 filter is measured on all three gen1 chips, and the
unaddressed-write claim covers all seven cards. I fold both into the reply and the measurement
record, and note the residue/geometry restores in the inventory. Nothing here changes shipped code,
so the fork build does not move for it — only the reply and notes.
