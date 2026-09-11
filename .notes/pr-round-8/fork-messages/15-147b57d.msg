NFC Magic ISO15693: name the bitmap's block range once, instead of spelling it four times

ISO15693_POLLER_BLOCK_BITMAP_SIZE * 8 appeared at four sites -- the wipe ceiling,
both halves of the clone clamp, and the details scene's list bound -- all meaning
the same thing: the block range the failure bitmap can address.
ISO15693_POLLER_WIPE_MAX_BLOCKS already names it, but wipe-scoped, so the clone
and the scene could not use it and re-derived instead.

Suggested in review, and it cuts mildly against 44617f89, so here is why it goes
in anyway. That preference is about not naming a value whose expression already
reads clearly at its one site. This is the opposite case: four sites, and the
expression does NOT say what it means -- a scene computing BITMAP_SIZE * 8 to
bound a list is reaching into the poller's storage layout to recover a protocol
fact. The name is the fact; the multiplication is an implementation detail that
happens to equal it.

ISO15693_POLLER_MAX_BLOCKS goes in the header beside BLOCK_BITMAP_SIZE, which is
where the relationship between them lives. WIPE_MAX_BLOCKS stays and is now
defined as it, keeping the 21 lines of hardware argument for using that bound as
the sweep's ceiling -- the 2026-08-04 measurement, proxmark's matching loop, and
the inertness of writes above capacity. That argument is about the WIPE and would
be wrong to hoist.

So: one arithmetic site, two names for one number where the second carries an
argument the first does not need.

Code change, so it is verified rather than reasoned: both firmwares rebuilt from a
cleared object dir and warning-free, and clang-format reports the whole tree clean
(the define's alignment is its own doing).

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
