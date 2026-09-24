NFC Magic ISO15693: the gen1 opt-in's grant is for one run, not for the mode

BEHAVIOURAL. Bench-confirmed symptom and bench-confirmed fix; see below for what
that covers and what it does not.

iso15693_force_gen1 is set in exactly one place, the gen1 opt-in, and was cleared
in exactly one place, the ISO15693 menu. Retry is scene_manager_previous_scene --
straight back into the write scene, which re-runs on enter and reads the flag
still set. So every route back in that bypasses the menu repeated the destructive
gen1 write without asking.

That is worse than a missing prompt, because Retry does not re-identify the card
and the case Retry exists for is CardLost. Staged on the bench: consent given on
card A, card A removed, card B placed on the coil, Retry pressed -- card B took
four ordinary WRITE BLOCKs into 56/57/62/63 having never been offered the screen.
With this, gen2 runs first on B, fails, and the opt-in appears for B.

The write scene consumes the grant on enter. Cleared above the branches rather
than in them, because the wipe arm never reads it and a wipe reached while a
grant was standing would otherwise pass it on.

Its reach is narrower than "every Retry", which is worth stating rather than
leaving implied. On the SAME magic card after a successful UID write the branch
is unreachable: the opt-in fires only when gen2 leaves the UID unchanged and
wrong, and once the gen1 write has landed that is false for that target, so Retry
finds the UID already correct and owes no consent. The cases that reach it are a
card that never takes the UID, and a Retry landing on a different card.

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
