# Round 11 — thread replies (DRAFT, not posted)

Seven threads. Post each payload between its `~~~~` markers, on the comment id named.

Rules applied: disposition, anything HE got wrong, anything WE found. No restating his reasoning,
no describing the diff, no process narration, no dev-repo SHAs, no thread numbers. Commits are cited
by SUBJECT, since several threads land in one.

**Check before posting:** `tools/` and `.notes/` never reach the fork, so the host harness is not in
this PR. The reply below says so where it matters rather than citing a file he cannot open.

---

## `iso15693_poller.h:83` — comment 4018550915

~~~~
Fixed in "the no-latch result is three chips, not gen1 silicon", and the fix is the measurement
rather than the wording. You were right that the three-chip result cannot carry this — set,
read-back and restore all run through a power-cycle, so it is silent on latching, and only the
same-session inventory shows immediacy.

So it has now been run on one card of each chip. Same frames, same test UID, field held up with
`-k`: the LRi2K as before, plus an NXP ICODE SLIX-S at IC ref 0x02 and an NXP ICODE SLIX at IC ref
0x01. All three return the newly written UID with no power-cycle. The immediacy result now has the
same sample as the sentence beside it, so neither needs a narrower scope than the other. Three
chips, still not the family, and the line says so.

**One claim did not widen, and it is now split off.** Finding 6 in our notes reads the LRi2K's
`01 10 1E 06` as showing the backdoor registers *answer* rather than sit silent. On the other two
the client reported a plain command failure, which does not separate an error frame from silence.
The refusal itself replicated on all three; the in-band part is one chip, and the comment no longer
lets it ride along with the latch result.

Both cards were restored from their recorded UIDs and confirmed byte-identical.

The power-cycle stays, which was always your point — it buys a clean re-activation regardless.
~~~~

---

## `CHANGELOG.md:112` — comment 4018550923

~~~~
Done your way, in "the sweep's reach depends on reads, not on the card's claim": no number, and the
read-everywhere card named as the case it happens to.

One thing you should know, because it is in the same push and it cuts against you. **The comment cut
had already deleted the poller wording you quoted here as correct** — the carve-out at what was
`:1303-1308`, "a card that refuses every write but still serves a read never accumulates a run, so
it walks past 56/57 whatever it claims". What survived was still true, since that card never
satisfies "answers nothing above its claim", but it left the reader to get there by negation, and
it removed exactly the sentence you were pointing the release notes at.

Restored in "the cut deleted the sweep's carve-out, which was the part that was right", without the
cross-reference to the backstop note at `ISO15693_POLLER_PASS_MAX_MS`, which the same cut removed.

So the third round for this sentence was nearly a fourth, in the other file.
~~~~

---

## `scenes/nfc_magic_scene_iso15693_gen1_optin.c:4` — comment 4018550936

~~~~
Correct, and written now, in "the note the consent screen was supposed to have". The commit removed
the stale claim from both bodies and from the file header and added nothing, so the message asserted
a guard the tree did not have.

The note says what the message said it would: the omission is deliberate, a corrected validation
line would be no better than the stale one because validation is not something a user can act on at
the moment of consent, and on a consent screen the one thing a reassurance can change is whether
they say yes.
~~~~

---

## `iso15693_poller.c:218` — comment 4018550949

~~~~
Right, and the whole footnote is gone in the same push — "cut the argument out of the poller's
comments, keeping the constraints" removed the wipe-vs-clone cost comparison, so the bad "since"
went with it rather than being corrected in place.

Your reading of it is what the code does: a refused write whose block reads back empty increments
nothing, so the card described two lines up ends with `failed_count == 0` and the inventory gate is
false.
~~~~

---

## `scenes/nfc_magic_scene_iso15693_write_fail.c:69` — comment 4018550959

~~~~
Kept and labelled, which is your second option, in "the dead conjunct is defensive, and a test was
calling it reachable". Your analysis holds: `uid_verified` is set only inside
`Iso15693WriteStateVerifyWipe`, whose one exit is `success_or_partial`, and that returns Fail,
Partial or Success and never CardLost.

Kept rather than dropped because this scene reads a result struct it did not fill and cannot enforce
the poller's invariant — unlike the round-7 case, which was inside the state machine that guarantees
it. The comment now says defensive rather than implying the pairing occurs.

**What you could not see: the same false rationale was also in the test pinning it.** The host
harness — not part of this PR — has a case that builds `CardLost` together with `uid_verified` and
asserts the button reads Exit, and its comment described that as "a wipe that lost the card AFTER
the check answered". That is the state you just showed the poller cannot produce. So the dead term
had a passing test appearing to justify it, which is a fair part of why it survived two rounds. The
test keeps its coverage and drops the claim.
~~~~

---

## `iso15693_poller.c:59` — comment 4018550970

~~~~
All four rewrapped in "four comment paragraphs that lost their reflow, and a note naming one mode of
three". Text unchanged, wraps only, and all four verified against the 99-column limit by hand since
`ReflowComments: false` means the formatter cannot see them.
~~~~

---

## `scenes/nfc_magic_scene_write.c:353` — comment 4018550983

~~~~
Dropped, in the same commit as the rewraps. The branch does fire for the wipe and the Write-UID, and
the reasoning under it is mode-independent.
~~~~
