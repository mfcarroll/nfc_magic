Taking the gate seriously was the right call, and the bench output is the most useful thing on this PR. `36 of 64 blocks` surviving a wipe that reported unqualified success, with markers matching their own block numbers - that turns a code-reading argument into a measured privacy bug, and it's the kind of evidence that should decide whether a card-cloning tool ships.

One commit per issue was also the right call. It made this review possible.

**Corrections I owe you first, since both are mine.**

You're right about `hf 15 wipe`. I quoted the `0..0xFF` loop bound and missed the `break` three lines below it - on a 64-block card it examines 65, not 256. I'd opened that file specifically to check the claim and still read it carelessly.

And in the revert commit I wrote that the pre-pass was "the reverse of the only ordering the hardware is **documented** to accept". Nothing documents it. proxmark sends `0x3E`, `0x3F`, UID with no explanatory comment; `doc/magic_cards_notes.md`'s ISO15693-magic section is literally `**TODO**`; the words *unlock*, *arm* and *commit* appear nowhere in an ISO15693 context in that tree. It's one implementation's observed order. Your framing - "there is no safe order, because the only way to reach the latch is through the sequence that sets it" - is better than mine and doesn't depend on the overclaim.

You were also right that I was too quick to call the post-wipe read-back untestable. Separating its benefit from its regression risk is the correct move and I should have made it.

---

## Verified fixed

Traced end to end, poller counters through to the rendered string: the clone capacity **proof** (the read-probe genuinely proves what it claims now), `uid_unexpected`, `gen1_attempted` and the block naming, `uid_unverifiable`, and the `uid_changed` reporting path. The `NotMagic` fallback really is unreachable for a gen2 Write-UID now. Progress bound still holds - the 256 ceiling didn't widen it, 11 events against 16 slots. Builds clean here at Unleashed `dev` 88.2, zero warnings, `ufbt format` clean, no non-ASCII.

I also tried hard to break the sweep's accounting and couldn't: provisional bits are set once, each resolution site consumes a disjoint range, `run_start` after a partial re-probe always begins above the last resolved block, and bitmap popcount matches `failed_count` in every case I could construct. `failed_count < blocks_total` holds, so the Partial screen can't go negative.

## Blocking

Three, all in the new wipe. Details inline; the shape of them is that the *mechanism* is fixed and the *reporting* isn't.

1. **The wipe's UID re-read has no field power-cycle**, so it probably can't observe the change it exists to catch.
2. **The sweep can attempt fewer blocks than the loop it replaced**, and reports bare Success for the ones it skipped.
3. **A truncated sweep and a clean one render identically** - there's no wipe success screen with counts at all.

## Worth fixing

- "Card too small" is still purely positional, on the one screen that makes a factual claim about the user's hardware - and your own bench log shows it firing on `oversize_edgedata_70`.
- The re-probe classifies by presence where the main loop classifies by content, so a block reading back as zeros gets reported "not cleared".
- The clone's new read-probe passes an unclamped `block_size` from a loaded `.nfc` into a 32-byte stack buffer; the wipe clamps in the same situation.
- No wall-clock bound on the sweep. A card that answers reads everywhere runs all 256 blocks, and the loop can't be aborted.

## Documentation drift, which is now the systemic issue

The density went the **wrong** way this round: `iso15693_poller.c` is at **40.8%** comment lines and the header at **65.3%**, against 9.9% for `gen2_poller.c` and 10.0% for `uscuid_ul_poller.c`. These 13 commits added 93 code lines and 176 comment lines.

That isn't a style complaint. The Success/Partial/Fail contract is now stated in **13 places** and the gen1 register facts in about **35 sites across 10 files** - and three of the defects below are drift between copies. The header still claims a UID read-back that Success doesn't establish; still cites proxmark for behaviour proxmark lacks; and `failed_count`'s wipe definition omits a contributor that prints to the user as "Not cleared: %u". Every fact stated in eight places is a fact that can rot in seven, which is what happened to "UID unchanged" twice already.

Suggested owners: the event enum owns the outcome contract; the `ISO15693_MAGIC_BLK_*` defines own the wire facts; `gen1_optin.c`'s strings own the user-facing gen1 consequence; `ISO15693_POLLER_WIPE_MAX_BLOCKS` owns the sweep rationale and the hardware measurement. Everything else cross-references.

## Two smaller notes

Three of the 13 commits regressed something a later commit in the same batch fixed - the progress denominator, the conditional retry, and the dropout truncation. Worth noting the retry one (`a55ce29c`) exists **because** the retry loop is copy-pasted between the clone and wipe paths and the copies drifted. That duplication is still there, along with three more shared primitives. It's the one simplification I'd treat as non-optional, since it has already produced a shipped bug once.

## Your three questions

**1. Back mid-write.** Yes, swallow it while a write is in flight, and yes it's mine to call - go ahead. Your reset-gap analysis is right, and the 256 ceiling makes it sharper: the un-abortable window is now potentially 10-18 seconds on a card that answers reads everywhere, where before it was bounded by the advertised count.

**2. Finding 5's clone half.** Close it. Same ambiguity, and the clone writes far more than a UID.

**3. Does the gate close?** Not this round - but for a reason that argues *for* the gate rather than against it. It caught a real privacy bug; the fix introduced a narrower member of the same class, and added a verification that may not verify. One more pass on the three blocking items and I think we're there. I don't need another full bench matrix for it - the clone results above are convincing and I'm not asking you to re-run them.

**And yes, please open the issue** for the unaddressed-frame hazard. It's real, it's pre-existing, and it deserves to outlive this PR.

One request on scope: I'd rather you didn't take the simplification items in the same pass as the blocking ones. They're worth doing, but mixing a hazard fix with a 300-line comment cut makes both harder to review, and this PR has already shown how easily a fix-in-passing goes wrong - most of that being my doing, not yours.
