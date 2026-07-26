# Full SLIX clone — feasibility & scoping

_2026-07-26. Scopes expanding the SLIX feature from a UID-only writer to writing data blocks and,
eventually, a full card clone. Read [analysis.md](analysis.md) and
[protocol-reference.md](protocol-reference.md) first._

## The key distinction: Flipper SDK (standard) vs. magic backdoor (this app)

Two separate code sources are in play, and keeping them straight is the whole point:

- **Flipper firmware SDK** — the Momentum checkout at `../Momentum-Firmware-slix/lib/nfc/...`. This
  is what our `.fap` links against. It ships a **complete, standard SLIX implementation**
  (`lib/nfc/protocols/slix/`): read/write data blocks, AFI/DSFID, GET SYSTEM INFO, the SLIX
  password / privacy / EAS commands, signature read. These are all **documented ISO15693 + NXP ICODE
  operations**, so Flipper — a general-purpose reader — includes them to talk to *genuine* SLIX tags.
- **proxmark3** — `../proxmark3/`, a *separate* RFID project. We do **not** link against it. We used
  it only as a **reference** for the reverse-engineered magic backdoor byte sequences, then wrote our
  own Flipper implementation (`magic/protocols/slix/slix_poller.c`).

Why the magic UID write isn't in the Flipper SDK: **a genuine SLIX UID is factory-locked
(laser-programmed, read-only)** — you physically cannot change it. Changing a UID only works on
grey-market **"magic" clone chips** via a *secret, undocumented vendor backdoor*. That backdoor isn't
in any standard, only exists on counterfeit hardware, and is exactly the niche the base firmware keeps
out of core and pushes into apps — which is what `nfc_magic` (this app) is for.

Same pattern you already know from the MIFARE side of this app:

| Tag | Genuine card | "Magic" clone | Read/std ops | Magic write |
|-----|--------------|---------------|--------------|-------------|
| MIFARE Classic | UID / block-0 read-only | Gen1a/Gen2 backdoor rewrites block 0 | Flipper SDK | this app |
| ISO15693 / SLIX | UID factory-locked | backdoor rewrites UID | Flipper SDK | this app (ported from proxmark) |

**Consequence for cloning:** the magic backdoor is needed for exactly one thing — the **UID**.
Everything else in a clone (data blocks, AFI, DSFID) uses **standard** ISO15693 writes that the
Flipper SDK already implements. So a full clone = *Flipper's standard SLIX read/write for the data* +
*our magic backdoor for the UID.*

## Part A — writing data blocks & full clone: mostly plumbing

- **Data blocks need no magic.** Standard ISO15693 `WRITE BLOCK` (0x21) writes any unlocked user
  block; the SDK exposes `iso15693_3_poller_write_block(s)` and `slix_poller_write_block(s)`.
- **We already read the data.** Our Info poller reads every block into `Iso15693_3Data` during
  activation today — we just don't display or write it back.
- **Reuse, don't reinvent.** The cleanest path is to move the data path onto the SDK's `slix`
  protocol / poller instead of the raw `iso15693_3` layer. Reference implementation already exists in
  the stock Flipper NFC app: `../Momentum-Firmware-slix/applications/main/nfc/scenes/nfc_scene_slix_*`
  (SLIX read, save-to-`.nfc`, privacy-unlock) — mirror it.

Non-crypto gotchas (real, but not cryptographic):
- Locked / protected blocks need the write password (or can't be written).
- Source and target must match on block count and block size.
- Our gen2 CFG write declares a fixed 64-block geometry (`slix_poller.c` `SLIX_MAGIC_V2_CFG_*`).

## Part B — the cryptographic boundary

Three tiers; only the third is genuinely cryptographic.

| Tier | Card / state | Barrier | Cloneable? |
|------|--------------|---------|-----------|
| 1 | Plain SLIX / open data | none | Yes — read + write blocks freely |
| 2 | SLIX-S / SLIX2 password-protected | **access control, not crypto** | Only with the password |
| 3 | SLIX2 originality signature; **ICODE DNA** (AES) | **real cryptography** | No |

**Tier 2 is not real crypto.** The SLIX password scheme (`init_password_15693_Slix` in
`../proxmark3/armsrc/iso15693.c`, cmds GET RANDOM NUMBER `0xB2` + SET PASSWORD `0xB3`) just XORs the
32-bit password with a repeated 2-byte random — replay obfuscation, not a cipher. The password is the
secret and is **write-only** (you can't read it off the card). So a read/privacy-locked *source* card
is an access-control barrier: you can't read it to clone it without the password (defaults like
`0F0F0F0F` sometimes work; brute force is 2³² and rate-limited by AUTHLIM). Given the data + write
password, writing the clone is trivial.

**Tier 3 is the genuine wall:**
- **NXP originality signature** (SLIX2 / ICODE DNA, `READ SIGNATURE 0xBD`): a 32-byte **ECDSA
  signature over the UID, signed with NXP's private key**. Readable but unforgeable; a plain magic
  card can't return a valid signature bound to a spoofed UID. If a reader validates it, a
  cloned/UID-spoofed card fails.
- **ICODE DNA** adds **AES-128 mutual authentication** (challenge-response, like DESFire/NTAG5).
  Without the diversified key you can't authenticate, so you can't clone a DNA-auth system. Same class
  as cloning a MIFARE Classic with unknown keys.

No amount of magic-card writing defeats tier 3 — the secret/signature is something you fundamentally
cannot reproduce.

## Phased plan

**Phase 1 — surface what we already read (low effort).**
Display block data in the Info scene and add **save-to-`.nfc`** (reuse the SDK's `slix` save). No
writing, no magic. Gets the read/dump half of a clone done and useful on its own.

**Phase 2 — write-back clone of plain/unlocked cards (moderate).**
Load a source dump (or read live) → on a magic target: UID via our backdoor + all data blocks via
`WRITE BLOCK` + restore AFI/DSFID. Add per-block **Success / Partial / Fail** reporting, mirroring the
existing Gen2/USCUID clone paths. Adopt the SDK `slix` protocol for the data path here.

**Phase 3 — passwords (harder, access-control not crypto).**
Privacy-unlock a locked source to read it (GET RANDOM NUMBER + SET PASSWORD, default + user-supplied
password), and write-with-password to a protected target. The stock NFC app's `nfc_scene_slix_unlock*`
is the reference.

**Out of scope (cryptographically infeasible):** reproducing a SLIX2/ICODE DNA **originality
signature** for a spoofed UID, or cloning an **ICODE DNA AES-authenticated** system. Detect and
clearly report these rather than pretending to clone them.

## Suggested architecture move
Migrate the data path from raw `iso15693_3` to the SDK `slix` protocol (device model + poller), keep
`slix_poller.c`'s magic backdoor as the UID-write step. This unlocks blocks / AFI / DSFID / passwords /
signature-read for free and aligns the app with the stock NFC app's SLIX handling.
