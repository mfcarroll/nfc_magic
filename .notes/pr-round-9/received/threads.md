# Round 9 — 11 review threads, verbatim

Review 5185694197, 2026-09-12.

---

## 01. `magic/protocols/iso15693/iso15693_poller.c:1252` — comment 3995478934

<https://github.com/xMasterX/all-the-plugins/pull/250#discussion_r3995478934>

The fix traded one unconditional claim for another, and the new one is false for exactly the card this branch is about.

> (Not every dead card gets that far -- no usable geometry returns above the loop, and **a card claiming under 57 blocks ends the sweep near its own claim**.)

The first exception is right (`:853`). The second is true only of a card that **answers nothing**, and that qualifier is missing.

`absent_run` increments only at `:955`, on the path where the write *and* the read both failed. A card that refuses every write but still answers a read calls `wipe_note_present` at `:940`, which zeroes the run - so it never trips, and the sweep walks to the 256 ceiling or the clock **whatever the card claims**. At this file's own 40-70ms per refused block, index 56 arrives around 2-4s against a 10s budget.

That card has `wiped == 0`, so it lands here. And three other comments in the tree already say so:

- `:167-170` - "A card that refuses the write and still serves a read at every address never accumulates a run, so it walks all 256 blocks"
- `nfc_magic_scene_iso15693_write_fail.c:276-278` - "exactly the card the sweep's time limit exists for, and it is also the one that ends here"
- `iso15693_poller.h:174-177` - the bullet this same round rewrote, built on that card

The real condition is *stops answering reads*, not *claims fewer than 57*. The sweep misses 56/57 only when the card answers nothing **and** claims fewer than 57.

---

## 02. `CHANGELOG.md:108` — comment 3995478935

<https://github.com/xMasterX/all-the-plugins/pull/250#discussion_r3995478935>

Same overcorrection as `iso15693_poller.c:1252`, and here it contradicts its own sentence four lines up.

> on the card it can actually happen to - **one that answers reads at every address** - it is not benign: the sweep does reach blocks 56/57 [...] (A card that reports no usable geometry transmits nothing at all, and **one claiming fewer than 57 blocks never gets that far**.)

A card that answers reads at every address *and* claims fewer than 57 blocks satisfies both. The main clause says it reaches 56/57; the parenthetical says it never gets there.

Two more in the same passage:

- The main clause is now too **narrow** as well. A card that answers nothing but advertises >= 58 never trips the run below its own claim (`:970`, `claimed_range_attempted = (block + 1 >= advertised)`), so it also walks through 56/57 with three refused writes at each.
- "reports no usable geometry **transmits nothing at all**" - it already transmitted an inventory and a GET SYSTEM INFO during activation. The poller's own wording, "no usable geometry returns above the loop", is the precise one and does not need weakening for a changelog.

---

## 03. `magic/protocols/iso15693/iso15693_poller.c:1379` — comment 3995478938

<https://github.com/xMasterX/all-the-plugins/pull/250#discussion_r3995478938>

`00892750` fixed this claim at `:863-866` and left it standing here, 510 lines away - where it now contradicts its own replacement.

> the gen1 arm sequence leaves commit = 0x6996 and **nothing ever clears it**

`:865-866`, this round:

> **This sweep does reach commit and zero it**; ORDER is what decides the outcome, since 56/57 go first, while commit still holds 0x6996.

`ISO15693_MAGIC_BLK_COMMIT` is 0x3F = 63, and the sweep at `:889` writes zeros at every index up to `ISO15693_POLLER_WIPE_MAX_BLOCKS`, so it reaches 63 like any other block.

Worst possible location for the stale version: this comment is inside `Iso15693WriteStateVerifyWipe`, i.e. it runs *after* the sweep that just zeroed commit - and the next line points the reader at the OPEN QUESTION that now says so.

---

## 04. `CHANGELOG.md:161` — comment 3995478939

