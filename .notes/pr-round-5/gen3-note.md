# Draft: declaring gen3 (unsent)

To go into the Round 5 reply, or as a short standalone comment. **NOT POSTED.**

Found 2026-08-11 while checking whether a candidate card was gen1. Read out of the local proxmark
checkout, `client/src/cmdhf15.c` and `armsrc/iso15693.c`, not from documentation.

## The finding

`hf 15 csetuid` has three modes, not two:

```
hf 15 csetuid -u E011223344556677       -> use gen1 command
hf 15 csetuid -u E011223344556677 --v2  -> use gen2 command
hf 15 csetuid -u E011223344556677 --v3  -> use gen3 (V3) magic tag
```

We support gen1 and gen2. **gen3 is a third mechanism and we do not handle it at all.** It is not a
variant of either:

| | where the UID lives | how |
|---|---|---|
| gen1 | blocks `0x38`/`0x39`, unlock `0x3E`, commit `0x3F` | ordinary WRITE BLOCK, latches on next power-up |
| gen2 | refs `0x40`/`0x41` in a separate register space | the `0xE0 0x09` magic command |
| **gen3** | blocks **`0x10`/`0x11`** | ordinary WRITE BLOCK, unaddressed, byte-reversed |

gen3 also carries a **signature** in blocks `0x14`/`0x15` — `A5 2B 44 2C` / `21 AE 93 00` — which is how
proxmark detects an un-finalized tag (`hf15_magic_v3_is_config_mode`). `hf 15 cfinalize` overwrites
`0x15` with `69 E2 5D 00`, locking the UID permanently. So gen3 is repeatable until finalized, and
detectable by reading two blocks while it is not.

## Why it matters to this PR

**1. A gen3 card reports "not a magic tag".** It takes neither the gen2 backdoor nor gen1, so the flow
ends at the NotGen2 screen and then, if the user accepts, at a failed gen1 attempt — which has already
spent four ordinary WRITE BLOCKs on blocks 56/57/62/63 for nothing. Wrong report on a card that plainly
is magic, which is the class of thing this review keeps catching.

**2. gen3's registers sit in ordinary data space, low down.** Blocks `0x10`, `0x11`, `0x14`, `0x15` are
16, 17, 20 and 21 — inside the data range of even a 32-block card. So a **wipe** zeroes them, and a
**clone** writes source data over them. On an un-finalized gen3 card that means writing an all-zero UID
and destroying the config signature.

This is the gen1 open-question hazard again, but strictly worse in one respect: gen1's registers are at
56–63, outside a small card entirely, and the hazard needs the card to be *armed already*. An
un-finalized gen3 card is always in config mode.

**3. What already covers part of it.** The wipe's post-write UID re-check runs behind a field
power-cycle, so a gen3 UID moved by a wipe would be *reported* — the same mitigation built for the armed
gen1 case, working here for free. Nothing detects the signature loss, and arguably nothing should: the
user asked to zero every block.

## What I would not do about it

Supporting gen3 is a feature, not a fix, and this PR is four rounds deep. Reading two blocks to detect
the signature is cheap, but acting on it is a whole flow. Not worth widening the scope now.

What is worth doing is **declaring it**, so "not a magic tag" on a gen3 card is a documented limit rather
than a surprise. One CHANGELOG line under Behaviour, e.g.:

> **gen1 and gen2 magic are supported; gen3 is not.** A third magic generation exists (proxmark's
> `hf 15 csetuid --v3`) which keeps its UID in blocks 0x10/0x11 with a signature in 0x14/0x15. Such a
> card reports "not a magic tag", and a wipe or clone will write over those blocks like any other data.

## Status

**PUSHED 2026-08-17** as part of the Round 5 response (`fe7305d` on dev). It went into `### Validation (at 2.1)` rather than `### Behaviour`, because it is a limit of the feature, not a
behaviour of it, and that section already states scope the same way for the untested gen1 path.

`fe7305d` is **shipped code** — the only shipped-code commit since the clock-cut fix. Everything else
since then is `tools/` and `.notes/`.

### What to do with it in Round 5

1. **Push it with the Round 5 response**, not before. It is documentation, nothing is unsafe in the
   meantime, and a second push into a review he has not started is a notification for no benefit.
2. **Mention it in the reply** in a sentence or two — the declaration itself carries the substance, so the
   reply only needs to say gen3 exists, that it is declared rather than handled, and why not handling it is
   the right call for a PR four rounds deep.
3. If he **asks for gen3 support**, that is a feature and should be its own PR. Detection is cheap (read
   `0x14`/`0x15` and compare the signature); acting on it is a whole flow — a third generation in the
   opt-in ladder, a new set of result screens, and no card to test against.
4. If he **asks where the wire facts are**: deliberately not in `iso15693_poller.c`. They belong beside the
   `ISO15693_MAGIC_BLK_*` defines, which is exactly the territory Pass C item 6's ownership model
   reorganises — so they go in when item 6 does, not before.

Nothing in the app changes either way.
