# PR #250, round 3 — state and plan

His review landed 2026-08-11T07:39, **CHANGES_REQUESTED**: 5.6k body + 24 inline comments. Verbatim in
[received/](received/).

## Where the PR stands

| | |
|---|---|
| Head | `a3e13a3a` (our 6 pushed commits), mergeable |
| **The three round-1 blocking items** | **Confirmed fixed** — he re-traced each rather than trusting the commit messages |
| Build | Clean at Unleashed `dev` API 88.0, zero warnings, `ufbt format` clean |
| Threads | 19 resolved (all from the July round-0 review), **40 open**: 16 from round 1 + 24 new |
| New blocking | **1** |
| Issues #251/#252/#253 | No activity yet. Expected — they are for after this PR. |

He also conceded the Back scoping — *"You were right to narrow it and I was wrong to approve it
wholesale"* — and verified both stranding cases himself at `gen2_poller.c:523-524` and
`uscuid_ul_poller.c:379-382`. **Ship it as-is.** But two of the four *supporting* sentences in our
comment are wrong, and he wants them corrected because that comment is what a future maintainer will
widen or narrow the swallow from.

Note he has **not** resolved the round-1 threads, including the three blocking ones he says are fixed.
Worth asking whether he wants us to resolve them or prefers to do it himself at the end.

## The blocking item — `iso15693_poller.c:859`

The tail-drop and the floor we added in `cfa1b37f` assume **opposite things about the same blocks**.
The floor exists because the card's claim is evidence a block exists; the drop discards blocks assuming
they don't; below `advertised` the drop wins.