<https://github.com/xMasterX/all-the-plugins/pull/250#discussion_r3995478939>

`cbd9093e` widened the #251 scope in the source and the release notes kept the old one.

> **A wipe** reaches every ISO15693 tag in the field [...] The **WRITE BLOCK** frames go out unaddressed

`iso15693_poller.c:23-31` now says the opposite of both halves:

> UNADDRESSED IS THE WHOLE OF #251, AND IT COVERS EVERY WRITE THIS APP SENDS, not just data blocks

and enumerates four categories. Two of them are missing here entirely:

- **WRITE AFI / WRITE DSFID**, from the *clone's* identity pass - and your own note says these are STANDARD commands, so they reach a bystander of any size, and a changed AFI "can drop a tag out of selective inventory". That is arguably the nastiest item on the list and it is not in the notes.
- **the gen1 backdoor**, plain 0x21 into a bystander's 56/57/62/63 - four blocks of user data on an ordinary tag.

So a reader of the changelog thinks this is a wipe-only, data-block-only hazard. It is neither. The bullet also needs to stop saying "A wipe" in the lead, since a clone triggers it too.

---

## 05. `scenes/nfc_magic_scene_write_confirm.c:81` — comment 3995478942

<https://github.com/xMasterX/all-the-plugins/pull/250#discussion_r3995478942>

**My wording, and it is too strong - I gave you this in round 8 and it needs narrowing.**

> that card is neither gen1 nor gen2, and **in this app those words name the MIFARE Classic protocols**.

Half true: `NfcMagicProtocolGen1` / `Gen2` do exist (`magic/protocols/nfc_magic_protocols.c:6-7`). But those words are overloaded app-wide, and the ISO15693 sense is the one in force on this very screen - `:82` installs "Wipe? (gen1/gen2 only)" meaning the ISO15693 generations, `:85` says "gen1 magic 56/57/62/63", `nfc_magic_scene_iso15693_gen1_optin.c:28` renders "Not gen2 magic card", and `nfc_magic_scene_iso15693_write_fail.c:99` returns "gen1 failed".

If the premise held, the title this comment is defending would itself be wrong on the screen it appears on.

The reason the title must not leak is **scope**, not vocabulary: a USCUID-UL wipe is an Ultralight operation, so an ISO15693 generation qualifier says nothing about the card in hand - under either reading of the words. The fix itself is correct; only the rationale needs rewording.

---

## 06. `magic/protocols/iso15693/iso15693_poller.c:1505` — comment 3995478945

<https://github.com/xMasterX/all-the-plugins/pull/250#discussion_r3995478945>

The count this round deleted was the anchor for the number that stayed.

> while being **sixteen fields short** of the real one

`alloc` now sets three fields (`running`, `callback`, `context`, `:1512-1514`). `start_internal` touches 27. A subset of three is 24 short, not sixteen.

"Sixteen" was computed when `alloc` zeroed eleven, and `bfab2b76` correctly removed the "28" that used to let a reader check it - so the figure now has nothing to reconcile against in either direction.

Given the new text already names the four fields `start_internal` skips, which is the genuinely useful part, the cleanest fix is to drop the number rather than re-derive it.

---

## 07. `magic/protocols/iso15693/iso15693_poller.c:23` — comment 3995478948

<https://github.com/xMasterX/all-the-plugins/pull/250#discussion_r3995478948>

Two scope words in an otherwise excellent rewrite. The blast-radius list is right and finding WRITE AFI / WRITE DSFID was a real addition - I had missed those.

> AND IT COVERS EVERY WRITE **THIS APP** SENDS

This app also writes MIFARE Classic Gen1A / Gen2 / Gen4 and USCUID-UL over 14443-A, all after anticollision and SELECT, i.e. addressed. The scoped truth is *every ISO15693 write this app sends* - and your next clause already says it correctly with "this file".

> no frame from **this file** carries the ADDRESSED flag or a UID

