# Round 10 — thread replies (DRAFT, not posted)

Eleven threads. Post each payload between its `~~~~` markers, on the thread named.

Rules applied: disposition, anything HE got wrong, anything WE found. No restating his reasoning,
no describing the diff, no process narration, no dev-repo SHAs, and **no thread numbers — those are
ours, not his.** Every cited commit subject is checked to exist against the rebuilt round: it is
grouped by SUBJECT, so several threads land in one commit.

---

## 01 — `iso15693_poller.c:1252` — comment 3995478934

~~~~
Fixed in "the wipe sweep, an armed gen1 card, and how far past the claim it reaches". The qualifier
is in, and the four descriptions of this card in the tree now use one wording rather than five.

Two things moved it again after that. There is a second exception neither version had: the geometry
guard returns above the loop before the first write, so 56/57 can be missed because nothing was
*asked* rather than because nothing answered, and that case lands on this branch too since it
returns 0.

Then the bench run moved the threshold. **"Fewer than 57" is wrong — it is 49.** The difference is
the absent-run tolerance: the sweep keeps writing past the claim until eight blocks in a row answer
nothing, so a card silent from A is attempted through A+7, and a write that lands resets the run,
which is why 57 goes with 56 rather than one claim later. Measured across 44 to 52 and pinned by
tests at 48 and 49. The band it called safe is 49-56 — cards where the sweep does reach the UID
registers.

That figure came from this thread and I took it as read. It only became visible because there are
now gen1 cards that are not 56 blocks.
~~~~

---

## 02 — `CHANGELOG.md:108` — comment 3995478935

~~~~
All three fixed, in "gen1 is validated across three chips, and four claims the code no longer
supports". The bullet takes the poller's wording for the no-usable-geometry case and states it as
the separate exception it is, rather than folding it into the run condition.

The threshold correction lands here too: the passage now says the sweep reaches 56/57 only from a
claim of 49 or more, and which of the cards tested that is.
~~~~

---

## 03 — `iso15693_poller.c:1379` — comment 3995478938

~~~~
Fixed in "the wipe sweep, an armed gen1 card, and how far past the claim it reaches". Removed rather
than restated — two copies of a mechanism is what let these drift, so this one points at the owner.

The replacement you quoted has since been corrected too, in the opposite direction. "This sweep does
reach commit and zero it" got the reaching half right and the zeroing half wrong: an armed card
refuses 62/63 with error 0x10. The armed-card wipe reported "Wiped 58/58" on a card advertising 56,
so 56 and 57 were accepted while 62 and 63 were attempted and refused — and the card was still armed
afterwards. The ORDER clause went with it, since ordering decides nothing when commit cannot be
cleared at all.

The same state carried a second stale claim: "on the only hardware this PR has, this is a regression
test that should never fire." There is a gen1 card now and that check has fired on it deliberately.
It is about gen2 cards, not about what is on the bench.
~~~~

---

## 04 — `CHANGELOG.md:161` — comment 3995478939

~~~~
Fixed in "gen1 is validated across three chips, and four claims the code no longer supports". Both
missing categories are in, the lead no longer says "A wipe", and the AFI/DSFID item carries the
selective-inventory consequence, since that is the part a user cannot infer.
~~~~

---

## 05 — `nfc_magic_scene_write_confirm.c:81` — comment 3995478942

~~~~
Reworded to scope, in "is_wipe stops pretending, and two shared fields stop being labelled for one
protocol". The vocabulary claim is gone.

The title itself is unchanged, and I nearly changed it. The self-review read "(gen1/gen2 only)" as
promising a gate the wipe does not have, since it performs no magic detection at any point. That was
wrong and it is reverted: the title and the bolded "can brick a gen3 card" above the button are one
instruction — which cards to use it on, and what happens otherwise. A restriction the user applies,
not a gate the app enforces. The comment says that now rather than leaving the next reader to work
it out.
~~~~

---

## 06 — `iso15693_poller.c:1505` — comment 3995478945

~~~~
Fixed in "nine claims in the poller that the code beside them contradicts" — the number is gone and
the named fields stand on their own.
~~~~

---

## 07 — `iso15693_poller.c:23` — comment 3995478948

~~~~
Both scoped in "nine claims in the poller that the code beside them contradicts". It is "every
ISO15693 write this app sends" now, and the frame claim is narrowed to writes rather than leaning on
the shipped-headers disclosure — the four hand-built frames are the four that can be checked from
what ufbt ships, so that is what it claims.

The parenthetical added alongside it, naming the 14443-A writes as addressed and out of scope, has
since come out: the sentence already scopes itself to ISO15693, so it said the same thing twice.
~~~~

---

## 08 — `iso15693_poller.c:505` — comment 3995478949

~~~~
Both fixed in "nine claims in the poller that the code beside them contradicts". The pointer no
longer attributes, and the second sentence names the gen1 backdoor *sequence* rather than the four
block numbers.
~~~~

---

## 09 — `nfc_magic_scene_iso15693_write_fail.c:275` — comment 3995478951

~~~~
Restored as its own clause, in "a wipe that loses the card never said the identity check had not
run".

This is the shape I have least defence against: nothing in the result looks wrong afterwards, so
nothing prompts a re-check. The guard is to read what a sentence does *besides* carry the false
claim before cutting it.
~~~~

---

## 10 — `nfc_magic_scene_write_confirm.c:24` — comment 3995478954

~~~~
Dropped in "is_wipe stops pretending, and two shared fields stop being labelled for one protocol".
`is_wipe` is `uscuid_ul_is_wipe_mode` at its only use site and now says so.
~~~~

---

## 11 — `iso15693_poller.h:15` — comment 3995478957

~~~~
Fixed in "three header contracts that the scenes state more carefully than the header" — it claims
every fixed block ceiling now, which is the claim that holds.
~~~~