His trace, verified: 64-block card, blocks 20..63 answer neither write nor read. The run never trips
below `advertised` (correct, that's our floor), the break is permitted at block 63, the re-probe uses
`read_block` which is what already failed, so all 44 stay absent — and the tail-drop clears bits 20..63.
`blocks_total = 20`, `failed_count = 0` → **`Success`, success chime, "Cleared 20 blocks. / Card claims
64."** 44 blocks still hold the previous card's data.

Not a regression, and rarer than the 28-of-64 bug — it needs blocks that fail *both* a write and a read
and stay that way. What earns the blocking mark is the shape: a silent privacy failure reported with a
success tone, which is the same shape as the last three.

**His fix, and it's a good one.** Don't drop below `advertised` without evidence. The discriminator is
already in the file and is the *safe* direction of the inference our own comment at `:749-755` warns
about: activation's block read stops at the first failure and zero-fills the rest, so the cache can't
prove a block is empty — **but non-zero in the cache is one-way proof the block existed and held data.**

So before dropping a run member below `advertised`, check `iso15693_3_get_block_data(target, block)`:

- **non-zero** → keep the bit and count it. Provably existed, provably held data, provably not cleared.
- **zero** → drop as now.

Verified available: `target` is in scope at `:658`, and the accessor is already used at `:530`.
Fake-flash stays fixed — blocks 64/65 failed the activation read and sit as zeros, so they still drop.

**Residual he wants stated, not coded:** if the degradation predates activation the cache is zeros there
too and the block is still dropped. Smaller hole, different symptoms, better said than left implied.

## Pass A — close the gate

The blocking fix plus the three he'd bundle. Note item 1 comes first because item 3 needs it.

1. **`write_fail.c:344` — one Details predicate.** The gate is stated twice and *already disagrees*
   (routing accepts `wipe_uid_changed && failed_count == 0`, the button gate rejects it — latent, not
   live). He gives the helper. This is where `sweep_truncated` has to go for item 3, so it lands first.
2. **`:859` — the blocking fix above**, plus the residual comment.
3. **`sweep_truncated` on all four wipe screens.** Today `write_fail.c:126` is the only reader.
   - `:978` NothingWiped — *the card the clock was added for*: it refuses every write, so `wiped == 0`
     short-circuits to Fail before any truncation reporting. Add `\nStopped: time limit`.
   - `write_fail.c:126` Partial — prints `blocks_total` as the denominator, which on a cut sweep is the
     cut point, not the card. Change the qualifier to `"\nStopped at %u of %u"` — free, no extra line.
   - `write_fail.c:196` UID-changed — `uid_changed` pre-empts Partial. Fix via Details (the 4th-line
     constraint is real), one entry in `nfc_magic_scene_iso15693_partial_details.c`'s caveat section.
4. **`:532` — the clone data pass never got the clock.** `65d71105` killed Back for the whole ISO15693
   write; `write_source_blocks` is bounded only by the source count and its presence check is *after*
   the loop. ~6s frozen with Back dead on a 79-block source, ~19s at the ceiling. Our own Back comment
   argues the clone is the worse case, and it's the one left unbounded.
5. **`:1125` / `:1194` — stop asserting a check we skipped.** A `uid_verified` flag, false on both
   no-observation branches, rendered as "UID not re-checked." He is explicit that *not raising an error
   is the right call* — only the silence is wrong. `:1194` is the sharper half: it fires after a
   deliberate power-cycle, and the likeliest non-lift cause is exactly the armed-gen1 latch
   `VerifyWipe` exists to catch.
6. **`write_fail.c:307` — Retry/Exit on a cut-short wipe.** It's the one wipe outcome where re-running
   is the correct action, and it currently gets a lone Finish. Same screen edit as item 3.

**Bench:** 66-advertised/64-physical is the regression test that matters (it must still report clean
Success), plus 28/64 as before. The degraded-card case that motivates the fix is not readily stageable —
say so rather than implying it was tested.

## Pass B — numbers, then documentation

Three arithmetic corrections he settled from `firmware.elf` rather than argued, so they are facts now:

- **`:56`** — "roughly a 5-7 second timeout" is wrong; `iso15693_3_poller_run` calls
  `furi_delay_ms(100)` per failed activation, so 40 errors is ~4s. **Our two new figures at `:62`/`:67`
  are right and the pre-existing one is the outlier.**
- **`:137`** — "two waits" should be three; the retry loop has no `break` before the final delay, so a
  block failing all three attempts pays 15ms. Error is in our favour — correcting it *gains* margin.
- **`write_fail.c:70`** (and copies at `:188`, `:213`) — FontSecondary height is `max_char_height 0x0a
  + 1 = 11`, not 9. Line tops are 13/24/35/**46**/57, so a 4th line is *compromised* by the button box
  rather than overpainted. **Conclusion right, mechanism wrong.**

Then all documentation — his eleven plus the six still open from round 1:

- Dead cross-references this delta created: `:260`, `:695` (both still point at "the read-back at the end
  of the wipe branch", which `ad4272c4` moved). `:695` deserves more than find-and-replace — it's the
  OPEN QUESTION's conclusion, and that argument is now *stronger* than it has ever been.
- Contracts the delta made stale: `poller.h:24` (Success no longer implies a UID was observed),
  `poller.h:209` (absent-run still given as the only stop condition; no `sweep_truncated`; orphan line at
  `:214`), `app_i.h:129` ("a clean success" — truncated shares that reason code), `CHANGELOG:39`
  ("neither of which is a fault" — a dead stretch and a clock cut both are), plus `CHANGELOG:34-38`
  self-contradicting inside one bullet and `:74-75` repeating the tail-drop premise.
- `:853` — the tail-drop rationale doesn't survive the clock exit; either skip the drop when truncated or
  say the flag is what carries that path.
- `:866` — "every exit" isn't: the two card-lost paths `return` and skip the timing log, which is the
  case you'd most want timed.
- `scene_write.c:519` — the two wrong supporting sentences on the Back swallow, plus `:521-522` citing
  only one of the two budgets that now exist.
- Round-1 leftovers: the positional "Card too small" claim, the re-probe classifying by presence, the
  unclamped `block_size` into the 32-byte buffer, and the six remaining drift items.

## Pass C — simplification

His five, minus the two pulled into Pass A:

- `scene_write.c:396` — two locals so the gate and the reason read off the same fact.
- `:711` / `:799` — hoist `furi_ms_to_ticks` out of the loop (recomputed up to 256 times from two
  compile-time constants), and name the off-by-one `claimed_range_attempted`. Keep the elapsed-form
  comparison; an absolute deadline would not be wraparound-safe.
- Leave the eleven `const bool`s at `write_fail.c:20-31` — the queued `{reason, title, body}` table
  deletes them all, and a partial cleanup would just conflict.

Plus the round-1 queue: the duplicated retry loop (he called it non-optional), the result-scene table,
the duplicated confirm scene. `:741` is now folded into Pass A.

## One question to answer him

He asks whether a behaviour change was deliberate: **a clean ISO15693 wipe no longer auto-dismisses**
through `NfcMagicSceneSuccess` (1.5s to the main menu) — it now needs a button press and lands on the
ISO15693 submenu. He assumes yes, since it matches the over-capacity clone route. It was deliberate;
confirm it.

## What he verified so we don't re-check

Recorded so this isn't re-done: the sweep arithmetic holds on all four exits (`block - absent_run` can't
underflow, `run_start` stays in range, `clone_failed_count` and the bitmap can't disagree — he walked the
mixed re-probe case); geometries 28/64, 66/64, 200/64, advertised 0, advertised 256, lifted at block 5,
and a dead stretch that recovers all report correctly; `activation_errors` is zeroed on the Ready that
runs the sweep so `VerifyWipe` gets a genuine fresh 15; the wipe result is reported exactly once; and
there is no Back-swallow trap — every terminal event navigates, `write_step` never returns Continue, and
`on_exit` resets the state.
