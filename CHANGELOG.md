# Changelog

## 2.3

Adds magic **ISO15693 / NfcV** support. Detect an ISO15693 tag, show its Info, and
**clone / wipe** a magic ISO15693 card the same way the app handles its other magic types.

### Added

- **Detection** — any ISO15693 tag that activates is routed to a dedicated menu (Write / Wipe /
  Write UID / Info), mirroring the other magic types.
- **Info** — UID, manufacturer, chip type, GET SYSTEM INFO (memory / DSFID / AFI / IC ref), and the
  full block data (scrollable, `*` marks a locked block). Chip decode tells NXP **SLI / SLIX / SLIX2**
  (and the -S / -L variants) apart.
- **Clone from a saved `.nfc`** — writes the UID and all data blocks. On a **gen2** card it also writes
  the source's identity (IC ref / block geometry / AFI / DSFID), so the copy advertises the same chip.
  **gen1 has no geometry register**, so a gen1 clone carries the source's UID and data on a card that
  still reports its own size and IC reference. gen2 is tried first; if the card turns out not to be
  gen2, gen1 is offered as an explicit opt-in.
- **Wipe** — zeros every data block the card physically holds, **including 56/57/62/63**. On a gen2
  card those are ordinary user data, so sparing them would leave real data behind. On a **gen1** card
  they are the UID / unlock / commit registers, so a wipe cannot promise to leave the UID intact — it
  **re-reads the UID afterwards, where it can, and reports a move when it sees one**. It cannot
  report the absence of one.
- **Write UID** — manual backdoor UID write. Tries gen2 first and, only if that leaves the UID
  unchanged, offers the same opt-in gen1 attempt the clone does.
- **Live "Writing X / N" progress** during a clone or wipe, as the USCUID-UL clone already had.

### Behaviour

- **The wipe is bounded by the card, not by what the card claims.** A magic card's advertised block
  count is programmable — cloning a 28-block source onto a 64-block card makes it advertise 28, while
  the blocks above stay readable and writable — so a wipe that trusted it would clear 28 of 64, report
  success, and leave the previous card's data reachable. The sweep runs past the advertised count until
  a run of blocks answers neither a write nor a read.
- **A wipe reports the range it covered** — "Cleared *N* blocks. Card claims *M*." Both figures, no
  verdict, because the count is programmable and a mismatch either way is usually benign: a card cloned
  from a smaller source advertises less than it holds, and a card that over-claims — from the factory,
  or cloned from a larger source — advertises more.
  A sweep cut short by its time limit is **partial** if anything was cleared, names where it stopped
  and offers a retry; one that cleared nothing is a failure and offers none. A cut sweep that also
  moved the UID reports the UID instead, which takes priority and is deliberately not retryable —
  the cut is still named in Details.
- **The clone attempts every source block and reports only real data loss.** A non-empty block that
  won't write is **Partial**, naming the blocks. An empty block past the card's real capacity loses
  nothing, so the clone is a **Success** carrying a note that the card advertises more blocks than it
  physically holds — every block that fits is written and acknowledged.
- **No data is written until the card takes the magic UID.** Data blocks and identity fields follow
  only once the UID reads back as the target, so a card that doesn't take it is left untouched — which
  is why cloning has no up-front prompt. A wipe does prompt, since destruction is a wipe's only product.
- **The gen1 fallback is opt-in**, offered only when the gen2 write leaves the UID unchanged, and only
  after a screen stating what gen1 writes and what it costs on a tag that turns out not to be gen1. It
  writes the UID registers first and the data blocks only if that UID took, so such a tag loses at most
  those four blocks — and **a failed attempt names them**, so they can be restored from a backup.
- **gen1 fidelity is surfaced.** Because the gen1 backdoor lives in blocks 56/57/62/63, a gen1 clone
  cannot reproduce a source that keeps data there. The opt-in screen says so before anything is
  written, and a clone whose source reached those blocks reports Partial and names them.
- **Writes are verified by read-back.** The UID is re-read after an RF field power-cycle; AFI / DSFID
  are re-read and compared, and a field the copy doesn't carry is reported as Partial with a note.
  Block contents are not compared — a data block counts as written when the card acknowledges it.
