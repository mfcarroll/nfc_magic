# #255 — a correction to the body posted 2026-09-30 (DRAFT, NOT POSTED)

Found while writing the ISO15693.md draft, after #255's title and body were edited and its comment
posted. The body's "Where this comes from" paragraph says a gen3 card's owner sees "only the generic
'Wipe card?' confirm". That has been false since 2026-09-08: an ISO15693 wipe's confirm is titled
"Wipe? (gen1/gen2 only)" and ends "This can brick a gen3 card!"
(`scenes/nfc_magic_scene_write_confirm.c`, already in `1d411dec`, the build pushed before round 15).
The sentence came
from round 6's draft, and the round-15 correction was checked against the wipe's poller note and the
2.3 Validation bullets, not the confirm screen.

**Order:** an edit to the body only, on mfcarroll's go-ahead. An edit notifies no one, so it needs no
comment of its own; the next post on #255 can mention it if one is ever made.

## The paragraph as posted

~~~~
Raised during review of #250 (ISO15693 / NfcV support). That PR declares gen3 unsupported in its
CHANGELOG, including that a wipe overwrites those blocks — which warns someone who reads release notes,
but not someone who picks Wipe with a gen3 card on the reader. They see only the generic "Wipe card?"
confirm.
~~~~

## The paragraph as it should read

~~~~
Raised during review of #250 (ISO15693 / NfcV support). That PR declares gen3 unsupported in its
CHANGELOG, including that a wipe overwrites those blocks, and the wipe's confirm screen carries the
same warning: "Wipe? (gen1/gen2 only)", over "This can brick a gen3 card!". A warning is all it can
be, though. The wipe performs no magic detection, so the screen reads the same whatever card is on
the reader and cannot say whether this one is gen3.
~~~~

## Also in the comment, optional

The posted comment ends "so nothing but that probe stands between it and a wipe". The confirm screen
does stand between the card and a wipe, as a warning every wipe shows; what nothing but the probe can
do is tell this card from any other. Tighter:

~~~~
This card carries the exact config-mode signature proxmark checks before `cfinalize`, so nothing but
that probe can tell it from any other card before a wipe.
~~~~
