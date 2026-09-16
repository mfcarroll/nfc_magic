# Round 11 — thread replies (DRAFT, not posted)

Seven threads. Post each payload between its `~~~~` markers, on the comment id named.

Rules applied: disposition, anything HE got wrong, anything WE found. **No internal process** — not
when something was cut, not that an earlier commit in this push already removed it, not that we
broke and restored something he never saw broken. He reads the delta between the round he reviewed
and the next one. "Cut" is a complete answer.

Also excluded, as ever: restating his reasoning, describing the diff, dev-repo SHAs, thread numbers.
`tools/` and `.notes/` never reach the fork, so the host harness is not a file he can open.

---

## `iso15693_poller.h:83` — comment 4018550915

~~~~
Fixed, and by measurement rather than by narrowing the sentence. You were right that the three-chip
result cannot carry this: set, read-back and restore all run through a power-cycle, so only the
same-session inventory shows immediacy.

It has now been run on one card of each chip — the ST LRi2K, an NXP ICODE SLIX-S at IC ref 0x02 and
an NXP ICODE SLIX at IC ref 0x01 — same frames, same UID, field held up with `-k`. All three return
the newly written UID with no power-cycle, and both new cards restored byte-identically afterwards.
The immediacy result now rests on the same sample as the sentence beside it. Three chips, not the
family, and the line says so.

**One claim deliberately did not widen.** On the LRi2K the 62/63 refusal is in band, `01 10 1E 06`.
On the other two the client reported a plain command failure, which does not distinguish an error
frame from silence. The refusal itself replicated on all three; "the registers answer" is one chip,
and the comment keeps those apart.

The power-cycle stays regardless, which was your point.
~~~~

---

## `CHANGELOG.md:112` — comment 4018550923

~~~~
Done your way: no number, and the card that answers reads at every address named as the case it
happens to. The bullet now follows the poller's rule instead of restating it with a threshold.

The poller wording you quoted is unchanged apart from losing its cross-reference to the backstop
note at `ISO15693_POLLER_PASS_MAX_MS`, which is gone from this round.
~~~~

---

## `scenes/nfc_magic_scene_iso15693_gen1_optin.c:4` — comment 4018550936

~~~~
Correct, and the note is in. It says the omission is deliberate, that a corrected validation line
would be no better than the stale one because validation is not something a user can act on at the
moment of consent, and that on a consent screen the one thing a reassurance can change is whether
they say yes.
~~~~

---

## `iso15693_poller.c:218` — comment 4018550949

~~~~
Gone — the whole wipe-vs-clone cost comparison is out of this round.

Your reading of it is what the code does, for the record: a refused write whose block reads back
empty increments nothing, so the card described two lines up ends with `failed_count == 0` and the
inventory gate is false.
~~~~

---

## `scenes/nfc_magic_scene_iso15693_write_fail.c:69` — comment 4018550959

~~~~
Kept and labelled defensive, which is your second option. Your analysis holds: `uid_verified` is set
only inside `Iso15693WriteStateVerifyWipe`, whose one exit is `success_or_partial`, and that returns
Fail, Partial or Success, never CardLost.

Kept rather than dropped because of where it sits. The round-7 conjunct was inside the state machine
that guarantees it, so the guarantee was local. This scene reads a result struct it did not fill and
cannot enforce the poller's invariant, so the term is worth having against a poller that grows that
exit. The comment now says that instead of implying the pairing occurs.

**Something you could not see from here.** The same false rationale was in the test pinning this
term — the host harness, which is not part of this PR, has a case that builds `CardLost` together
with `uid_verified` and asserts the button reads Exit, describing it as a wipe that lost the card
after the check answered. That is the state you have just shown cannot occur, so the dead term had a
passing test appearing to justify it. Corrected there too.
~~~~

---

## `iso15693_poller.c:59` — comment 4018550970

~~~~
All four rewrapped, text unchanged.
~~~~

---

## `scenes/nfc_magic_scene_write.c:353` — comment 4018550983

~~~~
Dropped. The branch does fire for the wipe and the Write-UID, and the reasoning under it is
mode-independent.
~~~~
