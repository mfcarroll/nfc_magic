# Round 15 — main reply (DRAFT, NOT POSTED)

Post between the `~~~~` markers. **No internal process in the payload.** `[👤]` marks a paragraph
written by mfcarroll; it is POSTED as-is, so mishamyte can see which words are his.

Not answering a review — he has not replied to round 14. This is the addressing work, and a
correction to what round 14 told him.

~~~~
## A correction

I told you TI Tag-it HF-I Plus refuses an unaddressed WRITE BLOCK, and that addressing the writes
was therefore a functional requirement rather than only the safety fix #251 asks for. That was
wrong. What that chip refuses is a write with the **OPTION flag** clear — addressed or not.

All four combinations, on the same card, each read back from it:

| flags | addressed | OPTION | result |
|---|---|---|---|
| `0x02` | no | no | refused, error 0x01 |
| `0x22` | yes | no | refused, error 0x03, "option not supported" |
| `0x42` | no | **yes** | accepted |
| `0x62` | yes | **yes** | accepted |

Its control was `hf 15 wrbl`, which silently forces OPTION on for any tag whose manufacturer byte is
TI's — so the frame that "worked addressed" differed in two bits, not one. A second TI card behaves
the same way: an **addressed** write with the flag clear is refused `0x03` there too.

**Nothing measured here requires an addressed write.** All seven cards the addressed writes were
validated on take an unaddressed one — the two TI cards once the OPTION flag is set, which is
orthogonal to the address.

**A second correction, smaller but user-facing.** The release notes scoped the wipe's identity hazard to
a gen1 card "left armed by an earlier UID write". Five gen1 cards here take a write to block 56 with
neither "unlock" nor "commit" sent first — one of them a card this app had never written — and no gen1
card has accepted either frame. A user cannot know a card's history and nothing can detect it, so the
hazard is any gen1 card whose claim lets the sweep reach 56/57: the reach rule, not the card's history.
The release notes say so now, and so does #255.

[👤] I would add, it is still totally possible that gen1 cards do in fact require those frames. I assume
those went into proxmark for a reason, even if the lack of any comments or documentation in there makes
it feel like a black box. I've just never _seen_ a card in a state where those frames were required
before a block 56/57 write would take. That said, I only have a handful of gen1 cards, and no duplicates,
so my focus was on categorizing and ensuring they worked with this app initially - it's possible some
arrived in that state and I "armed" them early on without realising. Worth checking carefully on factory
fresh gen1 if either of us get any more. That requires sending carefully crafted frames and intentionally
not using the standard pm write commands.

## What went in

Each commit message carries its own argument and measurements; this is the short version.

- **01 — data-block writes carry the card's address**, which is what #251 asks for. All seven cards
  accept them and ignore one addressed a byte wrong, read back rather than inferred. The wipe retakes
  its address after 56/57, which on gen1 are the UID and move immediately.
- **02 — the OPTION flag, decided by what the card says** rather than by its UID, since a clone
  rewrites the UID; and a read-back, because the flag costs the acknowledgement. With OPTION set the
  card answers only after a standalone EOF, and the SDK has no call to send one.

[👤] I'd argue that SDK limit is a real gap in the firmware for iso15693, but it's one we can work
around, and much better to work around it than expand the scope of this to requiring a firmware
upgrade, even if that means we aren't able to get the write acknowledgments back directly.

  That call is about six lines, and on a firmware branch carrying it the TI card answers its writes
  directly — which matters more than the frame saved, since a read-back cannot tell a write that took
  from a refused write to a block that already held the value. The app cannot depend on it: a FAP
  resolves its imports at load time, so naming a symbol the firmware lacks fails the whole load. If
  that call ever lands in the minimum firmware supported here, it replaces the read-back outright.

- **03 — the gen1 loss claim is made only where there was a loss.** A gen1 clone is Partial on those
  four addresses only where the file held data in them. One that reached them with nothing there lost
  nothing, and ends as "Clone finished" with a note, like an empty tail past the card's end.
- **04 — the identity writes are addressed, and take the OPTION flag.** WRITE AFI and WRITE DSFID are
  standard commands any tag takes, and a changed AFI can drop a bystander out of the selective
  inventory its own system runs.
- **05 — a clone that lands in a gen1 card's UID repairs it.** A gen1 card already wearing the file's
  UID passes the gen2 verify, and the data pass then wrote the file into its UID registers. The write
  that moves the UID identifies the card, so the run converts, puts the UID back, and reads it back
  before reporting: a lost repair frame, or a re-address that missed, shows as the UID the card answers
  to rather than as a clone.
- **06 — what a clone leaves behind, and what it says about it**: notes for data still above the file,
  a card answering reads past the count it reports, and a geometry or IC reference it goes on
  reporting. None makes the clone Partial.

  Two tags here defeat the survey's premise that a block which answers a read exists: both answer at
  all 256 addresses because their address space aliases, so the survey would call a 128-cell card a
  256-block one. Neither is a card this PR supports, so the wording is left alone rather than hedged
  for them.

[👤] That tag appears similar to gen3, but doesn't match the configuration patterns expected by the
gen3 code in proxmark. The seller told me there is no support for it in proxmark and it requires
custom writer software, so it may be a proprietary variant. Not worth worrying about for this PR.

