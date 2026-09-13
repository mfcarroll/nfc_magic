# Round 10 — thread replies (DRAFT, not posted)

Eleven threads. Post each payload between its `~~~~` markers, on the thread named.

Rules applied: disposition, anything HE got wrong, anything WE found. No restating his reasoning,
no describing the diff, no process narration, no dev-repo SHAs — commit subjects only.

---

## 01 — `iso15693_poller.c:1252` — comment 3995478934

~~~~
Fixed in "the sweep reaches 56/57 unless the card is BOTH silent and small". The qualifier is in,
and the four descriptions of this card in the tree now use one wording rather than five.

Two things since, both changing this sentence again. The self-review found a second exception
neither version had: the geometry guard returns above the loop before the first write, so 56/57 can
be missed because nothing was *asked* rather than because nothing answered, and that case lands here
too since it returns 0.

Then the bench run moved the threshold. **"Fewer than 57" is wrong — it is 49**, and the difference
is the absent-run tolerance: the sweep keeps writing past the claim until eight blocks answer
nothing, so a card silent from A is attempted through A+7, and a landing write resets the run, which
is why 57 goes with 56 rather than one claim later. Measured, 44 through 52, and pinned by two tests
at 48 and 49. The band it got wrong is 49-56, called safe when the sweep reaches the UID registers.

That figure came from this thread and I took it as read. It only became visible because there are
now gen1 cards that are not 56 blocks.
~~~~

---

## 02 — `CHANGELOG.md:108` — comment 3995478935

~~~~
All three fixed in the same commit as the source side. The bullet takes the poller's wording for the
no-usable-geometry case and states it as the separate exception it is, rather than folding it into
the run condition.
~~~~

---

## 03 — `iso15693_poller.c:1379` — comment 3995478938

~~~~
Fixed in "the stale clears-commit claim, 522 lines from its own replacement". Removed rather than
restated — two copies of a mechanism is what let these drift, so this one points at the owner.

The same state carried a second stale claim that the gen1 round then made false: "on the only
hardware this PR has, this is a regression test that should never fire." There is a gen1 card now
and that check has fired on it deliberately. It is about gen2 cards, not about what is on the bench,
so it says that instead — a claim about which hardware exists goes stale without anything noticing.
~~~~

---

## 04 — `CHANGELOG.md:161` — comment 3995478939

~~~~
Fixed in "#251 is every ISO15693 write, in the release notes too". Both missing categories are in,
the lead no longer says "A wipe", and the AFI/DSFID item carries the selective-inventory
consequence, since that is the part a user cannot infer.
~~~~

---

## 05 — `nfc_magic_scene_write_confirm.c:81` — comment 3995478942

~~~~
Reworded to scope in "the confirm title's rationale is scope, and is_wipe stops pretending". **And
the title changed too** — it is "Wipe ISO15693 tag?" now, naming the protocol rather than a
generation.

That is your closing point taken the rest of the way: the premise did not hold, so the title it was
defending was wrong. The self-review arrived at the same place from the other side — a generation in
the title reads as a gate, one line above a body saying a gen3 card is what gets destroyed, and the
wipe does no magic detection at any point to back it.
~~~~

---

## 06 — `iso15693_poller.c:1505` — comment 3995478945

~~~~
Fixed in "two figures that outlived what anchored them" — the number is gone and the named fields
stand on their own.

The second half of that commit answers your note on the block-ceiling claim in the header.
~~~~

---

## 07 — `iso15693_poller.c:23` — comment 3995478948

~~~~
Both scoped in "scope the #251 note to ISO15693 writes, and stop the pointer over-claiming". It is
"every ISO15693 write this app sends" now, and the frame claim is narrowed to writes rather than
leaning on the shipped-headers disclosure — the four hand-built frames are the four that can be
checked from what ufbt ships, so that is what it claims.
~~~~

---

## 08 — `iso15693_poller.c:505` — comment 3995478949

~~~~
Both fixed in the same commit. The pointer no longer attributes, and the second sentence names the
gen1 backdoor *sequence* rather than the four block numbers.
~~~~

---

## 09 — `nfc_magic_scene_iso15693_write_fail.c:275` — comment 3995478951

~~~~
Restored in "restore why the NothingWiped branch exists, without the UID claim", as its own clause.

This is the shape I have least defence against: nothing in the result looks wrong afterwards, so
nothing prompts a re-check. The guard is to read what a sentence does *besides* carry the false
claim before cutting it, and that is now the rule rather than the incident.
~~~~

---

## 10 — `nfc_magic_scene_write_confirm.c:24` — comment 3995478954

~~~~
Dropped in the same commit as the confirm-title rationale. `is_wipe` is `uscuid_ul_is_wipe_mode` at its only use site
and now says so.
~~~~

---

## 11 — `iso15693_poller.h:15` — comment 3995478957

~~~~
Fixed in "two figures that outlived what anchored them" — it claims every fixed block ceiling now,
which is the claim that holds.
~~~~