- **A wipe counts a block as unwiped unless it can show the block is clear**, by reading it back after
  a failed write. An *interior* dead stretch — one with a block above it that still answers — is
  reported rather than written off; a run of silent blocks is re-probed first, so a momentary dropout is
  not mistaken for the end of the card.
  **Limit:** the read taken at activation stops at the first block that does not answer, so nothing
  above that point can be proven either way. A stretch that was already dead when the card was presented
  therefore cannot be told apart from memory the card never had, and is dropped rather than reported.
- **A wipe re-reads the UID when it finishes.** It sends no UID command, but on a gen1 card blocks
  56/57 *are* the UID registers. A UID that differs from the one the card presented gets a **"UID
  changed"** screen printing the UID the card now answers to — without which the card would be
  unreachable. Observed on a gen1 card left armed by an earlier UID write: it went to **all zeros**,
  which is not a valid ISO15693 identity at all. Where the check cannot run the screen says **"UID not
  re-checked"** rather than implying the identity was confirmed. **Limit:** a wipe that clears
  *nothing* reports "Wipe failed" and does not attempt the check at all, yet on an armed gen1 card the
  UID may still have moved. How far the sweep reaches depends on both where the card stops answering
  *reads* and what it claims, and the closed form is stated in the poller beside that branch: a card
  that answers a read at every address walks past 56/57 whatever it claims, and a card that answers
  nothing still reaches them if its claim is high enough. Staying short of 56/57 takes a low claim
  *and* early silence together. Tracked in #255.
- **A card lifted mid-write says so** rather than being reported as a card too small: losing the card
  makes every remaining block fail, which looks identical to reaching its capacity, so presence is
  re-checked before any capacity verdict. A time limit bounds both passes, so the screen is not held
  for the length of the whole block range.
- **Partial and over-capacity results are a summary plus a Details screen**, matching the Gen2 /
  USCUID-UL partial screens: the summary carries the counts and the most significant caveat, **Details**
  lists the blocks involved and any further caveats. Each outcome has its own message, including
  **"Nothing to clone"** for a source with no data blocks, and a clone failure for the case where the
  UID was written but not one data block took — which would otherwise look right to a UID-only reader
  while holding none of the data.
- **A UID that moves somewhere unasked-for is reported as magic, not as a dud.** A UID changing to
  neither the original nor the one requested is the one result that *proves* the card is magic — an
  inert tag cannot change its UID — so the screen says so and prints the UID the card now answers to.
- **Back is ignored during an ISO15693 write**, from the moment a card is found until the write reports
  an outcome. It cannot abort a write in any case, and on a clone pressing it between the UID write and
  the data pass could leave the card carrying a new UID with none of the source's data. Other magic
  protocols are unchanged.
- **Write UID refuses to "verify" a UID the card already has** — a read-back against the card's own UID
  is passed by any tag at all, magic or not, so it would report Success having proved nothing.

- **Data-block writes are addressed to the card's UID**, so a second tag in the field is not written by
  them. Confirmed on five cards — **TI Tag-it HF-I Plus**, **NXP ICODE SLIX** and **SLIX-S**, **ST
  LRi2K**, and one gen2 card of unrecorded type: every one accepts an addressed **WRITE BLOCK**, and
  a UID one byte wrong is answered by nothing.
- **A card that needs the OPTION flag on writes now gets it**, and is no longer reported as having
  refused them. Some silicon refuses a write whose OPTION bit is clear and says so with its own error
  code; the app reads that answer and sets the bit for the rest of the operation. A card in that mode
  then owes its acknowledgement only after a signal the Flipper's radio cannot send, so the write
  lands and nothing comes back — and the block is now read back to settle it rather than counted as a
  refusal. **TI Tag-it HF-I Plus** needs both halves. Without them no data block could be written to
  that chip at all, and with only the first a wipe zeroed the whole card and then reported that
  nothing had been cleared.
- **The clone's identity writes are addressed too.** **WRITE AFI** and **WRITE DSFID** are *standard*
  commands, so an unaddressed one lands on a tag of any size, and a changed AFI can drop that tag out
  of a selective inventory.
- **A clone says what it left behind.** It writes the file's blocks and nothing else — destroying what
  you did not ask about is Wipe's job — so on a card bigger than the file, everything above it keeps the
  previous card's data. A **gen2** clone then reprograms the advertised count down to the file's, so the
  card claims to end where the file did and an ordinary dump shows a clean copy over data that is still
  readable. The clone now reads above the file's last block and reports three things it can find: data
  left up there, a card that answers past the count it reports, and a card still announcing a geometry
  the file did not. None is a failure and none makes the clone Partial; they appear as notes on the
  result screen, with all of them behind **Details**.
