NFC Magic ISO15693: the clone's identity writes are addressed, and take the OPTION flag

A clone onto a TI Tag-it wrote all 64 data blocks and then reported Partial,
"AFI/DSFID not set".

MEASURED on that card, each result read back through GET SYSTEM INFO:

    hf 15 info                     system info flags 0x0F -- DSFID and AFI both
                                   advertised, so the verify had something to read
    22 29 <uid> 05    addressed, no option   refused, error 0x03, no change
    42 29 05          unaddressed, OPTION    accepted, DSFID reads back 05
    62 27 <uid> 27    addressed, OPTION      accepted, AFI reads back 27

The first rules out the other candidate -- that the card simply does not report
those fields, in which case no flag would have helped and "AFI/DSFID not set"
would have been the honest answer. The last rules out the remaining one:
addressing alone is not enough either.

So the identity writes now carry the same flags as a data block, from one
helper, and learn the OPTION requirement from the same 0x03 answer through the
same helper. Which command drew the refusal does not matter -- the card is
objecting to the flag, not to the request -- and this pass runs BEFORE the first
data block, so on such a clone it is where the refusal first arrives.

ADDRESSED as well. These had the strongest claim to it of anything left: WRITE
AFI and WRITE DSFID are STANDARD commands, so a bystander of any size takes
them, and a changed AFI can drop that tag out of a selective inventory -- a card
that then looks absent to the reader that uses it.

That closes the write half of #251. The backdoor sequences stay unaddressed, and
the post-wipe UID re-read remains unfixable by addressing.

The file's own list of what remains unaddressed is corrected with them. It had
counted these two among the unaddressed frames and called them the strongest
case for addressing with nothing behind it -- which this commit is. It also said
the identity pass stays unaddressed, twenty lines from the code that addresses
it.

The gen1 backdoor's entry is now an OPEN QUESTION rather than a decision, and
says why the obvious defence of it does not hold. Addressed WRITE BLOCKs to
56/57/62/63 are already measured to work: the wipe's sweep zeroes exactly those
addresses through the addressed path, re-addressing when the UID moves under it,
and the clone's conversion path writes 56/57 addressed and watches the UID
follow. So "it is the magic sequence" is not on its own a reason to leave it
open to a bystander. What is genuinely unmeasured is narrower -- unlock and
commit ACCEPTED addressed, which no run has shown because an armed card refuses
them either way, and the fact that this sequence is fire-and-forget while
addressing it would need an inventory between frames.

Nothing was needed here for the missing acknowledgement. write_identity never
trusted the send's return, because the SDK has no response parser for these two
commands, so it was already built around a GET SYSTEM INFO read-back.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
