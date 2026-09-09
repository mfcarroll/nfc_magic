# Round 7 — per-thread replies. DRAFT, NOT POSTED.

One block per thread, `id` then body. **Quote text, never a line number** — the comment cut moves every
reference in `iso15693_poller.h` and `nfc_magic_scene_iso15693_write_fail.c`.

Order below is his review order.

---
## `3948188925` — poller.c, the `COUNT_OF` rationale

Fixed, and your arithmetic needed one correction of its own.

`sizeof(array)` goes 4 → 8 while the count stays 4, so a `sizeof`-bounded loop runs eight iterations
over a four-element array — touching indices 4..7, which is **four** elements past the end, not two. I
took your figure at first and had to check it; the comment now says four.

The failure mode is flipped as you asked: widening lengthens the loop into an out-of-bounds read, it
cannot shorten it.

---
## `3948188938` — CHANGELOG, the gen3 entry

Fixed, including the omission, which is the sharper half.

The entry now says a gen3 card ignores the gen2 backdoor, so the UID reads back unchanged and a clone or
Write UID lands on the **"Not gen2 magic card"** opt-in screen. I checked the string against
`gen1_optin.c` rather than trusting the enum doc.

And it now says what that means: **accepting that opt-in sends four ordinary WRITE BLOCKs into
56/57/62/63, so the clone path can damage a gen3 card, not only the wipe.** That was the part worth
having — the bullet exists to warn someone about gen3 and it described the safer of the two paths.

---
## `3948188946` — write_fail.c, the sentence that cost you a false finding

Fixed, and thank you for flagging it against yourself rather than dropping it.

It is a claim about the screen it is printed on, and the code four lines below disproves it. The
routing half was right, so the comment now names *which* reporting is skipped — the `WipeStopped`
screen — and says the cut is reported here instead, in place of the prose.

You are right that we had written the accurate version ninety lines up. That is the whole argument for
the comment cut in one example, and I have used it as such in the round reply.

---
## `3948188955` — app_i.h, the same overreach

Fixed the same way: it is the `WipeStopped` truncation reporting that is skipped, and that screen states
the cut itself. The "any" is gone.

---
## `3948188962` — poller.h, `cut_block`'s framing sentence

Fixed, with your phrasing.

You are right that no card can separate them in opposite directions — `blocks_total` is
`highest_present + 1` and is structurally `<=` the cut, which the line above already says. What two
cards differ in is which side of the **advertised count** the cut lands on, which is what the bullets'
own labels say.

Worth noting the comment cut had already removed the `:257` propagation you flagged — `start_wipe` no
longer says "see the field's own doc for the two cards that separate them."

---
## `3948188969` — poller.c, the wipe/clone cost comparison

Accepted in full, all three points.

Both extra costs are absence-driven and `absent_run` increments only on the read-failure path, so the
card this bound exists for — refuses every write, answers a read everywhere — accumulates zero
absences, pays no inventory, has no run to re-probe, and the two passes cost the same. The comment now
says so.

"After the loop" corrected: the re-probe is inside the loop, and what runs after it is the tail-drop,
which reads the activation cache and costs no airtime.

And your third point: the inventory is gated on `!claimed_range_attempted` too, so it only runs below
the advertised count. That strengthens the argument rather than qualifying it — the card in question
is above the claim by definition.

---
## `3948188977` — poller.c, the wrong pass credited

Accepted. The wipe's read is what *determines* emptiness, so "not only the empty ones" was never a
property it could have had. The pass that changed is the clone, and the comment now points at
`write_source_blocks` where this file documents it.

Last round's wording had it the right way round and the rewrite moved the attribute. That is one of
four cases this round where the comment cut compressed a claim and kept the error, which I have set out
in the round reply.

---
## `3948188987` — write_fail.c, the opening sentence

Fixed. It described only the below-the-claim case, and was contradicted both eleven lines down and by
the whole branch in `partial_details.c` that prints "Every claimed block was attempted."

It now frames both sides: below the claim there are claimed blocks unattempted; above it every claimed
block was attempted and what remains is past the claim entirely.

---
## `3948188996` — poller.h, the absent-run bound

Accepted — right bound, wrong reason, and your replacement reasoning is in.

You can hit 8+ consecutive absences below the claim and still get past it, because the trip falls
through to `continue` there rather than ending the sweep. What bounds the gap is the run still open at
the cut: the deadline is tested at the top of the iteration, so `absent_run <= ABSENT_RUN - 1` there on
every path, giving `cut_block - blocks_total <= 7`.

I checked that on all three paths into the top of the loop before writing it down.

---
## `3948189002` — write_fail.c, the qualifier list

Fixed. Four qualifiers, not three, and the omitted one ranks **first** — the line below says so itself.

The comment now lists all four in priority order and says the cut ranks first, and that the cut it means
is a cut **clone**: a cut wipe has its own reason code and screen, but a cut clone has none, which is
why this branch is where it lands. I verified the order against the `if`/`else if` chain rather than
against the old comment.

---
## `3948189009` — poller.c, the proximity argument

Fixed, and you were right that the proximity is not there — but the replacement needed a second pass too.