- **A gen1 clone that lost nothing now reports plain success.** gen1 writes the UID into blocks
  56/57/62/63, so a source containing those blocks loses them and the clone is still Partial — but a
  source below block 57 has none of them, nothing is missing, and every such clone was reported as
  Partial with a note saying four blocks "differ" from a file that has no blocks that high. Those runs
  are now a clean success, and where the note does apply it says what was actually skipped.

### Validation

- **gen1** was validated on hardware across three chips: ST LRi2K (56 blocks), NXP SLIX (28) and NXP
  SLIX-S (40). On every one the four-frame UID sequence sets the UID, it reads back, and the original
  restores byte-identically. The armed-card hazard above was reproduced on the LRi2K.
- **gen2** was validated end-to-end: byte-identical clones across 28 / 56 / 64 / 70-block geometries,
  plus wipe and the over-capacity reporting.
- **gen3 is not supported, and a wipe can destroy one.** A gen3 card ignores the gen2 backdoor, so a
  clone or Write UID lands on the **"Not gen2 magic card"** opt-in, and accepting that sends four
  ordinary writes into 56/57/62/63 — user data on a gen3 card, so what that costs is those four
  blocks. **A wipe does not check at all** — it sweeps any ISO15693 tag presented to it, and a gen3
  card keeps its UID in blocks 0x10/0x11 with a configuration signature in 0x14/0x15, well inside any
  claim, so the sweep zeroes both. Per 0x6r1an0y, who wrote proxmark's ISO15693 V3 magic support,
  zeroing those on an un-finalized card **bricks it permanently**: the cost is the card, not just its
  identity. Stated on their authority rather than ours: no gen3 card exists on either side of this PR,
  so nothing here has been observed. Tracked as #255.
- **The gen1 and gen2 backdoor sequences are still unaddressed**, and with two tags present the
  post-wipe UID re-read can still answer with the bystander's UID rather than the card's — which
  addressing cannot fix, since that read exists to find out whether the UID changed and so cannot be
  aimed at a UID already in doubt. Keep one tag in the field at a time; a badge holder or a wallet is
  enough to break this. Tracked as #251.

## 2.2

### Changed

- **Gen2 detection now tries the per-UID key cache for sector 0.** Confirming a CUID means
  authenticating to sector 0 before block 0 can be probed, and the probe only ever knew the
  default FF..FF key, so a clone with a personalised sector 0 and no recognised ATS came back as
  **Magic Not Confirmed**. The sector-0 key A and key B the NFC app recorded for that UID in
  `/ext/nfc/.cache/<UID>.keys` are now offered first, ahead of FF..FF, so such a card can be
  confirmed as Gen 2 / CUID and still gets its static-nonce classification. The cache is read once
  per card per scan, a card with no entry behaves exactly as before, and a cached key that doesn't
  fit costs one more RF session and nothing else: the probe itself is unchanged, still only the
  first phase of the write, so block 0 is never modified.

## 2.1

### Added

- **MIFARE Classic key cache phase** - the dictionary attack now tries the NFC app's per-UID key
  cache (`/ext/nfc/.cache/<UID>.keys`) before the user and system dictionaries, for both **Write**
  and **Wipe**. A magic clone carries the original card's UID, so keys the NFC app recovered when
  the original was saved are already on the SD card under the clone's own name. They are fed to
  the poller as a dictionary, so each one is still authenticated against the card in front of you;
  a stale entry costs a few failed auths and nothing more. When the cache alone finishes the card,
  the two dictionary phases are skipped instead of being run for nothing. A card with no cache
  entry runs exactly as before.

### Fixed

- **Wiping a Gen2 clone of a static-encrypted-nonce card (FM11RF08S)** dead-ended at
  **"No keys found"**. Those keys only ever reach the per-UID dictionary, so neither shared
  dictionary has them; the key cache phase now does.

## 2.0

Major release. Adds magic **Ultralight / NTAG (USCUID-UL)** support, and reworks the magic
**MIFARE Classic (Gen2)** wipe & clone with honest **Success / Partial / Fail** reporting.

