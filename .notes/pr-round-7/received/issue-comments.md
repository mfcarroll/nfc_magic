--- mishamyte 2026-09-08T07:27:16Z
@mfcarroll heads up - I pushed a merge commit to this branch to clear the conflicts it had picked up, so you will want to pull before your next push.

`d31f5162`, merging `dev` (`a6fc8dea`) into `049029c9`. It is a merge, not a rebase: none of your 72 commits were rewritten, and the push was a fast-forward.

The conflicts came from #258 and #261, which reworked the same NFC Magic files while this branch was open. All four were "both sides added something at the same place" rather than a real disagreement:

**`application.fam` / `CHANGELOG.md`** - `dev` shipped 2.1 and 2.2 in the meantime, and your section was also numbered 2.1. I renumbered yours **2.1 -> 2.3** and bumped `fap_version` to match, keeping dev's 2.2 and 2.1 sections verbatim underneath. No entry was dropped or reworded, and your section's body is byte-identical to what you pushed. Say the word if you would rather it were numbered differently.

**`magic/nfc_magic_scanner.c`** - your ISO15693 activation probe and dev's sector-0 key-cache helpers landed at the same offset. Both kept, dev's first so `load_mfc_probe_keys` still sits next to the `read_identity` its comment refers to.

One thing worth flagging, since it is an edit to your side rather than a straight pick: both conflict sides ended *mid-function*, sharing the single closing brace that sat below the conflict. Concatenating them left `load_mfc_probe_keys` unclosed and `detect_iso15693` nested inside it. I restored the brace by hand at `:192`. Brace balance is even and it compiles, but it is the one spot in this merge where the resolution is not purely mechanical, so it is worth your eye.

**`nfc_magic_app.c`** - kept your ISO15693 init and took dev's two-argument `nfc_magic_scanner_alloc(nfc, storage)`. `instance->storage` is opened at `:76`, well before the call at `:132`, so the new argument is valid there.

Builds clean against API 88.4 with no warnings, and `ufbt format` reports no changes.

Round 7 review is posted separately - nothing blocking, and all 22 items from the last round are addressed.

