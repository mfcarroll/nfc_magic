# gen1 bench run — the four pm3-marked candidates

**What this is for:** the CHANGELOG's "validated on hardware, on one ST LRi2K card". That is a claim
about evidence, and more cards is what moves it. **It is not for the consent screen** — that no
longer reports validation status at all, on the argument that a sample size is a fact about the
project rather than about the card in the user's hand.

## The question, and why it is not a foregone negative

gen1 sets the UID by writing blocks **56/57**, with unlock/commit at **62/63**. All four candidates
measure 28 or 40 blocks, so those addresses sit **above** their advertised and measured capacity.

That does not settle it. A read-based capacity figure is a **LOWER BOUND** — only a write proves a
block exists — and `lri2k-keychain` is exactly the case that proves the point: it advertises 56, and
it took writes at 56/57/62/63 anyway. So the probe is a real test of whether these carry writable
memory above their claim.

## State before the run — all four are ready

Baselines already exist from campaign `iso15_20260908_012746`, all `from_baseline_probe: true`, with
restore scripts in `raw/<card>_baseline_restore_script.txt`. **The `original` record is WRITE-ONCE**,
so no re-baseline is needed and none should be attempted — by now the tool's own probes would be
what got recorded.

| tag | adv | non-zero blocks | unreadable | note |
|---|---:|---|---|---|
| `slix-1k-50x28` | 28 | none | none | cleanest — run first |
| `SL2S5302` | 40 | none | none | SLIX-S, 40 blocks — **different silicon**, most informative |
| `slix-1k-coin18` | 28 | none | none | confirms the SLIX result reproduces |
| `slix-1k-50mm` | 28 | none | **[0, 1]** | incomplete baseline — run last |

All four are blank, so a failed restore costs nothing.

## THE HAZARD, and it is not reversible

**A card that turns out to be gen1 will be left ARMED, permanently.** The arm is part of the
sequence — commit (block 63) takes `0x6996` and *nothing clears it*. An armed card's UID can move on
any later write to 56/57, including this app's own wipe. That is what `lri2k-keychain` is now, and
why it is a reusable fixture for the armed-card case.

So: a gen1 result costs you a permanently armed card. That is inherent to testing gen1 at all, not a
tool defect.

**Do NOT pass `--allow-arming`.** Without it the tool prompts before the arming frame, per card,
which is the last point you can decline.

## Order

1. `slix-1k-50x28` — cleanest of the SLIX trio
2. `SL2S5302` — different silicon; if gen1 behaves differently anywhere, most likely here
3. `slix-1k-coin18` — does the SLIX result reproduce
4. `slix-1k-50mm` — weakest baseline, run last

The three `slix-1k-*` are all NXP SLIX, 28 blocks; testing all three adds less than testing one
plus `SL2S5302`. If time is short, do 1 and 2.

## Procedure, per card

**Step 0 — confirm which tag is on the antenna.** Labels live on paper, UIDs live on silicon, and
several of these are physically similar. This ASSERTS and exits non-zero if it is the wrong tag:

```
python3 tools/iso15693_magic_probe.py --identify --card slix-1k-50x28
```

**Step 1 — safe pass, reads only.** `--capacity-max 70` is the point: the default stops at
advertised+2, which cannot see memory above the claim, and a search that hits its own bound reports
that bound as the capacity.

```
python3 tools/iso15693_magic_probe.py --card slix-1k-50x28 --probes info,capacity --capacity-max 70
```

Read the capacity result as a lower bound. If it answers reads above 28, that is already the finding.

**Step 2 — the classifier. Writes.** gen2 is tried FIRST and stops on success, which is the safe
ordering: gen2 sends custom `0xE0` frames a non-magic tag simply refuses, while gen1 sends ordinary
WRITE BLOCKs that **any** writable tag accepts.

```
python3 tools/iso15693_magic_probe.py --card slix-1k-50x28 --probes magictype --destructive --capacity-max 70
```

Answer the arming prompt deliberately. The tool restores the original UID afterwards.

**Step 3 — verify and restore.** In the pm3 client:

```
hf 15 info
```

Compare the UID against the inventory. If anything is off, paste
`tools/campaigns/iso15_20260908_012746/raw/<card>_baseline_restore_script.txt` into the client. Note
the script restores **blocks, not the UID** — a moved UID needs `hf 15 csetuid` with the original,
which the inventory holds.

**Step 4 — re-identify**, to catch a UID that moved without the probe noticing:

```
python3 tools/iso15693_magic_probe.py --identify
```

If it no longer matches the expected entry, the UID moved — record that, it is a finding in itself.

## Everything records itself

Each run writes `tools/campaigns/iso15_<stamp>/` with `campaign.log`, `manifest.json` and
`raw/*.txt`, and folds the verdict into `tools/tag-inventory.json` plus the rendered
`.notes/tag-inventory.md`. Nothing needs transcribing by hand.

## What the outcomes mean for the CHANGELOG

- **Any additional gen1** → "validated on hardware, on one ST LRi2K card" becomes a claim across two
  silicon families, which is materially stronger.
- **All four refuse gen1** → also worth stating. It bounds gen1 to the LRi2K-style parts and confirms
  the seller's annotation, and it means the four-block destruction warning on the consent screen is
  the common case rather than the rare one.
- **A gen2 hit** → these were sold as proxmark-writable without a generation; a gen2 result is a
  finding about the listing, not about this PR.

Either way the sentence stops resting on a single sample, which is the point.

## What this run does NOT cover

The **app-level** gen1 opt-in path — reaching the consent screen from the app on a card that fails
gen2, accepting, and watching the four-block warning render. That is the standing backlog item
(`source_uses_gen1_blocks`) and needs a Flipper rather than a proxmark. Separate run.
