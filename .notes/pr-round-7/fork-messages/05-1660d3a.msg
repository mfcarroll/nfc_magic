NFC Magic ISO15693: three facts in the poller that had two homes each

  - the write-retry rationale was in ISO15693_POLLER_WRITE_ATTEMPTS and again,
    near-verbatim, in write_block_retried. The constant owns it. Fixing that
    exposed a drift in the constant itself: it said "a failed clone WRITE
    BLOCK" while both block passes use it, which is exactly the claim
    write_block_retried's own first line contradicts.
  - the fake-flash counting rule (a card claiming 66 against 64 physical still
    drops 64/65) was stated in WIPE_ABSENT_RUN and at the sweep's absence
    handling. The tail-drop implements it, so the sweep owns it and the
    constant now says which rule does the work -- which is the more useful
    thing for a reader of that constant anyway, since it is NOT this number.
  - the AFI/DSFID command defines restated write_identity's purpose.

Comment -1, code +0. Barely a cut by line count, and worth doing anyway: each
of the three was a fact with somewhere to drift to, and one had already drifted.
101 host tests before and after.

