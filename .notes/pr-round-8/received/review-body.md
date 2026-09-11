## Round 8 - all 20 addressed, and most of them exactly

I checked each of the round-7 items individually rather than taking the commit titles on
trust. The standouts: the `COUNT_OF` rationale now has the direction AND the arithmetic right
("EIGHT iterations over a four-element array, touching indices 4..7"); the `cut_block` doc
drops "opposite directions" for the advertised-count reframe; and the boundedness comment now
explicitly repudiates its own previous reasoning ("NOT 'you cannot reach the claim after that
many absences' -- you can"). The PASS_MAX_MS block took all four corrections including the two
small ones. `blocks_total` now reads the same in all three places. The switch order matches the
render chain position for position.

Structurally: the clone tail is `finish_write`, the three UID formatters are one helper with
named policies, `alloc` is down to what must hold before a start, and the dead conjunct is
gone. I verified the UID helper's four call sites produce byte-identical output to what they
replaced, including the `i == 3`-after vs `i == 4`-before pair, and that nothing reads the
struct between `alloc` and `start_internal` now that eight initialisers are gone.

Build clean on API 88.6, `ufbt format` clean, and the 2.3 versioning from the merge held.

## One regression, and it is outside this feature

`nfc_magic_scene_write_confirm.c:30`. The new wipe title keys off `is_wipe`, which is
`uscuid_ul_is_wipe_mode || iso15693_wipe`. The gen3 body is correctly protocol-gated; the title
is not. So a magic Ultralight wipe now reads **"Wipe? (gen1/gen2 only)"** over "Blank factory
dump: config & password cleared, UID zeroed." That flow lives in `nfc_magic_scene_uscuid_ul_menu.c`,
which this PR does not touch. One token to fix.

## The one I would fix first

`iso15693_poller.c:494` - "Every block write in this file funnels through here" is false, and
it is load-bearing because the #251 hazard note is scoped to it. The gen1 sequence sends four
raw WRITE BLOCKs through `send_frame`, built with `ISO15693_MAGIC_FLAGS`, which line 21 of the
same file defines as "unaddressed". So an opt-in gen1 run also reaches a bystander tag, at
56/57/62/63. #251's blast radius is larger than the comment that tracks it.

## The pattern this round

Four comments contradict another comment in the same delta, and in three of those the correct
version is the newer one sitting a few lines away:
- `write_fail.c:276` still asserts "The UID was never touched", which `e71dce03` added
  `iso15693_poller.c:1238-1244` specifically to retract - and you edited that same comment
  block this round.
- `iso15693_poller.c:260` vs `:279`, nineteen lines apart in one struct.
- `iso15693_poller.h:166` vs `:173`, six lines apart in one field doc.
- `iso15693_poller.c:855` vs `:853`, two lines apart.

The consolidation pass also produced its predicted failure: `write_fail.c:180` deletes prose on
the premise the header owns the fact, and points at a header that states the reading the
pointer disclaims.

And `9c14213d` pasted `finish_write` into the middle of `write_step`'s two-line doc comment, so
neither function is correctly documented now. That one is purely mechanical - the extraction
itself is right.

## Tests

`partial_details.c:94` cites `test_write_fail_scene.c`. That file is not in the PR, not in the
repo, and has never been added on any branch; the PR adds 27 files and none is a test.

I can see you have a real harness - 15 of this round's 19 commits cite it, the counts move
coherently (101 -> 106 -> 107), and "mutation-verified: restoring `<=` fails the new case" is
exactly the right kind of evidence. The problem is that none of it is reviewable here, and a
shipped comment now points a future maintainer at a file this repo will never contain.

Not a blocker, and the ask is the generous version rather than the strict one: **contribute the
harness**. This app has zero tests today. A mutation-verified suite for the cut arithmetic and
the UID formatter would be worth more to this codebase than any of the comments describing it,
and it would make the round-over-round claims checkable instead of asserted. If that is out of
scope for this PR, then the citation has to go.

Nothing blocking beyond the regression, which is a one-token fix.

