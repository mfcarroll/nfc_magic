## Round 9 - 16 for 16, and three of them went past what I asked for

Every item from round 8 is addressed. I traced each one rather than taking the commit titles
on trust, and the three that stood out:

- **#251.** I flagged that the gen1 sequence bypasses `write_block_retried`. You moved the
  whole scope onto the `ISO15693_MAGIC_FLAGS` define with a per-command blast radius, and
  found something I had missed: **WRITE AFI / WRITE DSFID are standard commands**, so unlike
  the proprietary backdoors they reach a bystander of any size, and a changed AFI can drop a
  tag out of selective inventory. I verified every hand-built frame in the file - `:321`,
  `:339`, `:437`, `:444` all open with the unaddressed flag and append no UID.
- **The field count.** Rather than correcting 28 to 27, you dropped the count and named the
  four fields `start_internal` skips with the reason for each. That is the more useful artifact.
- **The `<= 7` bound.** Scoped, the mechanism corrected to the tripped-run handling, and it
  now explicitly repudiates both wrong versions - including the one I supplied.

The regression fix is clean: I enumerated all four paths into the confirm scene and only the
USCUID-UL title changed back. Build clean on API 88.6, `ufbt format` clean, every
cross-reference resolves, and `test_write_fail_scene.c` is gone with nothing invented to
replace it.

## What this round introduced

Three of these are the same shape: a correction that overshot, or that fixed one copy of a
fact and left another.

**`iso15693_poller.c:1252` and `CHANGELOG.md:108`** are the one I would fix first. The fix for
"the sweep sent three WRITE BLOCKs each at 56 and 57" replaced an unconditional claim with a
different unconditional claim - and the new one is false for precisely the card the sentence
is about. A card that refuses every write but answers every read never accumulates an absent
run, so it walks to the ceiling or the clock no matter what it claims. In the CHANGELOG the
two halves of one bullet now contradict each other four lines apart.

**`iso15693_poller.c:1379`** still carries "nothing ever clears it" - the exact sentence
`00892750` was written to remove - 510 lines from its own replacement, and inside the state
that runs *after* the sweep that zeroes commit.

**`CHANGELOG.md:161`** kept the pre-fix #251 scope: "A wipe", "The WRITE BLOCK frames". The
source now says every write the app sends, including the clone's AFI/DSFID pass. The two items
missing from the notes are the two a user would most want.

## One of these is mine

`nfc_magic_scene_write_confirm.c:81` says "in this app those words name the MIFARE Classic
protocols". That is my round-8 wording and it is too strong - gen1/gen2 are overloaded, and the
ISO15693 sense is the one in force on that very screen. If the premise held, the title it
defends would be wrong where it appears. The fix is right; only the rationale needs rewording,
and that is on me.

That is twice now my own phrasing has gone into the code and needed correcting. Worth saying
plainly rather than letting it look like your error.

## Where this leaves the PR

Nothing blocking. Everything above is prose accuracy plus two leftovers, and the behavioural
surface has been stable for two rounds. I have no outstanding concern about what this code
does - only about what its comments claim, which is a much better place to be than round 6.