What sits three and five lines from the clears is `clone_failed_count++`, not a set, and the nearest set
is 69 and 133 lines away as you say. I first rewrote it as "each in an if/else whose other arm marks a
block failed" and that is also wrong: **neither** arm calls `mark_failed` at either site. Both clears
are the else-arm of a two-way decision about a block's *content*, whose other arm counts the block as a
real failure and leaves its provisional bit standing.

So the hazard is exactly what you said — `mark_failed` where `unmark_failed` belongs — and the comment
now argues for the named-call pair rather than for a `|=` / `&= ~` adjacency that does not apply.

---
## `3948189016` — write_fail.c and partial_details.c, three descriptions of `blocks_total`

Fixed, and both defer to the header's unqualified "a COUNT", which is the description that survives.

You are right that `highest_present + 1` is the size of the range up to the highest proven block rather
than a tally of proven blocks, because interior absences fold in. `partial_details.c` now says "a
COUNT, one past the highest block that answered" instead of describing `highest_present`.

---
## `3948189021` — partial_details.c, the `<` → `<=` revert

Reverted, and no apology needed — the round trip cost less than the wrong string would have.

You are right on count-versus-index: at equality the claimed blocks are 0..N-1 and the cut sits at index
N, past the claim. Your original wording was the accurate one. The three-line justification is dropped
as you asked, since "block N itself was not attempted" is equally true of both branches.

**And the boundary is now pinned.** Nothing tested it — every existing case sat strictly below the claim
(23 of 70, 55 of 64, 10 of 256). There is a case asserting `cut_block == blocks_advertised` renders
"past the N this card claims" and *not* "of the N", plus that one block lower still reads as inside.
Both of us have had it backwards once; a third round trip is a red test now.

---
## `3948189029` — poller.c, the dead conjunct

Dropped. Info returns unconditionally six lines up, so the test cannot be false there. The comment now
says that early return is what guarantees `CardDetected`'s "not sent in Info mode", rather than leaving
the guarantee looking like it comes from the conjunct.

---
## `3948189038` — poller.c, `alloc`'s partial reset list

Taken the first of your two options: cut to the allocations plus `running`, `callback` and `context`.

Your reasoning holds — `running` starts false and every public entry point routes through
`start_internal`, which asserts `!running`. The comment names `start_internal` as the authoritative
list. One correction to the figure: it sets **28** distinct fields, not 26.

---
## `3948189049` — poller.c, the duplicated write tail

Extracted as `iso15693_poller_finish_write(instance, iso_poller, bool skip_backdoor)`, exactly as you
sketched. Agreed it is the highest-value of the dedup items, and for the reason you give rather than the
line count.

Reporting how that went, because it is the more useful part: **mutation-testing it showed nothing tested
the flag.** Flipping `skip_backdoor` at either call site left all 107 cases green. An extraction whose
whole justification is "these must stay in step", with nothing holding them in step, is worth very
little — so there is now a case asserting a gen2 clone counts the backdoor blocks and a gen1 clone
deducts them, verified in both directions.

**The 4-line CardLost preamble I have left alone, and would rather you decided.** Folding it needs an
out-param or a sentinel `NfcCommand`, and I think that costs more clarity than four identical lines do —
the tail paid because it carried a *parameter* that could diverge silently, which the preamble does not.
Say if you would rather have it folded anyway.

(Also: there are two of those preambles, not three. The wipe verify's inventory call reads identically on
its condition line but only logs and continues, leaving `uid_verified` false rather than reporting
CardLost. I checked because I had miscounted it.)

---
## `3948189059` — write_fail.c, the three duplicated UID loops

Folded — but into **four** call sites, not three, which is a deliberate divergence from what you
proposed and I would rather flag it than have you find it.

`iso15693_info_cat_uid(FuriString*, const uint8_t*, Iso15693UidFormat)` takes a two-policy enum,
`Spaced` and `Grouped`, and the Info screen uses `Spaced` rather than staying out of the helper. Your
reason for excluding it was the distinction the comment was drawing; my reading is that naming both
widths on the enum — 23 characters against 17 — makes that distinction enforceable instead of prose
living on one copy of three.

It is also tested now: `test_uid_format.c` asserts both policies' output and both widths, and it caught
the third copy's drift for free. Happy to go to the three-site version if you prefer it.

This one shipped in the comment cut rather than this round, so it is in the earlier commits.

---
## `3948189071` — write_fail.c, the title switch order

Moved. `Partial` now sits at chain position rather than eleventh, so the switch and the render chain read
in lockstep — which is what makes "does every branch still have the right title?" a single-pass check,
and you are right that it was part of the point of hoisting them out.

---
## `3948189078` — write_fail.c, 189 columns

Rewrapped.

And the observation lands: the cut deliberately did not rewrap these files to 99, because moving every
line would have buried a comment-only diff in churn. That reasoning holds for a 104-column line. It does
not hold for 189, which is twice the limit — and you are right that a commit in the same batch is titled
for a 155-column line.

No comment in the ISO15693 surface now exceeds 106, the files' own de-facto width. One line still does:
a 117-column string literal in `iso15693_info.c`'s chip-description table, pre-existing and left alone —
splitting a data-table string across a concatenation is churn on a table, and it is not a claim that can
drift.

---
## `3948189092` — file_select.c, 110 columns

Rewrapped to match the 96-104 of the surrounding block.
