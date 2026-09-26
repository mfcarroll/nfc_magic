# Card baselines — the artifact, not a summary of it

`tools/tag-inventory.json` records which blocks held data. It does not record WHAT they held, and
the per-block raw captures under `tools/campaigns/` are pm3 console output that can fail to parse.
When both of those leave a card unrestorable, the dump itself goes here.

One file per card per capture, named `<card>_<date>_<state>.json`, straight from
`hf 15 dump` with nothing edited. Provenance and what the capture does and does not cover belong in
that card's inventory note, not in a second copy here.

**pm3 stores the UID REVERSED** in these files. `slix2-gold-30mm`'s reads `36F1CD01000348E0`; the
card answers `E0 48 03 00 01 CD F1 36`. Do not lift that field into anything without reversing it.
