# DRAFT — the #250 PR description

**This file never reaches the fork.** `sync-to-fork.sh:17` excludes `.notes/` outright, so there is no
removal step to forget and no risk of it shipping. Edit here, paste into the PR when it is ready.

**Why it is load-bearing:** the pack squash-merges (verified — every commit on `dev` has one parent,
#258's four commits are not ancestors of it), and the squash message is the PR title + `(#250)` + this
body, verbatim. **All 20 commit messages are gone at merge. This is the permanent record.**

**What belongs here, and what does not.** This is the home for durable facts and decisions a maintainer
might want but does not need at the moment of editing a line: hardware measurements, rejected
alternatives, why a constant has the value it has. It is NOT a dumping ground for everything the C pass
deletes — process narration ("measured on device rather than counted") is worthless in both places.
Sections marked 🔨 are the ones that grow as C runs.

⚠️ **DECISION NEEDED — the AI disclosure.** The current body says *"PR is human-written."* If this draft
goes in as written that stops being true. Options at the bottom; your call, not mine.

---

# What's new

This adds support for magic iso15693 cards, e.g. slix. The magic uid write is a direct port from
proxmark's handling for these card types (also GPLv3). The clone flow, capacity handling, and warnings
are built on top.

- Recognizes iso15693 during the "check magic tag" scan. (Any iso15693 is a candidate since magic status
  can only be confirmed by writing on this card type.)
- Allows for writing a .nfc, wipe, write manual uid, and view info (similar framework as other card types)
- Carefully handles different card lengths. Testing confirmed that magic iso15693 cards can be configured
  to report a block length shorter *or longer* than the real memory, meaning the magic card can be written
  with a .nfc for a card where the block length is longer than the magic card can actually hold. This will
  still work in some circumstances, depending on whether those high blocks are actually inspected by the
  reader.
- The user is informed with a "partial clone" warning when blocks containing data are dropped from the
  write. When the high blocks are all blank, the user is still informed, with a custom "complete" note,
  since for most readers that will still work.
- Handles magic gen2 to gen1 fallback, and also warns the user if the gen1 fallback would overwrite blocks
  containing data, which may prevent the clone from working.
- Supports wipe, which sweeps every block the card physically holds rather than the count it advertises,
  and re-reads the UID afterwards to report an identity change rather than promise there was none.

## The two magic generations, and why they are not symmetrical

Both are ports from proxmark's `armsrc/iso15693.c` (`SetTag15693Uid` / `_v2`).

- **gen2** takes a custom `E0 09 ...` backdoor frame, and its UID lives in a **separate register space**.
  Data-block writes cannot disturb it, so a gen2 clone can write the UID first and the blocks after.
- **gen1** takes four *ordinary* `WRITE BLOCK`s at 62/63/56/57 — unlock, commit, then the UID — so its
  UID lives **inside the data-block address space**. Consequences that shape the whole feature: any
  writable tag accepts those writes, so gen1 is destructive on a non-magic card and is gated behind an
  explicit opt-in; and a gen1 clone must skip 56/57/62/63, which is why a gen1 clone reports Partial and
  never a clean Success.
- **gen3** is not supported. A wipe can destroy one — see Known limitations.

## Hardware measurements

These decided the design and are not recoverable from the code. Each is one run on one card — two gen2
cards are on hand (both 64/64) and exactly one gen1.

**Wiping the advertised count is not enough — measured 2026-08-04, gen2 card.** Seed all 64 blocks with
per-block markers, clone a 28-block source over it, wipe (the screen said Success), then read with a
proxmark: block 20 was zeroed; blocks 28, 40 and 63 still returned their markers. **36 of 64 blocks
survived a wipe that reported unqualified success.** The advertised block count is programmable, so a
clone leaves the card advertising 28 of its 64 physical blocks and a wipe bounded by that count clears
28. Hence the sweep runs upward past the claim until a run of blocks answers neither a write nor a read.

**Only a write settles whether a block exists.** A high block that has never been written may refuse a
read as well, so a read-based capacity probe under-detects. Past physical capacity, writes return
`Iso15693_3ErrorInternal` and reads fail outright — which is what lets a refused write be classified: a
block that *answers* a read exists, so its write failure was something else and must not be reported as
the card's capacity edge.

**Writing above physical capacity is inert on this silicon, not destructive.** The probe suite's
`edgepages` test found phantom writes rejected, phantom reads failing and block 0 unchanged across four
runs. A card that *did* alias would only receive the zeros a wipe is writing anyway.

**gen1 on real silicon — 2026-09-08, `ST LRi2K` keychain.** Six rounds of this PR carried "no gen1 card
confirmed on either side"; that is now closed.
- The UID write works and is **reversible** — set, read back exactly, original restored.
- **The backdoor registers accept writes without acknowledging.** An unaddressed zero write to block 62,
  gen1's own first frame, returned no ACK — and the full sequence then worked, so that write *was*
  accepted, silently. Consequence: **no write-based probe of these registers can have a meaningful
  negative**, which is why the tooling's gen1 gate is one-sided.
- **The armed-gen1 wipe hazard is reproduced.** A card left armed by an earlier gen1 UID write can have
  its UID moved by a wipe zeroing 56/57. Recovery worked byte-identically via `hf 15 csetuid`.
- Still unmeasured: whether the UID latches on power-up or changes immediately.

Shopping note, since it cost a round to work out: a listing saying **"Lua Script by Iceman"** means gen1.
`hf_15_magic.lua` sends byte-for-byte what `SetTag15693Uid` sends.

## Design decisions worth not re-deriving

🔨 *grows as C runs*

**Progress events are capped at 8 per pass, and it is a safety bound rather than a tuning knob.** The
block loops run to completion inside a single poller callback on the Nfc worker thread, and a progress
event reaches `view_dispatcher_send_custom_event`, which blocks with `FuriWaitForever` on a queue of
`VIEW_DISPATCHER_QUEUE_LEN` (16). A Back press during a write puts the GUI thread inside
`nfc_poller_stop` → `furi_thread_join` waiting for that same worker, so it stops draining the queue.
Emit more events than the queue holds and the worker blocks forever, the join never returns, and the
device hangs with the write popup on screen. **Per-block progress is only safe if the loop first yields
to the worker between blocks** (return `NfcCommandContinue` and resume from a cursor), the way
`uscuid_ul_poller.c` does.

**The wipe clears the gen1 backdoor registers deliberately.** On a gen2 card 56/57/62/63 are ordinary
user data, and the wipe performs no magic detection, so sparing them would leave four blocks of real data
behind on every gen2 card to hedge a hazard only gen1 has — a certain loss on the common card against a
conditional one. What ships instead is a UID re-read behind a field power-cycle, which converts a silent
identity change into a reported one.

**No blind write ordering can de-arm a gen1 card, so none is attempted.** Writing the commit block before
unlock reverses the only order the hardware has been observed to accept, so it is either rejected or —
worse — leaves unlock freshly zeroed, one step *into* the arm sequence, immediately before the sweep
reaches the UID registers. The conclusion survives the unlock/commit reading being wrong, since it
follows from not knowing what those registers do.

**Back is swallowed for ISO15693 only.** It never aborted a write in the first place: leaving the scene
runs `on_exit` → `poller_stop` → `furi_thread_join`, which waits for the worker to finish anyway, so Back
only discarded the report. Worse for a clone, which has an `NfcCommandReset` between the UID write and
the data pass — Back landing in that gap leaves the card carrying a new UID with none of the source's
data, silently. Not extended to the other four protocols, because for them it would be a trap:
swallowing Back is only safe where the write is guaranteed to report an outcome, and gen2/Classic,
USCUID-direct and gen4 all stop advancing once the card is gone (measured: 88 seconds with no state
machine activity — #252, #253).

**Both passes carry a wall-clock bound.** A card that refuses every write while still answering reads at
every address never accumulates an absent run, so it would walk all 256 blocks at 40–70 ms each. The
bound is set generously rather than tuned: cutting a legitimate sweep early leaves real data unwiped,
which is the privacy failure the sweep exists to remove. A truncated pass is reported as Partial and
names where it stopped, so no screen passes a cut range off as a finding about the card.

## Known limitations

- **gen3 is not supported, and a wipe can destroy one.** @0x6r1an0y, who wrote proxmark's ISO15693 V3
  magic support, reports that zeroing blocks `0x14`/`0x15` on an un-finalized V3 card does not merely
  clear the signature but bricks the card permanently. No gen3 card exists on either side of this PR, so
  this is attributed, not observed. The wipe confirm carries it; a pre-flight probe is #255.
- **gen1 is validated on one card, and the latch is still an inference.** The header says so at each
  affected entry.
- **Writes and inventory are unaddressed (#251).** The SDK builds `WRITE BLOCK` with no ADDRESSED flag
  and no UID, so a second ISO15693 tag in the field receives them too, and the 1-slot inventory can
  answer from the bystander. Split out rather than fixed here.
- **A very large source can lose its "Card too small" verdict to the clock** — the cost of that pass is
  dominated by failing blocks, and a genuinely too-small card presents one long run of them. Left as
  under-claiming rather than over-claiming: a cut run reports Partial with Retry instead of asserting a
  verdict about the user's hardware that the pass never finished testing.
- **#252 / #253** are pre-existing behaviours in the shared write scene, found while doing this work and
  filed separately.

## The comment surface

🔨 *this section is the C pass's landing site*

`iso15693_poller.c` currently carries about 90 comment lines per 100 lines of code, against 11 for
`gen2_poller.c`. That is out of step with the app and a deduplication pass has already run (repeated
comment phrases 135 → 34). A further pass is planned in two stages — deletion of what a maintainer
editing a line would not need, then simplification of what remains — and the durable facts above are
where the deleted reasoning lands. Sequenced deliberately: deletion cannot make a fact wrong, whereas
restating one in fewer words is where this PR's comment errors have actually come from.

# Verification

- Build and run the app e.g. `fbt launch APPSRC=applications_user/nfc_magic`
- Read an iso15693 card using the standard nfc app, or upload an iso15693 .nfc file to the flipper storage
- In nfc_magic, "Check Magic Tag" on an iso15693 magic card (will report iso15693 candidate)
- More --> Write --> select .nfc file
- Repeat read using regular nfc app to verify clone
- **Wipe, then re-read with a proxmark** rather than trusting the on-screen count — `hf 15 dump` is what
  caught the 36-of-64 bug above.
- Host tests: `make -C tools/hosttest` (108 tests). They `#include` the shipped `.c` files, so they pin
  the report strings and the boundary arithmetic without a device.

# Author Checklist (Fill this out):

- [x] I have performed a self-review of my own code
- [x] I have commented my code, particularly in hard-to-understand areas
- [x] My code is released under GPLv3 license and can be edited, or published according to the opensource license
- [x] I have bumped `fap_version` in the app's `application.fam`
- [x] I have added an entry to the app's `CHANGELOG.md` describing the change

# AI usage disclosure (Fill this out):

Tick exactly one - leaving all three blank reads as "not filled out" rather than "no AI".

- [ ] No AI assistance.
- [x] Partially AI assisted (clarify below which code was AI assisted and briefly explain what it does).
- [ ] Fully AI generated (explain what all the generated code does in moderate detail).

Claude was used extensively in the research, porting from proxmark, testing and verification. Most of the
code was AI-generated. The final code has all been manually reviewed, with some manual edits / cleanup.
Features are described in moderate detail above.

<!-- ⚠️ DECISION: the line below currently reads "PR is human-written." That was true of the July body.
     Three honest options, pick one and delete the others:

     (a) "The original description was human-written. This expanded version is AI-drafted from the
          session notes and hardware logs, then reviewed and edited by me."
     (b) Rewrite the added sections yourself, and the existing line stays true.
     (c) "Description is AI-drafted and human-reviewed throughout."

     (a) is the accurate one if you keep this draft substantially as it stands. -->

PR is human-written. Detailed changelog entry is AI (and also carefully checked / edited).

# Checklist (For Reviewer) (Don't fill this out!):

- [x] PR has proper description of new app/feature/bugfix
- [x] Description contains actions to verify app/feature/bugfix on the hardware
- [ ] No obvious issues with the code was found
- [ ] I've built this app/code, uploaded it to the device and verified app/feature/bugfix
