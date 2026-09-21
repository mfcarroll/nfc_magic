## Round 11 - every round-10 item addressed, and one of them sent you back to the bench

All seven are in. The one I flagged as the priority - the no-latch result stated for "gen1
silicon" when one armed chip had shown it - you did not fix by re-wording. You armed one card
of each of the three chips and re-measured, and then added a distinction I had not asked for:
on the LRi2K the 62/63 refusal is IN BAND (error 0x10), while on the other two the client could
not separate an error frame from silence, "so 'the registers answer' is one chip". That is the
right way to hold a measurement.

The rest: all four mangled wraps fixed, the consent-screen note written and useful, the
"clone" mislabel corrected, and the dead conjunct kept with a reason I accept - the scene
cannot enforce an invariant about a struct it did not fill.

I had every commit checked for hidden code changes by stripping comments at each parent and
child and diffing. **All seven are provably comment-only, and because the strip preserves
string literals byte-for-byte, no rendered string changed anywhere in this delta.** The
round-8 regression class is now structurally closed as well: the ISO15693 wipe title sits
inside a branch gated on the protocol, so it cannot reach the USCUID-UL screen again.

Build clean on API 88.6, `ufbt format` clean.

## What the trim cost

`9b80e348` is net -146 lines and mostly argumentation, which is the right thing to cut. Three
deletions did damage, and two land in the gen1/gen3 hazard text.

**`CHANGELOG.md:107` is the one I would fix first.** It now says the gen1 opt-in writes "reach
the same blocks" as the gen3 registers a wipe bricks. They do not - gen3 keeps its UID at
0x10/0x11 and its signature at 0x14/0x15, the opt-in writes go to 56/57/62/63, and on a gen3
card those four are ordinary user data. The same commit deleted the only statement of those
addresses in the repo, so the claim got stronger and unfalsifiable in one edit. The wipe really
can brick a gen3 card; the opt-in cannot, and the pre-trim wording said so.

**A fact went from one place to zero**: "the four backdoor addresses are not memory ... 56/57/62/63
are write-only registers sitting outside it". That is measured hardware behaviour, nothing else
records it, and it is what makes both the 49-block threshold and the "Wiped 58/58" reproduction
on a 56-block card make sense.

**`CHANGELOG.md:21`** now promises "reports whether it moved" - a binary answer the code cannot
give. You wrote the correct version into a scene in the same push: "It cannot report the absence
of a move."

## The reach rule, fourth round

`iso15693_poller.c:1253` models one of the two gates that end the sweep. `:963` shows the other:
below the advertised count the run trip falls through to `continue`, so the last block attempted
is `max(A + 7, claim - 1)` and 56 is reached from `A >= 49` **or** `claim >= 57`. A card that
answers no read but advertises 57+ does reach 56/57, and this paragraph says it cannot - the
wrong direction, on the one path where the UID check never runs. The CHANGELOG has the same rule
wrong in the opposite corner.

I have written out the closed form in the inline comment. Rather than patch the sentence a
fourth time I would state it once in the poller and let the CHANGELOG carry only the qualitative
half, which is the half that cannot be wrong.

## Nothing blocking

No code changed this round, no string changed, the consent text is intact, and the behavioural
surface has been stable for three rounds.

