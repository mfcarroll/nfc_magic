# Round 14 — main reply (DRAFT, NOT POSTED)

Post between the `~~~~` markers. **No internal process in the payload.**

Answers his two comments covering `a27d187d..749f10e6` (twelve commits, four behavioural).

~~~~
## Your commits, and what the bench says

Every one of the twelve is in, unrebased and unreworded.

**All three behavioural ones are benched and pass.** `0c5a7d69`: card lifted during the consent
screen, and the failure screen no longer claims 56/57/62/63 were spent. `62b60e2b`: the cursor stays
where it was left. `4d20f06e`: a hand-built source claiming 257 blocks, cloned onto the one gen2 card
here that accepts unaddressed writes — Info reads `256 blocks x 4 bytes` afterwards, where unclamped
the cast wraps and it would read 1. The clone's own line corroborates it: "Holds 64/256, top 192 were
empty" means the pass attempted 256, so the source count was clamped too.

**The fourth needed no bench, and I checked it rather than taking it.** `749f10e6` holds up at
every step against the firmware source: `send_frame` ends in
`bit_buffer_copy(rx_buffer, instance->rx_buffer)` after the CRC trim; `instance->rx_buffer` is
`ISO15693_3_POLLER_MAX_BUFFER_SIZE`, 64; `bit_buffer_copy` is `if(buf == other) return;` then
`furi_check(capacity_bytes * 8 >= other->size_bits)`, a halt; and **all five** in-tree callers pass
`instance->rx_buffer` as destination and source, so that check has never been live in the firmware.
Our three raw senders really are the first callers anywhere to hand it a foreign, smaller buffer.

On `0c5a7d69` — the part you flagged as worth attention is the right part. The struct comment
enumerates the two paths that return from `Start` before the send, and that enumeration is correct.
It just presupposes `Start` is entered. I wrote that enumeration and did not see past its own frame
either.

## What I have added on top

Four of these are the simplification pass, which your twelve did not touch:

- **the clock cut**, written out in the clone's loop and the wipe's sweep with half the reason at
  each. One helper, one write point for the two fields that gate Retry in seven places.
- **the block-size clamp** — which your `4d20f06e` turned from two callers into three. The helper
  says why they are not one case: the clone's and the CFG's arrive from a loaded `.nfc`, the wipe's
  off the wire in a five-bit field and cannot fire. Your point that the CFG one matters most, because
  those bytes outlive the run, is the reason it leads.
- **the succeeded-blocks figure**, subtracted on three lines with no reason at any of them. It points
  at `blocks_total`'s contract in the header rather than restating it, since your version of that
  contract is the precise one.
- **the sweep's attempted-count invariant**, stated three times in three vocabularies, one of them
  needing a see-also to another exit. Hoisted to the loop head, with the part no site carried: the
  deadline check is the one break that does not increment.

Plus one small thing in the same file: its LINE BUDGET note explained both y values and never the x,
so the two bodies at `(4, 20)` read as an inconsistency. They are not — that pair appears in exactly
three files in the repo, and the two branches here are the ones mirroring the other two screens. The
note now says so, because the tempting fix is to make all twelve agree and break the match.

## A correction back to you

One, on the reach rule, and it is in the thread where you raised it rather than repeated here:
`5c97e16f` needs a third gate, and `A` is the first block of the final unbroken run rather than the
first silence anywhere. Both omissions under-warn.

## Ours, and something to raise

**A consent defect, found while explaining a bench observation that turned out to be innocent.** A
second write of the same file gave no gen1 consent — correctly, because the first write had already
moved the card's UID to the source's, so gen2 found it matching. Tracing why exposed something real
underneath: `iso15693_force_gen1` was set at the opt-in and cleared only at the ISO15693 menu, and
Retry is `scene_manager_previous_scene` straight back into the write scene. So any route back in that
bypassed the menu repeated the destructive gen1 write without asking.

It matters because Retry does not re-identify the card, and the case Retry exists for is CardLost.
Benched end to end: consent on card A, remove it, put card B on the coil, hit Retry — before, B took
four ordinary WRITE BLOCKs into 56/57/62/63 having never been offered the screen. The write scene now
consumes the grant on enter, so a Retry earns consent again against whatever card is actually there.

Its reach is narrower than "every Retry", which is worth saying rather than leaving implied: on the
same magic card after a successful UID write the branch is unreachable, since the opt-in fires only
when gen2 leaves the UID unchanged and wrong. The cases that reach it are a card that never takes the
UID, and a Retry landing on a different card.

This is the first behavioural change from my side rather than prose, so: bench-confirmed symptom,
bench-confirmed fix, four harness tests, mutation-checked.

**And one for you rather than for this PR.** The protocol menus keep their cursor across a whole new
card scan, not just within one — write UID, success, Check Magic Tag, More, and the cursor is still
on Write UID. It is app-wide: all six set scene state on event, read it on enter and never clear it,
and `magic_info.c` is the single place a fresh scan dispatches, so it would be about six lines for
all six or none.

The argument for changing it is consistency with the app's own behaviour rather than taste. The
scene state is only ever written by the user's own menu selection, so on first load it is 0 and the
first scan lands on the first item. Every scan after that lands wherever they last were. **The same
action produces a different cursor depending on whether it is the first scan of the session** — and
whichever position is right, it should be the same one both times.

That also disposes of the objection I had: index 0 is `Write` in all six menus, so resetting looked
like it would put the cursor on the destructive item. It already does, on the first scan. Resetting
adds no exposure — it makes scans two onwards behave like scan one. And it does not cut against
`62b60e2b`, which is about returning from Info within one flow; that is a different axis.

Leaving it to you because it is shared app code in a PR about ISO15693, not because I think it should
stay.

## Where this stands

**Still not ready to merge**, and for the same reason as before: every ISO15693 write this app sends
is unaddressed, and TI Tag-it HF-I Plus refuses unaddressed WRITE BLOCK outright. Addressed writes
and the gen1 caveat gate that goes with them are the next piece of work, and they belong in this PR —
shipping ISO15693 support that cannot write a TI Tag-it is not something to fix later.
~~~~