### Added

- **Magic Ultralight / NTAG (USCUID-UL) support** — the app can now write, clone and wipe magic
  Ultralight-family tags, not just MIFARE Classic:
  - **Write / full clone** with the transport auto-selected from detection — **direct** (CUID/ATS,
    ISO14443-3 + `A2`) or **backdoor** (raw wakeup). ACK-only, continue-on-fail; the UID page is
    written last; live "Writing X/N" progress; **Partial Write** with a Details list of the pages
    that didn't take (and resume if the card is briefly removed mid-write).
  - **Wipe** to factory default.
  - **Auth with Password (PWD-AUTH)** to write protected/locked tags (single attempt, so it can't
    burn `AUTHLIM`).
  - **"Write anyway"** for tags that don't confirm as magic.
  - **Detection** of UL11 / UL21 (incl. Mikron "Ultra"), NTAG213/215/216, UL-C, UL-5, and
    backdoor-mode tags (both wakeup sequences), with a raw-config view for unrecognised presets;
    family-first scanner.
- **Gen2 / MFC wipe & clone now report Success / Partial / Fail** — a **Partial** screen plus a
  **Details** list of the exact blocks that couldn't be written/wiped (it was previously always
  "Success").
- **"No keys found"** screen when a wipe has no usable keys (instead of a doomed write-check).
- **Exit → menu** button on the Wipe / Write / Dump failure screens.

### Changed

- Gen2 / MFC wipe & clone outcomes are tracked **per block** (counts + the not-done block list).
- A wiped **block 0** preserves the card's real **SAK/ATQA** (a 1K mis-detected as 4K is no longer
  stamped 4K) and zeroes the UID.
- **Write-check warnings** reworked: one shared Gen2/Classic handler and consistent
  **Back / Next / Skip** buttons.
- Clearer rejection message for a cross-family wrong dump.

### Fixed

- **False "Success"** on a Gen2/MFC **wipe** with missing keys, and on a **clone** that could write
  nothing (unknown keys, or a read-only block 0).
- **Stale-key carry-over** between operations — the dictionary attack now re-reads the card fresh
  each time instead of replaying the previous tag's keys.
- **Per-sector → per-block** failure tracking — a read-only block 0 no longer reports the whole of
  sector 0 as failed.
- Dead **"Next"** button on a write-check warning whose first item wasn't the UID warning.
- Gen2 write-check **"Back"** did nothing (it targeted the wrong menu); the misleading **"Retry"**
  label → **"Back"** (the button always navigates back).
- **KeysDict** memory leak when backing out of a write-check.
- The wipe-failure screen could show **"No keys found"** for an unrelated failure (uninitialised
  scene state).

## 1.12

### Added

- **Gen2 CUID / ATS + static-nonce detection** — classify magic MIFARE Classic sub-types (direct
  CUID, ATS-fingerprinted, and CUID with a static nonce).
- **Gen1 4- and 7-byte UID** handling, including writing a 7-byte MIFARE Classic dump to a Gen1 tag.
- **Length-aware wipe & write guards** — validate block counts before writing.

### Changed

- Updated for new firmware API / SDK.

### Fixed

- Gen4 max block number for the NTAG protocol.

## 1.11

### Changed

- Description / metadata update.

### Fixed

- Gen4 poller fix.

## 1.10

- Upstream sync and maintenance.

## 1.9

### Changed

- Gen4 sync.

### Fixed

- Minor UI fix.

## 1.8

### Added

- **Gen4 (UMC / GTU) magic-card support.**

## 1.7

### Changed

- Reverted not-release-ready Gen2 changes (Gen4 still pending).

### Fixed

- UI and newline fixes.

## 1.6

### Changed

- Incremental updates; Gen2 preparation added, then reverted as not release-ready.

## 1.5

### Changed

- Reworked Back-button event handling; GUI cleanup.

### Fixed

- Incorrect total-block usage (#102); ufbt build compatibility.

## 1.4

- Maintenance / upstream sync.

## 1.3

### Fixed

- New-API compatibility fixes.

## 1.1

- Early release updates; new-API compatibility pass.

## 1.0

### Added

- Initial release in the plugin pack: write & wipe magic cards (**Gen1a**, **Gen2**) with
  block-0 / UID editing.
