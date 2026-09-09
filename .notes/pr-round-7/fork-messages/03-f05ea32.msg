NFC Magic ISO15693: the constants stop deriving each other's figures

The four constant blocks are mostly legitimate owners -- the 2026-08-04
36-of-64 measurement, the edgepages probe suite, the view_dispatcher queue
deadlock, the 40-70ms bench range are all things the code cannot show. Those
stay. What they were doing wrong is deriving each other:

  - the per-block cost (3 writes, 3 waits, 1 read) was worked out three times:
    in WIPE_ABSENT_RUN, which has the bench detail and owns it, and twice more
    in PASS_MAX_MS. PASS_MAX_MS now cites it.
  - the not-instrumented caveat was stated in both. WIPE_ABSENT_RUN keeps it;
    PASS_MAX_MS keeps only the part that is about PASS_MAX_MS, that estimates
    argue for a wide margin.
  - "the gen2 CFG frame programs what the card advertises" is the header's
    blocks_advertised now, so WIPE_MAX_BLOCKS points there and keeps the
    hardware measurement that is its own reason to exist.
  - "a run cut here reports what it covered and says it was cut" is the
    header's pass_truncated, stated better there. Dropped here.
  - WIPE_ABSENT_RUN's "it never ends the sweep below the advertised count" is
    start_wipe's contract; reduced to a parenthesis so the paragraph still
    makes its own point, that the run length is not the only guard.

wipe_blocks' own header restated start_wipe's contract in its first four lines
before getting to what it actually decides. It now names its owners and leads
with the read-back classification table, which is genuinely its own. Its gen1
paragraph dropped the "a wipe cannot ARM a change by itself" half, because the
OPEN QUESTION twelve lines below says exactly that.

Comment -10, code +0. The constants were not where the mass was.
101 host tests before and after.

