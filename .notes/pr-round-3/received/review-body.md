Round 4. The three blocking items are fixed, and fixed properly -- I re-traced each rather than taking the commit messages for it. Commits are cleanly scoped, one decision each. Build is clean at Unleashed `dev` (API 88.0), zero warnings, `ufbt format` clean, no artifacts tracked.

Two acknowledgements before the findings.

**The Back scoping.** You were right to narrow it and I was wrong to approve it wholesale. I checked rather than took it: gen2 halts after every block (`gen2_poller.c:523-524`) so it needs a re-activation to reach the next one, and USCUID-direct has the `NfcCommandReset` at `uscuid_ul_poller.c:379-382`. Both genuinely strand. Ship it as you have it. Two of the four *supporting* sentences don't hold, though, and since that comment is what a future maintainer will widen or narrow this from, they're worth correcting -- inline at `nfc_magic_scene_write.c:519`.

**"Untestability should have raised the severity, not lowered it"** is right, and it generalises past the one case. I'd rather you apply it than I restate it.

---

## One blocking item

The wipe can still report an unqualified "Wipe complete", with the success chime, over blocks the card itself claims and that were never cleared.

It is not a regression -- the tail-drop rule is untouched. What puts it in reach is that `cfa1b37f`'s floor and that drop now assume opposite things about the same blocks: the floor exists because the card's claim is evidence a block exists, the drop discards blocks on the assumption they don't, and below `advertised` the drop wins. Full trace and a fix that doesn't re-break the fake-flash card at `iso15693_poller.c:859`.

Straight about reachability: this is narrower than the 28-of-64 bug. That one fired on every wipe of a cloned card; this needs a card whose blocks stop answering *both* a write and a read and stay that way to the end of the advertised range. What earns it the blocking mark isn't the odds, it's that the failure mode is a silent privacy failure reported with a success tone -- the same shape as the last three.

## Three worth taking in the same pass

- **`sweep_truncated` is read on one wipe screen out of four.** `write_fail.c:126` is the only reader. Missing from NothingWiped (`iso15693_poller.c:978`) -- which is the exact card the clock was added for, since it refuses every write and short-circuits before any of the truncation reporting -- from Partial, and from UID-changed.
- **The clone data pass got the Back swallow but not the clock** (`iso15693_poller.c:532`). ~6s of frozen popup with Back dead for a 79-block source, ~19s at the ceiling. Your own Back comment argues the clone is the worse case, and it's the one left unbounded.
- **`VerifyWipe` asserts a check it skipped** (`:1125`, `:1194`). Not raising an error is the right call; saying nothing isn't.

## Numbers I settled rather than asserted

Two of the estimate-shaped comments needed a real figure, so I went and got them from `firmware.elf` rather than argue:

- `iso15693_3_poller_run` calls `furi_delay_ms(100)` per failed activation -- exactly 100ms. So your two new figures at `:62`/`:67` are right and the pre-existing "5-7 second" at `:56` is the one that's wrong.
- `canvas_current_font_height` for FontSecondary returns `max_char_height` 0x0a **+1 = 11**, not 9, and `elements_button_left` boxes rows 52-63. Which means the 4th-line claim at `write_fail.c:70` (and its two copies) is wrong in its mechanism but right in its conclusion.

## Documentation

Eleven items inline. Two are dead cross-references this delta created (`:260`, `:695` still point at "the read-back at the end of the wipe branch of write_step", which `ad4272c4` moved). Four are contracts the delta made stale: `iso15693_poller.h:24` (Success no longer implies a UID was observed), `:209` (start_wipe still gives the absent-run as the only stop condition), `nfc_magic_app_i.h:129` ("a clean success" -- the truncated case shares that reason code), and CHANGELOG `:39` ("neither of which is a fault" -- a dead stretch and a clock cut both are).

## Simplification

Still yours to sequence, and I'm not asking you to pull the queued items forward. Five inline; two of them exist *because of* this round, so they're the ones I'd take alongside the fixes: the three copies of the "a block answered, so resolve the absences below it" rule (`:741`), and the Details gate now stated twice and already disagreeing (`write_fail.c:344`).

## Checked and sound

Recording these so you know what not to re-verify. The sweep arithmetic holds on all four exits: `block - absent_run` cannot underflow, `run_start` stays in range, `clone_failed_count` and the bitmap cannot disagree (I walked the mixed re-probe case), and the geometries all report correctly -- 28/64, 66/64, 200/64, advertised 0, advertised 256, lifted at block 5, and a dead stretch that recovers. `activation_errors` is zeroed on the Ready that runs the sweep, so `VerifyWipe` genuinely gets a fresh 15. The wipe result is reported exactly once: both exits from that state return `NfcCommandStop`, and the SDK's event type has only Error and Ready. And there is no Back-swallow trap -- every terminal event navigates, `write_step` never returns Continue, and `on_exit` resets the state. The remaining cost is duration, which is the clone finding.

One behaviour change worth confirming was deliberate: a clean ISO15693 wipe no longer auto-dismisses through `NfcMagicSceneSuccess` (1.5s to the main menu) -- it now needs a button press and lands on the ISO15693 submenu. Matches the over-capacity clone route, so I assume yes.
