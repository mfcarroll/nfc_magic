# Round 14 — thread replies (DRAFT, NOT POSTED)

Seven threads from his round-12 review. **He fixed all seven himself** in the twelve commits he
pushed, so these close the loop rather than report work. Only the reach rule has anything added.

Post each payload between its `~~~~` markers, on the comment id named.

---

## `CHANGELOG.md:107` — comment 4035110553

~~~~
Fixed by your own `2b64c380`, and taken as written. Damage from the opt-in, bricking from the wipe,
with both address sets named rather than referred to by position.

The shape is the part I want on the record: the same edit made the claim stronger and deleted the
only record of the addresses that falsify it. Unfalsifiable and wrong arrived together, which is a
failure mode worth watching for in any trim rather than a fact about this one.
~~~~

---

## `CHANGELOG.md:21` — comment 4035110565

~~~~
Fixed in `21419c30`, taken as written.
~~~~

---

## `iso15693_poller.c:1253` — comment 4035110573

~~~~
`5c97e16f` is right in its gates, its derivation and its disjunct framing, and I have built on it
rather than replaced it. Two things were missing, both in the under-warning direction.

There is a **third gate**: the run has to survive the re-probe that follows the trip. Your text
already says `wipe_note_present` zeroes the run from the read path and the re-probe as well as from a
landed write — the mechanism is there, but "A the first block that answers nothing" cannot stand
beside it. A is the first block of the FINAL unbroken run, and the third gate is what makes it
terminal.

It bites on a card that recovers: one **advertising 56** that reads 0-4, goes silent at 5, reads 6-59
and is silent from 60 reaches 56 and 57, while `A = 5` with `claim = 56` makes both disjuncts false.
The claim has to be named or the example proves nothing — at any higher claim the claim term carries
it regardless.

The 256 ceiling is in the form now as well; it binds the A term alone.
~~~~

---

## `iso15693_poller.c:855` — comment 4035110582

~~~~
Fixed in `0edfbad9`, taken as written — including at the site inside `wipe_blocks`, which is the
load-bearing one for the reason you gave.
~~~~

---

## `CHANGELOG.md:99` — comment 4035110593

~~~~
Restored in `bb937361` and then scoped in `88d757b5`, both taken as written.

Worth saying that I had found the same problem with the restored clause independently and resolved it
worse — I deleted "the advertised count is its capacity" outright. Scoping it to the three gen1 chips
and naming the programmable case keeps the measurement rather than losing it, and it reads as one
rule instead of two halves.
~~~~

---

## `nfc_magic_scene_iso15693_write_fail.c:70` — comment 4035110599

~~~~
Fixed in `21419c30`, taken as written.
~~~~

---

## `CHANGELOG.md:42` — comment 4035110604

~~~~
Fixed in `21419c30`, taken as written.
~~~~
