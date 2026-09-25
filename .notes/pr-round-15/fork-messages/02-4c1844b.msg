NFC Magic ISO15693: the OPTION flag, and the acknowledgement it costs

TI Tag-it HF-I Plus refuses a WRITE BLOCK whose OPTION bit is clear, addressed
or not, and this app could not write a data block on that silicon at all. Both
halves of the fix are here, because the first without the second is worse than
neither.

MEASURED on one such card, all four combinations, each read back from it:

    02 unaddressed, no option   refused, error 0x01
    22 addressed,   no option   refused, error 0x03, "option not supported"
    42 unaddressed, OPTION      accepted
    62 addressed,   OPTION      accepted

So the flag is necessary and sufficient there, and the addressing is orthogonal
to it.

NOT DECIDED FROM THE UID, although proxmark decides it that way -- it sets the
flag when the manufacturer byte is TI's. A UID is not a statement about silicon
in an app whose purpose is changing it. A clone writes the SOURCE's UID onto the
card, so a TI card cloned from an NXP image stops looking like TI immediately
before the data pass that needs the flag; a wipe zeroing block 57 rewrites
uid[3..0], where the manufacturer byte lives; and a card that arrived carrying
someone else's UID never looked like its own silicon at all. All three are
reachable here, and the first is the ordinary path rather than a corner.

Decided by what the TAG SAYS instead. The first write of a run goes out without
the flag; a 0x03 refusal sets it for the rest of the run. The write already
retries, so the attempt that would have been a retry is the one that lands, and
the recovery costs no frames the budget had not already allowed for. Sticky, and
never re-asked inside a run, for the same reason it is not read off the UID.

AND THE FLAG COSTS THE ACKNOWLEDGEMENT, which is why it does not ship alone.
ISO15693-3 10.3.1: with OPTION set, the card answers a write only after the
READER sends a standalone EOF, and this SDK cannot send one --
iso15693_3_poller_encode_frame appends exactly one EOF as the tail of the frame
and there is no call for another. proxmark sends it, in SendDataTagEOF, which is
the only reason its own raw write to this card ever produced an answer.

So the write lands and nothing comes back. With the flag and without the
read-back, a wipe zeroes all 64 blocks of a TI card and then reports that
nothing was cleared. MEASURED: a block holding AA BB CC DD before such a wipe
read back as zeros after it, while the device log showed the flag being set and
then 216 consecutive frame timeouts -- 72 blocks at three attempts -- with no
timeout for any of the reads interleaved with them.

Silence from a card that asked for the flag is therefore settled by reading the
block back and comparing it. Reads need no OPTION flag and are answered
normally, which is what makes this available at all.

Scoped three ways, and each is deliberate:

  - only on silence. An in-band error frame is the card deciding, and the
    dangerous shape is a refused block that ALREADY holds what is being written
    -- a locked, already-empty block under a wipe reads back as a match, and a
    write-protected blank card would then report "Wiped 64/64" when the truth is
    that it accepted nothing.
  - only on cards that asked for the flag. Elsewhere silence means the card is
    gone or the coupling is bad, the retries are the right answer, and a read on
    every failed write would double the cost of the exact path the pass budget
    exists to bound.
  - compared, not merely performed. A block that answers reads, discards writes
    and says nothing about it is indistinguishable from one that took the write
    until someone looks at what it holds.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