I verified this for every hand-built frame: `:321` gen1, `:339` gen2 0xE0, `:437` DSFID, `:444` AFI all open with `ISO15693_MAGIC_FLAGS` and append no UID. But "no frame" now also sweeps in the SDK-built `read_block` / `inventory` / `get_system_info` frames, and the bullet list only establishes flag bytes for `write_block`. ufbt ships headers only, so nothing in reach verifies those. Either narrow it to writes, or fold it into the existing "read in firmware source, not shipped" disclosure at `:131-135`.

---

## 08. `magic/protocols/iso15693/iso15693_poller.c:505` — comment 3995478949

<https://github.com/xMasterX/all-the-plugins/pull/250#discussion_r3995478949>

> All of them are unaddressed; **ISO15693_MAGIC_FLAGS carries the full #251 scope**.

Reads as *the define is what makes them unaddressed*, which the note it points at explicitly denies. `:26-27`:

> The SDK's write_block builds its own frame (SUBCARRIER_1 | DATA_RATE_HI), so it is **unaddressed on its own account rather than via this define**.

"see the #251 note at `ISO15693_MAGIC_FLAGS`" says what you mean without the attribution.

Second, smaller: "the backdoor and identity writes do not [funnel through here]" can be read as *writes to 56/57/62/63 do not funnel through here*, which is false for the wipe - its sweep zeroes those four indices through this function like any other block. It is the gen1 *backdoor sequence* that bypasses it, not those block numbers.

---

## 09. `scenes/nfc_magic_scene_iso15693_write_fail.c:275` — comment 3995478951

<https://github.com/xMasterX/all-the-plugins/pull/250#discussion_r3995478951>

A true fact went out with the false one.

`62b5f975` correctly dropped "The UID was never touched" - the poller had retracted it. But the same edit removed the rest of the sentence:

> since the generic "not a magic tag / UID write" message would be wrong for a wipe

That half was accurate, and it was the only statement anywhere of *why this branch exists* rather than letting a wipe fall through to the generic write-failed screen. The branch now opens straight into the sweep's time-limit argument, which explains which card lands here but not why it gets its own screen.

Worth restoring as its own clause, without the UID claim attached.

---

## 10. `scenes/nfc_magic_scene_write_confirm.c:24` — comment 3995478954

<https://github.com/xMasterX/all-the-plugins/pull/250#discussion_r3995478954>

Leftover from the fix, and it is the kind that invites the regression back.

```c
const bool is_wipe = instance->uscuid_ul_is_wipe_mode || iso15693_wipe;
```

`is_wipe` is read once, at `:30`. For an ISO15693 wipe `:82` overwrites the title anyway, so the `|| iso15693_wipe` term has no observable effect - `is_wipe` is now exactly `uscuid_ul_is_wipe_mode` at its only use site.

Harmless today. But it leaves a variable whose name says "any wipe" feeding a string that must only describe the USCUID-UL one, which is the shape the regression had in the first place. Either drop the term or rename it to what it now means.

---

## 11. `magic/protocols/iso15693/iso15693_poller.h:15` — comment 3995478957

<https://github.com/xMasterX/all-the-plugins/pull/250#discussion_r3995478957>

> **Every block bound in this feature is this number**, so it is named once here

Not every one: `source_count` (`iso15693_poller.c:562`, from the source image) and `advertised` (`:838`, from the card) are block bounds and are card-derived. The clone clamp at `:581-582` exists precisely because `source_count` can exceed this constant.

The sentence is clearly about the *constant* ceilings - the next clause, "rather than spelled BITMAP_SIZE * 8 at each site", scopes it - so nothing downstream is wrong. But "every fixed block ceiling in this feature" is the claim that holds, and this file gets held to that standard everywhere else.

The constant itself is right, all four sites use it, and putting it in the header next to the bitmap it derives from matches how `GEN2_POLLER_MAX_BLOCKS` is already done.