- **07 — the gen1 registers, addressed**, and the wipe hazard scoped to every gen1 card the sweep
  reaches rather than to an armed one. At block 62 addressing is what makes the NXP parts answer at
  all. Unlock and commit are addressed on the safety argument alone: no gen1 card here has accepted
  either, in any form, and the cards that would show them necessary are ones I have never had.
- **08 — the gen2 backdoor cannot be addressed**: on all four gen2 cards it takes only the unaddressed
  form, while each takes an addressed ordinary write and filters a wrong address. Another gen2 magic
  card in the field takes these frames, and nothing in the app can stop that; the release notes say
  so. 08 is also the 2.3 notes.
- **09, 10 — comments from earlier rounds** that had gone false (among them the gen1 opt-in string,
  which said the UID goes to all four blocks), or that told their own history.
- **11, 12 — hardening.** The failure bitmap checks the index it is given, and both switches on the
  write-fail reason list every reason with no default, the shape you gave the poller's write-state
  switch — so a reason added without an answer is a build error, and a screen entered before any
  reason is set crashes instead of saying "Not a magic tag".

## What this does to the scope

The OPTION flag is what makes a TI Tag-it writable, and it is separable from everything else here.
The addressing is the safety fix #251 was filed as. I have kept both, for the range: a bystander does
not have to be touching the antenna on ISO15693 — a wallet or a badge holder is enough.

**It does not close #251.** The 1-slot inventory and the missing STAY QUIET are untouched, and the
post-wipe UID re-read can still be answered by a bystander; that check exists to find out whether the
UID changed, so it cannot be aimed at a UID already in doubt.

## The bench

The addressed writes were validated on seven cards, over four identified chips plus two whose silicon
cannot be named — both have only ever worn a written UID, so the type line a reader prints for them
describes what was put on rather than the chip underneath. The runs that decide it, on those and three
more gen1 cards:

- a **TI Tag-it** wipe and clone — the card that could not be written at all before this
- a **gen1 ST LRi2K** wipe, where the UID moves under the sweep: 58/58 and the identity change
  reported, which is the point — the re-address is what stops the addressing breaking that path
- a **70-block source onto 64-block silicon**, on the second TI card, where the six blocks past the
  top burn their retries and fail their read-backs, while every real block is read back against a
  card first filled with a different pattern in every block
- the **same source cloned twice onto a gen1 NXP SLIX**: the second run converts, the UID reads back
  intact, and it reports as the first did
- a **64-block file cloned through gen1 onto a 28-block SLIX**, once with nothing at 56/57/62/63,
  which finishes with its notes, and once with data at 62/63, which is Partial
- a **file of 8-byte blocks onto a gen2 card and a gen1 SLIX**: the gen2 card took the geometry and
  then refused every write, the SLIX refused them outright, so the geometry note compares the block
  count alone
- a **64-block card carrying a distinct per-block pattern**, cloned from a 28-block source, for the
  residue, and a **40-block SLIX-S** for the geometry
- **a gen1 Write UID on each of the three gen1 chips**, to a target differing from each card's own UID
  in BOTH halves, so half a UID could not pass as a whole one
- **the address filter on each of the seven**, read back rather than inferred: a write aimed a byte
  wrong at a block holding something else, the block unchanged, then the same frame correctly
  addressed changing it
- **the gen2 backdoor in four flag and address combinations on each of the four gen2 cards**

## Where this stands

That closes the list at the end of my reply to your round 9: the cut, the simplification pass, the
release-notes trim, the addressed writes and the re-test on hardware are all in, which was the
condition I put on the squash message, since it has to describe the final state.

**A question about the release notes.** The 2.3 entry is now 160 lines, about half the file, and
most of it describes how the ISO15693 support behaves and what it was tested on, rather than what
changed. As fixes and gen3 support land, later entries would have to correct it. Would you rather
have a short 2.3 entry in the style of 2.0, keeping the limits a user needs before a wipe, with the
rest in an `ISO15693.md` beside the changelog that is kept current? A few apps here keep a reference
like that, such as `SUPPORTED_CHIPS.md` in fake_chip_detector and `docs/DESIGN.md` in pocketlab. It
would be much the same material, laid out as a reference rather than as release notes, with room for
general notes on the gen1, gen2 and gen3 magic types. If you want it, it can go in before merge.

[👤] I have a squash message drafted. I'll wait until you're ready to merge in case there are further
changes still, then post it as its own comment.

**One thing tested since, out of scope on purpose.** A genuine gen3 card kindly sent by @0x6r1an0y
— un-finalized, its configuration signature an exact match to proxmark's V3 config mode. The gen2
backdoor left its UID untouched, so a clone or Write UID lands on the gen1 opt-in exactly as the
release note says; accepting that wrote 56/57/62/63 as ordinary data and moved no identity, since a
gen3 card keeps its UID in a separate register. The configuration blocks were never touched, and it
restored byte-identical. I left the brick itself untested — it is irreversible, and a warning does not
need it confirmed. It is on #255 as well, and gen3 support stays out of scope.

Three things are deliberately not in this PR, so they are not waiting on me:

- **#251 is not closed**, for the reasons above — the inventory is a different change.
- **#255 stays open.** The gen3 pre-flight probe it asks for is not part of this work.
- **The host-test harness** stays out, as its own PR, for the size reason I gave before.

I think it is ready.

[👤] Or at least close. Claude may be slightly more confident than me. :)
~~~~
